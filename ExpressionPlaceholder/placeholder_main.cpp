#include <tuple>
#include <utility>
#include <functional>
#include <type_traits>

#include <string>
#include <iostream>

template <size_t I>
struct Placeholder {
	template <typename... Ts>
	decltype(auto) operator()(const Ts&... args) const {
		return std::get<I>(std::tie(args...));
	}
};

Placeholder<0> x0;
Placeholder<1> x1;
Placeholder<2> x2;
Placeholder<3> x3;
Placeholder<4> x4;
Placeholder<5> x5;
Placeholder<6> x6;
Placeholder<7> x7;
Placeholder<8> x8;
Placeholder<9> x9;

template <class Func, class Left, class Right>
class Expression {
private:
	Func f;
	Left l;
	Right r;
public:
	Expression(Func ff, Left ll, Right rr)
		: f(std::move(ff)), l(std::move(ll)), r(std::move(rr))
	{
	}

	template <typename... Ts>
	decltype(auto) operator()(const Ts&... args) const {
		return f(l(args...), r(args...));
	}

	template <typename... Ts>
	decltype(auto) operator()(const Ts&... args) {
		return f(l(args...), r(args...));
	}
};

template <class Func, class Left, class Right>
Expression<Func, Left, Right>
makeExpression(const Func& f, const Left& l, const Right& r)
{
	return Expression<Func, Left, Right>(f, l, r);
}

template <typename T>
struct Constant {
	T value;

	template <typename... Ts>
	T& operator()(const Ts&... args) & {
		return value;
	}
	template <typename... Ts>
	const T& operator()(const Ts&... args) const& {
		return value;
	}
	template <typename... Ts>
	T operator()(const Ts&... args) && {
		return std::move(value);
	}
	template <typename... Ts>
	const T operator()(const Ts&... args) const&& {
		return std::move(value);
	}
};

template <typename T>
std::enable_if_t<std::is_move_constructible<T>::value, Constant<T> >
makeConstant(T value)
{
	return Constant<T>{std::move(value)};
}

template <typename T>
struct Reference {
	std::reference_wrapper<T> reference;

	template <typename... Ts>
	T& operator()(const Ts&... args) {
		return reference.get();
	}
	template <typename... Ts>
	const T& operator()(const Ts&... args) const {
		return reference.get();
	}
};

template <typename T>
std::enable_if_t<!std::is_move_constructible<T>::value, Reference<T> >
makeConstant(T& object) {
	return Reference<T>{std::ref(object)};
}

#define OP +
#define FUNC std::plus<>
#include "placeholder_operators.hpp"
#undef FUNC
#undef OP

#define OP -
#define FUNC std::minus<>
#include "placeholder_operators.hpp"
#undef FUNC
#undef OP

#define OP *
#define FUNC std::multiplies<>
#include "placeholder_operators.hpp"
#undef FUNC
#undef OP

#define OP /
#define FUNC std::divides<>
#include "placeholder_operators.hpp"
#undef FUNC
#undef OP

std::string IndentTemplate(const std::string& type) {
	std::string ans;
	int indent = 0;
	for (char c : type) {
		if ('<' == c) {
			ans.push_back('<');
			ans.push_back('\n');
			++indent;
			for (int i = 0; i < indent; ++i) {
				ans.push_back('\t');
			}
		} else if ('>' == c) {
			ans.push_back('\n');
			--indent;
			for (int i = 0; i < indent; ++i) {
				ans.push_back('\t');
			}
			ans.push_back('>');
			ans.push_back('\n');
			for (int i = 0; i < indent; ++i) {
				ans.push_back('\t');
			}
		} else {
			ans.push_back(c);
		}
	}
	return ans;
}

int main() {
	// Test opeartors on all permutations of Placeholder, Expression and Constant, except Constant-Constant
	auto ex = x0 + x0;
	auto pp = x0 + x0;
	auto pe = x0 + ex;
	auto pc = x0 + 1;
	auto ep = ex + x0;
	auto ee = ex + ex;
	auto ec = ex + 1;
	auto cp = 1 + x0;
	auto ce = 1 + ex;
	std::function<int(int)> things[] = { pp,pe,pc,ep,ee,ec,cp,ce };
	for (auto&& f : things) {
		std::cout << f(10) << " ";
	}
	std::cout << std::endl;

	// Polynomial
	auto polynomial3 = (x0*x0*x0) / 4 + 3 * (x0*x0) / 4 - 3 * x0 / 2 - 2;
	for (double n : {-4, -1, 0, +2}) {
		std::cout << polynomial3(n) << " ";
	}
	std::cout << '\n' << IndentTemplate(typeid(polynomial3).name()) << std::endl;
}

