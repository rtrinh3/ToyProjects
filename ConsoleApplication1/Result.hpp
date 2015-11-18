#pragma once
#include <utility>
#include <exception>
#include <memory>
#include "Variant.hpp"

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
			}
			catch (...) {
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
			}
			catch (...) {
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
	}
	catch (...) {
		return Return::build_err(std::current_exception());
	}
}
