#include <functional>
#include <vector>
#include <thread>

class ThreadPool {
  public:
    // Specifies that a constructor or conversion function (since C++11) 
    // is explicit, that is, it cannot be used for implicit conversions 
    // and copy-initialization.
    explicit ThreadPool(std::size_t numThreads) {
      start(numThreads);
    }
    
    ~ThreadPool() {
      stop();
    }
  private:
    void start(std::size_t numThreads) {
  
    }
    
    // Noexcept returns true if an expression is declared to not throw 
    // any exceptions. The result is true if the set of potential exceptions 
    // of the expression is empty.
    void stop() noexcept {
      
    }
};

int main() {

  return 0;
}
