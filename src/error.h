//
// Created by Rakesh on 18/03/2022.
//

#pragma once

#include <exception>
#include <string>

namespace spt::http::router
{
  struct DuplicateRouteError : std::exception
  {
    DuplicateRouteError( std::string&& msg ) : std::exception(), msg{ std::move( msg ) } {}

    const char* what() const noexcept override { return msg.c_str(); }

  private:
    std::string msg;
  };

  struct InvalidParameterError : std::exception
  {
    InvalidParameterError( std::string&& msg ) : std::exception(), msg{ std::move( msg ) } {}

    const char* what() const noexcept override { return msg.c_str(); }

  private:
    std::string msg;
  };

  struct InvalidWildcardError : std::exception
  {
    InvalidWildcardError( std::string&& msg ) : std::exception(), msg{ std::move( msg ) } {}

    const char* what() const noexcept override { return msg.c_str(); }

  private:
    std::string msg;
  };
}