#include <iostream>
#include <vector>
#include <thread>
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
        // container for an abstract functor
        auto wrapper = std::make_shared<std::packaged_task<decltype(task()) ()>>(std::move(task));

        // scope for the mutex
        {
            std::unique_lock<std::mutex> lock {mEventMutex};
            mTasks.emplace([=] {
                // call through the shared pointer
                (*wrapper)();
            });
        }
        mEventVar.notify_all();

        // the future contained by the packaged task will contain the result of the task
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
                        mEventVar.wait(lock, [=] { return mStopping || !mTasks.empty(); }); // if I remove !mTasks.empty(); it ends immediately [AND]

                        // only if the tasks are also complete
                        if (mStopping && mTasks.empty()) {       // [AND] if I remove mTasks.empty() it ends immediately
//                            std::cout << "break" << std::endl;
                            break;
                        }

                        // If we are not stopping.
                        // Take the first task from the queue.
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
        // scope for the mutex
        {
            std::unique_lock<std::mutex> lock{mEventMutex};
            mStopping = true;
        }
        mEventVar.notify_all();
        std::cout << "joining treads.." << std::endl;
        for(auto &thread : mThreads) {
            thread.join();
        }
        std::cout << "stopped." << std::endl;
    }
};

// Display.
void display_matrix(const std::vector<std::vector<int>>& matrix) {
    for(const std::vector<int>& v : matrix) {
        for(int i : v) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// Row sum.
void row_sum(std::vector<int>* row1, std::vector<int>* row2, int thread) {
    int iRow = 0;
    for(int value : *row1) {
        (*row1)[iRow] = value + (*row2)[iRow];
        iRow++;
    }
}

void row_matrices_sum(std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size();
    std::vector<std::future<void>> future;

    // start time for our study case
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    for(unsigned long iRow = 0; iRow < n; iRow++) {
        future.push_back(std::async(row_sum, &(*matrix1)[iRow], &(*matrix2)[iRow], iRow));
    }
    for(auto & f : future) {
        f.wait();
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "\n n threads computing the sum on each row took: "
              << duration
              << "us, "
              << duration / 1000
              << "ms, "
              << duration / 1000000
              << "s.\n";
}
// ~Row sum.

// Element by element sum.
void elem_sum(int* elem1, const int* elem2, int thread) {
    *elem1 += *elem2;
}

void elem_matrices_sum(std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size(), iThread = 0;
    std::vector<std::future<void>> future;

    // start time for our study case
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    for(unsigned long iRow = 0; iRow < n; iRow++) {
        for(unsigned long iCol = 0; iCol < n; iCol++) {
            future.push_back(std::async(elem_sum, &(*matrix1)[iRow][iCol], &(*matrix2)[iRow][iCol], iThread++));
        }
    }
    for(auto & f : future) {
        f.wait();
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "\n n x m threads computing the sum on each row took: "
              << duration
              << "us, "
              << duration / 1000
              << "ms, "
              << duration / 1000000
              << "s.\n";
}
// ~Element by element sum.

// Multiply one row with the other matrix algorithm.
int row_prod(std::vector<int>* result, std::vector<int>* row, std::vector<std::vector<int>>* matrix, int thread) {
    int n = (*matrix).size(), m = (*matrix)[0].size(), sum;
    for(unsigned int jCol = 0; jCol < m; jCol++) {
        sum = 0;
        for (unsigned int jRow = 0; jRow < n; jRow++) {
            sum += (*row)[jRow] * (*matrix)[jRow][jCol];
        }
        result->push_back(sum);
    }
    return thread;
}

std::vector<std::vector<int>> row_col_matrices_product(std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size();
    std::vector<std::vector<int>> result(n);
    std::vector<std::future<int>> future;

    // start time for our study case
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    for(unsigned int iRow = 0; iRow < n; iRow++) {
        future.push_back(std::async (row_prod, &result[iRow], &(*matrix1)[iRow], matrix2, iRow));
    }
    for(auto & f : future) {
        int row = f.get();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "\nrow "
                  << row
                  << " computer after: "
                  << duration
                  << "us, "
                  << duration / 1000
                  << "ms, "
                  << duration / 1000000
                  << "s.\n";
    }
    return result;
}
// ~Multiply one row with the other matrix algorithm.

// Classic algorithm matrix product.
void matrix_product(std::vector<std::vector<int>>* result, std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size(), m = (*matrix2)[0].size(), sum;
    for(unsigned int iRow = 0; iRow < n; iRow++) {
        for(unsigned int jCol = 0; jCol < m; jCol++) {
            sum = 0;
            for(unsigned int jRow = 0; jRow < n; jRow++) {
                sum += (*matrix1)[iRow][jRow] * (*matrix2)[jRow][jCol];
            }
            (*result)[iRow].push_back(sum);
        }
    }
}

std::vector<std::vector<int>> matrices_product(std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size();
    std::vector<std::vector<int>> result(n);

    // start time for our study case
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    // instead of creating a thread, we'll have an async call returning a future
    std::future<void> fut = std::async (matrix_product, &result, matrix1, matrix2);

    std::cout << "Right after the async call the value of the result matrix is: " << std::endl;
    if(result.empty()) {
        std::cout << "Non empty result." << std::endl;
    } else {
        std::cout << "Empty result. For our study, it means that the we have a `future` after which we are waiting to finish." << std::endl;
    }

    // wait for the promise to end
    fut.wait();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "\n1 thread running the clasic matrix product took: "
              << duration
              << "us, "
              << duration / 1000
              << "ms, "
              << duration / 1000000
              << "s.\n";
    std::cout << "The results regarding matrix multiplication were already proved to be valid in week 3 assign. " << std::endl;

    return result;
}
// ~ Classic algorithm matrix product.

void target1() {
    // Due to the circumstances, for our purposes a matrix of 1000 x 1000 is enough to analyse
    // how future, async and thread pools are working.
    std::vector<std::vector<int>> big_query;
    for(int i=0; i<1000; i++) {
        std::vector<int> row;
        row.reserve(1000);
        for(int j=0; j<1000; j++) {
            row.push_back(j);
        }
        big_query.push_back(row);
    }

    // 1. Analyse the behaviour of future right after async calling the matrix_product function
    // and after waiting.
    std::cout << "MATRICES PRODUCT: \n(classic algorithm)" << std::endl;
    matrices_product(&big_query, &big_query);

    // 2. Then we are analysing how future promises complete as the time passes. Each promise that
    // ends will be notified on the screen with its timestamp and row position. We will be able
    // to see how much time each row took to be computed.
    std::cout << "\n(one row from left M multiplied to the entire matrix N)" << std::endl;
    row_col_matrices_product(&big_query, &big_query);

    // 3. Compare time spent to compute the sum element by element vs row by row.
    std::cout << "\n(sum element by element)" << std::endl;
    elem_matrices_sum(&big_query, &big_query);

    std::cout << "\n(sum row by row)" << std::endl;
    row_matrices_sum(&big_query, &big_query);
}

// TO DO if nec.
// Row sum.
void tp_row_matrices_sum(std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size();
    std::vector<std::future<void>> future;

    // start time for our study case
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    for(unsigned long iRow = 0; iRow < n; iRow++) {
        future.push_back(std::async(row_sum, &(*matrix1)[iRow], &(*matrix2)[iRow], iRow));
    }
    for(auto & f : future) {
        f.wait();
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "\n n threads computing the sum on each row took: "
              << duration
              << "us, "
              << duration / 1000
              << "ms, "
              << duration / 1000000
              << "s.\n";
}
// ~Row sum.

// Element by element sum.
void tp_elem_matrices_sum(std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size(), iThread = 0;
    std::vector<std::future<void>> future;

    // start time for our study case
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    for(unsigned long iRow = 0; iRow < n; iRow++) {
        for(unsigned long iCol = 0; iCol < n; iCol++) {
            future.push_back(std::async(elem_sum, &(*matrix1)[iRow][iCol], &(*matrix2)[iRow][iCol], iThread++));
        }
    }
    for(auto & f : future) {
        f.wait();
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "\n n x m threads computing the sum on each row took: "
              << duration
              << "us, "
              << duration / 1000
              << "ms, "
              << duration / 1000000
              << "s.\n";
}
// ~Element by element sum.
// ~ TO DO

// Multiply one row with the other matrix algorithm.
std::vector<std::vector<int>> tp_row_col_matrices_product(std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size();
    std::vector<std::vector<int>> result(n);
    ThreadPool tp{static_cast<size_t>(n)};

    // start time for our study case
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    for(unsigned int iRow = 0; iRow < n; iRow++) {
        tp.enqueue(
                [&result, matrix1, matrix2, iRow] () {
                    row_prod(&result[iRow], &(*matrix1)[iRow], matrix2, iRow);
                });
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "\nn threads in TP running the 'tp_row_col_matrices_product' took:: "
              << duration
              << "us, "
              << duration / 1000
              << "ms, "
              << duration / 1000000
              << "s.\n";

    return result;
}
// ~Multiply one row with the other matrix algorithm.

// Classic algorithm matrix product.
std::vector<std::vector<int>> tp_matrices_product(std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size();
    std::vector<std::vector<int>> result(n);

    // start time for our study case
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    ThreadPool tp {1};

    // std::future<void> ft =
    tp.enqueue(
            [&result, matrix1, matrix2] () {
                 matrix_product(&result, matrix1, matrix2);
            }
    );

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "\n1 thread in TP running the classic matrix product took: "
              << duration
              << "us, "
              << duration / 1000
              << "ms, "
              << duration / 1000000
              << "s.\n";
    std::cout << "The results regarding matrix multiplication were already proved to be valid in week 3 assign. " << std::endl;

    return result;
}
// ~ Classic algorithm matrix product.

void target2() {

    // Due to the circumstances, for our purposes a matrix of 1000 x 1000 is enough to analyse
    // how future, async and thread pools are working.
    std::vector<std::vector<int>> big_query;
    for(int i=0; i<1000; i++) {
        std::vector<int> row;
        row.reserve(1000);
        for(int j=0; j<1000; j++) {
            row.push_back(j);
        }
        big_query.push_back(row);
    }

    std::vector<std::vector<int>>
            result,
            matrix1{
            {1, 2, 3},
            {4, 5, 6},
            {7, 8, 9}
    },
            matrix2{
            {2, 0, 0},
            {0, 2, 0},
            {0, 0, 2}
    },
            matrix3{
            {1, 2, 3},
            {1, 3, 4},
            {2, 2, 2}
    },
            matrix4{
            {1, 7},
            {8, 9},
            {0, 0}
    };

    tp_matrices_product(&big_query, &big_query);

    std::vector<std::vector<int>> m;
    m = tp_row_col_matrices_product(&big_query, &big_query);
    //    display_matrix(m);
}

int main() {
    target1();
    target2();
    return 0;
}
