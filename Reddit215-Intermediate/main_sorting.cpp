// https://www.reddit.com/r/dailyprogrammer/comments/36m83a/20150520_challenge_215_intermediate_validating/
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>

using BitVector = std::vector<bool>;
void SetVectorToNumber(BitVector& vec, uint64_t number, size_t size) {
	vec.resize(size);
	for (size_t i = 0; i < size; ++i) {
		vec[i] = (0 != (number & (size_t(1) << i)));
	}
}

namespace Challenges {
	const char Input1[] =
		R"(4 5
0 2
1 3
0 1
2 3
1 2)";

	const char Input2[] =
		R"(8 19
0 2
1 3
0 1
2 3
1 2
4 6
5 7
4 5
6 7
5 6
0 4
1 5
2 6
3 7
2 4
3 5
1 2
3 4
6 7)";
}

template <class T>
void Sort(T& numbers, const std::vector<size_t>& comparators) {
	using std::swap;
	for (size_t i = 0; i < comparators.size(); i += 2) {
		if (numbers[comparators[i]] > numbers[comparators[i + 1]]) {
			swap(numbers[comparators[i]], numbers[comparators[i + 1]]);
		}
	}
}

int main() {
	std::ifstream input("challenge2.txt");
	// Read sorting network
	size_t wires, comparators;
	input >> wires >> comparators;
	std::vector<size_t> network;
	std::copy_n(std::istream_iterator<size_t>(input), comparators * 2, std::back_inserter(network));

	// Test
	static_assert(false < true, "I just assumed that false < true");
	bool good = true;
	const size_t limit = size_t(1) << wires;
	BitVector buf;
	for (size_t i = 0; i < limit && good; ++i) {
		SetVectorToNumber(buf, i, wires);
		Sort(buf, network);
		if (!std::is_sorted(buf.begin(), buf.end())) {
			good = false;
		}
	}

	std::cout << (good ? "V" : "Inv") << "alid network\n";
}
