//
// Created by Rakesh on 29/04/2020.
//

#pragma once

#include <algorithm>
#include <string_view>
#include <vector>

namespace spt::util
{
  inline std::vector<std::string_view> split( std::string_view csv,
      std::size_t sizehint = 8, std::string_view delims = "," )
  {
    std::vector<std::string_view> output;
    output.reserve( sizehint );
    auto first = csv.cbegin();

    while ( first != csv.cend() )
    {
      const auto second = std::find_first_of( first, std::cend( csv ),
          std::cbegin( delims ), std::cend( delims ) );

      if ( first != second )
      {
        output.emplace_back(
            csv.substr( std::distance( csv.begin(), first ),
                std::distance( first, second ) ) );
      }

      if ( second == csv.cend() ) break;
      first = std::next( second );
    }

    return output;
  }
}
