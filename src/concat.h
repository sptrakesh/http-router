//
// Created by Rakesh on 29/03/2022.
//

#pragma once

#include <cstring>
#include <string>

namespace spt::util
{
  namespace impl
  {
    inline std::size_t size( const char* s )
    {
      return std::strlen( s );
    }

    inline std::size_t size( const std::string& s )
    {
      return std::size( s );
    }

    inline std::size_t size( std::string_view s )
    {
      return std::size( s );
    }

    template <typename Head, typename... Tail>
    std::size_t size( const Head& head, Tail const&... tail )
    {
      return size( head ) + size( tail... );
    }

    template <typename Head>
    void append( std::string& out, const Head& head )
    {
      out.append( head );
    }

    template <typename Head, typename... Args>
    void append( std::string& out, const Head& head, Args const&... args )
    {
      out.append( head );
      append( out, args... );
    }
  }

  template <typename... Args>
  std::string concat( Args const&... args )
  {
    std::size_t total = impl::size( args... );
    std::string out;
    out.reserve( total );
    impl::append( out, args... );
    return out;
  }
}