//
// Created by Rakesh on 16/03/2022.
//

#include <catch2/catch_test_macros.hpp>
#include "../src/router.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "Sensor Device routes" )
{
  GIVEN( "Router configured for Sensor device API endpoints" )
  {
    const auto method = "GET"s;
    struct UserData {} userData;
    spt::http::router::HttpRouter<const UserData *, bool> r;
    r.add( method, "/device/sensor/", []( const UserData*, auto&& args )
    {
      REQUIRE( args.size() == 0 );
      return true;
    } );
    r.add( method, "/device/sensor/id/{id}", []( const UserData*, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/identifier/{identifier}", []( const UserData*, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/customer/code/{code}", []( const UserData*, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/facility/id/{id}", []( const UserData*, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/count/references/{id}", []( const UserData*, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/history/summary/{id}", []( const UserData*, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/history/document/{id}", []( const UserData*, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/{created}/between/{start}/{end}", []( const UserData*, auto&& args )
    {
      REQUIRE( args.size() == 3 );
      return true;
    } );

    WHEN( "Testing /device/sensor/" )
    {
      auto url = "/device/sensor/"s;
      auto resp = r.route( method, url, &userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1")
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, &userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/identifier/Integration Test Identifier" )
    {
      auto url = "/device/sensor/identifier/Integration Test Identifier"s;
      auto resp = r.route( method, url, &userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/customer/code/int-test" )
    {
      auto url = "/device/sensor/customer/code/int-test"s;
      auto resp = r.route( method, url, &userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/history/summary/6230f3069e7c9be9ff4b78a1" )
    {
      auto url = "/device/sensor/history/summary/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, &userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/history/document/6230f3069e7c9be9ff4b78a1" )
    {
      auto url = "/device/sensor/history/document/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, &userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/count/references/6230f3069e7c9be9ff4b78a1" )
    {
      auto url = "/device/sensor/count/references/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, &userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/created/between/2022-03-14T20:11:50.620Z/2022-03-16T20:11:50.620Z" )
    {
      auto url = "/device/sensor/created/between/2022-03-14T20:11:50.620Z/2022-03-16T20:11:50.620Z"s;
      auto resp = r.route( method, url, &userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1")
    {
      auto url = "/device/sensor/modified/between/2022-03-14T20:11:50.621Z/2022-03-16T20:11:50.621Z"s;
      auto resp = r.route( method, url, &userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }
  }
}

