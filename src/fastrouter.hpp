//
// Created by Rakesh on 18/03/2022.
//

#pragma once

#include "killvxk/httprouter.h"

namespace spt::http::router
{
  /**
   * Simple path based HTTP request router.  Configured paths are stored in
   * a sorted vector, and request path matching is performed via binary search.
   * @tparam UserData User defined structure with the context necessary for
   *   the handler function.
   * @tparam Response The response from the handler function.
   */
  template<typename UserData, typename Response>
  class FastRouter
  {
  public:
    using Handler = std::function<Response(UserData, std::vector<killvxk::string_view> &&)>;

    FastRouter& add( std::string_view method, std::string_view path, Handler&& handler )
    {
      std::string full;
      full.reserve( 1 + method.size() + path.size() );
      full.append( "/" ).append( method ).append( path );
      router.add( full.c_str(), std::move( handler ) );
      return *this;
    }

    std::optional<Response> route( std::string_view method, std::string_view path,
        UserData userData, bool checkWithoutTrailingSlash = false )
    {
      std::string full;
      full.reserve( 1 + method.size() + path.size() );
      full.append( "/" ).append( method ).append( path );
      auto resp = router.route( full.data(), full.size(), userData );
      if ( !resp && checkWithoutTrailingSlash && path.ends_with( '/' ) )
      {
        auto fv = std::string_view{ full };
        fv = fv.substr( 0, fv.size() - 1 );
        return router.route( fv.data(), fv.size(), userData );
      }

      return resp;
    }

  private:
    killvxk::HttpRouter<UserData, Response> router;
  };
}