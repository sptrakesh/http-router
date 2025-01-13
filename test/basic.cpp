//
// Created by Rakesh on 15/03/2022.
//

#if __GNUC__ > 10 || defined _WIN32
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#include "../src/router.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "HttpRouter test suite" )
{
  struct Request{} request;

  GIVEN( "Router with a set of API endpoints" )
  {
    spt::http::router::HttpRouter<const Request&, bool> r;

    r.add( "GET"sv, "/service/candy/{kind}"sv, [](const Request&, spt::http::router::HttpRouter<const Request&, bool>::MapType params)
    {
      REQUIRE( params.size() == 1 );
      REQUIRE( params.contains( "kind"s ) );
      return true;
    } );

    r.add( "GET"sv, "/service/shutdown"sv, [](const Request&, auto params)
    {
      REQUIRE( params.empty() );
      return true;
    } );

    r.add( "GET"sv, "/"sv, [](const Request&, auto params)
    {
      REQUIRE( params.empty() );
      return true;
    } );

    r.add( "GET"sv, "/{filename}"sv, [](const Request&, auto params)
    {
      REQUIRE( params.size() == 1 );
      REQUIRE( params.contains( "filename" ) );
      return true;
    } );

    WHEN( "Checking /service/candy/lollipop" )
    {
      auto url = "/service/candy/lollipop"s;
      auto resp = r.route( "GET"s, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
      CHECK( r.canRoute( "GET", url ) );
    }

    AND_WHEN( "Checking /service/candy/gum" )
    {
      auto url = "/service/candy/gum"s;
      auto resp = r.route( "GET"s, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
      CHECK( r.canRoute( "GET", url ) );
    }

    AND_WHEN( "Checking /service/candy/seg_råtta" )
    {
      auto url = "/service/candy/seg_råtta"s;
      auto resp = r.route( "GET"s, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
      CHECK( r.canRoute( "GET", url ) );
    }

    AND_WHEN( "Checking /service/candy/lakrits" )
    {
      auto url = "/service/candy/lakrits"s;
      auto resp = r.route( "GET"s, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
      CHECK( r.canRoute( "GET", url ) );
    }

    AND_WHEN( "Checking /service/shutdown" )
    {
      auto url = "/service/shutdown"s;
      auto resp = r.route( "GET"s, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
      CHECK( r.canRoute( "GET", url ) );
    }

    AND_WHEN( "Checking /" )
    {
      auto url = "/"s;
      auto resp = r.route( "GET"s, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
      CHECK( r.canRoute( "GET", url ) );
    }

    AND_WHEN( "Checking /some_file.html" )
    {
      auto url = "/some_file.html"s;
      auto resp = r.route( "GET"s, url, request );
      REQUIRE( *resp );
      CHECK( r.canRoute( "GET", url ) );
    }

    AND_WHEN( "Checking /another_file.jpeg" )
    {
      auto url = "/another_file.jpeg"s;
      auto resp = r.route( "GET"s, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
      CHECK( r.canRoute( "GET", url ) );
    }
  }

  GIVEN( "Router configured with paths with trailing slash" )
  {
    spt::http::router::HttpRouter<const Request&, bool> r;
    auto method = "POST"sv;
    r.add( method, "/path/entity/"sv, []( const Request&, auto&& ) { return true; } );

    WHEN( "Checking root paths" )
    {
      auto resp = r.route( method, "/path/entity", request );
      REQUIRE( resp );
      REQUIRE( *resp );
      CHECK( r.canRoute( method, "/path/entity" ) );

      resp = r.route( method, "/path/entity/", request );
      REQUIRE( resp );
      REQUIRE( *resp );
      CHECK( r.canRoute( method, "/path/entity/" ) );

      resp = r.route( method, "/path/entity/id", request );
      REQUIRE_FALSE( resp );
      CHECK_FALSE( r.canRoute( method, "/path/entity/id" ) );

      resp = r.route( method, "/path/entity/id/abc", request );
      REQUIRE_FALSE( resp );
      CHECK_FALSE( r.canRoute( method, "/path/entity/id/abc" ) );

      resp = r.route( method, "/path/", request );
      REQUIRE_FALSE( resp );
      CHECK_FALSE( r.canRoute( method, "/path/" ) );
    }
  }

  GIVEN( "Router configured with paths without trailing slash" )
  {
    spt::http::router::HttpRouter<const Request&, bool> r;
    auto method = "POST"sv;
    r.add( method, "/path/entity"sv, []( const Request&, auto&& ) { return true; } );

    WHEN( "Checking root paths" )
    {
      auto resp = r.route( method, "/path/entity", request );
      REQUIRE( resp );
      REQUIRE( *resp );

      resp = r.route( method, "/path/entity/", request, true );
      REQUIRE( resp );
      REQUIRE( *resp );

      resp = r.route( method, "/path/entity/id", request );
      REQUIRE_FALSE( resp );

      resp = r.route( method, "/path/entity/id/abc", request );
      REQUIRE_FALSE( resp );

      resp = r.route( method, "/path/", request );
      REQUIRE_FALSE( resp );
    }
  }
}