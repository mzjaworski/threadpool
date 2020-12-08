cmake_minimum_required(VERSION 3.17)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 20)

add_executable(ThreadPool example/main.cpp include/ThreadPool/ThreadPool.h)