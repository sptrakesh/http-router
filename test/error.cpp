//
// Created by Rakesh on 20/03/2022.
//

#if __GNUC__ > 10 || defined _WIN32
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#include "../src/router.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "HttpRouter errors test suite" )
{
  struct Request {} request;

  GIVEN( "Router configured with path with multiple parameters" )
  {
    spt::http::router::HttpRouter<const Request&, bool> r;
    std::string start;
    std::string end;
    r.add( "GET"sv, "/device/sensor/created/between/{start}/{end}"sv, [&start, &end](const Request&, auto args)
    {
      REQUIRE( args.size() == 2 );
      REQUIRE( args.contains( "start"s ) );
      REQUIRE( args["start"s] == start );
      REQUIRE( args.contains( "end"s ) );
      REQUIRE( args["end"s] == end );
      return true;
    });

    WHEN( "Checking path parameters" )
    {
      start = "2022-02-14T22:25:05.147Z"s;
      end = "2022-03-14T22:25:05.147Z"s;
      auto url = "/device/sensor/created/between/2022-02-14T22:25:05.147Z/2022-03-14T22:25:05.147Z"s;
      auto resp = r.route( "GET"s, url, request );
      REQUIRE( resp );
      CHECK( r.canRoute( "GET", url ) );
      REQUIRE_FALSE( r.route( "DELETE"s, url, request ) );
      REQUIRE_FALSE( r.route( "OPTIONS"s, url, request ) );
      REQUIRE_FALSE( r.route( "POST"s, url, request ) );
      REQUIRE_FALSE( r.route( "PUT"s, url, request ) );
      REQUIRE_FALSE( r.route( "PATCH"s, url, request ) );

      start = "2022-01-04T22:25:05.147Z"s;
      end = "2022-02-14T22:25:05.147Z"s;
      url = "/device/sensor/created/between/2022-01-04T22:25:05.147Z/2022-02-14T22:25:05.147Z"s;
      REQUIRE( r.route( "GET"s, url, request ) );
      CHECK( r.canRoute( "GET", url ) );
      REQUIRE_FALSE( r.route( "DELETE"s, url, request ) );
      REQUIRE_FALSE( r.route( "OPTIONS"s, url, request ) );
      REQUIRE_FALSE( r.route( "POST"s, url, request ) );
      REQUIRE_FALSE( r.route( "PUT"s, url, request ) );
      REQUIRE_FALSE( r.route( "PATCH"s, url, request ) );
    }

    AND_WHEN( "Making request with non-configured method" )
    {
      auto url = "/device/sensor/created/between/2022-02-14T22:25:05.147Z/2022-03-14T22:25:05.147Z"s;
      auto resp = r.route( "PUT"s, url, request );
      REQUIRE_FALSE( resp );
      CHECK_FALSE( r.canRoute( "PUT", url ) );
    }

    AND_WHEN( "Checking non-matching paths" )
    {
      auto url = "/device/created/between/2022-02-14T22:25:05.147Z/2022-03-14T22:25:05.147Z"s;
      REQUIRE_FALSE( r.route( "GET"s, url, request ) );
      CHECK_FALSE( r.canRoute( "GET", url ) );

      url = "/device/sensor/between/2022-02-14T22:25:05.147Z/2022-03-14T22:25:05.147Z"s;
      REQUIRE_FALSE( r.route( "GET"s, url, request ) );
      CHECK_FALSE( r.canRoute( "GET", url ) );
    }

    AND_WHEN( "Registering duplicate route" )
    {
      REQUIRE_THROWS_AS(
          r.add( "GET"sv, "/device/sensor/created/between/{start}/{end}"sv, [](const Request&, auto&&) { return true; }),
          spt::http::router::DuplicateRouteError
      );
    }

    AND_WHEN( "Registering route with :param form" )
    {
      REQUIRE_THROWS_AS(
          r.add( "GET"sv, "/device/sensor/created/between/:start/{end}"sv, [](const Request&, auto&&) { return true; }),
          spt::http::router::InvalidParameterError
      );
    }

    AND_WHEN( "Registering route without ending }" )
    {
      REQUIRE_THROWS_AS(
          r.add( "GET"sv, "/device/sensor/created/between/{start/{end}"sv, [](const Request&, auto&&) { return true; }),
          spt::http::router::InvalidParameterError
      );
    }

    AND_WHEN( "Registering a route that only differs in parameter name" )
    {
      REQUIRE_THROWS_AS(
          r.add( "GET"sv, "/device/sensor/created/between/{end}/{start}"sv, [](const Request&, auto&&) { return true; }),
          spt::http::router::DuplicateRouteError
      );
    }
  }
}

