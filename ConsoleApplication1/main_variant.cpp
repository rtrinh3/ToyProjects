#include <iostream>
#include <string>
#include <typeinfo>
#include "Variant.hpp"
#include <cmath>
using namespace std;

struct Print {
	template <class T>
	void operator()(const T& x) const {
		cout << typeid(T).name() << "{" << x << "}\n";
	}
};

struct Sqrt {
	template <class T>
	auto operator()(const T& x) const {
		return sqrt(x);
	}
};

using MyEnum = Variant<Print, Sqrt>;

int main() {
	const Sqrt func_sqrt;

	Variant<int, string> mything("Hello");
	mything.call(Print{});

	Variant<int, float, double> myotherthing(Pos<0>{}, 10);
	auto what = myotherthing.apply(func_sqrt);
	cout << what << "\n";

	const Variant<string, int> whatWillThisBe;
	whatWillThisBe.match(
		[](const string& s) 
			{ cout << "I got a string: [" << s << "]\n"; },
		func_sqrt
	);

	MyEnum rust_style;
	cout << "Rust-style enum: " << sizeof(rust_style) << endl;
	rust_style.match(
		[](Print) { cout << "Print\n"; },
		[](Sqrt) { cout << "Sqrt :)\n"; }
	);
	
	// Failures
	//Variant<int, string> widething(L"Hello");
}
