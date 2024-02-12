
#include <iostream>
#include "tests/Tests.h"

int main() {

	if (!passedAllTests()) {
		__debugbreak();
	}

	return 0;
}