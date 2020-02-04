#include <memory>
#include <vector>
#include <list>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <functional>
using namespace std;

class IThreadPool {
public:
    virtual void enqueue(function<void()> f) = 0;
};

template<typename T>
class Future {
private:
    T val;
    bool isR;

    std::mutex mtx;
    std::condition_variable condVar;
    
    vector<function<void(T)> > continuations;

public:
    Future()
    {
        isR = false;
    }

    void set(T v)
    {
        std::unique_lock<std::mutex> lock(mtx);
        
        val = v ;
        isR = true;
        
        condVar.notify_all();
        
        for(function<void(T)>& cont : continuations) {
            cont(v);
        }
    }

    T get()
    {
            std::unique_lock<std::mutex> lock(mtx);
            
            while(!isR)
            {
                condVar.wait(lock);
            }
            
            return val;
    }
    
    template<typename R>
    shared_ptr<Future<R> > continueWithSync(function<R(T)> func) {
        shared_ptr<Future<R> > ret = make_shared<Future<R> >();
        
        std::unique_lock<std::mutex> lock(mtx);
        if(isR) {
            ret->set(func(val));
        } else {
            continuations.push_back([ret,func](T v){
                ret->set(func(v));
            });
        }
        
        return ret;
    }

    template<typename R>
    shared_ptr<Future<R> > continueWithAsync(function<R(T)> func, IThreadPool* tp) {
        shared_ptr<Future<R> > ret = make_shared<Future<R> >();
        
        std::unique_lock<std::mutex> lock(mtx);
        if(isR) {
            tp->enqueue( [ret, func, this](){
                ret->set(func(val));
            });
        } else {
            continuations.push_back([ret,func,tp](T v){
                tp->enqueue([ret,func,v](){ ret->set(func(v)); });
            });
        }
        
        return ret;
    }

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



int compute1(int x) {
    return x+1;
}

int compute2(int x) {
    return x+2;
}

int main() {
    ThreadPool tp(10);
    
    Future<int> f;
    
    shared_ptr<Future<int> > f1 = f.continueWithAsync<int>(compute1, &tp);
    thread t([&f](){
        int j=0;
        for(int i=0 ; i<100000000 ; ++i) j+=i;
        f.set(j);
    });
    shared_ptr<Future<int> > f2 = f.continueWithAsync<int>(compute2, &tp);
    
    cout<<"Result: " << f1->get() << ", " << f2->get() << "\n";
    t.join();
    
    
    const int n = 12;
    vector<Future<int> > ft(n);
    tp.enqueue([&ft,n](){ft[n-1].set(0);});
    for(int i=0 ; i+1<n ; ++i) {
        tp.enqueue([&ft,i](){ft[i].set(ft[i+1].get() + 1);});
    }
    cout<<"Result 2: " << ft[0].get() << "\n";
}
