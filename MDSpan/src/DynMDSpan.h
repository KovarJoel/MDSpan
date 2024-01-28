#pragma once

#include <type_traits>
#include <vector>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <ranges>

template <typename T>
class DynMDSpan {
public:
	DynMDSpan() = default;
	DynMDSpan(T* begin, T* end, std::integral auto... dimensions) {
		reset(begin, end, dimensions...);
	}
	
	const T& at(std::integral auto... indices) const {
		assert("DynMDSpan::at() -> Count of indices passed to function doesn't match dimensions of DynMDSpan!"
			&& sizeof...(indices) == dimensions());
		assert("DynMDSpan::at() -> Accessing uninitialized DynMDSpan!"
			&& m_begin && m_end);

		std::size_t i{ sizeof...(indices) };
		const std::size_t offset{ ((getStrideProduct(i--) * indices) + ...) };

		assert("DynMDSpan::at() -> Accessing DynMDSpan out of bounds!"
			&& std::cmp_less(offset, std::distance(m_begin, m_end)));

		return *(m_begin + offset);
	}
	inline T& at(std::integral auto... indices) {
		return const_cast<T&>(std::as_const(*this).at(indices...));
	}

	DynMDSpan operator[](std::size_t index) {
		assert("DynMDSpan::operator[] -> Already at single dimension!"
			&& dimensions());
		assert("DynMDSpan::operator[] -> Accessing DynMDSpan out of bounds!"
			&& index < m_strides[0]);

		const std::size_t offset{ getStrideProduct(dimensions()) };

		DynMDSpan subscript(*this);
		subscript.m_begin += index * offset;
		subscript.m_end = subscript.m_begin + offset;
		subscript.m_strides.erase(subscript.m_strides.begin());

		return subscript;
	}

	void reset(T* begin, T* end, std::integral auto... dimensions) {
		m_begin = begin;
		m_end = end;

		auto distance{ std::distance(begin, end) };
		if (distance < 0) {
			std::swap(m_begin, m_end);
			distance = -distance;
		}

		if constexpr (sizeof...(dimensions)) {
			reshape(dimensions...);
		}
		else {
			m_strides.clear();
			m_strides.push_back(static_cast<std::size_t>(distance));
		}
	}

	void reshape(std::integral auto... dimensions) {
		assert("DynMDSpan::reshape() -> Can't have a 0-dimensional DynMDSpan!"
			&& sizeof...(dimensions));
		
		m_strides.resize(sizeof...(dimensions));
		std::size_t i{};
		((m_strides[i++] = dimensions), ...);

		assert("DynMDSpan::reshape() -> Can't have a dimension of size 0!"
			&& std::ranges::none_of(m_strides, [](std::size_t dim) {return dim == 0; }));
		assert("DynMDSpan::reshape() -> DynMDSpan can't be of bigger size than array!"
			&& std::cmp_less_equal(
			std::reduce(m_strides.begin(), m_strides.end(), 1ull, std::multiplies{}),
			std::distance(m_begin, m_end)));
	}

	inline std::size_t dimensions() const {
		return m_strides.size();
	}
	inline bool empty() const {
		return !m_begin || !m_end;
	}
	inline std::size_t stride(std::size_t index) const {
		assert("DynMDSpan::stride() -> Index out of bounds!"
			&& index < dimensions());
		return m_strides[index];
	}

	inline T* begin() {
		return m_begin;
	}
	inline const T* begin() const {
		return m_begin;
	}
	inline T* end() {
		return m_end;
	}
	inline const T* end() const {
		return m_end;
	}

	inline bool operator==(const DynMDSpan& other) const {
		return m_begin == other.m_begin && m_end == other.m_begin && m_strides == other.m_strides;
	}
	inline bool operator!=(const DynMDSpan& other) const {
		return !(*this == other);
	}

private:
	inline std::size_t getStrideProduct(std::size_t index) const {
		return std::reduce(m_strides.rbegin(), m_strides.rbegin() + index - 1, 1ull, std::multiplies{});
	};

private:
	T* m_begin;
	T* m_end;
	std::vector<std::size_t> m_strides;
};