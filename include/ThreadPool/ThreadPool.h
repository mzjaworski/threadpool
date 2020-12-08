#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>

namespace mz{

    class ThreadPool{
        public:
            ThreadPool(unsigned int threads = std::thread::hardware_concurrency());
            ~ThreadPool();

            template <typename Func, typename... Args>
            auto execute(Func func, Args... args);

            ThreadPool& operator=(ThreadPool&) = delete;
            ThreadPool(ThreadPool&) = delete;

        private:

    };

    ThreadPool::ThreadPool(unsigned int threads = std::thread::hardware_concurrency()){}
    ThreadPool::~ThreadPool(){}

    template <typename Func, typename... Args>
    auto ThreadPool::execute(Func func, Args... args){}
}



#endif //THREADPOOL_H
