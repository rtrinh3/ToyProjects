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

template <typename To, typename From>
To convert(const void* p) {
	return *(const From*)p;
}

template <typename... Ts>
std::common_type_t<Ts...>
coalesce(const Variant<Ts...>& var)
{
	using Res = std::common_type_t<Ts...>;
	using ConvertPtr = Res(*)(const void*);
	static constexpr ConvertPtr conversion[] = {
		convert<Res, Ts>...
	};
	return conversion[var.getIndex()](&var);
}

int main() {
	Variant<int, string> mything("Hello");
	mything.call(Print{});

	Variant<float, double> myotherthing(Pos<0>{}, 10.0f);
	auto what = myotherthing.Apply(Sqrt{});
	auto wut = coalesce(what);
	cout << wut << "\n";

	const Variant<string, int> whatWillThisBe;
	whatWillThisBe.match(
		[](const string& s) 
			{ cout << "I got a string: [" << s << "]\n"; },
		[](int x) 
			{ cout << "I got an int: " << x << "\n"; }
	);
}
