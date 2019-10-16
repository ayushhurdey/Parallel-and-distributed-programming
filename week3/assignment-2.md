```cpp
#include <iostream>
#include <vector>
#include <thread>

void row_sum(std::vector<int>* row1, std::vector<int>* row2, int thread) {
    int iRow = 0;
    for(int value : *row1) {
        (*row1)[iRow] = value + (*row2)[iRow];
        iRow++;
    }
}

void elem_sum(int* elem1, const int* elem2, int thread) {
    *elem1 += *elem2;
}

void row_matrices_sum(std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size();
    std::vector<std::thread*> threads;
    for(unsigned long iRow = 0; iRow < n; iRow++) {
        threads.push_back(new std::thread(row_sum, &(*matrix1)[iRow], &(*matrix2)[iRow], iRow));
    }
    for(std::thread* thread : threads) {
        thread->join();
    }
}

void elem_matrices_sum(std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size(), iThread = 0;
    std::vector<std::thread*> threads;
    for(unsigned long iRow = 0; iRow < n; iRow++) {
        for(unsigned long iCol = 0; iCol < n; iCol++) {
            threads.push_back(new std::thread(elem_sum, &(*matrix1)[iRow][iCol], &(*matrix2)[iRow][iCol], iThread++));
        }
    }
    for(std::thread* thread : threads) {
        thread->join();
    }
}

void row_prod(std::vector<int>* result, std::vector<int>* row, std::vector<std::vector<int>>* matrix, int thread) {
    int n = (*matrix).size(), m = (*matrix)[0].size(), sum;
    for(unsigned int jCol = 0; jCol < m; jCol++) {
        sum = 0;
        for (unsigned int jRow = 0; jRow < n; jRow++) {
            sum += (*row)[jRow] * (*matrix)[jRow][jCol];
        }
        result->push_back(sum);
    }
}


std::vector<std::vector<int>> row_col_matrices_product(std::vector<std::vector<int>>* matrix1, std::vector<std::vector<int>>* matrix2) {
    int n = (*matrix1).size();
    std::vector<std::vector<int>> result(n);
    std::vector<std::thread*> threads;
    for(unsigned int iRow = 0; iRow < n; iRow++) {
        threads.push_back(new std::thread(row_prod, &result[iRow], &(*matrix1)[iRow], matrix2, iRow));
    }
    for(std::thread* thread : threads) {
        thread->join();
    }
    return result;
}

// one thread product
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
    auto* t = new std::thread(matrix_product, &result, matrix1, matrix2);
    t->join();
    return result;
}

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

int main() {
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

    // 1 thread
    result = matrices_product(&matrix3, &matrix4);
    display_matrix(result);

    // matrices_sum(&matrix1, &matrix2);

    // n threads
    row_matrices_sum(&matrix1, &matrix2);
    display_matrix(matrix1);

    // n * n threads
    elem_matrices_sum(&matrix1, &matrix2);
    display_matrix(matrix1);

    // n threads
    result = row_col_matrices_product(&matrix3, &matrix4);
    display_matrix(result);

    return 0;
}
```
