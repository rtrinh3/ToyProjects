#include <iostream>
#include <string>
#include <typeinfo>
#include "Variant.hpp"
using namespace std;

struct Print {
	template <class T>
	void operator()(const T& x) {
		cout << typeid(T).name() << "{" << x << "}\n";
	}
};

struct Sqrt {
	template <class T>
	auto operator()(const T& x) {
		return sqrt(x);
	}
};

int main() {
	Variant<int, string> mything("Hello");
	mything.call(Print{});

	Variant<float, double, long double> myotherthing(Pos<2>{}, 10);
	auto what = myotherthing.Apply(Sqrt{});
	what.call(Print{});

	const Variant<string, int> whatWillThisBe;
	whatWillThisBe.call(Print{}); //int
}
