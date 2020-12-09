#include <iostream>

#include "../include/ThreadPool/ThreadPool.hpp"

int main() {
    //initialize a thread pool, on default
    mz::ThreadPool pool;

    auto future = pool.execute([](){ return 5 + 6; });
    std::cout << future.get() << std::endl;



    return 0;
}
