Include(FetchContent)

if(UNIX AND NOT APPLE)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.8
)
else()
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.0.0-preview3
)
endif()

FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE test_SRCS *.cpp *.h)
if(UNIX AND NOT APPLE)
  add_executable(unitTest ${test_SRCS} main.cpp)
  target_link_libraries(unitTest PRIVATE Catch2::Catch2)
else()
  add_executable(unitTest ${test_SRCS} )
  target_link_libraries(unitTest PRIVATE Catch2::Catch2WithMain)
endif()

if(APPLE)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
  include(CTest)
  include(Catch)
  catch_discover_tests(unitTest)
endif()