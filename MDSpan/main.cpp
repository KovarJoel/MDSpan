
#include <iostream>
#include <vector>
#include "src/MDSpan.h"

int main() {
	std::vector<int> vec;
	for (int i{}; i < 1000; ++i) {
		vec.emplace_back(i);
	}

	MDSpan<int, 3, 3> span{ vec.data() };

	for (auto it2 : span) {
		for (auto valIt : it2) {
			std::cout << valIt << ", ";
		}
		std::cout << std::endl;
	}

 	return 0;
}

/*
* Should `MDSpan::operator[]` be const? It doesn't modify `*this`, however you still get access to the "pointees".
* Same for `begin()` and `end()`.
* Easier way for `DropFirstPackValue`, is there something similar in the standard?
* Is the `const_cast` in `at()` well defined, is there a cleaner way?
* Is there an alternative to marking all methods as `constexpr`?
* Is it ok that `Iterator::operator*` and `MDSpan::operator[]` return new `MDSpan`s and not refernences (its a view anyways -> lightweight)?
* Should there be bounds checking (needs an `end` pointer)? If so, rather use `assert()` or exceptions (debug or always)?
* Should the bounds checking only perform "start <= iterator < end", or check for the strides as well?
*/