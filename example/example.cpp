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

    std::cout << (pool.getPoolSize() == std::thread::hardware_concurrency()) << std::endl;

    return 0;
}
