# HTTP Router

* [Install](#install)
* [Use](#use)
* [Docker](#docker)
* [Performance](#performance)

Simple general purpose HTTP path based request router.  No assumption is made
on the type of framework being used.  We have used it mainly with
[nghttp2](https://www.nghttp2.org/documentation/libnghttp2_asio.html).
* Supports static and parametrised URI paths.
* Parameters (slugs) are represented curly brace enclosed name `{param}`.
  * Curly brace form was chosen in favour of `:param` for sorting purpose.
  * Sorting with `{` also means use of non-ascii characters in path will have inconsistent behaviour.
* Templated on the **Response** type and an input **UserData**.
* Function based routing.  Successful matches are *routed* to the specified
  *callback* function.
  * Callback function has signature `Response( UserData, std::unordered_map<std::string_view, std::string_view>&& )` 
  * The `std::unordered_map` will hold the parsed *parameter*->*value* pairs.

## Install
No install is necessary.  Copy the [router.h](src/router.h) and [split.h](src/split.h)
into your project and use.

The headers may be installed into a standard location using `cmake`.

```shell
git clone https://github.com/sptrakesh/http-router.git
mkdir http-router/build && cd http-router/build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local/spt ..
sudo make install
```

## Use
The **HttpRouter<Response>** class exposes two methods that are used to set up
and perform routing:
* **add** - Use to add paths or parametrised paths to the router.
* **route** - When a client request is received, delegate to the router to handle
  the request.

The following shows sample use of the router.  See [unit test](test/basic.cpp)
for more samples.

```c++
#include <router/router.h>
using namespace std::string_literals;
using namespace std::string_view_literals;

int main()
{
  struct UserData
  {
    // pass whatever you need as user data
  } userData;
  
  spt::http::router::HttpRouter<const UserData&, bool> r;
  r.add( "GET"sv, "/service/candy/{kind}"sv, [](const UserData& /*user*/, auto&& params)
  {
    assert( params.size() == 1 );
    assert( params.contains( "kind"s ) );
    return true;
  } );
  
  r.add( "GET"sv, "/service/shutdown"sv, [](const UserData& /*user*/, auto&& params)
  {
    assert( params.empty() );
    return true;
  } );

  r.add( "GET"sv, "/"sv, [](const UserData& /*user*/, auto&& params)
  {
    assert( params.empty() );
    return true;
  } );

  r.add( "GET"sv, "/{filename}/type/{mime}"sv, [](const UserData& /*user*/, auto&& params)
  {
    assert( params.size() == 2 );
    assert( params.contains( "filename" ) );
    assert( params.contains( "mime" ) );
    return true;
  } );
  
  std::vector<std::string> urls = 
      {
        "/service/candy/lollipop"s, // kind=lollipop
        "/service/candy/gum"s, // kind=gum
        "/service/candy/seg_råtta"s, // kind=seg_råtta
        "/service/candy/lakrits"s, // kind=lakrits
        "/service/shutdown"s, // static route
        "/"s, // static route
        "/some_file.html/type/html"s, // filename=some_file.html; mime=html
        "/another_file.jpeg/type/jpg"s // filename=another_file.jpeg; mime=jpg
      };
  for ( auto&& url : urls )
  {
    auto resp = r.route( "GET"s, url, userData );
    assert( resp );
    assert( *resp );
  }
}
```

The `route` method returns a `std::optional<Response>`.  If no configured path
matches returns `std::nullopt`.  Otherwise, returns the response from the callback
function.

## Docker
A docker image with the header files is available at [Docker hub](https://hub.docker.com/repository/docker/sptrakesh/http-router).
A very simple image with the headers installed under `/opt/spt/include`.  Use
as a base image when building your target image.

## Performance
Router is not optimised for high performance.  Configured paths are stored in
a sorted `std::vector`, and searched for using binary search.

Benchmark numbers from [benchmark.cpp](test/benchmark.cpp) are below:
```shell
[2.46063 million req/sec] for URL: /service/candy/lollipop
[3.01114 million req/sec] for URL: /service/candy/gum
[2.53743 million req/sec] for URL: /service/candy/seg_råtta
[2.57798 million req/sec] for URL: /service/candy/lakrits
[15.2439 million req/sec] for URL: /service/shutdown
[16.7504 million req/sec] for URL: /
[3.61925 million req/sec] for URL: /some_file.html
[3.48675 million req/sec] for URL: /another_file.jpeg
Checksum: 80000000
```

These were by computing the average time to route each URI path 10,000,000 times.