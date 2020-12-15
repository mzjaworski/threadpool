#include <iostream>
#include <thread>

#include "../include/ThreadPool/ThreadPool.hpp"

int main() {
    //initialize a thread pool, if no value is given it is initialized using a std::thread::hardware_concurrency
    mz::ThreadPool pool;

    auto future = pool.execute([](){ return 5 + 6; });
    std::cout << (future.get() == 11) << std::endl;

    future = pool.execute([](int x, int y){ return x * y; }, 4, 5);
    std::cout << (future.get() == 20) << std::endl;

    //emits warning since we are disregarding the returned data
    pool.execute([](int x, int y){ return std::string("test"); }, 4, 5);

    // But this is fine since this function does not return anything
    pool.execute([](int x, int y){ x * y; }, 4, 5);

    std::cout << (pool.getPoolSize() == std::thread::hardware_concurrency()) << std::endl;

    return 0;
}