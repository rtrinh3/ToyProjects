#include <iostream>
#include <string>
#include <typeinfo>
#include <cmath>
#include <string>
#include "Variant.hpp"
#include "Result.hpp"
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

#define FUNC(f) [&](auto&& x){return f(x);}

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

	// Rust-style "enum" (sum of unit types)
	MyEnum rust_style;
	cout << "Rust-style enum: " << sizeof(rust_style);
	rust_style.match(
		[](Print) { cout << " Print :]\n"; },
		[](Sqrt) { cout << " Sqrt :)\n"; }
	);

	// Testing the Result monad
	auto getAThing = Attempt([&]{ return std::string("78"); })
		.Map(FUNC(std::stoi))
		.Map(func_sqrt);
	try {
		std::cout << "No problem: " << getAThing.Unwrap() << "\n";
	} catch (std::exception& e) {
		std::cout << "Unwrap: " << e.what() << "\n";
	}
	
	// Failures
	//Variant<int, string> widething(L"Hello");
}
