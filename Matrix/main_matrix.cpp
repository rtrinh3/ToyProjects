#include <vector>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <cassert>
#include <functional>

template <typename T>
class DynamicMatrix {
private:
	std::vector<T> elements_;
	std::vector<size_t> bounds_;

	bool checkIndices(const std::vector<size_t>& indices) const {
		// Check if all indices are smaller than their corresponding bound.
		// ie: true && indices[0] < bounds_[0] && indices[1] < bounds_[1] && ...
		return std::inner_product(
			indices.begin(), 
			indices.end(), 
			bounds_.begin(), 
			true, 
			std::logical_and<bool>{}, // The "sum"
			std::less<size_t>{} // The "product"
		);
	}

public:
	DynamicMatrix(const std::vector<size_t>& bounds) :
		bounds_(bounds),
		elements_(std::accumulate(bounds.begin(), bounds.end(), size_t(1), std::multiplies<size_t>{}))
	{
	}

	const T& get(const std::vector<size_t>& indices) const {
		assert(indices.size() == bounds_.size());
		assert(checkIndices(indices));

		// Calculate index
		// http://stackoverflow.com/a/678997
		size_t index = 0;
		size_t multiplier = 1;
		const size_t dimensions = bounds_.size();
		auto rindices = indices.crbegin();
		auto rbound = bounds_.crbegin();
		for (size_t i = 0; i < dimensions; i++)
		{
			index += rindices[i] * multiplier;
			multiplier *= rbound[i];
		}

		return elements_[index];
	}

	T& get(const std::vector<size_t>& indices) {
		return const_cast<T&>(const_cast<const DynamicMatrix*>(this)->get(indices));
	}

	// Internal iteration
	template <typename Func>
	void forEach(Func&& func) const {
		std::vector<size_t> indices(bounds_.size());
		auto it = elements_.begin();
		while (indices.front() < bounds_.front()) {
			// Call function with indices and element
			std::forward<Func>(func)(indices, *it);

			// Increment indices
			indices.back()++;
			for (size_t i = bounds_.size() - 1; 0 < i; --i) {
				if (indices[i] >= bounds_[i]) {
					indices[i] = 0;
					indices[i - 1]++;
				} else {
					break;
				}
			}

			// Increment iterator
			++it;
		}
	}
	
	template <typename Func>
	void forEach(Func&& func) {
		const_cast<const DynamicMatrix*>(this)->forEach(
			[&](const auto& indices, const T& item) {
				std::forward<Func>(func)(indices, const_cast<T&>(item));
			}
		);
	}
};

#include <tuple>
#include <array>
template <typename T, class ParameterTuple, size_t... Bounds>
class StaticMatrixImpl;

template <size_t... N>
struct Product;

template <size_t Head, size_t... Tail>
struct Product<Head, Tail...> {
	constexpr static size_t value = Head * Product<Tail...>::value;
};

template <>
struct Product<> {
	constexpr static size_t value = size_t(1);
};

template <typename T, typename... Parameters, size_t... Bounds>
class StaticMatrixImpl<T, std::tuple<Parameters...>, Bounds...>
{
private:
	std::array<T, Product<Bounds...>::value> elements_;

	bool checkIndices(Parameters... indexes) const {
		std::array<const size_t, sizeof...(Bounds)> indices{ indexes... };
		std::array<const size_t, sizeof...(Bounds)> bounds{ Bounds... };
		// Check if all indices are smaller than their corresponding bound.
		// ie: true && indices[0] < bounds_[0] && indices[1] < bounds_[1] && ...
		return std::inner_product(
			indices.begin(),
			indices.end(),
			bounds.begin(),
			true,
			std::logical_and<bool>{}, // The "sum"
			std::less<size_t>{} // The "product"
		);
	}
public:
	const T& get(Parameters... indexes) const {
		assert(checkIndices(indexes...));
		std::array<const size_t, sizeof...(Bounds)> indices{ indexes... };
		std::array<const size_t, sizeof...(Bounds)> bounds{ Bounds... };

		// Calculate index
		// http://stackoverflow.com/a/678997
		size_t index = 0;
		size_t multiplier = 1;
		const size_t dimensions = sizeof...(Bounds);
		auto rindices = indices.crbegin();
		auto rbound = bounds.crbegin();
		for (size_t i = 0; i < dimensions; i++)
		{
			index += rindices[i] * multiplier;
			multiplier *= rbound[i];
		}

		return elements_[index];
	}

	T& get(Parameters... indexes) {
		return const_cast<T&>(const_cast<const StaticMatrixImpl*>(this)->get(indexes...));
	}

	const T& operator()(Parameters... indexes) const {
		return get(indexes...);
	}

	T& operator()(Parameters... indexes) {
		return get(indexes...);
	}

	// Internal iteration
	template <typename Func>
	void forEach(Func&& func) const {
		std::array<size_t, sizeof...(Bounds)> indices{ 0 };
		std::array<const size_t, sizeof...(Bounds)> bounds{ Bounds... };
		auto it = elements_.begin();
		while (indices.front() < bounds.front()) {
			// Call function with indices and element
			std::forward<Func>(func)(indices, *it);

			// Increment indices
			indices.back()++;
			for (size_t i = bounds.size() - 1; 0 < i; --i) {
				if (indices[i] >= bounds[i]) {
					indices[i] = 0;
					indices[i - 1]++;
				} else {
					break;
				}
			}

			// Increment iterator
			++it;
		}
	}

	template <typename Func>
	void forEach(Func&& func) {
		const_cast<const StaticMatrixImpl*>(this)->forEach(
			[&](const auto& indices, const T& item) {
				std::forward<Func>(func)(indices, const_cast<T&>(item));
			}
		);
	}
};

template <size_t N, typename... Ts>
struct N_Times_Size_T : N_Times_Size_T<N - 1, size_t, Ts...>
{
};

template <typename... Ts>
struct N_Times_Size_T<0, Ts...> {
	using type = std::tuple<Ts...>;
};

template <typename T, size_t... Bounds>
using StaticMatrix = StaticMatrixImpl<T, typename N_Times_Size_T<sizeof...(Bounds)>::type, Bounds...>;

#include <iostream>
int main() {
	DynamicMatrix<short> hi({ 1, 2, 3 });
	short m = 1;
	for (size_t i = 0; i < 1; ++i) {
		for (size_t j = 0; j < 2; ++j) {
			for (size_t k = 0; k < 3; ++k) {
				hi.get({ i, j, k }) = m;
				++m;
			}
		}
	}

	hi.forEach([](auto&& indices, short n) {
		for (size_t i : indices) {
			std::cout << i << ',';
		}
		std::cout << '\t' << n << '\n';
	});

	StaticMatrix<short, 1, 2, 3> temp;
	short lang[1][2][3];
	short n = 1;
	for (size_t i = 0; i < 1; ++i) {
		for (size_t j = 0; j < 2; ++j) {
			for (size_t k = 0; k < 3; ++k) {
				temp(i, j, k) = n;
				lang[i][j][k] = n;
				++n;
			}
		}
	}

	auto cmp = std::memcmp(&temp, &lang, sizeof(short) * 1 * 2 * 3);
	std::cout << cmp << '\n'; // Zero means they are identical

	temp.forEach([](auto&& indices, short n) {
		const size_t i = indices[0], j = indices[1], k = indices[2];
		std::cout << i << ',' << j << ',' << k << " \t" << n << '\n';
	});
}
