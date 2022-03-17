//
// Created by Rakesh on 16/03/2022.
//
#include <catch2/catch_test_macros.hpp>
#include "../src/router.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "Cut sheet routes" )
{
  GIVEN( "Router configured for cut sheet API endpoints" )
  {
    const auto method = "GET"s;
    struct UserData {} userData;
    spt::http::router::HttpRouter<const UserData&, bool> r;
    r.add( method, "/cut/sheet/"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.empty() );
      return true;
    } );
    r.add( method, "/cut/sheet/id/{id}"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/identifier/{identifier}"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "identifier"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/customer/code/{code}"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "code"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/facility/id/{id}"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/count/references/{id}"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/history/summary/{id}"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/history/document/{id}"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/{type}/between/{start}/{end}"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.size() == 3 );
      REQUIRE( args.contains( "type"sv ) );
      REQUIRE( args.contains( "start"sv ) );
      REQUIRE( args.contains( "end"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/work/order/id/{id}/"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/reel/with/cable/id/{id}/"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( "DELETE"sv, "/cut/sheet/id/{id}"sv, []( const UserData&, auto&& args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );

    WHEN( "Testing /cut/sheet/id/6232a3639835db5d70462e3e" )
    {
      auto url = "/cut/sheet/id/6232a3639835db5d70462e3e"s;
      auto resp = r.route( method, url, userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }
  }
}

