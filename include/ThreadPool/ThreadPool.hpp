//          Copyright Mateusz Jaworski 2020 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <future>
#include <thread>
#include <vector>
#include <type_traits>

#include "function2/function2.hpp"
#include "concurrentqueue.h"

namespace mz{

    class ThreadPool{
        public:
            ThreadPool(unsigned int threads = std::thread::hardware_concurrency());
            ~ThreadPool();

            // Used when the provided function has a void return type and is noexcept
            template<typename Func, typename... Args,
                    std::enable_if_t<std::is_nothrow_invocable_v<Func&&, Args&&...>, bool> = true,
                    std::enable_if_t<std::is_same_v<std::invoke_result_t<Func&&, Args&&...>, void>, bool> = true>
            auto execute(Func&& func, Args&&... args) -> void;

            // Used when the provided function can throw exceptions or has a non-void return type
            template<typename Func, typename... Args,
                    std::enable_if_t<std::is_invocable_v<Func&&, Args&&...>, bool> = true,
                    std::enable_if_t<std::negation_v<std::conjunction<std::is_nothrow_invocable<Func&&, Args&&...>,
                    std::is_same<std::invoke_result_t<Func&&, Args&&...>, void>>>, bool> = true>
            auto execute(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>;

            auto getPoolSize() -> size_t;

            ThreadPool& operator=(ThreadPool&) = delete;
            ThreadPool(ThreadPool&) = delete;

        private:
            moodycamel::ConcurrentQueue<fu2::unique_function<void()>> _tasks;
            std::atomic<unsigned int> _size;

            std::vector<std::thread> _pool;

            std::condition_variable _new_task;
            std::mutex _mtx;
            bool _exit;
    };


    ThreadPool::ThreadPool(unsigned int threads): _size(0), _exit(false){
        _pool.reserve(threads);

        for(unsigned int i = 0; i < threads; i++){
            _pool.emplace_back([this]{
                fu2::unique_function<void()> task;

                for (;;){
                    {
                        std::unique_lock<std::mutex> lock(_mtx);
                        _new_task.wait(lock, [this]{ return _size || _exit; });
                    }

                    if (_exit && !_size)
                        return;

                    if (_tasks.try_dequeue(task)){
                        _size--;
                        task();
                    }
                }

            });
        }
    }

    ThreadPool::~ThreadPool(){

        {
            std::scoped_lock<std::mutex> lock(_mtx);
            _exit = true;
        }

        _new_task.notify_all();
        for (auto& thread: _pool)
            thread.join();
    }

    template<typename Func, typename... Args, std::enable_if_t<std::is_nothrow_invocable_v<Func&&, Args&&...>, bool>,
            std::enable_if_t<std::is_same_v<std::invoke_result_t<Func&&, Args&&...>, void>, bool>>
    auto ThreadPool::execute(Func&& func, Args&&... args) -> void {

        //No return type and function won't throw so no need to use a packaged_task
        auto task = [func = std::forward<Func>(func), ...args = std::forward<Args>(args)]() mutable
                { func(std::forward<Args>(args)...); };

        {
            std::scoped_lock<std::mutex> lock(_mtx);
            _size++;
        }

        _new_task.notify_one();
        _tasks.enqueue([task = std::move(task)]() mutable { task(); });
    }

    template<typename Func, typename... Args, std::enable_if_t<std::is_invocable_v<Func&&, Args&&...>, bool>,
            std::enable_if_t<std::negation_v<std::conjunction<std::is_nothrow_invocable<Func&&, Args&&...>,
            std::is_same<std::invoke_result_t<Func&&, Args&&...>, void>>>, bool>>
    [[nodiscard]] auto ThreadPool::execute(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {

        //This function has a non-void return type and can possibly throw, therefore we create a packaged_task
        // in the order to return a future object to the calling thread
        auto task = std::packaged_task<decltype(func(args...))()>(
                [func = std::forward<Func>(func), ...args = std::forward<Args>(args)]() mutable
                { return func(std::forward<Args>(args)...); });
        auto ret =  task.get_future();

        {
            std::scoped_lock<std::mutex> lock(_mtx);
            _size++;
        }

        _new_task.notify_one();
        _tasks.enqueue([task = std::move(task)]() mutable { task(); });

        return ret;
    }

    auto ThreadPool::getPoolSize() -> size_t {
        return _pool.size();
    }

}

#endif //THREADPOOL_H
