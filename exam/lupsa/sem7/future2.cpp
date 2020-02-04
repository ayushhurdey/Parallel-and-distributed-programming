#include <memory>
#include <functional>
#include <condition_variable>
#include <assert.h>
#include <iostream>
#include <vector>
#include <list>
#include <thread>

class IThreadPool {
public:
    virtual void enqueue(std::function<void()> func) = 0;
};

template<typename T>
class Future {
public:
    T get() {
        wait();
        return value;
    }
    void wait() {
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait(lck, [this](){return isCompleted;});
    }
    
    // func executes on the thread calling this function (if the future is already completed)
    // or on the thread calling set() (if the future is not completed yet)
    template<typename R>
    std::shared_ptr<Future<R> > continueWithOnSameThread(std::function<R(T)> func) {
        std::shared_ptr<Future<R> > ret = std::make_shared<Future<R> >();
        
        {
            std::unique_lock<std::mutex> lck(mtx);
            if (isCompleted) {
                ret->set(func(value));
            } else {
                continuations.push_back([ret,func](T v){
                    ret->set(func(v));
                });
            }
        }
        
        return ret;
    }

    template<typename R>
    std::shared_ptr<Future<R> > continueWith(std::function<R(T)> func, IThreadPool* threadPool) {
        std::shared_ptr<Future<R> > ret = std::make_shared<Future<R> >();
        
        {
            std::unique_lock<std::mutex> lck(mtx);
            if (isCompleted) {
                threadPool->enqueue([ret, func, this](){
                    ret->set(func(value));
                });
            } else {
                continuations.push_back([ret,func, threadPool](T v){
                    threadPool->enqueue([ret, func, v](){
                        ret->set(func(v));
                    });
                });
            }
        }
        
        return ret;
    }
    
    void set(T v) {
        std::unique_lock<std::mutex> lck(mtx);
        assert(!isCompleted);
        isCompleted = true;
        value = v;
        cv.notify_all();
        for(std::function<void(T)>& f : continuations) {
            f(v);
        }
    }
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool isCompleted;
    T value;
    std::vector<std::function<void(T)> > continuations;
};

class ThreadPool : public IThreadPool {
public:
    explicit ThreadPool(size_t nrThreads)
        :m_end(false),
        m_liveThreads(nrThreads)
    {
        m_threads.reserve(nrThreads);
        for(size_t i=0 ; i<nrThreads ; ++i) {
            m_threads.emplace_back([this](){this->run();});
        }
    }
    
    ~ThreadPool() {
        close();
        for(std::thread& t : m_threads) {
            t.join();
        }
    }
    
    void close() {
        std::unique_lock<std::mutex> lck(m_mutex);
        m_end = true;
        m_cond.notify_all();
        while(m_liveThreads > 0) {
            m_condEnd.wait(lck);
        }
    }
    
    void enqueue(std::function<void()> func) {
        std::unique_lock<std::mutex> lck(m_mutex);
        m_queue.push_back(std::move(func));
        m_cond.notify_one();
    }
    
//    template<typename Func, typename... Args>
//    void enqueue(Func func, Args&&... args) {
//        std::function<void()> f = [=](){func(args...);};
//        enqueue(std::move(f));
//    }
private:
    void run() {
        while(true) {
            std::function<void()> toExec;
            {
                std::unique_lock<std::mutex> lck(m_mutex);
                while(m_queue.empty() && !m_end) {
                    m_cond.wait(lck);
                }
                if(m_queue.empty()) {
                    --m_liveThreads;
                    if(0 == m_liveThreads) {
                        m_condEnd.notify_all();
                    }
                    return;
                }
                toExec = std::move(m_queue.front());
                m_queue.pop_front();
            }
            toExec();
        }
    }

    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::condition_variable m_condEnd;
    std::list<std::function<void()> > m_queue;
    bool m_end;
    size_t m_liveThreads;
    std::vector<std::thread> m_threads;
};


int main() {
    ThreadPool tp(10);
    std::shared_ptr<Future<int> > f1 = std::make_shared<Future<int> >();
    std::shared_ptr<Future<int> > f2 = f1->continueWith<int>([](int x){return x+1;}, &tp);
    f1->set(42);
    std::shared_ptr<Future<int> > f3 = f1->continueWith<int>([](int x){return x*2;}, &tp);
    std::shared_ptr<Future<int> > f4 = f3->continueWith<int>([](int x){return x*3;}, &tp);
    
    std::cout<<f1->get()<<", "<<f2->get()<<", "<<f3->get()<<", "<<f4->get()<<"\n";
}
