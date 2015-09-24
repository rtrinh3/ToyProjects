#include <tuple>
#include <utility>
#include <type_traits>
#include <functional>

#include <iostream>
#include <string>
#include <typeinfo>
using namespace std;

namespace Variant_impl {
	template <class T>
	using MaybeConstVoid = std::conditional_t<
		std::is_const<std::remove_reference_t<T> >::value,
		const void, 
		void>;

	template <class Arg, class Func>
	std::function<void(MaybeConstVoid<Arg>* arg)>
		MakeInvoker(Func&& func)
	{
		return [&](MaybeConstVoid<Arg>* arg) {
			std::forward<Func>(func)(*(Arg*)arg);
		};
	}

	template <class Func, class Arg>
	void Invoke1(
		Func&& func,
		MaybeConstVoid<Arg>* arg)
	{
		std::forward<Func>(func)(*(Arg*)arg);
	}

	template <class Func, class Arg, class Return, size_t I>
	Return Invoke2(
		Func&& func,
		MaybeConstVoid<Arg>* arg)
	{
		return Return{ Pos<I>{}, std::forward<Func>(func)(*(Arg*)arg) };
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

template <size_t I>
using Pos = std::integral_constant<size_t, I>;

// The Variant class. Holds one object of a type among Ts..., and 
// provides means to interact with the stored object.
template <class... Ts>
class Variant {
private:
	// Static definitions
	constexpr static size_t size = sizeof...(Ts);
	constexpr static size_t INVALID = size;
	// A valid index would be in [0, size[
	
	template <size_t I>
	using TypeAt = typename tuple_element<I, tuple<Ts...> >::type;
	
	// Member variables
	typename aligned_union<1, Ts...>::type storage;
	size_t index = INVALID;
	
	bool valid() const {
		return index < size;
	}
	
	void destroySelf() {
		if (valid()) {
			call(Variant_impl::Destruct{});
			index = INVALID;
		}
		// If it was invalid, we have nothing to destroy.
	}

	template <class Func, size_t... Is>
	Variant<result_of_t<Func&&(Ts)>...>
		Apply_impl(Func&& func, index_sequence<Is...>)
	{
		using ResultType = Variant<result_of_t<Func(Ts)>...>;
		using funcPtr = ResultType(*)(Func&&, void*);
		funcPtr funcArray[] = {
			Variant_impl::Invoke2<Func, Ts, ResultType, Is>...
		};
		return funcArray[index](std::forward<Func>(func), &storage);

	}

public:
	// Default constructor. Tries first to find a trivially constructible type, then a default constructible type.
	Variant() {
		constexpr auto firstTrivialDefault = Variant_impl::FirstTrue<std::is_trivially_default_constructible, Ts...>::value;
		constexpr auto firstDefault = Variant_impl::FirstTrue<std::is_default_constructible, Ts...>::value;
		constexpr auto finalIndex = firstTrivialDefault < size ? firstTrivialDefault : firstDefault;

		static_assert(finalIndex < size, "To default construct, you must have a default constructible type");
		new (&storage) TypeAt<finalIndex>;
		index = finalIndex;
	}

	// Deductive constructor. Tries first to find the exact type, then a convertible type.
	template <class T>
	Variant(T&& val) {
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
	template <size_t I, class T>
	Variant(Pos<I> tag, T&& val) {
		static_assert(I < size, "Out of bounds");
		new (&storage) TypeAt<I>(std::forward<T>(val));
		index = I;
	}

	
	// This factory might be easier to use than the positional constructor.
	template <size_t I>
	static Variant construct(const TypeAt<I>& item) {
		return Variant(Pos<I>{}, item);
	}
	
	// Direct access
	template <size_t I>
	TypeAt<I>& get() {
		if (index != I) {
			throw runtime_error("Wrong type");
		}
		return (TypeAt<I>&)storage;
	}
	
	size_t getIndex() const {
		return index;
	}
	
	// Takes n functors. According to the index, calls the nth functor.
	template <class... Funcs>
	void match(Funcs&&... funcs) {
		static_assert(sizeof...(Funcs) == size, 
			"Need as many functions as possible types.");
		function<void(void*)> funcArray[] = {
			Variant_impl::MakeInvoker<Ts>(std::forward<Funcs>(funcs))...
		};
		funcArray[index](&storage);
	}
	template <class... Funcs>
	void match(Funcs&&... funcs) const {
		static_assert(sizeof...(Funcs) == size, 
			"Need as many functions as possible types.");
		function<void(const void*)> funcArray[] = {
			Variant_impl::MakeInvoker<const Ts>(std::forward<Funcs>(funcs))...
		};
		funcArray[index](&storage);
	}
	
	// Takes one functor. Calls the functor with the actual type of the variant.
	template <class Fun>
	void call(Fun&& fun) {
		using funcPtr = void(*)(Fun&&, void*);
		funcPtr funcArray[] = {
			Variant_impl::Invoke1<Fun, Ts>...
		};
		funcArray[index](std::forward<Fun>(fun), &storage);
	}
	template <class Fun>
	void call(Fun&& fun) const {
		using funcPtr = void(*)(Fun&&, const void*);
		funcPtr funcArray[] = {
			Variant_impl::Invoke1<Fun, const Ts>...
		};
		funcArray[index](std::forward<Fun>(fun), &storage);
	}

	template <class Func>
	Variant<result_of_t<Func&&(Ts)>...>
		Apply(Func&& func)
	{
		return Apply_impl(
			std::forward<Func>(func),
			index_sequence_for <Ts...>{});
	}
	
	// Destructor
	~Variant() {
		destroySelf();
	}
	
	// Copy constructor
	Variant(const Variant& other) {
		other.call(Variant_impl::Copy(&storage));
		index = other.index;
	}
	
	// Move constructor
	Variant(Variant&& other) {
		other.call(Variant_impl::Move(&storage));
		index = other.index;
	}
	
	// Positional assignment
	template <size_t I>
	Variant& assign(const TypeAt<I>& item) {
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
	Variant& operator=(const Variant& rhs) {
		// A bit naive, but...
		if (this != &rhs) {
			destroySelf();
			rhs.call(Variant_impl::Copy(&storage));
			index = rhs.index;
		}
		return *this;
	}
};

struct Print {
	template <class T>
	void operator()(const T& x) {
		cout << typeid(T).name() << "{" << x << "}\n";
	}
};

struct Sqrt {
	template <class T>
	auto operator()(const T& x) {
		return sqrt(x);
	}
};

int main() {
	Variant<int, string> mything("Hello");
	mything.call(Print{});

	Variant<float, double, long double> myotherthing(Pos<2>{}, 10);
	auto what = myotherthing.Apply(Sqrt{});
	what.call(Print{});

	const Variant<string, int> whatWillThisBe;
	whatWillThisBe.call(Print{}); //int
}
