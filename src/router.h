//
// Created by Rakesh on 15/03/2022.
//

#pragma once

#include "error.h"
#include "split.h"
#include <optional>
#include <unordered_map>

namespace spt::http::router
{
  /**
   * Simple path based HTTP request router.  Configured paths are stored in
   * a sorted vector, and request path matching is performed via binary search.
   * @tparam UserData User defined structure with the context necessary for
   *   the handler function.
   * @tparam Response The response from the handler function.
   */
  template<typename UserData, typename Response>
  class HttpRouter
  {
    struct Path
    {
      Path( std::string&& p, std::size_t h ) : path{ std::move( p ) },
        parts{ util::split<std::string>( path ) },
        handler{ h } {}

      ~Path() = default;
      Path(Path&&) noexcept = default;
      Path& operator=(Path&&) noexcept = default;
      Path(const Path&) = delete;
      Path& operator=(const Path&) = delete;

      std::string path;
      std::vector<std::string> parts;
      std::size_t handler;
    };

  public:
    /**
     * Request handler callback function.  Path parameters extracted are passed as an unordered_map.
     */
    using Handler = std::function<Response( UserData, std::unordered_map<std::string_view, std::string_view>&& )>;

    /**
     * Add the specified path for the specified HTTP method/verb to the router.
     * This is not thread safe.
     *
     * @param method The HTTP method/verb for which the route is configured.
     * @param path The path to configure.  Either a static (no parameters in curly braces) or parametrised value.
     * @param handler The callback function to invoke if a request path matches.
     * @return A reference to the router for chaining.
     * @throws DuplicateRouteError If the specified `path` has been configured
     *   for the specified `method` already.
     */
    HttpRouter& add( std::string_view method, std::string_view path, Handler&& handler )
    {
      addParameter( method, path );
      handlers.push_back( std::move( handler ) );
      return *this;
    }

    /**
     * Attempt to route the request for specified path and method.
     * @param method The HTTP method/verb from the client.
     * @param path The request URI path.
     * @param userData The custom data used by the handler callback function.
     * @param checkWithoutTrailingSlash If `true` and if the `path` ends with
     *   a trailing slash ('/'), attempt to find a match after trimming the
     *   trailing slash in case the original path does not match.
     * @return Returns std::nullopt if no configured route matches.
     */
    std::optional<Response> route( std::string_view method, std::string_view path,
        UserData userData, bool checkWithoutTrailingSlash = false ) const
    {
      if ( method.empty() || path.empty() ) return std::nullopt;
      auto resp = routeParameters( method, path, userData );
      if ( !resp && checkWithoutTrailingSlash && path.ends_with( '/' ) )
      {
        return routeParameters( method, path.substr( 0, path.size() - 1 ), userData );
      }

      return resp;
    }

    HttpRouter()
    {
      handlers.reserve( 32 );
      paths.reserve( 32 );
    }

    ~HttpRouter() = default;

    HttpRouter(const HttpRouter&) = delete;
    HttpRouter& operator=(const HttpRouter&) = delete;

  private:
    void addParameter( std::string_view method, std::string_view path )
    {
      using namespace std::string_literals;
      using namespace std::string_view_literals;

      auto full = std::string{};
      full.reserve( 1 + method.size() + path.size() );
      full.append( "/" ).append( method ).append( path );

      auto iter = std::lower_bound( std::cbegin( paths ), std::cend( paths ), full,
          []( const Path& p, const std::string& pth )
          {
            return p.path < pth;
          } );
      if ( iter != std::cend( paths ) && full == iter->path )
      {
        throw DuplicateRouteError{ "Duplicate path "s + std::string{ path } };
      }

      paths.template emplace_back( std::move( full ), handlers.size() );

      std::sort( std::begin( paths ), std::end( paths ), []( const Path& p1, const Path& p2 )
      {
        return p1.path < p2.path;
      } );
    }

    std::optional<Response> routeParameters( std::string_view method,
        std::string_view path, UserData userData ) const
    {
      using namespace std::string_view_literals;
      std::unordered_map<std::string_view, std::string_view> params{};

      auto full = std::string{};
      full.reserve( 1 + method.size() + path.size() );
      full.append( "/" ).append( method ).append( path );

      auto iter = std::lower_bound( std::cbegin( paths ), std::cend( paths ), full,
          []( const Path& p, const std::string& pth )
          {
            return p.path < pth;
          } );

      if ( iter == std::cend( paths ) ) return std::nullopt;
      if ( full == iter->path ) return handlers[iter->handler]( userData, std::move( params ) );

      const auto parts = util::split<std::string_view>( full );
      auto handler = -1;
      for ( ; iter != std::cend( paths ); ++iter )
      {
        if ( parts.size() != iter->parts.size() ) continue;

        for ( std::size_t i = 0; i < parts.size(); ++i )
        {
          auto iview = std::string_view{ iter->parts[i] };
          if ( parts[i] == iview )
          {
            if ( i == parts.size() - 1 )
            {
              handler = iter->handler;
            }
            else continue;
          }
          if ( iview[0] != '{' ) break;

          params[iview.substr( 1, iview.size() - 2 )] = parts[i];
          handler = iter->handler;
        }

        if ( handler != -1 ) break;
      }

      if ( handler == -1 ) return std::nullopt;
      return handlers[handler]( userData, std::move( params ) );
    }

    std::vector<Handler> handlers{};
    std::vector<Path> paths;
  };
}