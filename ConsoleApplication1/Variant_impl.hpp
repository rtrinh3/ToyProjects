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
	template <class Func, class Arg, class Return>
	struct Apply_Invoker {
		static Return go(Func&& func, MaybeConstVoid<Arg>* arg) {
			return std::forward<Func>(func)(*(Arg*)arg);
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

	template <bool... bs>
	struct FirstTrue;
	
	template <bool... Tail>
	struct FirstTrue<false, Tail...> {
		static constexpr size_t value = 1U + FirstTrue<Tail...>::value;
	};
	
	template <bool... Tail>
	struct FirstTrue<true, Tail...> {
		static constexpr size_t value = 0U;
	};
	
	template <>
	struct FirstTrue<> {
		static constexpr size_t value = 0U;
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
	using namespace Variant_impl;
	constexpr auto firstTrivialDefault = FirstTrue<std::is_trivially_default_constructible<Ts>::value...>::value;
	constexpr auto firstDefault = FirstTrue<std::is_default_constructible<Ts>::value...>::value;
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
	constexpr auto matchingType = FirstTrue<std::is_same<ValueType, Ts>::value...>::value;
	constexpr auto convertibleType = FirstTrue<std::is_convertible<ValueType, Ts>::value...>::value;
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


// Positional assignment
template<class ...Ts>
template<size_t I>
void Variant<Ts...>::assign(const TypeAt<I>& item) {
	if (I == index) {
		get<I>() = item;
	} else {
		destroySelf();
		new (&storage) TypeAt<I>(item);
		index = I;
	}
	return *this;
}

// Direct access
template <class... Ts>
template <size_t I>
typename Variant<Ts...>::template TypeAt<I>&
Variant<Ts...>::get()
{
	if (index != I) {
		throw std::runtime_error("Wrong type");
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
	using InvokerPtr = void(*)(void*, void*);
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
std::common_type_t<std::result_of_t<Func && (Ts)>...>
Variant<Ts...>::apply(Func&& func) const
{
	using ResultType = std::common_type_t<std::result_of_t<Func && (Ts)>...>;
	using funcPtr = ResultType(*)(Func&&, const void*);
	static constexpr funcPtr funcArray[] = {
		&Variant_impl::Apply_Invoker<Func, const Ts, ResultType>::go...
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
