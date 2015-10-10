#pragma once
#include <tuple>
#include <type_traits>

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
	template <size_t I, class T>
	Variant(Pos<I> tag, T&& val);

	// This factory might be easier to use than the positional constructor.
	template <size_t I>
	static Variant construct(const TypeAt<I>& item);

	// Positional assignment
	template <size_t I>
	Variant& assign(const TypeAt<I>& item);

	// Copy assignment
	Variant& operator=(const Variant& rhs);

	// Direct access
	template <size_t I>
	TypeAt<I>& get();

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

	template <class Func>
	Variant<std::result_of_t<Func && (Ts)>...>
		Apply(Func&& func);
private:
	// Static definitions
	constexpr static size_t size = sizeof...(Ts);
	constexpr static size_t INVALID = size;
	// A valid index would be in [0, size[
	
	// Member variables
	typename std::aligned_union<1, Ts...>::type storage;
	size_t index = INVALID;

	bool valid() const;

	void destroySelf();

	template <class Func, size_t... Is>
	Variant<std::result_of_t<Func && (Ts)>...>
		Apply_impl(Func&& func, std::index_sequence<Is...>);
};

#include "Variant_impl.hpp"
