#pragma once
#include <utility>

namespace Variant_impl {
	template <class T>
	using MaybeConstVoid = std::conditional_t<
		std::is_const<std::remove_reference_t<T> >::value,
		const void,
		void>;

	// If I try to write Apply_Invoker as a function, I can't make constexpr the array in Apply_impl :[
	template <class Func, class Arg, class Return>
	struct Apply_Invoker {
		static Return go(Func&& func, MaybeConstVoid<Arg>* arg) {
			return std::forward<Func>(func)(*(Arg*)arg);
		}
	};

	template <class Func, class Arg, class Return>
	Return Invalid_Apply_Invoker(Func&&, Arg*) {
		throw InvalidVariantException{};
	}

	template <typename Fun, typename Arg, typename Return>
	Return Match_Invoker(void* fun, MaybeConstVoid<Arg>* arg) {
		return ((Fun&&)*(std::remove_reference_t<Fun>*)fun)(*(Arg*)arg);
	}

	template <typename Return>
	Return Invalid_Match_Invoker(void* fun, void*) {
		throw InvalidVariantException{};
	}

	template <typename Return>
	Return Invalid_Match_Invoker_Const(void* fun, const void*) {
		throw InvalidVariantException{};
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
	struct FirstTrue {
		static constexpr size_t value = 0U;
	};
	
	template <bool... Tail>
	struct FirstTrue<false, Tail...> {
		static constexpr size_t value = 1U + FirstTrue<Tail...>::value;
	};
};

template <class... Ts>
bool Variant<Ts...>::valid() const {
	return index < size;
}

template <class... Ts>
void Variant<Ts...>::destroySelf() {
	if (valid()) {
		apply(Variant_impl::Destruct{});
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
template <size_t I>
Variant<Ts...>::Variant(Pos<I> tag, TypeAt<I> val) {
	static_assert(I < size, "Out of bounds");
	new (&storage) TypeAt<I>(std::move(val));
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
}

// Direct access
template <class... Ts>
template <size_t I>
typename Variant<Ts...>::template TypeAt<I>&
Variant<Ts...>::get()
{
	if (index != I) {
		throw WrongIndexException(I, index);
	}
	return (TypeAt<I>&)storage;
}

template <class... Ts>
template <size_t I>
const typename Variant<Ts...>::template TypeAt<I>&
Variant<Ts...>::get() const
{
	if (index != I) {
		throw WrongIndexException(I, index);
	}
	return (const TypeAt<I>&)storage;
}

template <class... Ts>
size_t Variant<Ts...>::getIndex() const {
	return index;
}

// Takes n functors. According to the index, calls the nth functor.
template <class... Ts>
template <class... Funcs>
auto //std::common_type_t<std::result_of_t<Funcs(Ts&)>...>
Variant<Ts...>::match(Funcs&&... funcs) {
	static_assert(sizeof...(Funcs) == size,
		"Need as many functions as possible types.");
	void* funcPtrs[] = { (void*)&funcs... };

	using Return = std::common_type_t<std::result_of_t<Funcs&&(Ts&)>...>;
	using InvokerPtr = Return(*)(void*, void*);
	static const InvokerPtr invokers[] = {
		Variant_impl::Match_Invoker<Funcs, Ts, Return>... ,
		Variant_impl::Invalid_Match_Invoker<Return>
	};
	return invokers[index](funcPtrs[index], &storage);
}

template <class... Ts>
template <class... Funcs>
auto //std::common_type_t<std::result_of_t<Funcs(const Ts&)>...>
Variant<Ts...>::match(Funcs&&... funcs) const {
	static_assert(sizeof...(Funcs) == size,
		"Need as many functions as possible types.");
	void* funcPtrs[] = { (void*)&funcs... };

	// Oddly enough, I can declare an alias for this type, even though I can't spell this as the return type of this method...
	using Return = std::common_type_t<std::result_of_t<Funcs&&(const Ts&)>...>;
	using InvokerPtr = Return(*)(void*, const void*);
	static const InvokerPtr invokers[] = {
		Variant_impl::Match_Invoker<Funcs, const Ts, Return>... ,
		Variant_impl::Invalid_Match_Invoker_Const<Return>
	};
	return invokers[index](funcPtrs[index], &storage);
}

// Takes one functor. Calls the functor with the actual type of the variant.
template <class... Ts>
template <class Func>
std::common_type_t<std::result_of_t<Func&&(Ts&)>...>
Variant<Ts...>::apply(Func&& func)
{
	using ResultType = std::common_type_t<std::result_of_t<Func&&(Ts&)>...>;
	using funcPtr = ResultType(*)(Func&&, void*);
	static const funcPtr funcArray[] = {
		&Variant_impl::Apply_Invoker<Func, Ts, ResultType>::go... ,
		&Variant_impl::Invalid_Apply_Invoker<Func, void, ResultType>
	};
	return funcArray[index](std::forward<Func>(func), &storage);
}
template <class... Ts>
template <class Func>
std::common_type_t<std::result_of_t<Func&&(const Ts&)>...>
Variant<Ts...>::apply(Func&& func) const
{
	using ResultType = std::common_type_t<std::result_of_t<Func&&(const Ts&)>...>;
	using funcPtr = ResultType(*)(Func&&, const void*);
	static const funcPtr funcArray[] = {
		&Variant_impl::Apply_Invoker<Func, const Ts, ResultType>::go... ,
		&Variant_impl::Invalid_Apply_Invoker<Func, const void, ResultType>
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
	other.apply(Variant_impl::Copy(&storage));
	index = other.index;
}

// Move constructor
template <class... Ts>
Variant<Ts...>::Variant(Variant<Ts...>&& other) {
	other.apply(Variant_impl::Move(&storage));
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
		rhs.apply(Variant_impl::Copy(&storage));
		index = rhs.index;
	}
	return *this;
}
