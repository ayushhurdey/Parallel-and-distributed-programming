#include <iostream>
#include <vector>
#include <future>

void job(int level, int i, int j, int pos, std::vector<std::vector<int>>* m, int carry) {
    if((*m)[0].size()-1 == pos) {
        (*m)[j][0] += 1;
        (*m)[j][1] = 0;
        return;
    }
    while((*m)[i][0] != level-1 and (*m)[i][1] < pos and
            (*m)[j][0] != level-1 and (*m)[j][1] < pos);
    (*m)[j][2+pos] += (*m)[i][2+pos] + carry;
    carry = (*m)[j][2+pos]/10;
    if(carry == 1) {
        (*m)[j][2 + pos] = (*m)[j][2 + pos] % 10;
    }
    (*m)[j][1] += 1;
    job(level, i, j, pos+1, m, carry);
}

void sum(std::vector<std::vector<int>>* m) {
    unsigned int n = m->size();
    std::vector<std::future<void>> fts;
    int level = 1;
    for(int i=1; i<n; i*=2) {
        level += 1;
        std::cout << i << ": \n";
        for(int j=2*i-1; j<n; j+=2*i) {
            std::cout << " " << j-i << " " << j << std::endl;
            fts.push_back(std::async(std::launch::async, job, level, j-i, j, 0, m, 0));
        }
    }
    fts[fts.size()-1].get();
}

int main() {
    std::vector<std::vector<int>> m = {
            {1, 0, 0, 1, 0, 1, 0},
            {1, 0, 2, 3, 9, 0, 0},
            {1, 0, 0, 4, 0, 1, 0},
            {1, 0, 1, 3, 0, 0, 0},
            {1, 0, 1, 0, 0, 0, 0},
            {1, 0, 2, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0}
    };

    for(int i=0; i<8; i++) {
        for(int j=0; j<7; j++) {
            std::cout << m[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    sum(&m);

    for(int i=0; i<8; i++) {
        for(int j=0; j<7; j++) {
            std::cout << m[i][j] << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
