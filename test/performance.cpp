//
// Created by Rakesh on 21/03/2022.
//

#if __GNUC__ > 10
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include "../src/router.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "Multi-threaded performance test with realistic number of routes" )
{
  GIVEN( "A router with a reasonable number of routes" )
  {
    struct UserData
    {
      UserData& operator++()
      {
        routed.fetch_add( 1 );
        return *this;
      }
      std::atomic_int32_t routed{ 0 };
    };

    struct Request
    {
      Request( std::string_view m, std::string_view u ) : method{ m }, url{ u } {}
      std::string_view method;
      std::string url;
    };

    spt::http::router::HttpRouter<UserData &, bool> r;
    auto method = "GET"sv;
    for ( auto i = 0; i < 20; ++i )
    {
      auto entity = "entity"s + std::to_string( i );
      r.add( "POST"sv, entity + "/", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
      r.add( method, entity + "/", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
      r.add( "PUT"sv, entity + "/id/{id}", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
      r.add( "DELETE"sv, entity + "/id/{id}", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
      r.add( method, entity + "/id/{id}", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
      r.add( method, entity + "/identifier/{identifier}", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
      r.add( method, entity + "/customer/code/{code}", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
      r.add( method, entity + "/facility/id/{id}", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
      r.add( method, entity + "/count/references/{id}", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
      r.add( method, entity + "/history/summary/{id}", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
      r.add( method, entity + "/history/document/{id}", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
      r.add( method, entity + "/{property}/between/{start}/{end}", []( UserData& data, auto )
      {
        ++data.routed;
        return true;
      } );
    }

    std::vector<Request> requests;
    requests.reserve( 1000 );
    for ( auto i = 0; i < 20; ++i )
    {
      auto entity = "entity"s + std::to_string( i );
      requests.emplace_back( "POST"sv, entity + "/" );
      requests.emplace_back( method, entity + "/" );
      requests.emplace_back( method, entity + "/id/6230f3069e7c9be9ff4b78a1" );
      requests.emplace_back( "PUT"sv, entity + "/id/6230f3069e7c9be9ff4b78a1" );
      requests.emplace_back( method, entity + "/identifier/Test Identifier" );
      requests.emplace_back( method, entity + "/customer/code/int-test" );
      requests.emplace_back( method, entity + "/facility/id/6230f3069e7c9be9ff4b78a1" );
      requests.emplace_back( method, entity + "/history/summary/6230f3069e7c9be9ff4b78a1" );
      requests.emplace_back( method, entity + "/history/document/6230f3069e7c9be9ff4b78a1" );
      requests.emplace_back( method, entity + "/count/references/6230f3069e7c9be9ff4b78a1" );
      requests.emplace_back( method, entity + "/created/between/2022-03-14T20:11:50.620Z/2022-03-16T20:11:50.620Z" );
      requests.emplace_back( method, entity + "/modified/between/2022-03-14T20:11:50.620Z/2022-03-16T20:11:50.620Z" );
      requests.emplace_back( "DELETE"sv, entity + "/id/6230f3069e7c9be9ff4b78a1" );
    }

    WHEN( "Measuring performance with a sequence of routes in a single thread" )
    {
      UserData userData;
      auto start = std::chrono::high_resolution_clock::now();
      for ( auto i = 0; i < 1000000; ++i )
      {
        for ( auto&& req : requests )
        {
          r.route( req.method, req.url, userData );
        }
      }
      auto stop = std::chrono::high_resolution_clock::now();
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
      std::cout << "Single thread - [" << (double(userData.routed.load()) / (ms * 1000.0)) << " million req/sec]" << std::endl;
      std::cout << "Total urls routed: " << userData.routed.load() << " in "
        << ms/1000 << " seconds." << std::endl << std::endl;
    }

    AND_WHEN( "Measuring performance with a sequence of routes with multiple threads" )
    {
      UserData userData;
      auto start = std::chrono::high_resolution_clock::now();
      std::vector<std::thread> v;
      v.reserve( 10 );
      for ( auto i = 0; i < 10; ++i )
      {
        v.emplace_back( [&requests,&userData, &r]{
          for ( auto i = 0; i < 100000; ++i )
          {
            for ( auto&& req : requests )
            {
              r.route( req.method, req.url, userData );
            }
          }
        } );
      }

      for ( auto&& t : v )
      {
        if ( t.joinable() ) t.join();
      }

      auto stop = std::chrono::high_resolution_clock::now();
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
      std::cout << "10 threads - [" << (double(userData.routed.load()) / (ms * 1000.0)) << " million req/sec]" << std::endl;
      std::cout << "Total urls routed: " << userData.routed.load() << " in "
        << ms/1000 << " seconds." << std::endl << std::endl;
    }
  }
}

