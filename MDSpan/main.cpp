
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