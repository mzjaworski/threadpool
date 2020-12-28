cmake_minimum_required(VERSION 3.9)
project(threadpool VERSION 1.0.0)

include(GNUInstallDirs)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_subdirectory(libs/function2)
add_subdirectory(libs/concurrentqueue)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(threadpool INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(threadpool INTERFACE Threads::Threads function2 concurrentqueue)

install(FILES include/threadpool/threadpool.hpp LICENSE.md
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})