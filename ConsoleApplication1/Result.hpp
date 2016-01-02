#pragma once
#include <utility>
#include <exception>
#include <memory>
#include "Variant.hpp"

/*// Simple Optional type leveraging our Variant machinery
struct Empty {};
template <typename T>
class Optional {
private:
	typedef Variant<Empty, T> value_type;
	value_type contents;
public:
	Optional() = default;
	Optional(T val) : contents(Pos<1>{}, std::move(val))
	{
	}

	bool ok() const {
		return 1 == contents.getIndex();
	}

	const T& get() const& {
		return contents.template get<1>();
	}

	T& get() & {
		return contents.template get<1>();
	}

	T get() && {
		return contents.template get<1>();
	}

	Optional& operator=(const T& that) {
		contents.template assign<1>(that);
		return *this;
	}
};*/

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
		return value_type(Pos<1>{}, std::move(x));
	}
	static Result build_err(std::exception_ptr e) {
		return value_type(Pos<0>{}, e);
	}

	template <typename Fun>
	Result<std::result_of_t<Fun&&(const T&)> >
		Map(Fun&& func) const&
	{
		typedef Result<std::result_of_t<Fun&&(const T&)> > Return;
		return contents.match(
			[&](std::exception_ptr e) -> Return {
				return Return::build_err(e);
			},
			[&](const T& x) -> Return {
				return Attempt([&] {
					return std::forward<Fun>(func)(x);
				});
			}
		);
	}

	template <typename Fun>
	Result<std::result_of_t<Fun&&(T&&)> >
		Map(Fun&& func) &&
	{
		typedef Result<std::result_of_t<Fun&&(T&&)> > Return;
		return contents.match(
			[&](std::exception_ptr e) -> Return {
				return Return::build_err(e);
			},
			[&](T& x) -> Return {
				return Attempt([&] {
					return std::forward<Fun>(func)(std::move(x));
				});
			}
		);
	}

	const T& Unwrap() const& {
		return contents.match(
			[](std::exception_ptr e) -> const T& { std::rethrow_exception(e); },
			[](const T& x) -> const T& { return x; }
		);
	}

	T Unwrap() && {
		return contents.match(
			[](std::exception_ptr e) -> T { std::rethrow_exception(e); },
			[](T x) -> T { return x; }
		);
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
