#include "Tests.h"

#include <iostream>
#include <vector>
#include <string>
#include <array>

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

bool passedAllTests() {
	std::size_t index{};
	bool failed{ false };

	TEST(testEmptyness);
	TEST(testDataAndReset);
	TEST(testEquality);
	TEST(testDimensionsAndStrides);

	return !failed;
}