//
// Created by Rakesh on 15/03/2022.
//

#if __GNUC__ > 10 || defined _WIN32
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#include "../src/fastrouter.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "FastRouter test suite" )
{
  struct UserData {} userData;

  GIVEN( "Router with a set of API endpoints" )
  {
    spt::http::router::FastRouter<const UserData&, bool> r;

    r.add( "GET"sv, "/service/candy/:kind"sv, [](const UserData&, auto params)
    {
      REQUIRE( params.size() == 1 );
      return true;
    } );

    r.add( "GET"sv, "/service/shutdown"sv, [](const UserData&, auto params)
    {
      REQUIRE( params.empty() );
      return true;
    } );

    r.add( "GET"sv, "/"sv, [](const UserData&, auto params)
    {
      REQUIRE( params.empty() );
      return true;
    } );

    r.add( "GET"sv, "/:filename"sv, [](const UserData&, auto params)
    {
      REQUIRE( params.size() == 1 );
      return true;
    } );

    WHEN( "Checking /service/candy/lollipop" )
    {
      auto url = "/service/candy/lollipop"s;
      auto resp = r.route( "GET"s, url, userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Checking /service/candy/gum" )
    {
      auto url = "/service/candy/gum"s;
      auto resp = r.route( "GET"s, url, userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Checking /service/candy/seg_råtta" )
    {
      auto url = "/service/candy/seg_råtta"s;
      auto resp = r.route( "GET"s, url, userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Checking /service/candy/lakrits" )
    {
      auto url = "/service/candy/lakrits"s;
      auto resp = r.route( "GET"s, url, userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Checking /service/shutdown" )
    {
      auto url = "/service/shutdown"s;
      auto resp = r.route( "GET"s, url, userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Checking /" )
    {
      auto url = "/"s;
      auto resp = r.route( "GET"s, url, userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Checking /some_file.html" )
    {
      auto url = "/some_file.html"s;
      auto resp = r.route( "GET"s, url, userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Checking /another_file.jpeg" )
    {
      auto url = "/another_file.jpeg"s;
      auto resp = r.route( "GET"s, url, userData );
      REQUIRE( resp );
      REQUIRE( *resp );
    }
  }

  GIVEN( "Router configured with paths with trailing slash" )
  {
    spt::http::router::FastRouter<const UserData&, bool> r;
    auto method = "POST"sv;
    r.add( method, "/path/entity/"sv, []( const UserData&, auto ) { return true; } );

    WHEN( "Checking root paths" )
    {
      auto resp = r.route( method, "/path/entity", userData );
      REQUIRE( resp );
      REQUIRE( *resp );

      resp = r.route( method, "/path/entity/", userData );
      REQUIRE( resp );
      REQUIRE( *resp );

      resp = r.route( method, "/path/entity/id", userData );
      REQUIRE( resp ); // Issue Note this behaviour

      resp = r.route( method, "/path/entity/id/abc", userData );
      REQUIRE_FALSE( resp );

      resp = r.route( method, "/path/", userData );
      REQUIRE_FALSE( resp );
    }
  }

  GIVEN( "Router configured with paths without trailing slash" )
  {
    spt::http::router::FastRouter<const UserData&, bool> r;
    auto method = "POST"sv;
    r.add( method, "/path/entity"sv, []( const UserData&, auto ) { return true; } );

    WHEN( "Checking root paths" )
    {
      auto resp = r.route( method, "/path/entity", userData );
      REQUIRE( resp );
      REQUIRE( *resp );

      resp = r.route( method, "/path/entity/", userData, true );
      REQUIRE( resp );
      REQUIRE( *resp );

      resp = r.route( method, "/path/entity/id", userData );
      REQUIRE_FALSE( resp );

      resp = r.route( method, "/path/entity/id/abc", userData );
      REQUIRE_FALSE( resp );

      resp = r.route( method, "/path/", userData );
      REQUIRE_FALSE( resp );
    }
  }

  GIVEN( "Router configured with path with multiple parameters" )
  {
    spt::http::router::FastRouter<const UserData&, bool> r;
    std::string_view start;
    std::string_view end;
    r.add( "GET"sv, "/device/sensor/created/between/:start/:end"sv, [&start, &end](const UserData&, auto args)
    {
      REQUIRE( args.size() == 2 );
      REQUIRE( args[0].sv() == start );
      REQUIRE( args[1].sv() == end );
      return true;
    });

    WHEN( "Checking path parameters" )
    {
      start = "2022-02-14T22:25:05.147Z"sv;
      end = "2022-03-14T22:25:05.147Z"sv;
      auto url = "/device/sensor/created/between/2022-02-14T22:25:05.147Z/2022-03-14T22:25:05.147Z"s;
      auto resp = r.route( "GET"s, url, userData );
      REQUIRE( resp );
      REQUIRE_FALSE( r.route( "DELETE"s, url, userData ) );
      REQUIRE_FALSE( r.route( "OPTIONS"s, url, userData ) );
      REQUIRE_FALSE( r.route( "POST"s, url, userData ) );
      REQUIRE_FALSE( r.route( "PUT"s, url, userData ) );
      REQUIRE_FALSE( r.route( "PATCH"s, url, userData ) );

      start = "2022-01-04T22:25:05.147Z"sv;
      end = "2022-02-14T22:25:05.147Z"sv;
      url = "/device/sensor/created/between/2022-01-04T22:25:05.147Z/2022-02-14T22:25:05.147Z"s;
      REQUIRE( r.route( "GET"s, url, userData ) );
      REQUIRE_FALSE( r.route( "DELETE"s, url, userData ) );
      REQUIRE_FALSE( r.route( "OPTIONS"s, url, userData ) );
      REQUIRE_FALSE( r.route( "POST"s, url, userData ) );
      REQUIRE_FALSE( r.route( "PUT"s, url, userData ) );
      REQUIRE_FALSE( r.route( "PATCH"s, url, userData ) );
    }

    AND_WHEN( "Checking non-matching paths" )
    {
      auto url = "/device/created/between/2022-02-14T22:25:05.147Z/2022-03-14T22:25:05.147Z"s;
      REQUIRE_FALSE( r.route( "GET"s, url, userData ) );

      url = "/device/sensor/between/2022-02-14T22:25:05.147Z/2022-03-14T22:25:05.147Z"s;
      REQUIRE_FALSE( r.route( "GET"s, url, userData ) );
    }
  }
}
