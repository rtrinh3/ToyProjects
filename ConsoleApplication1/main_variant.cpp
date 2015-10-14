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

	Variant<int, float, double> myotherthing(Pos<0>{}, 10);
	auto what = myotherthing.apply(Sqrt{});
	cout << what << "\n";

	const Variant<string, int> whatWillThisBe;
	whatWillThisBe.match(
		[](const string& s) 
			{ cout << "I got a string: [" << s << "]\n"; },
		[](int x) 
			{ cout << "I got an int: " << x << "\n"; }
	);
}
