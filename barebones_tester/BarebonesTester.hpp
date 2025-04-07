#include <iostream>
#include <string>
#include <cstdlib>

#define TEST_CASE(name) void name()

#define ASSERT(condition) do { \
	if (!(condition)) { \
		std::cerr << "ASSERTION FAILED: " << #condition << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
		exit(1); \
	} \
} while (0)

#define ASSERT_EQ(a, b) do { \
	if (!((a) == (b))) { \
		std::cerr << "ASSERTION FAILED: " << #a << " == " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
		std::cerr << "  " << #a << " = " << (a) << std::endl; \
		std::cerr << "  " << #b << " = " << (b) << std::endl; \
		exit(1); \
	} \
} while (0)