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

            template <typename Func, typename... Args,
                    std::enable_if_t<std::is_invocable_v<Func&&, Args&&...>, bool> = true>
            auto execute(Func&& func, Args&&... args);

            size_t getPoolSize();

            ThreadPool& operator=(ThreadPool&) = delete;
            ThreadPool(ThreadPool&) = delete;

        private:
            moodycamel::ConcurrentQueue<fu2::unique_function<void()>> _tasks;
            std::atomic<unsigned int> _size;

            std::vector<std::thread> _pool;

    };

    ThreadPool::ThreadPool(unsigned int threads): _size(0){
        _pool.reserve(threads);

        for(unsigned int i = 0; i < threads; i++){
            _pool.emplace_back([](){});
        }
    }

    ThreadPool::~ThreadPool(){

        for (auto& thread: _pool)
            thread.join();
    }

    template <typename Func, typename... Args, std::enable_if_t<std::is_invocable_v<Func&&, Args&&...>, bool>>
    auto ThreadPool::execute(Func&& func, Args&&... args){

        auto task = std::packaged_task<decltype(func(args...))()>(
                [func = std::forward<Func>(func), ...args = std::forward<Args>(args)] { return func(args...); });
        auto ret =  task.get_future();

        _tasks.enqueue([task = std::move(task)]() mutable { task(); });
        _size++;

        return ret;
    }

    size_t ThreadPool::getPoolSize(){
        return _pool.size();
    }

}

#endif //THREADPOOL_H
