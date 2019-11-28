#include<iostream>
#include<vector>
#include<future>
#include<functional>


#define COEF 256


using namespace std;

void printPoly(vector<int>& a) {
	for (auto as : a) {
		cout << as << " ";
	}
	cout << '\n';
}

vector<int> increaseDegree(vector<int>& a, int deg) {
	vector<int> c;
	for (int i = 0; i < a.size(); i++) {
		c.push_back(a[i]);
	}
	for (int i = 0; i < deg; i++) {
		c.push_back(0);
	}

	return c;
}

vector<int> enlargePoly(vector<int>& a, int factor) {
	vector<int> c;
	for (int i = 0; i < factor; i++)
	{
		c.push_back(0);
	}
	for (int i = 0; i < a.size(); i++)
	{
		c.push_back(a[i]);
	}

	return c;
}

vector<int> addPoly(vector<int>& a, vector<int>& b) {
	vector<int> c;
	if (a.size() > b.size()) {
		for (size_t i = 0; i < b.size(); i++)
		{
			c.push_back(a[i] + b[i]);
		}
		for (size_t i = b.size(); i < a.size(); i++)
		{
			c.push_back(a[i]);
		}
	}
	else {
		for (size_t i = 0; i < a.size(); i++)
		{
			c.push_back(a[i] + b[i]);
		}
		for (size_t i = a.size(); i < b.size(); i++)
		{
			c.push_back(b[i]);
		}
	}
	
	return c;
}

vector<int> subPoly(vector<int>& a, vector<int>& b) {
	vector<int> c;
	for (size_t i = 0; i < a.size(); i++)
	{
		c.push_back(a[i] - b[i]);
	}
	return c;
}

vector<int> multiplyOne(vector<int>& a, vector<int>& b) {

	vector<int> c;
	for (size_t i = 0; i < 2*a.size()-1; i++)
	{
		c.push_back(0);
	}
	for (size_t i = 0; i < a.size(); i++)
	{
		for (size_t j = 0; j < b.size(); j++)
		{
			c[i + j] += a[i] * b[j];
		}
	}

	return c;
}

vector<int> multiplyOneThread(vector<int>& a, vector<int>& b,int coef) {


	if (coef == 2) {
		return vector<int>{a[0] * b[0], a[0] * b[1] + a[1] * b[0], a[1] * b[1]};
	}

	vector<int> a1, a2;
	vector<int> b1, b2;
	for (int i = 0; i <coef/2; i++)
	{
		a1.push_back(a[i]);
		a2.push_back(a[i + coef / 2]);
		b1.push_back(b[i]);
		b2.push_back(a[i + coef / 2]);
		
	}
	int newCoef = coef / 2;

	vector<int> res1 = multiplyOneThread(a1, b1, newCoef);
	vector<int> res4 = multiplyOneThread(a2, b2, newCoef);

	res1 = increaseDegree(res1, coef);
	res4 = enlargePoly(res4, coef);

	vector<int> res2 = multiplyOneThread(a1, b2, newCoef);
	vector<int> res3 = multiplyOneThread(a2, b1, newCoef);

	res2 = enlargePoly(res2, newCoef);
	res3 = enlargePoly(res3, newCoef);

	res2 = increaseDegree(res2, newCoef);
	res3 = increaseDegree(res3, newCoef);

	vector<int> res5 = addPoly(res1, res4);
	vector<int> res6 = addPoly(res2, res3);
	vector<int> res7 = addPoly(res5, res6);

	return res7;
}

vector<int> multiplyMulti(vector<int> a, vector<int> b, int coef) {
	if (coef == 16) {
		return multiplyOne(a, b);
	}
	if (coef == 2) {
		return vector<int>{a[0] * b[0], a[0] * b[1] + a[1] * b[0], a[1] * b[1]};
	}

	vector<int> a1, a2;
	vector<int> b1, b2;
	for (int i = 0; i < coef / 2; i++)
	{
		a1.push_back(a[i]);
		a2.push_back(a[i + coef / 2]);
		b1.push_back(b[i]);
		b2.push_back(a[i + coef / 2]);

	}
	int newCoef = coef / 2;
	future<vector<int>> op1 = async(launch::async, multiplyMulti, a1, b1, newCoef);
	future<vector<int>> op2 = async(launch::async, multiplyMulti, a2, b2, newCoef);
	future<vector<int>> op3 = async(launch::async, multiplyMulti, a1, b2, newCoef);
	future<vector<int>> op4 = async(launch::async, multiplyMulti, a2, b1, newCoef);
	vector<int> res1 = op1.get();
	vector<int> res4 = op2.get();

	res1 = increaseDegree(res1, coef);
	res4 = enlargePoly(res4, coef);

	vector<int> res2 = op3.get();
	vector<int> res3 = op4.get();

	res2 = enlargePoly(res2, newCoef);
	res3 = enlargePoly(res3, newCoef);

	res2 = increaseDegree(res2, newCoef);
	res3 = increaseDegree(res3, newCoef);

	vector<int> res5 = addPoly(res1, res4);
	vector<int> res6 = addPoly(res2, res3);
	vector<int> res7 = addPoly(res5, res6);

	return res7;
}


vector<int> multiplyOneKara(vector<int>& a, vector<int>& b, int coef) {


	if (coef == 2) {
		return vector<int>{a[0] * b[0], a[0] * b[1] + a[1] * b[0], a[1] * b[1]};
	}

	vector<int> a1, a2;
	vector<int> b1, b2;
	for (int i = 0; i < coef / 2; i++)
	{
		a1.push_back(a[i]);
		a2.push_back(a[i + coef / 2]);
		b1.push_back(b[i]);
		b2.push_back(a[i + coef / 2]);

	}
	int newCoef = coef / 2;

	vector<int> res1 = multiplyOneKara(a1, b1, newCoef);
	vector<int> res4 = multiplyOneKara(a2, b2, newCoef);

	

	vector<int> inter1 = addPoly(a1, a2);
	vector<int> inter2 = addPoly(b1, b2);
	vector<int> inter3 = multiplyOneKara(inter1, inter2, newCoef);
	vector<int> inter4 = subPoly(inter3, res1);
	vector<int> inter5 = subPoly(inter4, res4);

		//(P1(X)+P2(X)) * (Q1(X)+Q2(X)) - P1(X)* Q1(X) - P2(X)*Q2(X)

	res1 = increaseDegree(res1, coef);
	res4 = enlargePoly(res4, coef);
	vector<int> res6 = enlargePoly(inter5, newCoef);
	res6 = increaseDegree(res6, newCoef);

	vector<int> res5 = addPoly(res1, res4);
	vector<int> res7 = addPoly(res5, res6);

	return res7;
}

vector<int> multiplyMultiKara(vector<int> a, vector<int> b, int coef) {


	if (coef == 2) {
		return vector<int>{a[0] * b[0], a[0] * b[1] + a[1] * b[0], a[1] * b[1]};
	}

	vector<int> a1, a2;
	vector<int> b1, b2;
	for (int i = 0; i < coef / 2; i++)
	{
		a1.push_back(a[i]);
		a2.push_back(a[i + coef / 2]);
		b1.push_back(b[i]);
		b2.push_back(a[i + coef / 2]);

	}
	int newCoef = coef / 2;

	future<vector<int>> op1 = async(launch::async, multiplyMultiKara, a1, b1, newCoef);
	future<vector<int>> op2 = async(launch::async, multiplyMultiKara, a2, b2, newCoef);

	vector<int> res1 = op1.get();
	vector<int> res4 = op2.get();



	vector<int> inter1 = addPoly(a1, a2);
	vector<int> inter2 = addPoly(b1, b2);

	future<vector<int>> op3 = async(launch::async, multiplyMultiKara, inter1, inter2, newCoef);

	vector<int> inter3 = op3.get();
	vector<int> inter4 = subPoly(inter3, res1);
	vector<int> inter5 = subPoly(inter4, res4);

	//(P1(X)+P2(X)) * (Q1(X)+Q2(X)) - P1(X)* Q1(X) - P2(X)*Q2(X)

	res1 = increaseDegree(res1, coef);
	res4 = enlargePoly(res4, coef);
	vector<int> res6 = enlargePoly(inter5, newCoef);
	res6 = increaseDegree(res6, newCoef);

	vector<int> res5 = addPoly(res1, res4);
	vector<int> res7 = addPoly(res5, res6);
	//P*Q=P0q0+p1q1+(p0+p1)(q0+q1)-p0q0-p1q1
	return res7;
}

int main() {

	vector<int> a, b;

	for (unsigned int i = 0; i < COEF; i++)
	{
		a.push_back(i+1);
		b.push_back(i+1);
	}
	//printPoly(a);

	vector<int> result=multiplyMulti(a, b,COEF);
	//vector<int> result2=multiplyMultiKara(a, b,COEF);
	printPoly(result);
	//printPoly(result2);
	return 0;
}