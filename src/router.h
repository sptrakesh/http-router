//
// Created by Rakesh on 15/03/2022.
//

#pragma once

#include "split.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <unordered_map>

namespace spt::http::router
{
  template<typename UserData, typename Response>
  class HttpRouter
  {
  private:
    struct RawPath
    {
      std::string path;
      std::size_t handler;
    };

    struct ParameterPath
    {
      std::vector<std::string_view> parts;
      std::string path;
      std::size_t handler;
    };

  public:
    using Handler = std::function<Response( UserData, std::unordered_map<std::string_view, std::string_view>&& )>;

    HttpRouter& add( std::string_view method, std::string_view path, Handler&& handler )
    {
      const auto m = std::string{ method };
      if ( path.find( ':' ) == std::string_view::npos ) addRawPath( m, path );
      else addParameter( m, path );

      handlers.push_back( std::move( handler ) );
      return *this;
    }

    std::optional<Response> route( std::string_view method, std::string_view path, UserData userData )
    {
      if ( method.empty() || path.empty() ) return std::nullopt;

      const std::string p{ path };
      const std::string m{ method };

      if ( paths.contains( m ) )
      {
        auto opt = routeRaw( m, p, userData );
        if ( opt ) return opt;

        if ( path.ends_with( '/' ) )
        {
          auto trimmed = std::string{ path.substr( 0, path.size() - 1 ) };
          opt = routeRaw( m, trimmed, userData );
          if ( opt ) return opt;
        }
      }

      if ( pathParts.contains( m ) )
      {
        return routeParameters( m, path, userData );
      }

      return std::nullopt;
    }

    HttpRouter()
    {
      handlers.reserve( 32 );
      paths.reserve( 32 );
      pathParts.reserve( 32 );
    }

    ~HttpRouter() = default;

    HttpRouter(const HttpRouter&) = delete;
    HttpRouter& operator=(const HttpRouter&) = delete;

  private:
    void addRawPath( const std::string& method, std::string_view path )
    {
      if ( !paths.contains( method ) )
      {
        paths.template emplace( method, std::vector<RawPath>{} );
      }
      auto& v = paths[method];
      v.push_back( { std::string{ path }, handlers.size() } );

      std::sort( std::begin( v ), std::end( v ), []( const RawPath& p1, const RawPath& p2 )
      {
        return p1.path < p2.path;
      } );
    }

    void addParameter( const std::string& method, std::string_view path )
    {
      using namespace std::string_view_literals;
      const auto parts = util::split( path, 8, "/"sv );
      if ( !pathParts.contains( method ) )
      {
        auto v = std::vector<ParameterPath>{};
        v.reserve( 32 );
        pathParts.template emplace( method, std::move( v ) );
      }
      auto& v = pathParts[method];
      v.push_back( { parts, std::string{ path }, handlers.size() } );

      std::sort( std::begin( v ), std::end( v ), []( const ParameterPath& p1, const ParameterPath& p2 )
      {
        return p1.path < p2.path;
      } );
    }

    std::optional<Response> routeRaw( const std::string& method, const std::string& path, UserData userData )
    {
      auto& v = paths[method];
      auto iter = std::lower_bound( std::begin( v ), std::end( v ), path,
          []( const RawPath& p, const std::string& pth )
          {
            return p.path < pth;
          } );

      if ( iter != std::end( v ) )
      {
        if ( path == iter->path )
        {
          return handlers[iter->handler]( userData, {} );
        }

        if ( iter->path.ends_with( '/' ) )
        {
          auto view = std::string_view{ iter->path };
          view = view.substr( 0, view.size() - 1 );
          auto p = std::string_view{ path };
          if ( p == view )
          {
            return handlers[iter->handler]( userData, {} );
          }
        }
      }

      return std::nullopt;
    }

    std::optional<Response> routeParameters( const std::string& method, std::string_view path, UserData userData )
    {
      using namespace std::string_view_literals;
      std::unordered_map<std::string_view, std::string_view> params{};

      const auto parts = util::split( path, 8, "/"sv );
      auto handler = -1;
      for ( auto&& pp : pathParts[method] )
      {
        if ( parts.size() != pp.parts.size() ) continue;

        auto pview = std::string_view{ pp.path };
        auto iter = pview.find( ':' );
        if ( auto view = pview.substr( 0, iter ); !path.starts_with( view ) ) continue;

        for ( std::size_t i = 0; i < parts.size(); ++i )
        {
          if ( parts[i] == pp.parts[i] ) continue;
          if ( pp.parts[i][0] != ':' ) break;

          params[pp.parts[i].substr( 1 )] = parts[i];
          handler = pp.handler;
        }
      }

      if ( params.empty() ) return std::nullopt;
      return handlers[handler]( userData, std::move( params ) );
    }

    std::vector<Handler> handlers{};
    std::unordered_map<std::string, std::vector<RawPath>> paths;
    std::unordered_map<std::string, std::vector<ParameterPath>> pathParts;
  };

  /*
  template<typename UserData, typename Response>
  class HttpRouter
  {
  private:
    struct Node
    {
      using Ptr = std::unique_ptr<Node>;
      Node( std::string n, short h ) : name{ std::move( n ) }, handler{ h } {}
      std::string name;
      std::unordered_map<std::string, Node::Ptr> children{};
      short handler;
    };

  public:
    using Handler = std::function<Response( UserData, std::unordered_map<std::string, std::string>&& )>;

    HttpRouter()
    {
      handlers.reserve( 16 );
    }

    HttpRouter& add( std::string_view method, std::string_view path, Handler&& handler )
    {
      using namespace std::string_view_literals;

      handlers.push_back( std::move( handler ) );
      const auto parts = util::split( path, 8, "/"sv );

      if ( !tree ) tree = std::make_unique<Node>( std::string{ method }, -1 );

      add( method, parts, handlers.size() );
      handlers.push_back( std::move( handler ) );
      return *this;
    }

    std::optional<Response> route( std::string_view method, std::string_view path, UserData userData )
    {
      using namespace std::string_view_literals;

      if ( method.empty() || path.empty() ) return std::nullopt;
      if ( !tree ) return std::nullopt;

      const std::string m{ method };
      if ( !tree->children.contains( m ) ) return std::nullopt;

      std::unordered_map<std::string, std::string> params{};

      auto parent = tree->children[m].get();
      const auto parts = util::split( path, 8, "/"sv );
      short handler{ -1 };
      for ( auto&& part : parts )
      {
        if ( part.empty() ) return std::nullopt;
        if ( !parent ) return std::nullopt;

        const std::string p{ part };
        if ( !parent->children.contains( p ) )
        {
          bool found{ false };
          for ( auto&& pair : parent->children )
          {
            if ( pair.first[0] != ':' ) continue;
            const auto name = std::string{ std::string_view{ pair.first }.substr( 1 ) };
            params[name] = p;
            handler = parent->handler;
            found = true;
            parent = pair.second.get();
            break;
          }
          if ( !found ) return std::nullopt;
        }
        else
        {
          handler = parent->handler;
          parent = parent->children[p].get();
        }
      }

      if ( handler < 0 ) return std::nullopt;
      return handlers[handler]( userData, std::move( params ) );
    }

  private:
    void add( std::string_view method, const std::vector<std::string_view>& parts, short handler )
    {
      Node* parent = tree.get();
      const std::string m{ method };
      if ( !parent->children.contains( m ) ) parent->children[m] = std::make_unique<Node>( m, handler );
      parent = parent->children[m].get();

      for ( auto&& part: parts )
      {
        std::string name{ part };
        if ( !parent->children.contains( name ) )
        {
          parent->children[name] = std::make_unique<Node>( name, handler );
        }
        parent = parent->children[name].get();
      }

      parent->handler = handler;
    }

    std::vector<Handler> handlers{};
    typename Node::Ptr tree{ nullptr };
  };
   */
}