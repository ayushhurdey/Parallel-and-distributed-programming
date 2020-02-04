#include <memory>
#include <vector>
#include <list>
#include <iostream>
#include <functional>
#include <thread>
#include <condition_variable>
using namespace std;


class IThreadCreator{
public:
    virtual void enqueue(function<void()> func) = 0;
};


class ThreadPool : public IThreadCreator {
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


template<typename T>
class Future {
public:
    Future()
        :isSet(false)
    {
        
    }
    void set(T v) {
        unique_lock<mutex> ul(m);
        this->var = v;
        isSet = true;
        cv.notify_all();
        for(auto task : tasks) {
                task(this->var);
        }
    }
    T get() {
        wait();
        return var;
    }
    void wait() {
        unique_lock<mutex> ul(m);
        while (!isSet) {   
            cv.wait(ul);
        }
    }
    
    template<typename R>
    shared_ptr<Future<R>> continueWithSync(function<R(T)> func) {
        shared_ptr<Future<R>> ret = make_shared<Future<R>>();
        auto task = [ret,func](T t){ret->set(func(t));};
        unique_lock<mutex> ul(m);
        if(isSet){
                task(this->var);
        } 
        else {
            tasks.push_back(task);
        }
        return ret;
    }
    
    template<typename R>
    shared_ptr<Future<R>> continueWithAsync(function<R(T)> func, IThreadCreator& thread_pool) {
        shared_ptr<Future<R>> ret = make_shared<Future<R>>();
        auto task = [ret,func,this](){ret->set(func(this->var));};
        unique_lock<mutex> ul(m);
        if(isSet){
            thread_pool.enqueue(task);
        } 
        else {
            tasks.push_back([&thread_pool,task](T){thread_pool.enqueue(task);});
        }
        return ret;
    }
    
private:
    T var;
    bool isSet;
    condition_variable cv;
    mutex m;
    vector<function<void(T)>> tasks;
};

int main() {
    Future<int> f;
    
    thread t([&f](){f.set(10);});
    
    ThreadPool thread_pool(2);

    shared_ptr<Future<int>> f1 = f.continueWithAsync<int>([](int x) {return x+1;},thread_pool);
    shared_ptr<Future<int>> f2 = f.continueWithAsync<int>([](int x) {return x*2;},thread_pool);
    shared_ptr<Future<double>> f3 = f2->continueWithAsync<double>([](int x) {return x+10.1;},thread_pool);
    cout << "Result: " << f.get() << ", " << f1->get() << ", " << f2->get() << ", " << f3->get() << "\n";
    
    t.join();
}
