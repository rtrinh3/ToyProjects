#pragma once
#include <utility>

namespace Variant_impl {
	template <class T>
	using MaybeConstVoid = std::conditional_t<
		std::is_const<std::remove_reference_t<T> >::value,
		const void,
		void>;

	template <class Func, class Arg>
	void Call_Invoker(Func&& func, MaybeConstVoid<Arg>* arg) {
		std::forward<Func>(func)(*(Arg*)arg);
	}

	// If I try to write Apply_Invoker as a function, I can't make constexpr the array in Apply_impl :[
	template <class Func, class Arg, class Return, size_t I>
	struct Apply_Invoker {
		static Return go(Func&& func, MaybeConstVoid<Arg>* arg) {
			return Return{ Pos<I>{}, std::forward<Func>(func)(*(Arg*)arg) };
		}
	};

	template <typename Fun, typename Arg>
	void Match_Invoker(void* fun, MaybeConstVoid<Arg>* arg) {
		(*(Fun*)fun)(*(Arg*)arg);
	}

	// Helper. Destructs the given object.
	struct Destruct {
		template <class T>
		void operator()(T& thing) {
			(&thing)->~T();
		}
	};

	// Helper. Upon construction, takes a target to copy to.
	// Upon invocation, copies the argument to the target.
	struct Copy {
		void* target;

		Copy(void* target_) : target(target_) {}

		template <class T>
		void operator()(const T& thing) {
			new (target) T(thing);
		}
	};

	// Helper. Upon construction, takes a target to copy to.
	// Upon invocation, moves the argument to the target.
	struct Move {
		void* target;

		Move(void* target_) : target(target_) {}

		template <class T>
		void operator()(T& thing) {
			new (target) T(std::move(thing));
		}
	};

	template <template<class> class Pred, class... List>
	struct FirstTrue;

	template <template<class> class Pred, class Head, class... Tail>
	struct FirstTrue<Pred, Head, Tail...> {
		static constexpr size_t value = Pred<Head>::value
			? 0U
			: 1U + FirstTrue<Pred, Tail...>::value;
	};

	template <template<class> class Pred>
	struct FirstTrue<Pred> {
		static constexpr size_t value = 0U;
	};

	template <class T>
	struct SameAs {
		template <class U>
		using type = std::is_same<T, U>;
	};

	template <class From>
	struct ConvertibleFrom {
		template <class To>
		using type = std::is_convertible<From, To>;
	};
};

template <class... Ts>
bool Variant<Ts...>::valid() const {
	return index < size;
}

template <class... Ts>
void Variant<Ts...>::destroySelf() {
	if (valid()) {
		call(Variant_impl::Destruct{});
		index = INVALID;
	}
	// If it was invalid, we have nothing to destroy.
}

// Default constructor. Tries first to find a trivially constructible type, then a default constructible type.
template <class... Ts>
Variant<Ts...>::Variant() {
	constexpr auto firstTrivialDefault = Variant_impl::FirstTrue<std::is_trivially_default_constructible, Ts...>::value;
	constexpr auto firstDefault = Variant_impl::FirstTrue<std::is_default_constructible, Ts...>::value;
	constexpr auto finalIndex = firstTrivialDefault < size ? firstTrivialDefault : firstDefault;

	static_assert(finalIndex < size, "To default construct, you must have a default constructible type");
	new (&storage) TypeAt<finalIndex>;
	index = finalIndex;
}

// Deductive constructor. Tries first to find the exact type, then a convertible type.
template <class... Ts>
template <class T>
Variant<Ts...>::Variant(T&& val) {
	using namespace Variant_impl;
	using ValueType = std::decay_t<T>;
	constexpr auto matchingType = FirstTrue<SameAs<ValueType>::type, Ts...>::value;
	constexpr auto convertibleType = FirstTrue<ConvertibleFrom<ValueType>::type, Ts...>::value;
	constexpr auto finalIndex = matchingType < size ? matchingType : convertibleType;

	static_assert(finalIndex < size, "Could neither find a matching type nor a convertible type");
	new (&storage) TypeAt<finalIndex>(std::forward<T>(val));
	index = finalIndex;
}

// Positional constructor
template <class... Ts>
template <size_t I, class T>
Variant<Ts...>::Variant(Pos<I> tag, T&& val) {
	static_assert(I < size, "Out of bounds");
	new (&storage) TypeAt<I>(std::forward<T>(val));
	index = I;
}

// This factory might be easier to use than the positional constructor.
template <class... Ts>
template <size_t I>
static Variant<Ts...>
Variant<Ts...>::construct(const typename Variant<Ts...>::TypeAt<I>& item)
{
	return Variant(Pos<I>{}, item);
}

// Direct access
template <class... Ts>
template <size_t I>
typename Variant<Ts...>::TypeAt<I>&
Variant<Ts...>::get()
{
	if (index != I) {
		throw runtime_error("Wrong type");
	}
	return (TypeAt<I>&)storage;
}

template <class... Ts>
size_t Variant<Ts...>::getIndex() const {
	return index;
}

// Takes n functors. According to the index, calls the nth functor.
template <class... Ts>
template <class... Funcs>
void Variant<Ts...>::match(Funcs&&... funcs) {
	static_assert(sizeof...(Funcs) == size,
		"Need as many functions as possible types.");
	void* funcPtrs[] = { &funcs... };
	using InvokerPtr = void(*)(void*, VoidPtr);
	static constexpr InvokerPtr invokers[] = {
		&Variant_impl::Match_Invoker<Funcs, Ts>...
	};
	invokers[index](funcPtrs[index], &storage);
}

template <class... Ts>
template <class... Funcs>
void Variant<Ts...>::match(Funcs&&... funcs) const {
	static_assert(sizeof...(Funcs) == size,
		"Need as many functions as possible types.");
	void* funcPtrs[] = { &funcs... };
	using InvokerPtr = void(*)(void*, const void*);
	static constexpr InvokerPtr invokers[] = {
		&Variant_impl::Match_Invoker<Funcs, const Ts>...
	};
	invokers[index](funcPtrs[index], &storage);
}

// Takes one functor. Calls the functor with the actual type of the variant.
template <class... Ts>
template <class Fun>
void Variant<Ts...>::call(Fun&& fun) {
	using funcPtr = void(*)(Fun&&, void*);
	static constexpr funcPtr funcArray[] = {
		&Variant_impl::Call_Invoker<Fun, Ts>...
	};
	funcArray[index](std::forward<Fun>(fun), &storage);
}

template <class... Ts>
template <class Fun>
void Variant<Ts...>::call(Fun&& fun) const {
	using funcPtr = void(*)(Fun&&, const void*);
	static constexpr funcPtr funcArray[] = {
		&Variant_impl::Call_Invoker<Fun, const Ts>...
	};
	funcArray[index](std::forward<Fun>(fun), &storage);
}

template <class... Ts>
template <class Func>
Variant<std::result_of_t<Func && (Ts)>...>
Variant<Ts...>::Apply(Func&& func)
{
	return Apply_impl(
		std::forward<Func>(func),
		index_sequence_for <Ts...>{});
}

template <class... Ts>
template <class Func, size_t... Is>
Variant<std::result_of_t<Func && (Ts)>...>
Variant<Ts...>::Apply_impl(Func&& func, std::index_sequence<Is...>)
{
	using ResultType = Variant<result_of_t<Func(Ts)>...>;
	using funcPtr = ResultType(*)(Func&&, void*);
	static constexpr funcPtr funcArray[] = {
		&Variant_impl::Apply_Invoker<Func, Ts, ResultType, Is>::go...
	};
	return funcArray[index](std::forward<Func>(func), &storage);
}

// Destructor
template <class... Ts>
Variant<Ts...>::~Variant() {
	destroySelf();
}

// Copy constructor
template <class... Ts>
Variant<Ts...>::Variant(const Variant<Ts...>& other) {
	other.call(Variant_impl::Copy(&storage));
	index = other.index;
}

// Move constructor
template <class... Ts>
Variant<Ts...>::Variant(Variant<Ts...>&& other) {
	other.call(Variant_impl::Move(&storage));
	index = other.index;
}

// Positional assignment
template <class... Ts>
template <size_t I>
Variant<Ts...>&
Variant<Ts...>::assign(const typename Variant<Ts...>::TypeAt<I>& item)
{
	if (I == index) {
		get<I>() = item;
	} else {
		destroySelf();
		new (&storage) TypeAt<I>(item);
		index = I;
	}
	return *this;
}

// Copy assignment
template <class... Ts>
Variant<Ts...>&
Variant<Ts...>::operator=(const Variant<Ts...>& rhs)
{
	// A bit naive, but...
	if (this != &rhs) {
		destroySelf();
		rhs.call(Variant_impl::Copy(&storage));
		index = rhs.index;
	}
	return *this;
}
