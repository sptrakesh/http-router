//
// Created by Rakesh on 16/03/2022.
//
#if __GNUC__ > 10 || defined _WIN32
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#include "../src/router.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "Cut sheet routes" )
{
  GIVEN( "Router configured for cut sheet API endpoints" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request&, bool> r;
    r.add( method, "/cut/sheet/"sv, []( const Request&, auto args )
    {
      REQUIRE( args.empty() );
      return true;
    } );
    r.add( method, "/cut/sheet/id/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/identifier/{identifier}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "identifier"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/customer/code/{code}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "code"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/facility/id/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/count/references/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/history/summary/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/history/document/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/{type}/between/{start}/{end}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 3 );
      REQUIRE( args.contains( "type"sv ) );
      REQUIRE( args.contains( "start"sv ) );
      REQUIRE( args.contains( "end"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/work/order/id/{id}/"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/cut/sheet/reel/with/cable/id/{id}/"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );
    r.add( "DELETE"sv, "/cut/sheet/id/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    } );

    WHEN( "Testing /cut/sheet/id/6232a3639835db5d70462e3e" )
    {
      auto url = "/cut/sheet/id/6232a3639835db5d70462e3e"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
      auto [p, m] = r.canRoute( method, url );
      CHECK( p );
      CHECK( m );
    }
  }
}

