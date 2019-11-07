
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <queue>

class ThreadPool {
public:
    using Task = std::function<void()>;

    // Specifies that a constructor or conversion function (since C++11)
    // is explicit, that is, it cannot be used for implicit conversions
    // and copy-initialization.
    explicit ThreadPool(std::size_t numThreads) {
        start(numThreads);
    }

    ~ThreadPool() {
        stop();
    }

    template<class T>
    auto enqueue(T task) -> std::future<decltype(task())> {
        // Container for an abstract functor.
        auto wrapper = std::make_shared<std::packaged_task<decltype(task()) ()>>(std::move(task));

        // Scope for the mutex.
        {
            std::unique_lock<std::mutex> lock {mEventMutex};
            mTasks.emplace([=] {
                // Call through the shared pointer.
                (*wrapper)();
            });
        }
        mEventVar.notify_all();

        // The future contained by the packaged task will contain the result of the task.
        return wrapper->get_future();
    }
private:
    std::vector<std::thread> mThreads;
    std::condition_variable mEventVar;
    std::mutex mEventMutex;
    bool mStopping = false;
    std::queue<Task> mTasks;

    void start(std::size_t numThreads) {
        for(int i = 0; i < numThreads; i++) {
            mThreads.emplace_back([=]{
                while(true) {
                    Task task;

                    // Critical section scope:
                    // We don't want to keep the mutex lock while the task is executing
                    // because the task might take, well.. long enough, and the mutex might be
                    // locked for a long period of time.
                    {
                        std::unique_lock<std::mutex> lock{mEventMutex};

                        // We need this guard (the predicate) because a thread might wake up spontaneously.
                        mEventVar.wait(lock, [=] { return mStopping || !mTasks.empty(); });

                        // Only if the tasks are also complete.
                        if (mStopping && mTasks.empty()) {
                            break;
                        }

                        // If we are not stopping. Take the first task from the queue.
                        task = std::move(mTasks.front());

                        // Then pop it out.
                        mTasks.pop();
                    }
                    task();
                }
            });
        }
    }

    // Noexcept returns true if an expression is declared to not throw
    // any exceptions. The result is true if the set of potential exceptions
    // of the expression is empty.
    void stop() noexcept {
        {
            std::unique_lock<std::mutex> lock{mEventMutex};
            mStopping = true;
        }
        mEventVar.notify_all();
        for(auto &thread : mThreads) {
            thread.join();
        }
    }
};

class Matrix {
private:
    int rows = 0;
    int cols = 0;
    std::vector<std::vector<int>> grid;
    bool validity_of_last_operation = false;

public:
    Matrix(int n, int m, bool rand) {
        rows = n;
        cols = m;
        grid = std::vector<std::vector<int>>{static_cast<size_t>(n)};
        if(rand) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    int value = std::rand() % 10 + 1;
                    grid[i].push_back(value);
                }
            }
        }
    }

    std::vector<std::vector<int>> get_grid() {
        return grid;
    }

    void push_grid(int i, int j, int value) {
        this->grid[i].push_back(value);
    }

    void set_grid(int i, int j, int value) {
        this->grid[i][j] = value;
    }

    void set_validity(bool status) {
        this->validity_of_last_operation = status;
    }

    int get_rows() {
        return this->rows;
    }

    int get_cols() {
        return this->cols;
    }
};

void worker_thread(size_t i,
        std::vector<std::mutex*>* m,
        std::vector<std::condition_variable*>* cv,
        std::vector<bool>* ready_to_go,
        std::vector<bool>* processed,
        Matrix* final_result,
        Matrix* partial_result,
        Matrix* Q) {
    // Wait until main() sends data
    std::unique_lock<std::mutex> lk(*((*m)[i]));
    (*cv)[i]->wait(lk, [i, ready_to_go] { return (*ready_to_go)[i]; });

    // Line i from the partial result is ready.
    int sum;
    for (size_t k = 0; k < Q->get_cols(); k++) {
        sum = 0;
        for (size_t j = 0; j < partial_result->get_grid().size(); j++) {
            sum += partial_result->get_grid()[i][j] * Q->get_grid()[j][k];
        }
        final_result->push_grid(i, k, sum);
    }

    // Send data back to product()
    (*processed)[i] = true;

    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again
    lk.unlock();
    (*cv)[i]->notify_one();
}

Matrix* product(Matrix* M, Matrix* N, Matrix* Q) {
    size_t M_row_size = M->get_rows(),
           M_col_size = M->get_cols(),
           N_row_size = N->get_rows(),
           N_col_size = N->get_cols(),
           Q_col_size = Q->get_cols();

    std::vector<std::mutex*> mxs;
    std::vector<std::condition_variable*> cvs;
    std::vector<bool> ready_status;
    std::vector<bool> processed;
    std::vector<std::thread*> boss_threads, workers;

    ThreadPool boss_tp{20}, worker_tp{20} ;

    for(int i = 0; i < M_row_size; i++) {
        mxs.push_back(new std::mutex);
        cvs.push_back(new std::condition_variable);
        ready_status.push_back(false);
        processed.push_back(false);
    }

    if(M_col_size != N_row_size) {
        return nullptr;
    }

    auto partial_result = new Matrix(M_row_size, N_col_size, false);
    auto result = new Matrix(partial_result->get_grid().size(), Q_col_size, false);
    for(size_t i = 0; i < M_row_size; i++) {

        // Start a worker thread waiting for 'ready to go' status.
        // workers.push_back(new std::thread(worker_thread, i, &mxs, &cvs, &ready_status, &processed, result, partial_result, Q));
        worker_tp.enqueue([M, N, Q, i, N_col_size, M_row_size, partial_result, result, &ready_status, &cvs, &mxs, &processed] {
            worker_thread(i, &mxs, &cvs, &ready_status, &processed, result, partial_result, Q);
        });

        // isolate this block on a thread
        // boss_threads.push_back(new std::thread([M, N, i, N_col_size, M_row_size, partial_result, &ready_status, &cvs, &mxs, &processed] {
        boss_tp.enqueue([M, N, i, N_col_size, M_row_size, partial_result, &ready_status, &cvs, &mxs, &processed] {
            int sum;

            for (size_t k = 0; k < N_col_size; k++) {
                sum = 0;
                for (size_t j = 0; j < N_col_size; j++) {
                    sum += M->get_grid()[i][j] * N->get_grid()[j][k];
                }
                partial_result->push_grid(i, k, sum);
            }

            ready_status[i] = true;
            cvs[i]->notify_all();

            // When thread is done we wake up the worker.
            // }));
        });
    }

    // Instead of join I can have the processed vector of cvs waiting
    // for a sign from the worker threads, but I guess there is no
    // need to react as soon as the worker ends while I have to wait
    // all of them.
    for(std::thread* w : workers) {
        w->join();
    }

    for(std::thread* t : boss_threads) {
        t->join();
    }

    return result;
}

void display(Matrix* M) {
    int rows = M->get_rows() < M->get_grid().size() ? M->get_rows() : M->get_grid().size();
    int cols = M->get_cols() < M->get_grid()[0].size() ? M->get_cols() : M->get_grid()[0].size();

    if(rows != 0 and cols != 0) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                std::cout << M->get_grid()[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    } else {
        std::cout << "Empty matrix." << std::endl;
    }
}

int main() {
    std::srand(std::time(nullptr));
    Matrix A(10, 10, true),
           B(10, 10, true),
           C(10, 10, true);
    display(&A);
    display(&B);
    display(&C);
    Matrix* m = product(&A, &B, &C);
    display(m);
    return 0;
}
