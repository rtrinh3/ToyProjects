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
	template <class... Bs>
	static const char* me(Bs... bs) {
		return typeid(std::common_type_t<std::result_of_t<Bs&&(const As&)>...>).name();
	}
	
	template <class... Bs>
	static
		std::common_type_t<std::result_of_t<Bs && (const As&)>...>
		out(Bs... bs)
	{
		return{};
	}
};

int main() {
	Sqrt s;
	Variant<int, float> foo(Pos<0>{}, 10);
	auto bar = foo.apply(s);
	auto baz = foo.match(s, s);
	std::cout << baz << '\n';
	//std::cout << Help<int, float>::me(s, s) << '\n';
	auto qux = Attempt([] {return 1; })
		.Map([](int n) {return n + 2; })
		.Unwrap();
	std::cout << qux << '\n';
}
