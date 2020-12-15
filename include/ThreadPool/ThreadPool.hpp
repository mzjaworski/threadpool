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

            // Used when the provided function has a void return type
            template<typename Func, typename... Args,
                    std::enable_if_t<std::is_invocable_v<Func&&, Args&&...>, bool> = true,
                    std::enable_if_t<std::is_same_v<std::invoke_result_t<Func&&, Args&&...>, void>, bool> = true>
            void execute(Func&& func, Args&&... args);

            // Used when the provided function has a non-void return type
            template <typename Func, typename... Args,
                    std::enable_if_t<std::is_invocable_v<Func&&, Args&&...>, bool> = true,
                    std::enable_if_t<std::negation_v<std::is_same<std::invoke_result_t<Func&&, Args&&...>, void>>, bool> = true>
            auto execute(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>;

            size_t getPoolSize();

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


    template<typename Func, typename... Args, std::enable_if_t<std::is_invocable_v<Func&&, Args&&...>, bool>,
            std::enable_if_t<std::is_same_v<std::invoke_result_t<Func&&, Args&&...>, void>, bool>>
    void ThreadPool::execute(Func&& func, Args&&... args){

        //No void return type so no need to use a packaged_task
        auto task = [func = std::forward<Func>(func), ...args = std::forward<Args>(args)]() { func(args...); };

        {
            std::scoped_lock<std::mutex> lock(_mtx);
            _size++;
        }

        _new_task.notify_one();
        _tasks.enqueue([task = std::move(task)]() mutable { task(); });
    }


    template <typename Func, typename... Args, std::enable_if_t<std::is_invocable_v<Func&&, Args&&...>, bool>,
            std::enable_if_t<std::negation_v<std::is_same<std::invoke_result_t<Func&&, Args&&...>, void>>, bool>>
    [[nodiscard("Supplied function has a non-void return type, maybe it's result should not be disregarded?")]]
    auto ThreadPool::execute(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {

        //This function has a non-void return type, therefore we create a packaged_task in order to return a future
        // object to the calling thread
        auto task = std::packaged_task<decltype(func(args...))()>(
                [func = std::forward<Func>(func), ...args = std::forward<Args>(args)] { return func(args...); });
        auto ret =  task.get_future();

        {
            std::scoped_lock<std::mutex> lock(_mtx);
            _size++;
        }

        _new_task.notify_one();
        _tasks.enqueue([task = std::move(task)]() mutable { task(); });

        return ret;
    }

    size_t ThreadPool::getPoolSize(){
        return _pool.size();
    }

}

#endif //THREADPOOL_H