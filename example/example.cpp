//          Copyright Mateusz Jaworski 2020 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <functional>
#include <iostream>
#include <thread>

#include "../include/ThreadPool/ThreadPool.hpp"

int addNumbers(int x, int y) { return x + y; }

int main() {
    // initialize a thread pool, if no value is given it is initialized using a std::thread::hardware_concurrency
    mz::ThreadPool pool;

    // lets verify that this is true
    std::cout << (pool.getPoolSize() == std::thread::hardware_concurrency()) << std::endl;

    // execute method is capable of receiving a callable objects of any arbitrary type
    {
        // take in lambda
        std::future<int> future = pool.execute([](){ return 5 + 6; });
        std::cout << (future.get() == 11) << std::endl;

        future = pool.execute([](int x, int y){ return x * y; }, 4, 5);
        std::cout << (future.get() == 20) << std::endl;

        // ordinary function
        future = pool.execute(addNumbers, 4, 5);
        std::cout << (future.get() == 9) << std::endl;

        // std::function
        future = pool.execute(std::function<int(int, int)>(addNumbers), 4, 5);
        std::cout << (future.get() == 9) << std::endl;
    }

    // execute emits a warning since we are disregarding the future object holding the (possible) returned data or exception
    pool.execute([](int x, int y){ return std::string("test"); }, 4, 5);

    // same applies in this scenario even though it's return type is void. In this case the std::future<void> is returned
    // so we can verify whenever an exception was thrown inside of the callable object
    pool.execute([](int x, int y){ std::string("test"); }, 4, 5);

    // If the passed function has a void return type and is declared noexcept execute method does not return anything
    // moreover internally no exception handling facilities are used (such as std::packaged_task)
    pool.execute([](int x, int y) noexcept { x * y; }, 4, 5);

    return 0;
}
