Include(FetchContent)

if(APPLE)
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
  set(CMAKE_CXX_FLAGS_DEBUG "/MT /O2")
  set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

file(GLOB_RECURSE test_SRCS *.cpp *.h)
if(APPLE)
  add_executable(unitTest ${test_SRCS} )
  target_link_libraries(unitTest PRIVATE Catch2::Catch2WithMain)
else()
  add_executable(unitTest ${test_SRCS} main.cpp)
  target_link_libraries(unitTest PRIVATE Catch2::Catch2)
endif()

if(APPLE)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
  include(CTest)
  include(Catch)
  catch_discover_tests(unitTest)
endif()