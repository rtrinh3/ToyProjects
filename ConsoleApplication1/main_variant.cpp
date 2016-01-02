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

template <class... As>
struct Help {
	/*template <class... Bs>
	static std::tuple<std::pair<As, Bs>...> NonWorking(Bs... bs) {
		using Return = std::tuple<std::pair<As, Bs>...>;
		return Return{};
	}*/

	template <class... Bs>
	static auto Working(Bs... bs) {
		using Return = std::tuple<std::pair<As, Bs>...>;
		return Return{};
	}
};

template <typename T>
void PrintType() {
	std::cout << typeid(T).name() << '\n';
}

int main() {
	Sqrt s;
	Variant<int, float> foo(Pos<0>{}, 10);
	auto bar = foo.apply(s);
	foo.match(
		[](int) {puts("int"); },
		[](float) {puts("float"); }
	);

	auto qux = Attempt([] {return 1; })
		.Map([](int n) {return n + 2; })
		.Unwrap();
	std::cout << qux << '\n';

	//auto broken = Help<int, char>::NonWorking(1.0, 1.0f);
	auto good = Help<int, char>::Working(1.0, 1.0f);
	PrintType<decltype(good)>();
}
