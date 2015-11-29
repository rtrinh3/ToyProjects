#pragma once
#include <type_traits>
#include <utility>

enum ContainerQuery { Uninheritable, Inheritable, Empty };
template <typename T>
constexpr ContainerQuery ContainerCheckType() {
	return (std::is_final<T>::value || !std::is_class<T>::value)
		? ContainerQuery::Uninheritable
		: (std::is_empty<T>::value && std::is_default_constructible<T>::value)
		? ContainerQuery::Empty
		: ContainerQuery::Inheritable;
};

template <typename T, size_t Tag = 0, ContainerQuery isFinal = ContainerCheckType<T>()>
class Container {
private:
	static_assert(isFinal == ContainerQuery::Uninheritable, "How is this Uninheritable?");
	T val;
public:
	Container(T value) :
		val(std::move(value))
	{
	}
	T& get() {
		return val;
	}
	const T& get() const {
		return val;
	}
};

template <typename T, size_t Tag>
class Container<T, Tag, ContainerQuery::Inheritable> :
	private T
{
public:
	Container(T value) :
		T(std::move(value))
	{
	}
	T& get() {
		return *this;
	}
	const T& get() const {
		return *this;
	}
};

template <typename T, size_t Tag>
class Container<T, Tag, ContainerQuery::Empty>
{
public:
	Container(const T& value) {
	}
	T get() const {
		return T{};
	}
};
