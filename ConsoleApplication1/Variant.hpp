#pragma once
#include <tuple>
#include <type_traits>
#include <exception>
#include "BestFit.h"

using std::size_t;

template <size_t I>
using Pos = std::integral_constant<size_t, I>;

// The Variant class. Holds one object of a type among Ts..., and 
// provides means to interact with the stored object.
template <class... Ts>
class Variant {
public:
	template <size_t I>
	using TypeAt = typename std::tuple_element<I, std::tuple<Ts...> >::type;

	// Default constructor. Tries first to find a trivially constructible type, then a default constructible type.
	Variant();

	// Destructor
	~Variant();

	// Copy constructor
	Variant(const Variant& other);

	// Move constructor
	Variant(Variant&& other);

	// Deductive constructor. Tries first to find the exact type, then a convertible type.
	template <class T>
	Variant(T&& val);

	// Positional constructor
	template <size_t I>
	Variant(Pos<I> tag, TypeAt<I> val);

	// This factory might be easier to use than the positional constructor.
	template <size_t I>
	static Variant construct(TypeAt<I> item) {
		return Variant(Pos<I>{}, std::move(item));
	}

	// Positional assignment
	template <size_t I>
	void assign(const TypeAt<I>& item);

	// Copy assignment
	Variant& operator=(const Variant& rhs);

	// Direct access
	template <size_t I>
	TypeAt<I>& get();
	template <size_t I>
	const TypeAt<I>& get() const;

	size_t getIndex() const;

	// Takes n functors. According to the index, calls the nth functor.
	template <class... Funcs>
	void match(Funcs&&... funcs);
	template <class... Funcs>
	void match(Funcs&&... funcs) const;

	// Takes one functor. Calls the functor with the actual type of the variant.
	template <class Fun>
	void call(Fun&& fun);
	template <class Fun>
	void call(Fun&& fun) const;

	// Takes one functor. Calls the functor with the actual type of the variant, and returns the result.
	template <class Func>
	std::common_type_t<std::result_of_t<Func&&(Ts)>...>
		apply(Func&& func) const;
private:
	// Static definitions
	constexpr static size_t size = sizeof...(Ts);
	constexpr static size_t INVALID = size; // A valid index would be in [0, size[
	using IndexType = typename BestFit<size>::type;
	
	// Member variables
	typename std::aligned_union<0, Ts...>::type storage;
	IndexType index = INVALID;

	bool valid() const;

	void destroySelf();
};

struct WrongIndexException : public virtual std::exception {
	size_t expected, actual;
	char buffer[40];
	WrongIndexException(size_t expected, size_t actual) :
		expected(expected), actual(actual)
	{
		sprintf_s(buffer, "Expected index %zu, got %zu.", expected, actual);
	}
	virtual const char* what() const override {
		return buffer;
	}
};

struct InvalidVariantException : public virtual std::exception {
	virtual const char* what() const override {
		return "The Variant is in an invalid state.";
	}
};

#include "Variant_impl.hpp"
