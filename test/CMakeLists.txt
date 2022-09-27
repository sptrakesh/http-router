Include(FetchContent)

if (APPLE)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.0.0-preview3
)
else()
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.8
)
endif()

enable_testing()
FetchContent_MakeAvailable(Catch2)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_CXX_FLAGS_DEBUG "/MT")
  set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -D_FORTIFY_SOURCE=1")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -D_FORTIFY_SOURCE=1")
endif()

file(GLOB_RECURSE test_SRCS *.cpp *.h)
if (APPLE)
  add_executable(unitTest ${test_SRCS} )
  target_link_libraries(unitTest PRIVATE Catch2::Catch2WithMain)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
  include(CTest)
  include(Catch)
  catch_discover_tests(unitTest)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_executable(unitTest ${test_SRCS} main.cpp)
  if (Boost_FOUND)
    # Ugly.  No idea why MSVC needs to link to these libraries, and also no idea why cmake does not set Boost_LIBRARY_DIRS
    target_link_libraries(unitTest PRIVATE Catch2::Catch2 /usr/local/boost/lib/libboost_json-vc143-mt-s-x64-1_78.lib /usr/local/boost/lib/libboost_container-vc143-mt-s-x64-1_78.lib)
  else()
    target_link_libraries(unitTest PRIVATE Catch2::Catch2)
  endif()
else()
  add_executable(unitTest ${test_SRCS} main.cpp)
  target_link_libraries(unitTest PRIVATE Catch2::Catch2)
endif()