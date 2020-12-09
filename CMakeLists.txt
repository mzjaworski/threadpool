cmake_minimum_required(VERSION 3.17)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/function2)

# for convenience
include_directories(libs/function2/include libs/concurrentqueue)

add_executable(ThreadPool example/main.cpp include/ThreadPool/ThreadPool.hpp)
target_link_libraries(ThreadPool pthread function2)