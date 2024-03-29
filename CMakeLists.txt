cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(http-router)

# Enable C++20 features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif()

if (APPLE)
  set(CMAKE_MACOSX_RPATH 1)
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /opt/spt)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

find_package(Boost)
if (Boost_FOUND)
  add_definitions(-DHAS_BOOST)
  message("Boost found, enabling extra features")
  include_directories(
    ${Boost_INCLUDE_DIR}
  )
endif()

add_subdirectory(src)

if (BUILD_TESTING)
  add_subdirectory(performance)
  add_subdirectory(test)
endif()

