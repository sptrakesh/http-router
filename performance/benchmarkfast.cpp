//
// Created by Rakesh on 16/03/2022.
//

#include <chrono>
#include <iostream>
#include "../src/fastrouter.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

int main()
{
  struct UserData
  {
    int routed{ 0 };
  };

  spt::http::router::FastRouter<UserData *, bool> r;
  UserData userData;

  r.add( "GET"s, "/service/candy/{kind}", [](UserData *user, auto&& /* args */) {
    user->routed++;
    return true;
  } );

  r.add( "GET"s, "/service/shutdown", [](UserData *user, auto&& /* args */) {
    user->routed++;
    return true;
  } );

  r.add( "GET"s, "/", [](UserData *user, auto&& /* args */) {
    user->routed++;
    return true;
  } );

  r.add( "GET"s, "/{filename}", [](UserData *user, auto&& /* args */) {
    user->routed++;
    return true;
  } );

  // run benchmark of various urls
  std::vector<std::string> urls = {
      "/service/candy/lollipop",
      "/service/candy/gum",
      "/service/candy/seg_råtta",
      "/service/candy/lakrits",
      "/service/shutdown",
      "/",
      "/some_file.html",
      "/another_file.jpeg"
  };

  for ( auto&& url : urls )
  {
    auto start = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 10000000; ++i )
    {
      r.route( "GET"s, url, &userData );
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "[" << (10000.0 / ms) << " million req/sec] for URL: " << url << std::endl;
  }

  std::cout << "Checksum: " << userData.routed << std::endl << std::endl;
}