//
// Created by Rakesh on 16/03/2022.
//

#if __GNUC__ > 10 || defined _WIN32
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#include "../src/router.hpp"
#include "../src/fastrouter.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "Cable routes" )
{
  GIVEN( "Router configured for Cable API endpoints" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request&, bool> r;
    r.add( method, "/cable/installed/{type}/between/{start}/{end}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 3 );
      REQUIRE( args.contains( "type"sv ) );
      REQUIRE( args["type"sv] == "created"sv );
      REQUIRE( args.contains( "start"sv ) );
      REQUIRE( args["start"sv] == "2022-03-15T22:14:42.692Z"sv );
      REQUIRE( args.contains( "end"sv ) );
      REQUIRE( args["end"sv] == "2022-03-17T22:14:42.692Z"sv );
      return true;
    } );
    r.add( method, "/cable/installed/cut/sheet/id/{id}/", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      REQUIRE( args["id"sv] == "62326132e7a2e020c6652e38"sv );
      return true;
    } );

    WHEN( "Testing /cable/installed/cut/sheet/id/62326132e7a2e020c6652e38" )
    {
      auto url = "/cable/installed/cut/sheet/id/62326132e7a2e020c6652e38"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /cable/installed/created/between/2022-03-15T22:14:42.692Z/2022-03-17T22:14:42.692Z" )
    {
      auto url = "/cable/installed/created/between/2022-03-15T22:14:42.692Z/2022-03-17T22:14:42.692Z"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }
  }
}

SCENARIO( "Cable router fast" )
{
  GIVEN( "Router configured for Cable API endpoints" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::FastRouter<const Request&, bool> r;
    r.add( method, "/cable/installed/:type/between/:start/:end", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 3 );
      REQUIRE( args[0].sv() == "created"sv );
      REQUIRE( args[1].sv() == "2022-03-15T22:14:42.692Z"sv );
      REQUIRE( args[2].sv() == "2022-03-17T22:14:42.692Z"sv );
      return true;
    } );
    r.add( method, "/cable/installed/cut/sheet/id/:id/", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args[0].sv() == "62326132e7a2e020c6652e38"sv );
      return true;
    } );

    WHEN( "Testing /cable/installed/cut/sheet/id/62326132e7a2e020c6652e38" )
    {
      auto url = "/cable/installed/cut/sheet/id/62326132e7a2e020c6652e38"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /cable/installed/created/between/2022-03-15T22:14:42.692Z/2022-03-17T22:14:42.692Z" )
    {
      auto url = "/cable/installed/created/between/2022-03-15T22:14:42.692Z/2022-03-17T22:14:42.692Z"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }
  }
}
