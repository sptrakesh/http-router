//
// Created by Rakesh on 02/04/2022.
//

#if __GNUC__ > 10 || defined _WIN32
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#include <unordered_map>
#include "../src/router.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "String parameters test suite" )
{
  struct Request {} request;
  using Params = std::unordered_map<std::string, std::string>;

  GIVEN( "Router instantiated with string parameters" )
  {
    const auto method = "GET"sv;
    spt::http::router::HttpRouter<const Request&, Params, Params> r;
    r.add( method, "/device/sensor/", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 0 );
      return args;
    } );
    r.add( method, "/device/sensor/id/{id}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );

    WHEN( "Testing /device/sensor/" )
    {
      auto url = "/device/sensor/"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( resp->empty() );
    }

    AND_WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1")
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( resp->contains( "id"s ) );
      REQUIRE( (*resp)["id"s] == "6230f3069e7c9be9ff4b78a1"s );
    }
  }
}

#ifdef HAS_BOOST
SCENARIO( "map parameters test suite" )
{
  struct Request {} request;
  using Params = std::map<std::string, std::string>;

  GIVEN( "Router instantiated with string parameters" )
  {
    const auto method = "GET"sv;
    spt::http::router::HttpRouter<const Request&, Params, Params> r;
    r.add( method, "/device/sensor/", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 0 );
      return args;
    } );
    r.add( method, "/device/sensor/id/{id}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );

    WHEN( "Testing /device/sensor/" )
    {
      auto url = "/device/sensor/"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( resp->empty() );
    }

    AND_WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1")
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( resp->contains( "id"s ) );
      REQUIRE( (*resp)["id"s] == "6230f3069e7c9be9ff4b78a1"s );
    }
  }
}
#endif