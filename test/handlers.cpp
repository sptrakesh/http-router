//
// Created by Rakesh on 24/03/2022.
//

#if __GNUC__ > 10
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#include "../src/router.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "HttpRouter error handlers test suite" )
{
  struct Request
  {
    uint e404{ 0 };
    uint e405{ 0 };
    uint e500{ 0 };
  } request;

  const auto error404 = []( Request& req, std::unordered_map<std::string_view, std::string_view> )
  {
    ++req.e404;
    return false;
  };

  const auto error405 = []( Request& req, std::unordered_map<std::string_view, std::string_view> )
  {
    ++req.e405;
    return false;
  };

  const auto error500 = []( Request& req, std::unordered_map<std::string_view, std::string_view> )
  {
    ++req.e500;
    return false;
  };

  GIVEN( "Router configured with error handlers" )
  {
    auto path = "/device/sensor/created/between/{start}/{end}"sv;
    spt::http::router::HttpRouter<Request&, bool> r{ error404, error405, error500 };
    r.add( "GET"sv, path, [](Request&, auto ) { return true; });
    r.add( "GET"sv, "/throw/exception"sv, [](Request&, auto ) -> bool
    {
      throw std::runtime_error{ "Testing exception handling" };
    });
    r.add( "POST"sv, "/throw/exception"sv, [](Request&, auto ) -> bool
    {
      throw std::runtime_error{ "Testing exception handling" };
    });
    r.add( "PUT"sv, "/throw/exception"sv, [](Request&, auto ) -> bool
    {
      throw std::runtime_error{ "Testing exception handling" };
    });

    WHEN( "Making a POST request" )
    {
      auto count = request.e405;
      auto resp = r.route( "POST"s, path, request );
      REQUIRE( resp );
      REQUIRE_FALSE( *resp );
      REQUIRE( request.e405 == count + 1 );
    }

    AND_WHEN( "Making a PUT request" )
    {
      auto count = request.e405;
      auto resp = r.route( "PUT"s, path, request );
      REQUIRE( resp );
      REQUIRE_FALSE( *resp );
      REQUIRE( request.e405 == count + 1 );
    }

    AND_WHEN( "Making a DELETE request" )
    {
      auto count = request.e405;
      auto resp = r.route( "DELETE"s, path, request );
      REQUIRE( resp );
      REQUIRE_FALSE( *resp );
      REQUIRE( request.e405 == count + 1 );
    }

    AND_WHEN( "Making a request to an unconfigured path" )
    {
      auto count = request.e404;
      auto resp = r.route( "GET"s, "/device/sensor/"sv, request );
      REQUIRE( resp );
      REQUIRE_FALSE( *resp );
      REQUIRE( request.e404 == count + 1 );
    }

    AND_WHEN( "Making a GET request to a path that throws exception" )
    {
      auto count = request.e500;
      auto resp = r.route( "GET"s, "/throw/exception"sv, request );
      REQUIRE( resp );
      REQUIRE_FALSE( *resp );
      REQUIRE( request.e500 == count + 1 );
    }

    AND_WHEN( "Making a PUT request to a path that throws exception" )
    {
      auto count = request.e500;
      auto resp = r.route( "PUT"s, "/throw/exception"sv, request );
      REQUIRE( resp );
      REQUIRE_FALSE( *resp );
      REQUIRE( request.e500 == count + 1 );
    }

    AND_WHEN( "Making a POST request to a path that throws exception" )
    {
      auto count = request.e500;
      auto resp = r.route( "POST"s, "/throw/exception"sv, request );
      REQUIRE( resp );
      REQUIRE_FALSE( *resp );
      REQUIRE( request.e500 == count + 1 );
    }
  }
}