# HTTP Router

* [Install](#install)
* [Use](#use)
* [Docker](#docker)
* [Performance](#performance)
* [FastRouter](#fast-router)

Simple general purpose HTTP path based request router.  No assumption is made
on the type of framework being used.  We have used it mainly with
[nghttp2](https://www.nghttp2.org/documentation/libnghttp2_asio.html).
* Supports static and parametrised URI paths.
* Parameters (slugs) are represented using curly brace enclosed name `{param}`.
  * Curly brace form was chosen in favour of `:param` for sorting purpose.
  * Sorting with `{` implies use of non-ascii characters in path will have inconsistent behaviour.
* Templated on the **Response** type and an input **UserData**.
* Function based routing.  Successful matches are *routed* to the specified
  *handler* callback function.
  * Callback function has signature `Response( UserData, std::unordered_map<std::string_view, std::string_view>&& )` 
  * The `std::unordered_map` will hold the parsed *parameter->value* pairs.

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
The **HttpRouter<UserData, Response>** class exposes two methods that are used to set up
and perform routing:
* **add** - Use to add paths or parametrised paths to the router.
  * This is thread safe.  Configuring routing should generally not need
  thread safety, but just in case route additions are set up lazily in a
  multi-threaded environment, a `std::mutex` is used to ensure thread safety.
  * Duplicate routes will throw a [`spt::http::router::DuplicateRouteError`](src/error.h) exception.
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
  r.add( "GET"sv, "/service/candy/{kind}"sv, [](const UserData&, auto params)
  {
    assert( params.size() == 1 );
    assert( params.contains( "kind"s ) );
    return true;
  } );
  
  r.add( "GET"sv, "/service/shutdown"sv, [](const UserData&, auto params)
  {
    assert( params.empty() );
    return true;
  } );

  r.add( "GET"sv, "/"sv, [](const UserData&, auto params)
  {
    assert( params.empty() );
    return true;
  } );

  r.add( "GET"sv, "/{filename}/type/{mime}"sv, [](const UserData&, auto params)
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
matches, returns `std::nullopt`.  Otherwise, returns the response from the callback
function.

## Docker
A docker image with the header files is available at [Docker hub](https://hub.docker.com/repository/docker/sptrakesh/http-router).
A very simple image with the headers installed under `/opt/spt/include`.  Use
as a base image when building your target image.

## Performance
Router is not optimised for high performance.  Configured paths are stored in
a sorted `std::vector`, and searched for using binary search.

Benchmark numbers from [benchmark.cpp](test/benchmark.cpp) are in the following sections.
These were by computing the average time to route each URI path 10,000,000 times.
The Linux numbers were from a VM running on Parallels on Mac, bare metal
numbers may be higher.

<details>
  <summary><strong>Mac OS X Apple Clang</strong></summary>

```shell
[2.46063 million req/sec] for URL: /service/candy/lollipop
[3.11333 million req/sec] for URL: /service/candy/gum
[2.81373 million req/sec] for URL: /service/candy/seg_råtta
[2.84495 million req/sec] for URL: /service/candy/lakrits
[16.9779 million req/sec] for URL: /service/shutdown
[18.622 million req/sec] for URL: /
[3.89864 million req/sec] for URL: /some_file.html
[3.79075 million req/sec] for URL: /another_file.jpeg
Checksum: 80000000
```
</details>

<details>
  <summary><strong>Linux GCC 11.2</strong></summary>

```shell
[5.40833 million req/sec] for URL: /service/candy/lollipop
[5.58036 million req/sec] for URL: /service/candy/gum
[5.95593 million req/sec] for URL: /service/candy/seg_råtta
[5.87889 million req/sec] for URL: /service/candy/lakrits
[21.2314 million req/sec] for URL: /service/shutdown
[22.1239 million req/sec] for URL: /
[6.64452 million req/sec] for URL: /some_file.html
[6.45578 million req/sec] for URL: /another_file.jpeg
Checksum: 80000000
```
</details>

## Fast Router
The fast router is a wrapper around [HttpRouter](https://github.com/killvxk/HttpRouter).
The original implementation has been slightly modified and updated.  There are
some cases where it does not work exactly as the simple router, including incorrect
handling in some cases (search for comment with Issue in test suite), but when
those issues are not relevant (or do not kick in for your routes), use it for
an optimal router.

### Path Parameters
Path parameters are handled differently.  Use the `:<param name>` pattern to
specify parameters.  The handler callback function also returns a `std::vector`
of parameter values instead of `std::unordered_map`.

### Performance
Benchmark numbers from [benchmarkfast.cpp](test/benchmarkfast.cpp) are below:

<details>
  <summary><strong>Mac OS X Apple Clang</strong></summary>

```shell
[7.8125 million req/sec] for URL: /service/candy/lollipop
[15.4083 million req/sec] for URL: /service/candy/gum
[8.19001 million req/sec] for URL: /service/candy/seg_råtta
[8.16993 million req/sec] for URL: /service/candy/lakrits
[15.5763 million req/sec] for URL: /service/shutdown
[21.4133 million req/sec] for URL: /
[20.9205 million req/sec] for URL: /some_file.html
[21.8341 million req/sec] for URL: /another_file.jpeg
```
</details>

<details>
  <summary><strong>Linux GCC 11.2</strong></summary>

```shell
[16.2338 million req/sec] for URL: /service/candy/lollipop
[15.3139 million req/sec] for URL: /service/candy/gum
[17.3611 million req/sec] for URL: /service/candy/seg_råtta
[17.2414 million req/sec] for URL: /service/candy/lakrits
[17.452 million req/sec] for URL: /service/shutdown
[33.557 million req/sec] for URL: /
[21.9298 million req/sec] for URL: /some_file.html
[22.2717 million req/sec] for URL: /another_file.jpe
```
</details>