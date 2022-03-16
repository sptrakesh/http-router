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
    struct Path
    {
      Path( std::string&& p, std::size_t h ) : path{ std::move( p ) },
        parts{ util::split( path, 8, std::string_view{ "/" } ) },
        handler{ h } {}

      ~Path() = default;
      Path(Path&&) = default;
      Path& operator=(Path&&) = default;
      Path(const Path&) = delete;
      Path& operator=(const Path&) = delete;

      std::string path;
      std::vector<std::string_view> parts;
      std::size_t handler;
    };

  public:
    using Handler = std::function<Response( UserData, std::unordered_map<std::string_view, std::string_view>&& )>;

    HttpRouter& add( std::string_view method, std::string_view path, Handler&& handler )
    {
      addParameter( method, path );
      handlers.push_back( std::move( handler ) );
      return *this;
    }

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
      using namespace std::string_view_literals;

      auto full = std::string{};
      full.reserve( 1 + method.size() + path.size() );
      full.append( "/" ).append( method ).append( path );
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

      const auto parts = util::split( full, 8, "/"sv );
      auto handler = -1;
      for ( ; iter != std::cend( paths ); ++iter )
      {
        if ( parts.size() != iter->parts.size() ) continue;

        for ( std::size_t i = 0; i < parts.size(); ++i )
        {
          if ( parts[i] == iter->parts[i] )
          {
            if ( i == parts.size() - 1 )
            {
              handler = iter->handler;
            }
            else continue;
          }
          if ( iter->parts[i][0] != '{' ) break;

          auto key = std::string{ iter->parts[i].substr( 1, iter->parts[i].size() - 2 ) };
          if ( key == full ) {}
          params[iter->parts[i].substr( 1, iter->parts[i].size() - 2 )] = parts[i];
          handler = iter->handler;
        }
      }

      if ( handler == -1 ) return std::nullopt;
      return handlers[handler]( userData, std::move( params ) );
    }

    std::vector<Handler> handlers{};
    std::vector<Path> paths;
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