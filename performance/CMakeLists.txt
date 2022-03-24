set(CMAKE_CXX_FLAGS_DEBUG "-g -O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(benchmark benchmark.cpp)
add_executable(benchmarkfast benchmarkfast.cpp)
add_executable(performance performance.cpp)
