//
// Created by Rakesh on 16/03/2022.
//
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"

#if __GNUC__ > 10
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#endif
#include "boost.h"
#include "../src/router.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "Sensor Device routes" )
{
  GIVEN( "Router configured for Sensor device API endpoints" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request &, bool> r;
    r.add( method, "/device/sensor/", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 0 );
      return true;
    } );
    r.add( method, "/device/sensor/id/{id}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/identifier/{identifier}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/customer/code/{code}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/facility/id/{id}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/count/references/{id}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/history/summary/{id}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/history/document/{id}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      return true;
    } );
    r.add( method, "/device/sensor/{property}/between/{start}/{end}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 3 );
      return true;
    } );

    WHEN( "Testing /device/sensor/" )
    {
      auto url = "/device/sensor/"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1")
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/identifier/Integration Test Identifier" )
    {
      auto url = "/device/sensor/identifier/Integration Test Identifier"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/customer/code/int-test" )
    {
      auto url = "/device/sensor/customer/code/int-test"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/history/summary/6230f3069e7c9be9ff4b78a1" )
    {
      auto url = "/device/sensor/history/summary/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/history/document/6230f3069e7c9be9ff4b78a1" )
    {
      auto url = "/device/sensor/history/document/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/count/references/6230f3069e7c9be9ff4b78a1" )
    {
      auto url = "/device/sensor/count/references/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/created/between/2022-03-14T20:11:50.620Z/2022-03-16T20:11:50.620Z" )
    {
      auto url = "/device/sensor/created/between/2022-03-14T20:11:50.620Z/2022-03-16T20:11:50.620Z"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }

    AND_WHEN( "Testing /device/sensor/modified/between/2022-03-14T20:11:50.621Z/2022-03-16T20:11:50.621Z" )
    {
      auto url = "/device/sensor/modified/between/2022-03-14T20:11:50.621Z/2022-03-16T20:11:50.621Z"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }
  }

  GIVEN( "Route configured with id parameter" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request &, bool> r;
    r.add( method, "/device/sensor/id/{id}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      REQUIRE( args["id"sv] == "6230f3069e7c9be9ff4b78a1"sv );
      return true;
    } );

    WHEN( "Testing /device/sensor/id/6230f3069e7c9be9ff4b78a1")
    {
      auto url = "/device/sensor/id/6230f3069e7c9be9ff4b78a1"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }
  }

  GIVEN( "Router configured with identifier parameter" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request &, bool> r;
    r.add( method, "/device/sensor/identifier/{identifier}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "identifier"sv ) );
      REQUIRE( args["identifier"sv] == "Integration Test Identifier"sv );
      return true;
    } );

    AND_WHEN( "Testing /device/sensor/identifier/Integration Test Identifier" )
    {
      auto url = "/device/sensor/identifier/Integration Test Identifier"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }
  }

  GIVEN( "Router configured with code parameter" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request &, bool> r;
    r.add( method, "/device/sensor/customer/code/{code}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "code"sv ) );
      REQUIRE( args["code"sv] == "int-test"sv );
      return true;
    } );
    AND_WHEN( "Testing /device/sensor/customer/code/int-test" )
    {
      auto url = "/device/sensor/customer/code/int-test"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }
  }

  GIVEN( "Router configured with two date parameters" )
  {
    const auto method = "GET"s;
    struct Request {} request;
    spt::http::router::HttpRouter<const Request &, bool> r;
    r.add( method, "/device/sensor/{property}/between/{start}/{end}", []( const Request&, auto args )
    {
      REQUIRE( args.size() == 3 );
      REQUIRE( args.contains( "property"sv ) );
      REQUIRE( args[ "property"sv ] == "modified"sv );
      REQUIRE( args.contains( "start"sv ) );
      REQUIRE( args[ "start"sv ] == "2022-03-14T20:11:50.621Z"sv );
      REQUIRE( args.contains( "end"sv ) );
      REQUIRE( args[ "end"sv ] == "2022-03-16T20:11:50.621Z"sv );
      return true;
    } );

    AND_WHEN( "Testing /device/sensor/modified/between/2022-03-14T20:11:50.621Z/2022-03-16T20:11:50.621Z" )
    {
      auto url = "/device/sensor/modified/between/2022-03-14T20:11:50.621Z/2022-03-16T20:11:50.621Z"s;
      auto resp = r.route( method, url, request );
      REQUIRE( resp );
      REQUIRE( *resp );
    }
  }
}

#ifdef HAS_BOOST
SCENARIO( "Sensor device serialisation tests" )
{
  GIVEN( "A router with configured routes" )
  {
    const auto method = "GET"sv;
    struct Request {};
    spt::http::router::HttpRouter<const Request&, bool> r;
    r.add( "POST"sv, "/device/sensor/"sv, []( const Request&, auto args )
    {
      REQUIRE( args.empty() );
      return true;
    }, "./paths/sensor.yaml#/root"sv );
    r.add( method, "/device/sensor/"sv, []( const Request&, auto args )
    {
      REQUIRE( args.empty() );
      return true;
    }, "./paths/sensor.yaml#/root"sv );
    r.add( "PUT"sv, "/device/sensor/id/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    }, "./paths/sensor.yaml#/id"sv );
    r.add( method, "/device/sensor/id/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    }, "./paths/sensor.yaml#/id"sv );
    r.add( "DELETE"sv, "/device/sensor/id/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    }, "./paths/sensor.yaml#/id"sv );
    r.add( method, "/device/sensor/identifier/{identifier}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "identifier"sv ) );
      return true;
    }, "./paths/sensor.yaml#/identifier"sv );
    r.add( method, "/device/sensor/customer/code/{code}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "code"sv ) );
      return true;
    }, "./paths/sensor.yaml#/customer"sv );
    r.add( method, "/device/sensor/facility/id/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    }, "./paths/sensor.yaml#/facility"sv );
    r.add( method, "/device/sensor/count/references/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    }, "./paths/sensor.yaml#/refcount"sv );
    r.add( method, "/device/sensor/history/summary/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    }, "./paths/sensor.yaml#/history/summary"sv );
    r.add( method, "/device/sensor/history/document/{id}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 1 );
      REQUIRE( args.contains( "id"sv ) );
      return true;
    }, "./paths/sensor.yaml#/history/document"sv );
    r.add( method, "/device/sensor/{property}/between/{start}/{end}"sv, []( const Request&, auto args )
    {
      REQUIRE( args.size() == 3 );
      REQUIRE( args.contains( "property"sv ) );
      REQUIRE( args.contains( "start"sv ) );
      REQUIRE( args.contains( "end"sv ) );
      return true;
    }, "./paths/sensor.yaml#/between"sv );

    WHEN( "Serialising to JSON" )
    {
      auto json = r.json();
      std::cout << json << std::endl;
      REQUIRE( json.is_object() );
      auto obj = json.as_object();
      REQUIRE( obj.contains( "paths" ) );
      REQUIRE( obj["paths"].is_array() );
      REQUIRE( obj.contains( "total" ) );
      REQUIRE( obj["total"].is_uint64() );
      REQUIRE( obj["paths"].as_array().size() == obj["total"].as_uint64() );
      REQUIRE( obj.contains( "static" ) );
      REQUIRE( obj["static"].is_int64() );
      REQUIRE( obj["static"].as_int64() == 1 );
      REQUIRE( obj.contains( "dynamic" ) );
      REQUIRE( obj["dynamic"].is_int64() );
      REQUIRE( obj["dynamic"].as_int64() == ( static_cast<int64_t>( obj["total"].as_uint64() ) - obj["static"].as_int64() ) );
    }

    AND_WHEN( "Serialising to string" )
    {
      auto s = r.str();
      REQUIRE( s[0] == '{' );
      REQUIRE( s.ends_with( '}' ) );
      REQUIRE_FALSE( s.find( "paths" ) == std::string::npos );
      REQUIRE_FALSE( s.find( "[" ) == std::string::npos );
      REQUIRE_FALSE( s.find( "]" ) == std::string::npos );
      REQUIRE_FALSE( s.find( "total" ) == std::string::npos );
      REQUIRE_FALSE( s.find( "static" ) == std::string::npos );
      REQUIRE_FALSE( s.find( "dynamic" ) == std::string::npos );
    }

    AND_WHEN( "Generating YAML" )
    {
      auto yaml = r.yaml();
      std::cout << yaml << std::endl;
    }
  }
}
#endif
