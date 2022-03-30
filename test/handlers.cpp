//
// Created by Rakesh on 24/03/2022.
//

#if __GNUC__ > 10 || defined _WIN32
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
    uint16_t e404{ 0 };
    uint16_t e405{ 0 };
    uint16_t e500{ 0 };
  } request;

  const auto error404 = []( Request& req, auto )
  {
    ++req.e404;
    return false;
  };

  const auto error405 = []( Request& req, auto )
  {
    ++req.e405;
    return false;
  };

  const auto error500 = []( Request& req, auto )
  {
    ++req.e500;
    return false;
  };

  GIVEN( "Router configured with error handlers" )
  {
    auto path = "/device/sensor/created/between/{start}/{end}"sv;
    auto r = spt::http::router::HttpRouter<Request&, bool>::Builder{}.
      withNotFound( error404 ).
      withMethodNotAllowed( error405 ).
      withErrorHandler( error500 ).build();

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