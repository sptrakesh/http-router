file(GLOB_RECURSE router_SRCS *.cpp *.h)
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src
  DESTINATION include/router
  FILES_MATCHING PATTERN "*.h*")
