#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>  // std::packaged_task and std::future

class ThreadPool {
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    bool m_stop;
public:
    ThreadPool(size_t threads);
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(const ThreadPool&&) noexcept = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) noexcept = delete;
    // need a custom destructor to join them. 
    // since threads can't be trivially copied, needs custom copy and move operations (Rule of 5)
    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) {
        auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            if (m_stop) throw std::runtime_error("enqueue on stopped ThreadPool");
            m_tasks.emplace([task]() { (*task)(); });
        }
        m_condition.notify_one();
        return task->get_future();
    }

    static ThreadPool& GetInstance();
};
