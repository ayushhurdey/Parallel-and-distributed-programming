#include <iostream>
#include <vector>
#include <thread>
#include <future>

// display
void display_matrix(const std::vector<std::vector<int>>& matrix) {
    for(const std::vector<int>& v : matrix) {
        for(int i : v) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// ROW sum
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
// ~ROW sum

// ELEM sum
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
// ~ELEM sum

// multiply one row with the other matrix algorithm
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
// ~multiply one row with the other matrix algorithm

// classic algorithm matrix product
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
// ~ classic algorithm matrix product

int main() {

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

    return 0;
}
