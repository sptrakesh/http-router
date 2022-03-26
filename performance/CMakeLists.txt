set(CMAKE_CXX_FLAGS_DEBUG "-g -O3 -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

add_executable(benchmark benchmark.cpp)
add_executable(benchmarkfast benchmarkfast.cpp)
add_executable(performance performance.cpp)
