#include<iostream>
#include<vector>
#include<future>
#include<functional>

using namespace std;

void print(vector<int>& a) {
    for (auto as : a)
        cout << as << " ";
    cout << '\n';
}

vector<int> increase_degree(vector<int>& a, int degree) {
    vector<int> result;
    for (int i = 0; i < a.size(); i++)
        result.push_back(a[i]);
    for (int i = 0; i < degree; i++)
        result.push_back(0);
    return result;
}

vector<int> enlarge_polynomial(vector<int>& a, int factor) {
    vector<int> result;
    for (int i = 0; i < factor; i++)
        result.push_back(0);
    for (int i = 0; i < a.size(); i++)
        result.push_back(a[i]);
    return result;
}

vector<int> sum(vector<int>& a, vector<int>& b) {
    vector<int> result;
    if (a.size() > b.size()) {
        for (size_t i = 0; i < b.size(); i++)
            result.push_back(a[i] + b[i]);
        for (size_t i = b.size(); i < a.size(); i++)
            result.push_back(a[i]);
    } else {
        for (size_t i = 0; i < a.size(); i++)
            result.push_back(a[i] + b[i]);
        for (size_t i = a.size(); i < b.size(); i++)
            result.push_back(b[i]);
    }
    return result;
}

vector<int> subtract(vector<int>& a, vector<int>& b) {
    vector<int> result;
    for (size_t i = 0; i < a.size(); i++)
        result.push_back(a[i] - b[i]);
    return result;
}

vector<int> multiply(vector<int>& a, vector<int>& b) {
    vector<int> result;
    for (size_t i = 0; i < 2*a.size()-1; i++)
        result.push_back(0);
    for (size_t i = 0; i < a.size(); i++)
        for (size_t j = 0; j < b.size(); j++)
            result[i + j] += a[i] * b[j];
    return result;
}

vector<int> multiply_on_one_thread(vector<int>& a, vector<int>& b, int coefficient) {
    if (coefficient == 2)
        return vector<int>{a[0] * b[0], a[0] * b[1] + a[1] * b[0], a[1] * b[1]};

    vector<int> a1, a2;
    vector<int> b1, b2;
    for (int i = 0; i < coefficient / 2; i++) {
        a1.push_back(a[i]);
        a2.push_back(a[i + coefficient / 2]);
        b1.push_back(b[i]);
        b2.push_back(a[i + coefficient / 2]);
    }
    int half_coefficient = coefficient / 2;

    vector<int> res1 = multiply_on_one_thread(a1, b1, half_coefficient);
    vector<int> res4 = multiply_on_one_thread(a2, b2, half_coefficient);
    res1 = increase_degree(res1, coefficient);
    res4 = enlarge_polynomial(res4, coefficient);

    vector<int> res2 = multiply_on_one_thread(a1, b2, half_coefficient);
    vector<int> res3 = multiply_on_one_thread(a2, b1, half_coefficient);
    res2 = enlarge_polynomial(res2, half_coefficient);
    res3 = enlarge_polynomial(res3, half_coefficient);
    res2 = increase_degree(res2, half_coefficient);
    res3 = increase_degree(res3, half_coefficient);

    vector<int> res5 = sum(res1, res4);
    vector<int> res6 = sum(res2, res3);
    vector<int> res7 = sum(res5, res6);

    return res7;
}

vector<int> multiply_on_multiple_threads(vector<int> a, vector<int> b, int coefficient) {
    if (coefficient == 16)
        return multiply(a, b);
    if (coefficient == 2)
        return vector<int>{a[0] * b[0], a[0] * b[1] + a[1] * b[0], a[1] * b[1]};

    vector<int> a1, a2;
    vector<int> b1, b2;
    for (int i = 0; i < coefficient / 2; i++) {
        a1.push_back(a[i]);
        a2.push_back(a[i + coefficient / 2]);
        b1.push_back(b[i]);
        b2.push_back(a[i + coefficient / 2]);
    }
    int half_coefficient = coefficient / 2;

    future<vector<int>> op1 = async(launch::async, multiply_on_multiple_threads, a1, b1, half_coefficient);
    future<vector<int>> op2 = async(launch::async, multiply_on_multiple_threads, a2, b2, half_coefficient);
    future<vector<int>> op3 = async(launch::async, multiply_on_multiple_threads, a1, b2, half_coefficient);
    future<vector<int>> op4 = async(launch::async, multiply_on_multiple_threads, a2, b1, half_coefficient);
   
    vector<int> res1 = op1.get();
    vector<int> res4 = op2.get();
    res1 = increase_degree(res1, coefficient);
    res4 = enlarge_polynomial(res4, coefficient);
   
    vector<int> res2 = op3.get();
    vector<int> res3 = op4.get();
    res2 = enlarge_polynomial(res2, half_coefficient);
    res3 = enlarge_polynomial(res3, half_coefficient);
    res2 = increase_degree(res2, half_coefficient);
    res3 = increase_degree(res3, half_coefficient);
   
    vector<int> res5 = sum(res1, res4);
    vector<int> res6 = sum(res2, res3);
    vector<int> res7 = sum(res5, res6);
    return res7;
}


vector<int> karatsuba_multiply_on_one_thread(vector<int>& a, vector<int>& b, int coefficient) {
    if (coefficient == 2)
        return vector<int>{a[0] * b[0], a[0] * b[1] + a[1] * b[0], a[1] * b[1]};

    vector<int> a1, a2;
    vector<int> b1, b2;
    for (int i = 0; i < coefficient / 2; i++) {
        a1.push_back(a[i]);
        a2.push_back(a[i + coefficient / 2]);
        b1.push_back(b[i]);
        b2.push_back(a[i + coefficient / 2]);
    }

    int half_coefficient = coefficient / 2;
    vector<int> res1 = karatsuba_multiply_on_one_thread(a1, b1, half_coefficient);
    vector<int> res4 = karatsuba_multiply_on_one_thread(a2, b2, half_coefficient);

    vector<int> inter1 = sum(a1, a2);
    vector<int> inter2 = sum(b1, b2);
    vector<int> inter3 = karatsuba_multiply_on_one_thread(inter1, inter2, half_coefficient);
    vector<int> inter4 = subtract(inter3, res1);
    vector<int> inter5 = subtract(inter4, res4);

    //(P1(X)+P2(X)) * (Q1(X)+Q2(X)) - P1(X)* Q1(X) - P2(X)*Q2(X)
    res1 = increase_degree(res1, coefficient);
    res4 = enlarge_polynomial(res4, coefficient);
    vector<int> res6 = enlarge_polynomial(inter5, half_coefficient);
    res6 = increase_degree(res6, half_coefficient);

    vector<int> res5 = sum(res1, res4);
    vector<int> res7 = sum(res5, res6);
    return res7;
}

vector<int> karatsuba_on_multiple_threads(vector<int> a, vector<int> b, int coefficient) {
    if (coefficient == 2)
        return vector<int>{a[0] * b[0], a[0] * b[1] + a[1] * b[0], a[1] * b[1]};

    vector<int> a1, a2;
    vector<int> b1, b2;
    for (int i = 0; i < coefficient / 2; i++) {
        a1.push_back(a[i]);
        a2.push_back(a[i + coefficient / 2]);
        b1.push_back(b[i]);
        b2.push_back(a[i + coefficient / 2]);

    }
    int half_coefficient = coefficient / 2;

    future<vector<int>> op1 = async(launch::async, karatsuba_on_multiple_threads, a1, b1, half_coefficient);
    future<vector<int>> op2 = async(launch::async, karatsuba_on_multiple_threads, a2, b2, half_coefficient);
    
    vector<int> res1 = op1.get();
    vector<int> res4 = op2.get();
    vector<int> inter1 = sum(a1, a2);
    vector<int> inter2 = sum(b1, b2);
    
    future<vector<int>> op3 = async(launch::async, karatsuba_on_multiple_threads, inter1, inter2, half_coefficient);
    
    vector<int> inter3 = op3.get();
    vector<int> inter4 = subtract(inter3, res1);
    vector<int> inter5 = subtract(inter4, res4);
    
    //(P1(X)+P2(X)) * (Q1(X)+Q2(X)) - P1(X)*Q1(X) - P2(X)*Q2(X)
    res1 = increase_degree(res1, coefficient);
    res4 = enlarge_polynomial(res4, coefficient);
    vector<int> res6 = enlarge_polynomial(inter5, half_coefficient);
    res6 = increase_degree(res6, half_coefficient);
    vector<int> res5 = sum(res1, res4);
    vector<int> res7 = sum(res5, res6);
    return res7;
}

int main() {
    vector<int> a, b;
    for (unsigned int i = 0; i < 8; i++) {
        a.push_back(i+1);
        b.push_back(i+1);
    }
    print(a);
    print(b);

    vector<int> result = multiply_on_multiple_threads(a, b, 8);
    print(result);

    result = karatsuba_on_multiple_threads(a, b, 8);
    print(result);
    return 0;
}
