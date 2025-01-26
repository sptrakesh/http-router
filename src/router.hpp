//
// Created by Rakesh on 15/03/2022.
//

#pragma once

#include "concat.hpp"
#include "error.hpp"
#include "split.hpp"

#include <functional>
#include <mutex>
#include <optional>

#if defined __has_include
  #if __has_include(<log/NanoLog.hpp>)
    #include <log/NanoLog.hpp>
    #define HAS_LOGGER 1
  #endif
  #ifndef HAS_BOOST
    #if __has_include(<boost/container/flat_map.hpp>)
      #define HAS_BOOST 1
    #endif
#endif
#endif

#ifdef HAS_BOOST
  #include <ostream>
  #include <boost/algorithm/string/replace.hpp>
  #include <boost/container/flat_map.hpp>
  #include <boost/json/array.hpp>
  #include <boost/json/serialize.hpp>
#else
  #include <map>
#endif

namespace spt::http::router
{
  /**
   * Simple path based HTTP request router.  Configured paths are stored in
   * a sorted vector, and request path matching is performed via binary search.
   * @tparam Request User defined structure with the request context necessary for
   *   the handler function.
   * @tparam Response The response from the handler function.
   * @tparam Map The type of map to use to return the parsed path parameters.
   *   If boost has been found defaults to boost::container::flat_map, else std::map
   */
#ifdef HAS_BOOST
  template <typename Request, typename Response, typename Map = boost::container::flat_map<std::string_view, std::string_view>>
#else
  template <typename Request, typename Response, typename Map = std::map<std::string_view, std::string_view>>
#endif
  requires (std::same_as<std::string, typename Map::key_type> && std::same_as<std::string, typename Map::mapped_type>) ||
      (std::same_as<std::string_view, typename Map::key_type> && std::same_as<std::string_view, typename Map::mapped_type>)
  class HttpRouter
  {
    struct Path
    {
      Path( std::string&& p, std::string&& m, std::size_t h, std::string&& r = {} ) :
        path{ std::move( p ) }, ref{ std::move( r ) }, parts{ util::split<std::string>( path ) }
      {
        using std::operator""sv;

        epath.reserve( path.size() );
        for ( auto&& part : parts )
        {
          if ( ( part.starts_with( '{' ) && !part.ends_with( '}' ) ) || part.starts_with( ':' ) )
          {
            throw InvalidParameterError{ util::concat( "Path "sv, path, " has invalid parameter "sv, part ) };
          }

          if ( part.starts_with( '{' ) ) epath.append( "/{}" );
          else if ( part == "~" )
          {
            epath.append( "/" ).append( "*" );
            wildcard = true;
          }
          else epath.append( "/" ).append( part );
        }

        methods.push_back( std::move( m ) );
        handlers.push_back( h );
      }

      ~Path() = default;
      Path(Path&&) noexcept = default;
      Path& operator=(Path&&) noexcept = default;
      Path(const Path&) = delete;
      Path& operator=(const Path&) = delete;

      [[nodiscard]] std::optional<std::size_t> indexOf( const std::string& method ) const
      {
        auto it = std::find( std::cbegin( methods ), std::cend( methods ), method );
        if ( it == std::cend( methods ) ) return std::nullopt;
        return std::distance( std::cbegin( methods ), it );
      }

      std::string path;
      std::string epath;
      std::string ref;
      std::vector<std::string> parts;
      std::vector<std::string> methods;
      std::vector<std::size_t> handlers;
      bool wildcard{ false };
    };

  public:
    using MapType [[maybe_unused]] = Map;
    struct Builder;

    /**
     * The key in the path parameters map with the sub-path that matches a wildcard route.
     */
    static inline const auto WildcardKey = std::string{ "_wildcard_" };

    /**
     * Request handler callback function.  Path parameters extracted are passed
     * as either a std::map or boost::container::flat_map.
     */
    using Handler = std::function<Response( Request, Map&& )>;

    /**
     * Add the specified path for the specified HTTP method/verb to the router.
     * This is thread safe.
     *
     * @param method The HTTP method/verb for which the route is configured.
     * @param path The path to configure.  Either a static (no parameters in curly braces) or parametrised value.
     * @param handler The callback function to invoke if a request path matches.
     * @param ref Optional reference to associate with the path when outputting
     *   the YAML that could be used in developing the OpenAPI Specification for the API.
     * @return A reference to the router for chaining.
     * @throws DuplicateRouteError If the specified `path` has been configured
     *   for the specified `method` already.
     * @throws InvalidParameterError If the specified `path` has parameters and
     *   use the `:<parameter>` form, or if the trailing `}` in the
     *   `{parameter}` is missing.
     */
    HttpRouter& add( std::string_view method, std::string_view path,
        Handler&& handler, std::string_view ref = {} )
    {
      auto lock = std::scoped_lock<std::mutex>{ mutex };
      addParameter( method, path, ref );
      handlers.push_back( std::move( handler ) );
      return *this;
    }

    /**
     * Attempt to route the request for specified path and method.
     * @param method The HTTP method/verb from the client.
     * @param path The request URI path.
     * @param request The custom data used by the handler callback function.
     * @param checkWithoutTrailingSlash If `true` and if the `path` ends with
     *   a trailing slash ('/'), attempt to find a match after trimming the
     *   trailing slash in case the original path does not match.
     * @return Returns std::nullopt if no configured route matches.
     */
    std::optional<Response> route( std::string_view method, std::string_view path,
        Request request, bool checkWithoutTrailingSlash = false ) const
    {
      if ( method.empty() || path.empty() ) return std::nullopt;
      try
      {
        auto resp = routeParameters( method, path, request );
        if ( !resp && checkWithoutTrailingSlash && path.ends_with( '/' ) )
        {
          return routeParameters( method, path.substr( 0, path.size() - 1 ), request );
        }

        return resp;
      }
      catch ( const std::exception& e )
      {
        if ( errorHandler )
        {
#ifdef HAS_LOGGER
          LOG_WARN << "Error handling " << method << " request to " << path <<
            ". " << e.what();
#endif
          return (*errorHandler)( request, {} );
        }
        throw;
      }
    }

    /// Check if a handler has been registered for the specified resource using the specified method/verb.
    /// @param method The HTTP method/verb configured for the resourse
    /// @param path The path to check if a handler has been configured
    /// @return A tuple of two booleans.  The first value indicates the resource has a handler, while the
    ///   second indicates if the method has been configured for the resource.
    [[nodiscard]] std::tuple<bool, bool> canRoute( std::string_view method, std::string_view path ) const
    {
      using std::operator""sv;
      if ( method.empty() || path.empty() ) return { false, false };

      auto full = std::string{ path };
      auto m = std::string{ method };
      auto iter = std::lower_bound( std::cbegin( paths ), std::cend( paths ), full,
          []( const Path& p, const std::string& pth )
          {
            return p.path < pth;
          } );

      if ( iter == std::cend( paths ) ) return { false, false };

      if ( full == iter->path && !iter->wildcard )
      {
        if ( auto midx = iter->indexOf( m ); midx ) return { true, true };
        return { true, false };
      }

      const auto parts = util::split<std::string_view>( full );
      auto handler = -1;
      for ( ; iter != std::cend( paths ); ++iter )
      {
        if ( parts.size() != iter->parts.size() )
        {
          if ( !iter->wildcard ) continue;
          if ( parts.size() < iter->parts.size() ) continue;

          for ( std::size_t i = 0; i < iter->parts.size(); ++i )
          {
            auto iview = std::string_view{ iter->parts[i] };
            if ( parts[i] == iview ) continue;
            if ( iview != "~"sv ) break;

            if ( auto midx = iter->indexOf( m ); midx ) return { true, true };
            return { true, false };
          }
        }

        auto midx = iter->indexOf( m );

        for ( std::size_t i = 0; i < parts.size(); ++i )
        {
          auto iview = std::string_view{ iter->parts[i] };
          if ( parts[i] == iview )
          {
            if ( i == parts.size() - 1 )
            {
              if ( midx ) handler = static_cast<int>( iter->handlers[*midx] );
              else return { true, false };
            }
            else continue;
          }
          if ( iview == "~" && iter->wildcard )
          {
            if ( midx )
            {
              handler = static_cast<int>( iter->handlers[*midx] );
            }
            else return { true, false };
          }
          if ( iview[0] != '{' ) break;

          if ( i == parts.size() - 1 )
          {
            if ( midx ) handler = static_cast<int>( iter->handlers[*midx] );
            else return { true, false };
          }
        }

        if ( handler != -1 ) break;
      }

      using O = std::tuple<bool, bool>;
      return handler == -1 ? O{ false, false } : O{ true, true };
    }

#ifdef HAS_BOOST
    /**
     * Output the configured routes as a JSON structure.
     * @return JSON representation with some additional metadata about the configured routes.
     */
    [[nodiscard]] boost::json::value json() const
    {
      using std::operator""sv;

      auto arr = boost::json::array{};
      int s = 0;
      int d = 0;
      for ( auto&& p : paths )
      {
        auto m = boost::json::array{};
        for ( const auto& method : p.methods ) m.push_back( boost::json::value{ method } );

        if ( p.path.ends_with( '~' ) )
        {
          auto path = boost::algorithm::replace_last_copy( p.path, "~"sv, "*"sv );
          arr.push_back( boost::json::object{ { "path", path }, { "methods", m } } );
        }
        else
        {
          arr.push_back( boost::json::object{ { "path", p.path }, { "methods", m } } );
        }

        if ( p.path.find( "{" ) != std::string::npos ) ++d;
        else if ( p.path.ends_with( '~' ) ) ++d;
        else ++s;
      }

      return boost::json::object{
          { "paths", arr },
          { "total", paths.size() },
          { "static", s },
          { "dynamic", d }
      };
    }

    /**
     * Output a string representation of the configured routes.
     * @return String representation of the routes.
     */
    [[nodiscard]] std::string str() const
    {
      return boost::json::serialize( json() );
    }
#endif

    [[nodiscard]] std::string yaml() const
    {
      using std::operator""sv;

      std::string out;
      out.reserve( 1024 );
      out.append( "paths:\n" );
      for ( auto&& path : paths )
      {
        if ( path.path.ends_with( '~' ) )
        {
          auto p = boost::algorithm::replace_last_copy( path.path, "~"sv, "*"sv );
          out.append( "  " ).append( p ).append( ":\n" ).
              append( "    $ref: " ).append( "\"" ).append( path.ref ).append( "\"\n" );
        }
        else
        {
          out.append( "  " ).append( path.path ).append( ":\n" ).
              append( "    $ref: " ).append( "\"" ).append( path.ref ).append( "\"\n" );
        }
      }
      return out;
    }

    /**
     * Create a new instance of the router.
     * @param error404 Optional handler function to handle path not found condition.
     * @param error405  Optional handler function to handle path not configured for method condition.
     * @param error500 Optional handler function to handle exception caught while despatching the request to handler.
     */
    HttpRouter( std::optional<Handler>&& error404 = std::nullopt,
        std::optional<Handler>&& error405 = std::nullopt,
        std::optional<Handler>&& error500 = std::nullopt ) :
        notFound{ std::move( error404 ) }, methodNotAllowed{ std::move( error405 ) },
        errorHandler{ std::move( error500 ) }
    {
      handlers.reserve( 32 );
      paths.reserve( 32 );
    }

    ~HttpRouter() = default;

    HttpRouter(const HttpRouter&) = delete;
    HttpRouter& operator=(const HttpRouter&) = delete;

  private:
    void addParameter( std::string_view method, std::string_view path, std::string_view ref )
    {
      using std::operator""s;
      using std::operator""sv;

      auto full = std::string{ path };
      auto m = std::string{ method };
      auto iter = std::lower_bound( std::begin( paths ), std::end( paths ), full,
          []( const Path& p, const std::string& pth )
          {
            return p.path < pth;
          } );
      if ( iter != std::cend( paths ) && full == iter->path )
      {
        if ( iter->indexOf( m ) )
        {
          throw DuplicateRouteError{ util::concat( "Duplicate path "sv, path, " for method "sv, m ) };
        }
        iter->methods.push_back( std::move( m ) );
        iter->handlers.push_back( handlers.size() );
        return;
      }

      if ( const auto idx = full.find( '*' ); idx != std::string::npos )
      {
        if ( idx != full.size() - 1 ) throw InvalidWildcardError( "Wildcard character at invalid position"s );
        if ( idx > 0 && full[idx-1] != '/' ) throw InvalidWildcardError( "Wildcard character not preceded by /"s );
      }

      if ( full.ends_with( '*' ) ) full[full.size() - 1] = '~';
      auto ps = Path{ std::move( full ), std::move( m ), handlers.size(), std::string{ ref } };
      for ( auto&& p : paths )
      {
        if ( p.epath == ps.epath )
        {
          if ( auto it = std::find( std::cbegin( iter->methods ), std::cend( iter->methods ), ps.methods[0] );
              it != std::cend( iter->methods ) )
          {
            throw DuplicateRouteError{ util::concat( "Duplicate path "sv, ps.path, " clashes with "sv, p.path ) };
          }
        }
      }

      paths.push_back( std::move( ps ) );

      std::sort( std::begin( paths ), std::end( paths ), []( const Path& p1, const Path& p2 )
      {
        return p1.path < p2.path;
      } );
    }

    std::optional<Response> routeParameters( std::string_view method,
        std::string_view path, Request request ) const
    {
      using std::operator""sv;
      Map params{};

      auto full = std::string{ path };
      auto m = std::string{ method };
      auto iter = std::lower_bound( std::cbegin( paths ), std::cend( paths ), full,
          []( const Path& p, const std::string& pth )
          {
            return p.path < pth;
          } );

      if ( iter == std::cend( paths ) ) return std::nullopt;
      if ( full == iter->path && !iter->wildcard )
      {
        if ( auto midx = iter->indexOf( m ); midx )
        {
          return handlers[iter->handlers[*midx]]( request, std::move( params ) );
        }
#ifdef HAS_LOGGER
        LOG_INFO << "Method " << method << " not configured for path " << path;
#endif
        if ( methodNotAllowed ) return (*methodNotAllowed)( request, std::move( params ) );
        return std::nullopt;
      }

      const auto parts = util::split<std::string_view>( full );
      auto handler = -1;
      for ( ; iter != std::cend( paths ); ++iter )
      {
        if ( parts.size() != iter->parts.size() )
        {
          if ( !iter->wildcard ) continue;
          if ( parts.size() < iter->parts.size() ) continue;

          for ( std::size_t i = 0; i < iter->parts.size(); ++i )
          {
            auto iview = std::string_view{ iter->parts[i] };
            if ( parts[i] == iview ) continue;
            if ( iview[0] == '{' )
            {
              auto key = iview.substr( 1, iview.size() - 2 );
              params.try_emplace( { key.data(), key.size() }, parts[i] );
            }
            if ( iview != "~"sv ) break;

            if ( auto midx = iter->indexOf( m ); midx )
            {
              std::size_t idx = 0;
              for ( std::size_t j = 0; j < iter->parts.size() - 1; ++j )
              {
                idx += ( 1 + parts[j].size() );
              }
              params.try_emplace( WildcardKey, path.substr( ++idx ) );
              return handlers[iter->handlers[*midx]]( request, std::move( params ) );
            }
#ifdef HAS_LOGGER
            LOG_INFO << "Method " << method << " not configured for path " << path;
#endif
            if ( methodNotAllowed ) return (*methodNotAllowed)( request, std::move( params ) );
            return std::nullopt;
          }
        }

        auto midx = iter->indexOf( m );

        for ( std::size_t i = 0; i < parts.size(); ++i )
        {
          auto iview = std::string_view{ iter->parts[i] };
          if ( parts[i] == iview )
          {
            if ( i == parts.size() - 1 )
            {
              if ( midx ) handler = static_cast<int>( iter->handlers[*midx] );
              else
              {
#ifdef HAS_LOGGER
                LOG_INFO << "Method " << method << " not configured for path " << path;
#endif
                if ( methodNotAllowed ) return (*methodNotAllowed)( request, std::move( params ) );
                return std::nullopt;
              }
            }
            else continue;
          }
          if ( iview == "~" && iter->wildcard )
          {
            if ( midx )
            {
              handler = static_cast<int>( iter->handlers[*midx] );
              std::size_t idx = 0;
              for ( std::size_t j = 0; j < iter->parts.size() - 1; ++j )
              {
                idx += ( 1 + parts[j].size() );
              }
              params.try_emplace( WildcardKey, path.substr( ++idx ) );
            }
            else
            {
#ifdef HAS_LOGGER
              LOG_INFO << "Method " << method << " not configured for path " << path;
#endif
              if ( methodNotAllowed ) return (*methodNotAllowed)( request, std::move( params ) );
              return std::nullopt;
            }
          }
          if ( iview[0] != '{' ) break;

          auto key = iview.substr( 1, iview.size() - 2 );
          params.try_emplace( { key.data(), key.size() }, parts[i] );
          if ( i == parts.size() - 1 )
          {
            if ( midx ) handler = static_cast<int>( iter->handlers[*midx] );
            else
            {
#ifdef HAS_LOGGER
              LOG_INFO << "Method " << method << " not configured for path " << path;
#endif
              if ( methodNotAllowed ) return (*methodNotAllowed)( request, std::move( params ) );
              return std::nullopt;
            }
          }
        }

        if ( handler != -1 ) break;
        params.clear();
      }

      if ( handler == -1 )
      {
        if ( notFound ) return (*notFound)( request, std::move( params ) );
        return std::nullopt;
      }
      return handlers[handler]( request, std::move( params ) );
    }

    std::vector<Handler> handlers{};
    std::vector<Path> paths;
    std::optional<Handler> notFound{ std::nullopt };
    std::optional<Handler> methodNotAllowed{ std::nullopt };
    std::optional<Handler> errorHandler{ std::nullopt };
    std::mutex mutex;
  };

#ifdef HAS_BOOST
  template <typename Request, typename Response>
  std::ostream& operator<<( std::ostream& os, const HttpRouter<Request, Response>& router )
  {
    os << router.json();
    return os;
  }
#endif

  /**
   * A builder for the HttpRouter.  Use to configure error handlers when
   * instantiating the router.
   * @tparam Request User defined structure with the request context necessary for
   *   the router handler function.
   * @tparam Response The response from the handler function.
   */
  template <typename Request, typename Response, typename Map>
  requires (std::same_as<std::string, typename Map::key_type> && std::same_as<std::string, typename Map::mapped_type>) ||
      (std::same_as<std::string_view, typename Map::key_type> && std::same_as<std::string_view, typename Map::mapped_type>)
  struct HttpRouter<Request, Response, Map>::Builder
  {
    Builder() = default;
    ~Builder() = default;
    Builder(const Builder&) = delete;
    Builder& operator=(const Builder&) = delete;

    /**
     * Set the HTTP 404 error handler to use with the router.
     * @param h The handler function.
     * @return Reference to this builder for chaining.
     */
    Builder& withNotFound( typename HttpRouter<Request, Response>::Handler&& h )
    {
      notFound = std::move( h );
      return *this;
    }

    /**
     * Set the HTTP 405 error handler to use with the router.
     * @param h The handler function.
     * @return Reference to this builder for chaining.
     */
    Builder& withMethodNotAllowed( typename HttpRouter<Request, Response>::Handler&& h )
    {
      methodNotAllowed = std::move( h );
      return *this;
    }

    /**
     * Set the HTTP 500 error handler to use with the router.
     * @param h The handler function.
     * @return Reference to this builder for chaining.
     */
    Builder& withErrorHandler( typename HttpRouter<Request, Response>::Handler&& h )
    {
      errorHandler = std::move( h );
      return *this;
    }

    /**
     * Build the router with the error handlers provided.  The handlers are
     * moved, so no further use of the builder is possible.
     * @return The properly initialised router.
     */
    [[nodiscard]] HttpRouter<Request, Response> build()
    {
      return { std::move( notFound ), std::move( methodNotAllowed ), std::move( errorHandler ) };
    }

  private:
    std::optional<Handler> notFound{ std::nullopt };
    std::optional<Handler> methodNotAllowed{ std::nullopt };
    std::optional<Handler> errorHandler{ std::nullopt };
  };
}
