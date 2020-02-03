```
/*
  becPerm - generates all permutations
  becKPerm - generates all arangements
  becKPermUnordered - generates all combinations
*/
#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>
using namespace std;

vector<vector<int>> ans;
vector<vector<int>> vset;

void printAnswer(vector<vector<int>> ans) {
    for (const auto &a: ans) {
        std::cout << '\n';
        for (const auto &val: a)
            std::cout << val << ' ';
    }
}

bool checkValidPerm(vector<int>& res) {
    unordered_set<int> set;
    for (const auto &i: res)
        set.insert(i);
    return set.size() == res.size();
}

bool checkValidPermUnordered(vector<int> res) {
    sort(res.begin(), res.end());
    for (const auto& a : vset)
        if (res == a)
            return false;
    vset.push_back(res);
    return true;
}

void becPerm(int n, vector<int> res) {
    if (res.size() == n) {
        if (checkValidPerm(res))
            ans.push_back(res);
        return;
    }
    for (int i = 0; i < n; ++i) {
        res.push_back(i);
        becPerm(n, res);
        res.pop_back();
    }
}

void becKPerm(int n, int k, vector<int> res) {
    if (res.size() == k) {
        if (checkValidPerm(res))
            ans.push_back(res);
        return;
    }
    for (int i = 0; i < n; ++i) {
        res.push_back(i);
        becKPerm(n, k, res);
        res.pop_back();
    }
}

void becKPermUnordered(int n, int k, vector<int> res) {
    if (res.size() == k) {
        if (checkValidPerm(res))
            checkValidPermUnordered(res);
        return;
    }
    for (int i = 0; i < n; ++i) {
        res.push_back(i);
        becKPermUnordered(n, k, res);
        res.pop_back();
    }
}

int main() {
    becPerm(3, vector<int>());
    printAnswer(ans);
    ans.clear();
    becKPerm(3, 2, vector<int>());
    printAnswer(ans);
    ans.clear();
    becKPermUnordered(3, 2, vector<int>());
    printAnswer(vset);

    return 0;
}
