#include "Tests.h"

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <numeric>
#include <functional>

#include "../src/MDSpan.h"

#define TEST(func)\
do {\
	++index;\
	std::cout << "Running test nr " << index << ": " #func "()\n\t";\
	if (func()) {\
		std::cout << "passed" << std::endl;\
	}\
	else {\
		std::cout << "FAILED!" << std::endl;\
		failed = true;\
	}\
} while(0)

static bool testEmptyness() {
	std::array<int, 2> arr{ 0, 1 };

	MDSpan<int, 1> span;
	if (span.data() || !span.empty() || span) {
		return false;
	}

	span.reset(arr.data());
	if (!span.data() || span.empty() || !span) {
		return false;
	}

	span.reset(nullptr);
	if (span.data() || !span.empty() || span) {
		return false;
	}

	MDSpan<int, 4> defaultInit{};
	if (span.data() || !span.empty() || span) {
		return false;
	}

	MDSpan<int, 1, 2> initializedSpan{ arr.data() };
	if (!initializedSpan.data() || initializedSpan.empty() || !initializedSpan) {
		return false;
	}

	return true;
}
static bool testDataAndReset() {
	std::array<int, 2> arr{ 1, 2 };

	MDSpan<int, 1> span{ arr.data() };
	if (span.data() != arr.data()) {
		return false;
	}
	span.reset(nullptr);
	if (span.data() != nullptr) {
		return false;
	}
	span.reset(arr.data() + 1);
	if (span.data() != arr.data() + 1) {
		return false;
	}

	MDSpan<int, 1> defaultInit{};
	if (defaultInit.data() != nullptr) {
		return false;
	}

	MDSpan<int, 1> noInit;
	if (noInit.data() != nullptr) {
		return false;
	}

	return true;
}
static bool testEquality() {
	std::array<int, 1> arr{ 1 };

	MDSpan<int, 1, 2, 3> span3d{ arr.data() };
	MDSpan<int, 1, 2, 3, 4> span4d{ arr.data() };

	if (span3d != span4d || !(span3d == span4d)) {
		return false;
	}

	MDSpan<int, 1> empty1;
	MDSpan<int, 2> empty2{};
	if (empty1 != empty2 || !(empty1 == empty2)) {
		return false;
	}

	MDSpan<int, 1> arr0{ arr.data() };
	MDSpan<int, 1> arr1{ arr.data() + 1 };
	if (arr0 == arr1 || !(arr0 != arr1)) {
		return false;
	}

	return true;
}
static bool testDimensionsAndStrides() {
	MDSpan<int, 0> s1;
	if (s1.dimensions() != 1
		|| s1.stride(0) != 0 || s1.stride(s1.dimensions() - 1) != 0) {
		return false;
	}

	MDSpan<int, 1, 2, 3> s2;
	if (s2.dimensions() != 3
		|| s2.stride(0) != 1 || s2.stride(s2.dimensions() - 3) != 1
		|| s2.stride(1) != 2 || s2.stride(s2.dimensions() - 2) != 2
		|| s2.stride(2) != 3 || s2.stride(s2.dimensions() - 1) != 3) {
		return false;
	}

	return true;
}
static bool testAccess() {
	std::array<int, 6> arr{ 0, 1, 2, 3, 4, 5 };
	MDSpan<int, 2, 3> span{ arr.data() };

	for (std::size_t i{}; i < arr.size(); ++i) {
		// operator[] does not check for out of bounds
		if (arr[i] != span[0][i]) {
			return false;
		}
	}

	for (std::size_t i{}; i < 2; ++i) {
		for (std::size_t j{}; j < 3; ++j) {
			if (arr[i * 3 + j] != span[i][j]
				|| arr[i * 3 + j] != span.at(i, j)
				|| arr[i * 3 + j] != span[i].at(j)) {
				return false;
			}
		}
	}

#define INDICES_DATA 100, 8, 5, 25, 2
	constexpr std::size_t vecSize{ 200'000 };
	std::vector<int> vec{};
	vec.resize(vecSize);
	for (std::size_t i{}; i < vecSize; ++i) {
		vec[i] = static_cast<int>(i);
	}

	constexpr int indices[] = { INDICES_DATA };
	static_assert(std::accumulate(std::begin(indices), std::end(indices), 1, std::multiplies{}) == vecSize);
	MDSpan<int, INDICES_DATA> hugeSpan{ vec.data() };
	std::size_t offset{};
	for (int a = 0; a < indices[0]; ++a) {
		for (int b = 0; b < indices[1]; ++b) {
			for (int c = 0; c < indices[2]; ++c) {
				for (int d = 0; d < indices[3]; ++d) {
					for (int e = 0; e < indices[4]; ++e) {
						int subscript = hugeSpan[a][b][c][d][e];
						int at = hugeSpan.at(a, b, c, d, e);
						int both = hugeSpan[a][b][c].at(d, e);

						const auto& cref = hugeSpan;
						int constAt = cref.at(a, b, c, d, e);
						// operator[] is not const for multidimensionals!

						if (vec[offset] != subscript
							|| vec[offset] != at
							|| vec[offset] != both
							|| vec[offset] != constAt) {
							return false;
						}

						++offset;
					}
				}
			}
		}
	}
#undef INDICES_DATA

	return true;
}
static bool testAccessOutOfBounds() {
	std::array<int, 10> arr{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	MDSpan<int, 2, 5> span{ arr.data() };

	// should not throw
	try {
		for (std::size_t i{}; i < arr.size(); ++i) {
			span[i][i + -static_cast<int>(span.stride(span.dimensions() - 1) * i)];
			span[0][i];
		}
	}
	catch (...) {
		return false;
	}

	// should throw
	for (std::size_t i{}; i < arr.size(); ++i) {
		const std::size_t stride = span.stride(span.dimensions() - 1);
		try {
			span[0].at(i);
			if (i >= stride) {
				return false;
			}
		}
		catch (...) {
			if (i < stride) {
				return false;
			}
		}
	}

	try {
		span.at(2, 0);
		return false;
	}
	catch (...) {}
	try {
		span.at(0, 5);
		return false;
	}
	catch (...) {}
	try {
		span.at(0, -1);
		return false;
	}
	catch (...) {}
	try {
		span.at(-1, 0);
		return false;
	}
	catch (...) {}

	return true;
}

bool passedAllTests() {
	std::size_t index{};
	bool failed{ false };

	TEST(testEmptyness);
	TEST(testDataAndReset);
	TEST(testEquality);
	TEST(testDimensionsAndStrides);
	TEST(testAccess);
	TEST(testAccessOutOfBounds);

	return !failed;
}