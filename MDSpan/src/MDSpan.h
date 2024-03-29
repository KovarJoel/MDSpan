#pragma once

#include <concepts>
#include <utility>
#include <array>
#include <stdexcept>
#include <string>

template <template<typename, auto...> typename U, typename V, auto first, auto... others>
struct DropFirstPackValue {
	using type = U<V, others...>;
};
template <template<typename, auto...> typename U, typename V, auto first, auto... others>
using DropFirstPackValue_t = DropFirstPackValue<U, V, first, others...>::type;

template <typename T, std::size_t... strides>
	requires (sizeof...(strides) > 0)
class MDSpan {
public:
	constexpr MDSpan()
		: m_begin{ nullptr } {
	}
	constexpr MDSpan(T* begin)
		: m_begin{ begin } {
	}
	
	constexpr void reset(T* begin) {
		m_begin = begin;
	}
	
	constexpr const T& at(std::integral auto... indices) const requires(sizeof...(strides) == sizeof...(indices)) {
		return m_begin[getAtIndexWithException(indices...)];
	}
	constexpr T& at(std::integral auto... indices) requires(sizeof...(strides) == sizeof...(indices)) {
		return m_begin[getAtIndexWithException(indices...)];
	}
	constexpr const T& operator[](std::size_t index) const requires(sizeof...(strides) == 1) {
		return m_begin[getAtIndexWithoutException(index)];
	}
	constexpr T& operator[](std::size_t index) requires(sizeof...(strides) == 1) {
		return m_begin[getAtIndexWithoutException(index)];
	}
	
	constexpr auto at(std::integral auto... indices) requires(sizeof...(strides) > sizeof...(indices)) {
		return atSpanHelperWithException(indices...);
	}
	constexpr auto operator[](std::size_t index) requires(sizeof...(strides) > 1) {
		return atSpanHelperWithoutException(index);
	}

	constexpr T* data() {
		return m_begin;
	}
	constexpr auto begin() {
		return Iterator{ *this };
	}
	constexpr auto end() {
		return Iterator{ *this, stride(0) };
	}

	static constexpr std::size_t dimensions() {
		return m_strides.size();
	}
	static constexpr std::size_t stride(std::size_t index) {
		return m_strides[index];
	}

	constexpr bool empty() const {
		return !m_begin;
	}
	constexpr operator bool() const {
		return !empty();
	}

	constexpr bool operator==(const MDSpan& other) const {
		return m_begin == other.m_begin;
	}
	constexpr bool operator!=(const MDSpan& other) const {
		return !(*this == other);
	}

private:
	static void throwRangeError(std::size_t size, std::size_t index) {
		std::string errorMessage{ "MDSpan::at() access out of bounds! Size: "};
		errorMessage += std::to_string(size);
		errorMessage += ", Index: " + std::to_string(index);
		throw std::out_of_range(errorMessage.c_str());
	}

	static constexpr std::size_t getStridesProduct(std::size_t index) {
		std::size_t product{ 1 };
		for (std::size_t i{}; i + 1 < index; ++i) {
			product *= stride(dimensions() - 1 - i);
		}
		return product;
	}
	
	template <bool useException>	// since non base case needs template
	static constexpr auto atSpanHelper(auto span) {
		return span;
	}
	template <bool useException, std::size_t... s>
	static constexpr auto atSpanHelper(MDSpan<T, s...> span, std::size_t firstIndex, std::integral auto... indices) {
		if constexpr (useException) {
			if (firstIndex >= stride(0)) {
				throwRangeError(stride(0), firstIndex);
			}
		}

		constexpr std::size_t offset{ getStridesProduct(span.dimensions()) };
		T* begin{ span.m_begin + firstIndex * offset };
		auto subView = DropFirstPackValue_t<MDSpan, T, s...>{begin};
		return atSpanHelper<useException>(subView, indices...);
	}

	constexpr auto atSpanHelperWithException(std::integral auto... indices) requires(sizeof...(strides) > sizeof...(indices)) {
		return atSpanHelper<true>(*this, indices...);
	}
	constexpr auto atSpanHelperWithoutException(std::integral auto... indices) requires(sizeof...(strides) > sizeof...(indices)) {
		return atSpanHelper<false>(*this, indices...);
	}

	template <bool useException>
	static constexpr std::size_t getAtIndex(std::integral auto... indices) requires (sizeof...(strides) == sizeof...(indices)) {
		const std::array arr{ indices... };
		std::size_t offset{};
		for (std::size_t i{}, size{ arr.size() }; i < size; ++i) {
			if constexpr (useException) {
				if (arr[i] >= stride(i)) {
					throwRangeError(stride(i), arr[i]);
				}
			}
			offset += getStridesProduct(size - i) * arr[i];
		}

		return offset;
	}
	static constexpr std::size_t getAtIndexWithException(std::integral auto... indices) requires (sizeof...(strides) == sizeof...(indices)) {
		return getAtIndex<true>(indices...);
	}
	static constexpr std::size_t getAtIndexWithoutException(std::integral auto... indices) requires (sizeof...(strides) == sizeof...(indices)) {
		return getAtIndex<false>(indices...);
	}

private:
	// to access privates of MDSpans with different type/strides
	friend class MDSpan;

private:
	T* m_begin;
	inline static constexpr std::array m_strides{ strides... };

private:
	class Iterator {
	public:
		constexpr Iterator operator++(int) {
			return Iterator{ m_owner, m_index++ };
		}
		constexpr Iterator operator++() {
			++m_index;
			return *this;
		}
		constexpr Iterator operator--(int) {
			return Iterator{ m_owner, m_index-- };
		}
		constexpr Iterator operator--() {
			--m_index;
			return *this;
		}

		constexpr auto operator*() requires(sizeof...(strides) > 1) {
			return m_owner[m_index];
		}
		constexpr T& operator*() requires(sizeof...(strides) == 1) {
			return m_owner[m_index];
		}
		constexpr const T& operator*() const requires(sizeof...(strides) == 1) {
			return m_strides[m_index];
		}

		constexpr bool operator==(const Iterator& other) const {
			return m_owner == other.m_owner && m_index == other.m_index;
		}
		constexpr bool operator!=(const Iterator& other) const {
			return !(*this == other);
		}

	private:
		constexpr Iterator(MDSpan& owner, std::size_t index = 0)
			: m_owner{ owner }, m_index{ index } {
		}
		friend class MDSpan;

	private:
		MDSpan& m_owner;
		std::size_t m_index;
	};
};