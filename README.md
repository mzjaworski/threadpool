A simple ThreadPool written in C++17/20.

# Installation

```sh
# add this project as a submodule:
git submodule add <url>

# download its dependencies
cd threadpool
git submodule init
git submodule update
```

```cmake
# add the following to your CMakeLists:
add_subdirectory(threadpool)
target_link_libraries(my_project threadpool)
```

# Usage


Include it in your project:
```c++
#include "threadpool/threadpool.hpp"
```

Initialize the thread pool:
```c++
// initialized with std::thread::hardware_concurrency threads
mz::ThreadPool pool;
```

```c++
// initialized with 10 threads
mz::ThreadPool pool(10); 
```

Retrieve the number of threads:
```c++
std::cout << pool.getPoolSize() << std::endl;
```

Pass in tasks to execute:
```c++
// function used in a demo bellow
int addNumbers(int x, int y){ return x +y; }

.
.
.
.

// take in lambda:
// future's type is deduced based on the callable object's return type
std::future<int> future = pool.execute([](){ return 5 + 6; }); 
std::cout << future.get() << std::endl;

future = pool.execute([](int x, int y){ return x * y; }, 4, 5);
std::cout << future.get() << std::endl;


// lambda with a capture
int a = 5; int b = 6;
auto ret = pool.execute([&](){ return a + b; });
std::cout << ret.get() << std::endl;


// templated lambda
std::future<void> ret = pool.execute(
        []<typename ...Args>(Args&& ...args){ (std::cout << ... << args) << std::endl; },
        4, std::string(" it just works "), 56.7);
std::cout << ret.get() << std::endl;


// ordinary function
future = pool.execute(addNumbers, 4, 5);
std::cout << future.get() << std::endl;


// std::function
future = pool.execute(std::function<int(int, int)>(addNumbers), 4, 5);
std::cout << future.get() << std::endl;
```

Examples of improper use and compiler warnings:
```c++
// execute emits a warning since we are disregarding the future object holding the (possible) returned data or exception
pool.execute([](int x, int y){ return std::string("test"); }, 4, 5);


// same applies in this scenario even though it's return type is void. In this case the std::future<void> is returned
// in order to verify whenever an exception was thrown inside of the callable object
pool.execute([](int x, int y){ std::string("test"); }, 4, 5);


// this does not emit a warning since the passed callable object has a void return type and is declared noexcept,
// in this case the execute method has a void return type instead of std::future<void> 
pool.execute([](int x, int y) noexcept { x * y; }, 4, 5);
```

# Methods

mz::threadpool has three main methods:

```c++
// this overload is chosen whenever the callable object is explicitly declared as noexcept and has a void return type,
// in this case no internal exception handling is used which on one hand makes it faster but also more error prone 
// if you dont know what you are doing
- auto execute(Func&& func, Args&&... args) -> void

// this overload is chosen whenever the first one fails
- auto execute(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>;

// returns number of threads inside of a thread pool 
- auto getPoolSize() -> size_t
```

# Dependencies

This project introduces two dependencies:

- cameron314/concurrentqueue 
- Naios/function2

The concurrentqueue is used internally for thread safe enqueueing and dequeueing.

While function2 introduces fu2::unique_function, it allows us to holds move-only types whereas std::function requires 
the target to be CopyConstructible. This in turn allows this threadpool to omit lots of possibly costly copy operations.
