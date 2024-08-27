//
// Created by Rakesh on 16/03/2022.
//
#if __GNUC__ > 10 || defined _WIN32
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#include "../src/fastrouter.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "Cut sheet routes fast" )
{
  GIVEN( "Router configured for cut sheet API endpoints" )
  {
    const auto method = "GET"s;
    struct UserData {} userData;
    spt::http::router::FastRouter<const UserData&, std::vector<killvxk::string_view>> r;
    r.add( method, "/cut/sheet/"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.empty() );
      return args;
    } );
    r.add( method, "/cut/sheet/id/:id"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );
    r.add( method, "/cut/sheet/identifier/:identifier"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );
    r.add( method, "/cut/sheet/customer/code/:code"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );
    r.add( method, "/cut/sheet/facility/id/:id"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );
    r.add( method, "/cut/sheet/count/references/:id"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );
    r.add( method, "/cut/sheet/history/summary/:id"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );
    r.add( method, "/cut/sheet/history/document/:id"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );
    r.add( method, "/cut/sheet/:type/between/:start/:end"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.size() == 3 );
      return args;
    } );
    r.add( method, "/cut/sheet/work/order/id/:id/"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );
    r.add( method, "/cut/sheet/reel/with/cable/id/:id/"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );
    r.add( "DELETE"sv, "/cut/sheet/id/:id"sv, []( const UserData&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return args;
    } );

    WHEN( "Testing /cut/sheet/id/6232a3639835db5d70462e3e" )
    {
      auto url = "/cut/sheet/id/6232a3639835db5d70462e3e"s;
      auto resp = r.route( method, url, userData );
      REQUIRE( resp );
      REQUIRE( resp->at( 0 ).sv() == "6232a3639835db5d70462e3e"sv );
    }
  }
}

