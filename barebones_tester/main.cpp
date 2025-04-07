#include <stdlib.h>
#include <iostream>
#include "BarebonesTester.hpp"

int main(void) {
	system("../a.out &");
	system("sleep 2");

	// Run the tests
	if (test_server_get()) {
		std::cout << "Server GET test passed!" << std::endl;
	} else {
		std::cout << "Server GET test failed!" << std::endl;
	}
}