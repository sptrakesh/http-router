//
// Created by Rakesh on 22/09/2022.
//

#if __GNUC__ > 10 || defined _WIN32
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#include "../src/router.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "Wildcard paths test suite" )
{
  GIVEN( "Router with a simple wildcard path" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request&, std::string_view> r;

    r.add( method, "/device/sensor/*", [&r]( const Request&, auto args )
    {
      REQUIRE( args.contains( r.WildcardKey ));
      return args[r.WildcardKey];
    } );

    WHEN( "Testing /device/sensor/id" )
    {
      auto url = "/device/sensor/id"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == "id"sv );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }

    AND_WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1" )
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == "id/6230f3069e7c9be9ff4b78a1"sv );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }
  }

  GIVEN( "Router with multiple wildcard paths" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request&, std::string> r;

    r.add( method, "/device/sensor/*", [&r]( const Request&, auto args )
    {
      REQUIRE( args.contains( r.WildcardKey ));
      auto resp = "/device/sensor/"s;
      resp.append( args[r.WildcardKey] );
      return resp;
    } );

    r.add( method, "/cable/installed/*", [&r]( const Request&, auto args )
    {
      REQUIRE( args.contains( r.WildcardKey ));
      auto resp = "/cable/installed/"s;
      resp.append( args[r.WildcardKey] );
      return resp;
    } );

    WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1" )
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == "/device/sensor/id/6230f3069e7c9be9ff4b78a1"s );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }

    WHEN( "Testing /cable/installed/id/62326132e7a2e020c6652e38" )
    {
      auto url = "/cable/installed/id/62326132e7a2e020c6652e38"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == "/cable/installed/id/62326132e7a2e020c6652e38"s );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }
  }

  GIVEN( "Router with multiple wildcard paths with same base path" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request&, std::string> r;

    r.add( method, "/device/sensor/*", [&r]( const Request&, auto args )
    {
      REQUIRE( args.contains( r.WildcardKey ));
      auto resp = "/base/"s;
      resp.append( args[r.WildcardKey] );
      return resp;
    } );
    r.add( method, "/device/sensor/id/*", [&r]( const Request&, auto args )
    {
      REQUIRE( args.contains( r.WildcardKey ));
      auto resp = "/sub/"s;
      resp.append( args[r.WildcardKey] );
      return resp;
    } );
    r.add( method, "/device/sensor/id/{id}/*",
        [&r]( const Request&, auto args )
        {
          REQUIRE( args.contains( r.WildcardKey ));
          REQUIRE( args.contains( "id"sv ));
          auto resp = "/subsub/"s;
          resp.append( args[r.WildcardKey] );
          return resp;
        } );

    WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1" )
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == "/sub/6230f3069e7c9be9ff4b78a1"s );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }

    AND_WHEN( "Testing /device/sensor/6230f3069e7c9be9ff4b78a1" )
    {
      auto url = "/device/sensor/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == "/base/6230f3069e7c9be9ff4b78a1"s );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }

    AND_WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1/detail" )
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1/detail"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == "/subsub/detail"s );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }
  }

  GIVEN( "Router with regular paths and mis-matching wildcard path" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request&, std::string> r;

    r.add( method, "/cut/sheet/"sv, []( const Request&, auto args )
    {
      REQUIRE( args.empty() );
      return ""s;
    } );
    r.add( method, "/cut/sheet/id/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return ""s;
    } );
    r.add( method, "/device/sensor/*", [&r]( const Request&, auto args )
    {
      REQUIRE( args.contains( r.WildcardKey ));
      auto resp = "/device/sensor/"s;
      resp.append( args[r.WildcardKey] );
      return resp;
    } );

    WHEN( "Testing /cut/sheet/id/6232a3639835db5d70462e3e" )
    {
      auto url = "/cut/sheet/id/6232a3639835db5d70462e3e"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == ""s );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }

    AND_WHEN( "Testing /cut/sheet/history/document/6232a3639835db5d70462e3e does not match /device/sensor/*")
    {
      auto url = "/cut/sheet/history/document/6232a3639835db5d70462e3e"s;
      auto resp = r.route( method, url, request );
      REQUIRE_FALSE( resp );
      auto [p, m] = r.canRoute( method, url );
      CHECK_FALSE( p );
      CHECK_FALSE( m );
    }
  }

  GIVEN( "Router with wildcard path that contains parameters" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request&, std::string> r;

    r.add( method, "/device/sensor/id/{id}/*", [&r]( const Request&, auto args )
    {
      REQUIRE( args.contains( r.WildcardKey ) );
      REQUIRE( args.contains( "id"sv ) );
      auto resp = ""s;
      resp.append( args[r.WildcardKey] );
      return resp;
    } );

    WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1/detail" )
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1/detail"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == "detail"s );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }

    AND_WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1/detail/json" )
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1/detail/json"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == "detail/json"s );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }
  }

  GIVEN( "Router with catch all wildcard parameter (*)" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request&, std::string> r;

    r.add( method, "*", [&r]( const Request&, auto args )
    {
      REQUIRE( args.contains( r.WildcardKey ) );
      auto resp = ""s;
      resp.append( args[r.WildcardKey] );
      return resp;
    } );

    WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1/detail" )
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1/detail"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == url.substr( 1 ) );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }

    AND_WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1/detail/json" )
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1/detail/json"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == url.substr( 1 ) );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }

    AND_WHEN( "Testing /cut/sheet/id/6232a3639835db5d70462e3e" )
    {
      auto url = "/cut/sheet/id/6232a3639835db5d70462e3e"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == url.substr( 1 ) );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }
  }

  GIVEN( "Router with catch all wildcard parameter (/*)" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request&, std::string> r;

    r.add( method, "/*", [&r]( const Request&, auto args )
    {
      REQUIRE( args.contains( r.WildcardKey ) );
      auto resp = ""s;
      resp.append( args[r.WildcardKey] );
      return resp;
    } );

    WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1/detail" )
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1/detail"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == url.substr( 1 ) );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }

    AND_WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1/detail/json" )
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1/detail/json"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == url.substr( 1 ) );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }

    AND_WHEN( "Testing /cut/sheet/id/6232a3639835db5d70462e3e" )
    {
      auto url = "/cut/sheet/id/6232a3639835db5d70462e3e"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp == url.substr( 1 ) );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }
  }

  GIVEN( "Empty Router" )
  {
    const auto method = "GET"s;
    struct Request{};
    spt::http::router::HttpRouter<const Request&, std::string> r;

    WHEN( "Registering path with multiple wildcard characters" )
    {
      REQUIRE_THROWS_AS(
          r.add( method, "/device/*/id/{id}/*", []( const Request&, auto )
          {
            return ""s;
          } ),
          spt::http::router::InvalidWildcardError );
    }

    AND_WHEN( "Registering path with wildcard character before end" )
    {
      REQUIRE_THROWS_AS(
          r.add( method, "/device/*/id/{id}", []( const Request&, auto )
          {
            return ""s;
          } ),
          spt::http::router::InvalidWildcardError );
    }

    AND_WHEN( "Registering path with wildcard character at end of component (word)" )
    {
      REQUIRE_THROWS_AS(
          r.add( method, "/device/sensor/id*", []( const Request&, auto )
          {
            return ""s;
          } ),
          spt::http::router::InvalidWildcardError );
    }
  }
}
