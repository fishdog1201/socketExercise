#ifndef THREADPOOL
#define THREADPOOL

#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <map>
#include <future>
using namespace std;


class ThreadPool
{
public:
    ThreadPool(int min = 4, int max = thread::hardware_concurrency());
    ~ThreadPool();
    void addTask(function<void()> f);
    
    template<typename F, typename... Args>
    auto addTask(F&& f, Args&&... args) -> future<typename result_of<F(Args...)>::type>
    {
        using returnType = typename result_of<F(Args...)>::type;
        auto task = make_shared<packaged_task<returnType()>>(
            bind(forward<F>(f), forward<Args>(args)...)
        );
        future<returnType> res = task->get_future();
        {
            unique_lock<mutex> lock(m_queueMutex);
            m_tasks.emplace([task]() { (*task)(); });
        }
        m_condition.notify_one();
        return res;
    }

private:
    void manager();
    void worker();
private:
    thread* m_manager;
    map<thread::id, thread> m_workers;
    vector<thread::id> m_ids;
    int m_minThreads;
    int m_maxThreads;
    atomic<bool> m_stop;
    atomic<int> m_curThreads;
    atomic<int> m_idleThreads;
    atomic<int> m_exitNumber;
    queue<function<void()>> m_tasks;
    mutex m_idsMutex;
    mutex m_queueMutex;
    condition_variable m_condition;
};

class ThreadPoolSingleton {
public:
    // 获取单例实例
    static ThreadPoolSingleton& getInstance(size_t num_threads = std::thread::hardware_concurrency()) {
        static ThreadPoolSingleton instance(num_threads);
        return instance;
    }

    // 禁止拷贝构造和赋值
    ThreadPoolSingleton(const ThreadPoolSingleton&) = delete;
    ThreadPoolSingleton& operator=(const ThreadPoolSingleton&) = delete;

    template<typename Func, typename... Args>
    auto enqueue(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
        using return_type = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (stop_) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return result;
    }

    ~ThreadPoolSingleton() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread& worker : workers_) {
            worker.join();
        }
    }

private:
    // 构造函数
    ThreadPoolSingleton(size_t num_threads)
        : stop_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back(
                [this] {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(mutex_);
                            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                            if (stop_ && tasks_.empty()) {
                                return;
                            }
                            task = std::move(tasks_.front());
                            tasks_.pop();
                        }
                        task();
                    }
                }
            );
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    std::mutex mutex_;
    std::condition_variable condition_;
    bool stop_;
};

#endif