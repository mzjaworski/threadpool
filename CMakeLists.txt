#          Copyright Mateusz Jaworski 2020 - 2020.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE.md or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.17)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/function2)

# for convenience
include_directories(libs/function2/include libs/concurrentqueue)

add_executable(ThreadPool example/example.cpp include/ThreadPool/ThreadPool.hpp)
target_link_libraries(ThreadPool pthread function2)