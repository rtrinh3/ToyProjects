#include <memory>
#include <utility>
#include <type_traits>
#include <functional>
#include <iostream>

// General version
template <typename Return, typename... Args>
class Function {
	std::shared_ptr<void> functor = nullptr;

	using InvokerPtr = Return(*)(void*, Args...);
	InvokerPtr invoker = nullptr;

	template <class Func>
	static Return Invoke(void* func, Args... args) {
		// If we are supposed to return void, then cast the result to void;
		// if not, cast it to its natural return value (ie: don't cast it at all) and let implicit conversions handle the conversion to Return.
		// This allows us to handle int -> double, ReturningThings -> void, and forbid things like void* -> int.
		using ActualReturn = std::result_of_t<Func&(Args...)>;
		using MaybeVoid = std::conditional_t<
			std::is_same<void, Return>::value,
			void,
			ActualReturn>;
		return (MaybeVoid)std::invoke(*(Func*)func, std::move(args)...);
	}
public:
	Function() = default;

	template <typename Func>
	Function(Func func) :
		functor(std::make_shared<Func>(std::move(func))),
		invoker(&Invoke<Func>)
	{
	}

	Return operator()(Args... args) const {
		return invoker(functor.get(), std::move(args)...);
	}
};

struct MyDestructibleThing {
	~MyDestructibleThing() {
		std::cout << "Bye!\n";
	}
	int operator()(int n) const {
		return n + 2;
	}
};

int main() {
	Function<int, int> foo(MyDestructibleThing{});
	std::cout << foo(15) << "\n";
	foo(15);
	Function<void, int> bar = foo;
	bar(6);
	bar = [](int n) {std::printf("%d\n", n); };
	bar(7);

	// Fails because void* is not implicitly convertible to int.
	/*Function<int, int> whatIsThis(std::malloc);
	auto quack = whatIsThis(4);*/

}
