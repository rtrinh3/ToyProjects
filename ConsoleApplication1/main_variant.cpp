#include <iostream>
#include <string>
#include <typeinfo>
#include "Variant.hpp"
#include <cmath>
#include <utility>
#include <exception>
#include <memory>
#include <string>
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

// Simple Optional type leveraging our Variant machinery
struct Empty {};
template <typename T>
using Optional = Variant<Empty, T>;

// Inspired by http://www.codethatgrows.com/lessons-learned-from-rust-the-result-monad/
template <typename T>
class Result {
private:
	typedef Variant<std::exception_ptr, T> value_type;
	value_type contents;
	Result(value_type v) : contents(std::move(v))
	{
	}
public:
	static Result build_ok(T x) {
		return value_type::construct<1>(std::move(x));
	}
	static Result build_err(std::exception_ptr e) {
		return value_type::construct<0>(e);
	}

	template <typename Fun>
	Result<std::result_of_t<Fun&&(const T&)> >
		Map(Fun&& func) const&
	{
		typedef Result<std::result_of_t<Fun&&(const T&)> > Return;
		Optional<Return> res;
		contents.match(
			[&](std::exception_ptr e) { res.assign<1>(Return::build_err(e)); },
			[&](const T& x) {
			try {
				res.assign<1>(Return::build_ok(std::forward<Fun>(func)(x)));
			} catch (...) {
				res.assign<1>(Return::build_err(std::current_exception()));
			}
		}
		);
		// Our res shouldn't be Empty at this point...
		return res.get<1>();
	}

	template <typename Fun>
	Result<std::result_of_t<Fun&&(T&&)> >
		Map(Fun&& func) &&
	{
		typedef Result<std::result_of_t<Fun&&(T&&)> > Return;
		Optional<Return> res;
		contents.match(
			[&](std::exception_ptr e) { res.assign<1>(Return::build_err(e)); },
			[&](const T& x) {
				try {
					res.assign<1>(Return::build_ok(std::forward<Fun>(func)(std::move(x))));
				} catch (...) {
					res.assign<1>(Return::build_err(std::current_exception()));
				}
			}
		);
		// Our res shouldn't be Empty at this point...
		return res.get<1>();
	}

	const T& Unwrap() const& {
		const T* res = nullptr;
		contents.match(
			[&](std::exception_ptr e) { std::rethrow_exception(e); },
			[&](const T& x) { res = std::addressof(x); }
		);
		// If we made it here, res is no longer nullptr.
		return *res;
	}

	T Unwrap() && {
		T* res = nullptr;
		contents.match(
			[&](std::exception_ptr e) { std::rethrow_exception(e); },
			[&](const T& x) { res = std::addressof(x); }
		);
		// If we made it here, res is no longer nullptr.
		return std::move(*res);
	}
};

template <typename Fun>
Result<std::result_of_t<Fun&&()> >
Attempt(Fun&& func)
{
	using Return = Result<std::result_of_t<Fun&&()> >;
	try {
		return Return::build_ok(std::forward<Fun>(func)());
	} catch (...) {
		return Return::build_err(std::current_exception());
	}
}

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
