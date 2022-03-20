Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.0.0-preview3
)

FetchContent_MakeAvailable(Catch2)

file( GLOB_RECURSE test_SRCS *.cpp *.h )
add_executable(unitTest ${test_SRCS} )

if(Boost_FOUND)
  target_link_libraries(unitTest PRIVATE Catch2::Catch2WithMain Boost::boost)
else()
  target_link_libraries(unitTest PRIVATE Catch2::Catch2WithMain)
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(unitTest)