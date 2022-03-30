# HTTP Router

* [Install](#install)
* [Use](#use)
* [Docker](#docker)
* [Performance](#performance)
  * [Benchmark](#benchmark)
  * [Realistic](#realistic-scenario)
* [FastRouter](#fast-router)

Simple general purpose HTTP path based request router.  No assumption is made
on the type of framework being used.  We have used it mainly with
[nghttp2](https://www.nghttp2.org/documentation/libnghttp2_asio.html).
* Supports static and parametrised URI paths.
* Parameters (slugs) are represented using curly brace enclosed name `{param}`.
  * Curly brace form was chosen in favour of `:param` for sorting purpose.
  * Sorting with `{` implies use of non-ascii characters in path will have inconsistent behaviour.
* Templated on the **Response** type and an input **Request**.
* Function based routing.  Successful matches are *routed* to the specified
  *handler* callback function.
  * Parameters are returned as a *map*.  If [boost](https://boost.org/) is found,
    a `boost::container::flat_map` is returned, else a `std::unordered_map` is
    returned.
  * Callback function has signature `Response( Request, MapType<std::string_view, std::string_view>&& )` 
    where `MapType` is either `boost::container::flat_map` or `std::unordered_map`.
  * The `MapType` will hold the parsed *parameter->value* pairs.

## Install
No install is necessary.  Copy the [router.h](src/router.h), [split.h](src/split.h),
and [concat.h](src/concat.h) files into your project and use.

The headers may be installed into a standard location using `cmake`.

```shell
git clone https://github.com/sptrakesh/http-router.git
mkdir http-router/build && cd http-router/build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local/spt ..
sudo make install
```

## Use
The **HttpRouter<Request, Response>** class exposes two primary methods - 
`add` and `route` - that are used to set up and perform routing:
* **CTOR** - Create an instance with the optional handlers to handle standard
  scenarios such as *Not Found (404)*, *Method Not Allowed (405)*, and
  *Internal Server Error (500)*. 
* **add** - Use to add paths or parametrised paths to the router.
  * This is thread safe.  Configuring routing should generally not need
  thread safety, but just in case route additions are set up lazily in a
  multi-threaded environment, a `std::mutex` is used to ensure thread safety.
  * Duplicate routes will throw a [`spt::http::router::DuplicateRouteError`](src/error.h) exception.
  * Routes with invalid parameter will throw a [`spt::http::router::InvalidParameterError](src/error.h) exception.
    * This is thrown if a parameter uses the `:<parameter>` form. 
    * This is thrown if a parameter does not end with the `}` character.
* **route** - When a client request is received, delegate to the router to handle
  the request.
  * If a *notFound* handler was specified when creating the router (first optional
    constructor parameter), and the input request *path* was not found, the
    handler will be invoked.
  * If a *methodNotFound* handler was specified when creating the router (second
    optional constructor parameter), and the input request *method* was not
    configured for the specified *path*, the handler will be invoked.
  * If a *errorHandler* handler was specified when creating the router (third
    optional constructor parameter), and an exception was thrown by the configured
    handler function for the *method:path*, the handler will be invoked.
* If Boost has been found a few additional utility methods are exposed.
  * **json** - Output the configured routes and some additional metadata as a
    JSON structure.  See the sample output below from the [device](test/device.cpp) test.
  * **str** - Output the configured routes and some additional metadata as a string.
  This is just the JSON representation serialised.
  * **operator<<** - Appends the string representation to the output stream.
* **yaml** - Output the configured routes in YAML format which can be embedded
  or cross-verified against the API OpenAPI Specifications file.  If using this
  feature, please try to specify the optional `ref` parameter to the *add* method.
  For example see the output below from the [device](test/device.cpp) test.

<details>
  <summary><strong>Sample JSON Output</strong></summary>

```json
{
  "paths": [
    {
      "path": "/device/sensor/",
      "methods": ["POST", "GET"]
    },
    {
      "path": "/device/sensor/count/references/{id}",
      "methods": ["GET"]
    },
    {
      "path": "/device/sensor/customer/code/{code}",
      "methods": ["GET"]
    },
    {
      "path": "/device/sensor/facility/id/{id}",
      "methods": ["GET"]
    },
    {
      "path": "/device/sensor/history/document/{id}",
      "methods": ["GET"]
    },
    {
      "path": "/device/sensor/history/summary/{id}",
      "methods": ["GET"]
    },
    {
      "path": "/device/sensor/id/{id}",
      "methods": ["PUT", "GET", "DELETE"]
    },
    {
      "path": "/device/sensor/identifier/{identifier}",
      "methods": ["GET"]
    },
    {
      "path": "/device/sensor/{property}/between/{start}/{end}",
      "methods": ["GET"]
    }
  ],
  "total": 9,
  "static": 1,
  "dynamic": 8
}
```
</details>
<details>
  <summary><strong>Sample YAML Output</strong></summary>

```yaml
paths:
  /device/sensor/:
    $ref: "./paths/sensor.yaml#/root"
  /device/sensor/count/references/{id}:
    $ref: "./paths/sensor.yaml#/refcount"
  /device/sensor/customer/code/{code}:
    $ref: "./paths/sensor.yaml#/customer"
  /device/sensor/facility/id/{id}:
    $ref: "./paths/sensor.yaml#/facility"
  /device/sensor/history/document/{id}:
    $ref: "./paths/sensor.yaml#/history/document"
  /device/sensor/history/summary/{id}:
    $ref: "./paths/sensor.yaml#/history/summary"
  /device/sensor/id/{id}:
    $ref: "./paths/sensor.yaml#/id"
  /device/sensor/identifier/{identifier}:
    $ref: "./paths/sensor.yaml#/identifier"
  /device/sensor/{property}/between/{start}/{end}:
    $ref: "./paths/sensor.yaml#/between"
```
</details>

The following shows sample use of the router.  See [basic](test/basic.cpp) test
and other unit tests for more samples.

<details>
  <summary><strong>Sample code</strong></summary>

```c++
#include <router/router.h>
// If your project uses boost and has not already included boost::json sources
#include <boost/json/src.hpp>

using namespace std::string_literals;
using namespace std::string_view_literals;

int main()
{
  struct Request
  {
    // pass whatever you need as user data
  } request;
  
  const auto method = "GET"sv;
  spt::http::router::HttpRouter<const Request&, bool> r;
  r.add( "POST"sv, "/device/sensor/"sv, []( const Request&, auto args )
    {
      assert( args.empty() );
      return true;
    } );
    r.add( method, "/device/sensor/"sv, []( const Request&, auto args )
    {
      assert( args.empty() );
      return true;
    } );
    r.add( "PUT"sv, "/device/sensor/id/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/id/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/identifier/{identifier}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "identifier"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/customer/code/{code}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "code"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/facility/id/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/count/references/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/history/summary/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/history/document/{id}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 1 );
      assert( args.contains( "id"sv ) );
      return true;
    } );
    r.add( method, "/device/sensor/{property}/between/{start}/{end}"sv, []( const Request&, auto args )
    {
      assert( args.size() == 3 );
      assert( args.contains( "property"sv ) );
      assert( args.contains( "start"sv ) );
      assert( args.contains( "end"sv ) );
      return true;
    } );
  
  std::vector<std::string> urls = 
      {
        "/device/sensor/"s,
        "/device/sensor/id/6230f3069e7c9be9ff4b78a1"s, // id=6230f3069e7c9be9ff4b78a1
        "/device/sensor/identifier/Integration Test Identifier"s, // identifier=Integration Test Identifier
        "/device/sensor/customer/code/int-test"s, // code=int-test
        "/device/sensor/history/summary/6230f3069e7c9be9ff4b78a1"s, // id=6230f3069e7c9be9ff4b78a1
        "/device/sensor/history/document/6230f3069e7c9be9ff4b78a1"s, // id=6230f3069e7c9be9ff4b78a1
        "/device/sensor/count/references/6230f3069e7c9be9ff4b78a1"s, // id=6230f3069e7c9be9ff4b78a1
        "/device/sensor/created/between/2022-03-14T20:11:50.620Z/2022-03-16T20:11:50.620Z"s, // property=created, start=2022-03-14T20:11:50.620Z, end=2022-03-16T20:11:50.620Z
      };
  for ( auto&& url : urls )
  {
    auto resp = r.route( "GET"sv, url, request );
    assert( resp );
    assert( *resp );
  }
  
  auto resp = r.route( "PUT"sv, "/device/sensor/"sv );
  assert( resp );
  assert( !*resp ); // PUT not configured
  
  resp = r.route( "POST"sv, "/device/sensor/history/document/{id}"sv );
  assert( resp );
  assert( !*resp ); // POST not configured
  
  try
  {
    r.add( "PUT"sv, "/device/sensor/id/{id}"sv, []( const Request&, auto args ) { return true; } );
  }
  catch ( const spt::http::router::DuplicateRouteError& e )
  {
    // Will be caught as we registered the same route earlier
    std::cerr << e.what() << '\n';
  }
}
```
</details>

<details>
  <summary><strong>Use with nghttp2</strong></summary>

```c++
#include <nghttp2/asio_http2_server.h>
#include <log/NanoLog.h>
#include <router/router.h>

int main()
{
  struct Request
  {
    explicit Request( const nghttp2::asio_http2::server::request& req ) :
      header{ req.header() }, method{ req.method() },
      path{ req.uri().path }, query{ req.uri().raw_query } {}
      
    nghttp2::asio_http2::header_map header;
    std::string method;
    std::string path;
    std::string query;
    std::shared_ptr<std::string> body{ nullptr };
  };
  
  struct Response
  {
    nghttp2::asio_http2::header_map headers;
    std::string body{ "{}" };
    uint16_t status{ 200 };
    bool compressed{ false };
  };
  
  auto const error404 = []( const Request&, auto ) -> Response
  {
    return { {}, R"({"code": 404, "cause": "Not Found"})"s, 404, false }
  }
  
  auto const error405 - []( const Request&, auto ) -> Response
  {
    return { {}, R"({"code": 405, "cause": "Method Not Allowed"})"s, 405, false }
  }
  
  auto router = spt::http::router::HttpRouter<const Request&, Response>::Builder{}.
    withNotFound( error404 ).withMethodNotAllowed( error405 ).build();
  // set up router as in above sample
  nghttp2::asio_http2::server::http2 server;
  server.num_threads( 8 );
  
  server.handle( "/", [&router](const nghttp2::asio_http2::server::request& req,
        const nghttp2::asio_http2::server::response& res)
  {
    auto request = Request{ req };
    auto response = router.route( request.method, request.path, request );
    assert( response );
    res.write_head( response->status, response->headers );
    res.end( response->body );
  });
  
  boost::system::error_code ec;
  if ( server.listen_and_serve( ec, "0.0.0.0", port, true ) )
  {
    LOG_CRIT << "error: " << ec.message();
    return 1;
  }
}
```

</details>

The `route` method returns a `std::optional<Response>`.  If no configured path
matches, returns `std::nullopt` (or the response from the not found handler if
specified at construction time).  Otherwise, returns the response from the
callback function.

## Docker
A docker image with the header files is available at [Docker hub](https://hub.docker.com/repository/docker/sptrakesh/http-router).
A very simple image with the headers installed under `/opt/spt/include`.  Use
as a base image when building your target image.

## Performance
Router is not optimised for high performance.  Configured paths are stored in
a sorted `std::vector`, and searched for using binary search.

### Benchmark
Benchmark numbers from [benchmark.cpp](performance/benchmark.cpp) are in the following sections.
These were by computing the average time to route each URI path 10,000,000 times.
The Linux numbers were from a VM running on Parallels on a Mac Book Pro 2019 model
(limited to 6 of 16 available cores), bare metal numbers may be higher. Similarly
the Windows numbers where from a VM running on Parallels.

<details>
  <summary><strong>Mac OS X Apple clang version 13.1.6 (clang-1316.0.21.2)</strong></summary>

**With std::unordered_map**
```shell
[3.36474 million req/sec] for URL: /service/candy/lollipop
[4.22833 million req/sec] for URL: /service/candy/gum
[3.4118 million req/sec] for URL: /service/candy/seg_råtta
[4.16667 million req/sec] for URL: /service/candy/lakrits
[30.581 million req/sec] for URL: /service/shutdown
[43.29 million req/sec] for URL: /
[4.79616 million req/sec] for URL: /some_file.html
[4.62535 million req/sec] for URL: /another_file.jpeg
Checksum: 80000000
```

**With boost::container::flat_map**
```shell
[3.99361 million req/sec] for URL: /service/candy/lollipop
[5.29942 million req/sec] for URL: /service/candy/gum
[4.04694 million req/sec] for URL: /service/candy/seg_råtta
[5.21648 million req/sec] for URL: /service/candy/lakrits
[28.9017 million req/sec] for URL: /service/shutdown
[45.6621 million req/sec] for URL: /
[6.10128 million req/sec] for URL: /some_file.html
[5.9312 million req/sec] for URL: /another_file.jpeg
Checksum: 80000000
```
</details>

<details>
  <summary><strong>Linux GCC 11.2</strong></summary>

**With std::unordered_map**
```shell
[5.98802 million req/sec] for URL: /service/candy/lollipop
[6.44745 million req/sec] for URL: /service/candy/gum
[6.2461 million req/sec] for URL: /service/candy/seg_råtta
[6.35324 million req/sec] for URL: /service/candy/lakrits
[21.2314 million req/sec] for URL: /service/shutdown
[28.5714 million req/sec] for URL: /
[8.16993 million req/sec] for URL: /some_file.html
[6.93963 million req/sec] for URL: /another_file.jpeg
Checksum: 80000000
```

**With boost::container::flat_map**
```shell
[8.59845 million req/sec] for URL: /service/candy/lollipop
[8.78735 million req/sec] for URL: /service/candy/gum
[8.05153 million req/sec] for URL: /service/candy/seg_råtta
[8.48896 million req/sec] for URL: /service/candy/lakrits
[22.2717 million req/sec] for URL: /service/shutdown
[30.4878 million req/sec] for URL: /
[11.8765 million req/sec] for URL: /some_file.html
[9.37207 million req/sec] for URL: /another_file.jpeg
Checksum: 80000000
```
</details>

<details>
  <summary><strong>Windows 10 Visual Studio 2022</strong></summary>

```shell
[2.45881 million req/sec] for URL: /service/candy/lollipop
[2.4919 million req/sec] for URL: /service/candy/gum
[2.43368 million req/sec] for URL: /service/candy/seg_råtta
[2.3912 million req/sec] for URL: /service/candy/lakrits
[4.39947 million req/sec] for URL: /service/shutdown
[6.00962 million req/sec] for URL: /
[3.0248 million req/sec] for URL: /some_file.html
[2.54388 million req/sec] for URL: /another_file.jpeg
Checksum: 80000000
```

</details>

### Realistic Scenario
A more realistic scenario was mocked up in [performance.cpp](performance/performance.cpp)
and tested via both a single thread and multiple threads. Router is set up with
a couple of hundred routes to simulate a real API, and a few million requests
sent against the router to measure the average performance.

The results of the test are shown below:

<details>
  <summary><strong>Mac OS X Apple clang version 13.1.6 (clang-1316.0.21.2)</strong></summary>

**With std::unordered_map**
```shell
Single thread - [2.42359 million req/sec]
Total urls routed: 260000000 in 107 seconds.

10 threads - [12.3703 million req/sec]
Total urls routed: 260000000 in 21 seconds.
```

**With boost::container::flat_map**
```shell
Single thread - [2.812 million req/sec]
Total urls routed: 260000000 in 92 seconds.

10 threads - [14.638 million req/sec]
Total urls routed: 260000000 in 17 seconds.
```
</details>

<details>
  <summary><strong>Linux GCC 11.2</strong></summary>

**With std::unordered_map**
```shell
Single thread - [4.06155 million req/sec]
Total urls routed: 260000000 in 64 seconds.

10 threads - [18.4738 million req/sec]
Total urls routed: 260000000 in 14 seconds.
```

**With boost::container::flat_map**
```shell
Single thread - [4.84677 million req/sec]
Total urls routed: 260000000 in 53 seconds.

10 threads - [22.0115 million req/sec]
Total urls routed: 260000000 in 11 seconds.
```
</details>

<details>
  <summary><strong>Windows 10 Visual Studio 2022</strong></summary>

```shell
Single thread - [1.54279 million req/sec]
Total urls routed: 260000000 in 168 seconds.

10 threads - [7.24779 million req/sec]
Total urls routed: 260000000 in 35 seconds.
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
of parameter values instead of `std::unordered_map` or `boost::container::flat_map`.

### Performance
Benchmark numbers from [benchmarkfast.cpp](performance/benchmarkfast.cpp) are below:

<details>
  <summary><strong>Mac OS X Apple clang version 13.1.6 (clang-1316.0.21.2)</strong></summary>

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