#pragma once

#include <concepts>
#include <utility>
#include <array>

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
		const std::array arr{ indices... };
		std::size_t offset{};
		for (std::size_t i{}, size{ arr.size() }; i < size; ++i) {
			offset += getStridesProduct(size - i) * arr[i];
		}

		return m_begin[offset];
	}
	constexpr T& at(std::integral auto... indices) requires(sizeof...(strides) == sizeof...(indices)) {
		return const_cast<T&>(std::as_const(*this).at(indices...));
	}
	constexpr const T& operator[](std::size_t index) const requires(sizeof...(strides) == 1) {
		return at(index);
	}
	constexpr T& operator[](std::size_t index) requires(sizeof...(strides) == 1) {
		return at(index);
	}
	
	constexpr auto operator[](std::size_t index) requires(sizeof...(strides) > 1) {
		const std::size_t offset{ getStridesProduct(dimensions()) };
		T* begin{ m_begin + index * offset };
		return DropFirstPackValue_t<MDSpan, T, strides...>{ begin };
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
	static constexpr std::size_t getStridesProduct(std::size_t index) {
		std::size_t product{ 1 };
		for (std::size_t i{}; i + 1 < index; ++i) {
			product *= stride(dimensions() - 1 - i);
		}
		return product;
	}

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