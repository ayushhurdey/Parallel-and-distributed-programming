#include <iostream>
#include <vector>
#include <thread>
#include <future>

/*
    Utils:
        - near_2() is computing the smallest power of two greater than the given value
        - add_padding() is adding zeros to push the vector size through a power of 2
        - print() displays a list
        - println() same but with new line
*/

unsigned int near_2(unsigned int value) {
    unsigned int near = 2;
    while(near < value)
        near = near << 1;
    return near;
}

void add_padding(std::vector<long>* vector) {
    unsigned int size = vector->size();
    unsigned int near_size = near_2(size);
    for(unsigned int i = 0; i < near_size - size; i++) {
        vector->push_back(0);
    }
}

void print(std::vector<long> v) {
    for(auto& value: v) {
        std::cout << value << " ";
    }
}

void println(std::vector<long> v) {
    print(v);
    std::cout << std::endl;
}

/*
    Source: Wiki
    A work-efficient parallel prefix sum can be computed by the following steps.

    1. Compute the sums of consecutive pairs of items in which the first item of the
    pair has an even index: z0 = x0 + x1, z1 = x2 + x3, etc.

    2. Recursively compute the prefix sum w0, w1, w2, ... of the sequence z0, z1, z2,
    ...

    3. Express each term of the final sequence y0, y1, y2, ... as the sum of up to
    two terms of these intermediate sequences: y0 = x0, y1 = z0, y2 = z0 + x2,
    y3 = w0, etc. After the first value, each successive number yi is either
    copied from a position half as far through the w sequence, or is the previous
    value added to one value in the x sequence.
*/

void work_efficient_parallel_prefix_sum(std::vector<long>* v) {
    unsigned int n = v->size();
    std::vector<std::future<void>> wait_level;
    for(unsigned int i = 1; i < n; i*=2) {

        // We are waiting for all the operations on this level to finish.
        wait_level.clear();
        for(unsigned int j = 2*i-1; j < n; j += 2*i) {

            // Push into the vector of futures.
            wait_level.push_back(std::async(
                    [](std::vector<long> *v, unsigned int i, unsigned int j) { (*v)[j] += (*v)[j - i]; },
                    v, i, j));
        }

        // Wait all the future refs then go to the next level in the binary tree.
        for(auto& future: wait_level) future.wait();
    }
    unsigned int t = n/4;
    for(; t > 0; t=t/2) {
        wait_level.clear();
        for (unsigned int i = 3*t-1; i < n; i += 2*t) {
            wait_level.push_back(std::async(
                    [](std::vector<long> *v, unsigned int i, unsigned int t) { (*v)[i] += (*v)[i - t]; },
                    v, i, t));
        }
        for(auto& future: wait_level) future.wait();
    }
}

int main() {
    std::vector<long> v = {7, 1, 3, 9, 2, 4, 2, 1};
    add_padding(&v);
    work_efficient_parallel_prefix_sum(&v);
    println(v);
    return 0;
}
