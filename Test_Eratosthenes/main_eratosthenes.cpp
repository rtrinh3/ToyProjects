#include <vector>
#include <cmath>
#include <iostream>
#include <ctime>
#include <iomanip>
using namespace std;

using uint = unsigned int;

template <class SieveType>
vector<uint> Eratosthenes(uint n) {
	SieveType A(n + 1, true);
	const uint sqrtn = static_cast<uint>(floor(sqrt(n)));
	for (uint i = 2; i <= sqrtn; ++i) {
		if (A[i]) {
			for (uint j = i * i; j <= n; j += i) {
				A[j] = false;
			}
		}
	}
	// output loop
	vector<uint> ans;
	for (uint i = 2; i <= n; ++i) {
		if (A[i]) {
			ans.push_back(i);
		}
	}
	return ans;
}

int main() {
	constexpr uint Maximum = 1000U;
	const auto startBool = clock();
	const auto primesBool = Eratosthenes<vector<bool>>(Maximum);
	const auto endBool = clock();
	const auto startChar = clock();
	const auto primesChar = Eratosthenes<vector<char>>(Maximum);
	const auto endChar = clock();
	cout << "Bool: " << (endBool - startBool) << endl;
	cout << "Char: " << (endChar - startChar) << endl;
	cout << "Consistent: " << boolalpha << (primesBool == primesChar) << endl;
}