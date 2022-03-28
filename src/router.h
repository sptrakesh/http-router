//
// Created by Rakesh on 15/03/2022.
//

#pragma once

#include "error.h"
#include "split.h"
#include <functional>
#include <mutex>
#include <optional>
#include <unordered_map>
#if defined __has_include
  #if __has_include(<log/NanoLog.h>)
    #include <log/NanoLog.h>
    #define HAS_LOGGER 1
  #endif
#endif

#ifdef HAS_BOOST
  #include <ostream>
#include <boost/json/array.hpp>
#include <boost/json/serialize.hpp>
#endif

namespace spt::http::router
{
  /**
   * Simple path based HTTP request router.  Configured paths are stored in
   * a sorted vector, and request path matching is performed via binary search.
   * @tparam Request User defined structure with the request context necessary for
   *   the handler function.
   * @tparam Response The response from the handler function.
   */
  template<typename Request, typename Response>
  class HttpRouter
  {
    struct Path
    {
      Path( std::string&& p, std::string&& m, std::size_t h, std::string&& r = {} ) :
        path{ std::move( p ) }, ref{ std::move( r ) }, parts{ util::split<std::string>( path ) }
      {
        using namespace std::string_literals;

        epath.reserve( path.size() );
        for ( auto&& part : parts )
        {
          if ( ( part.starts_with( '{' ) && !part.ends_with( '}' ) ) || part.starts_with( ':' ) )
          {
            throw InvalidParameterError{ "Path "s + path + " has invalid parameter "s + part };
          }

          if ( part.starts_with( '{' ) ) epath.append( "/{}" );
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

      std::optional<std::size_t> indexOf( const std::string& method ) const
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
    };

  public:
    /**
     * Request handler callback function.  Path parameters extracted are passed as an unordered_map.
     */
    using Handler = std::function<Response( Request, std::unordered_map<std::string_view, std::string_view>&& )>;

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

#ifdef HAS_BOOST
    /**
     * Output the configured routes as a JSON structure.
     * @return JSON representation with some additional metadata about the configured routes.
     */
    [[nodiscard]] boost::json::value json() const
    {
      auto arr = boost::json::array{};
      int s = 0;
      int d = 0;
      for ( auto&& p : paths )
      {
        auto path = boost::json::object{ { "path", p.path }, { "methods", p.methods } };
        arr.template emplace_back( path );
        if ( p.path.find( "{" ) != std::string::npos ) ++d;
        else ++s;
      }

      auto obj = boost::json::object{};
      obj["paths"] = arr;
      obj["total"] = paths.size();
      obj["static"] = s;
      obj["dynamic"] = d;
      return obj;
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
      std::string out;
      out.reserve( 1024 );
      out.append( "paths:\n" );
      for ( auto&& path : paths )
      {
        out.append( "  " ).append( path.path ).append( ":\n" ).
          append( "    $ref: " ).append( "\"" ).append( path.ref ).append( "\"\n" );
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
      using namespace std::string_literals;
      using namespace std::string_view_literals;

      auto full = std::string{ path };
      auto m = std::string{ method };
      auto iter = std::lower_bound( std::begin( paths ), std::end( paths ), full,
          []( const Path& p, const std::string& pth )
          {
            return p.path < pth;
          } );
      if ( iter != std::cend( paths ) && full == iter->path )
      {
        auto midx = iter->indexOf( m );
        if ( midx )
        {
          throw DuplicateRouteError{ "Duplicate path "s + std::string{ path } + " for method "s + m };
        }
        iter->methods.push_back( std::move( m ) );
        iter->handlers.push_back( handlers.size() );
        return;
      }

      auto ps = Path{ std::move( full ), std::move( m ), handlers.size(), std::string{ ref } };
      for ( auto&& p : paths )
      {
        if ( p.epath == ps.epath )
        {
          if ( auto it = std::find( std::cbegin( iter->methods ), std::cend( iter->methods ), ps.methods[0] );
              it != std::cend( iter->methods ) )
          {
            throw DuplicateRouteError{ "Duplicate path "s + ps.path + " clashes with "s + p.path };
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
      using namespace std::string_view_literals;
      std::unordered_map<std::string_view, std::string_view> params{};

      auto full = std::string{ path };
      auto m = std::string{ method };
      auto iter = std::lower_bound( std::cbegin( paths ), std::cend( paths ), full,
          []( const Path& p, const std::string& pth )
          {
            return p.path < pth;
          } );

      if ( iter == std::cend( paths ) ) return std::nullopt;
      if ( full == iter->path )
      {
        auto midx = iter->indexOf( m );
        if ( midx ) return handlers[iter->handlers[*midx]]( request, std::move( params ) );
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
        if ( parts.size() != iter->parts.size() ) continue;
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
          if ( iview[0] != '{' ) break;

          params[iview.substr( 1, iview.size() - 2 )] = parts[i];
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
}