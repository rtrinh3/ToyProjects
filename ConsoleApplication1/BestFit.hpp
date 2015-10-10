#pragma once
#include <type_traits>
#include <cstdint>

static constexpr std::size_t highOrderBit(std::uintmax_t n, std::size_t r = 0) {
	return (n >> 1) ? highOrderBit(n >> 1, r + 1) : r;
};

template <std::uintmax_t N>
class BestFit {
private:
	static constexpr std::size_t bits = highOrderBit(N);
public:
	using type =
		std::conditional_t<bits <= 7, std::uint_least8_t,
		std::conditional_t<bits <= 15, std::uint_least16_t,
		std::conditional_t<bits <= 31, std::uint_least32_t,
		uint_least64_t> > >;
};
