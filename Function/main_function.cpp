#include <memory>
#include <utility>
#include <type_traits>
#include <functional>
#include <iostream>

// General version
template <typename Return, typename... Args>
class Function {
	std::shared_ptr<void> functor;

	using InvokerPtr = Return(*)(void*, Args...);
	InvokerPtr invoker;

	template <class Func>
	static Return Invoke(void* func, Args... args) {
		// If we are supposed to return void, then cast the result to void;
		// if not, cast it to its natural return value (ie: don't cast it at all) and let implicit conversions handle the conversion to Return.
		// This allows us to handle int -> double, ReturningThings -> void, and forbid things like void* -> int.
		using InvocationReturn = std::result_of_t<Func&(Args...)>;
		using ReturnVoid = std::is_same<void, Return>;
		using ConvertibleReturn = std::is_convertible<InvocationReturn, Return>;
		static_assert(ReturnVoid::value || ConvertibleReturn::value, "Can't convert this invocation to Return"); // Friendlier message
		using ActualReturn = std::conditional_t<
			ReturnVoid::value,
			void,
			InvocationReturn>;
		return (ActualReturn)std::invoke(*(Func*)func, std::move(args)...);
	}

	static Return EmptyInvoker(void*, Args... args) {
		throw std::runtime_error("Empty Function");
	}

public:
	Function(nullptr_t ptr = nullptr) :
		functor(nullptr),
		invoker(&EmptyInvoker)
	{
	}

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
	//Function<int, int> whatIsThis(std::malloc);
	//auto quack = whatIsThis(4);

}
