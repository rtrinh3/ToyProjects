#pragma once
#include <tuple>

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
