//
// Created by Rakesh on 29/03/2022.
//

#if __GNUC__ > 10 || defined _WIN32
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#include "../src/concat.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

SCENARIO( "String concatenation test suite" )
{
  GIVEN( "A target output string" )
  {
    auto output = "A long string of const char*, string and view"s;

    WHEN( "Concatenating c strings" )
    {
      const char* one = "A ";
      const char* two = "long";
      const char* three = " string";
      const char* four = " of const char*";
      const char* five = ", string and view";
      auto str = spt::util::concat( one, two, three, four, five );
      REQUIRE( str == output );
    }

    AND_WHEN( "Concatenating strings" )
    {
      auto str = spt::util::concat( "A "s, "long"s, " string"s, " of const char*"s, ", string and view"s );
      REQUIRE( str == output );
    }

    AND_WHEN( "Concatenating string views" )
    {
      auto str = spt::util::concat( "A "sv, "long"sv, " string"sv, " of const char*"sv, ", string and view"sv );
      REQUIRE( str == output );
    }

    AND_WHEN( "Concatenating mixed types" )
    {
      const char* three = " string";
      const char* four = " of const char*";
      auto str = spt::util::concat( "A "sv, "long"s, three, four, ", string and view"sv );
      REQUIRE( str == output );
    }
  }
}
