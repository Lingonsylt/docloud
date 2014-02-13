#ifndef TEST_H
#define TEST_H
#include <stdio.h>

#define TEST_ERR(test_str) { \
	printf("ERROR - %s\n", test_str); \
}

#define TEST_PASS(test_str) { \
		printf("PASS - %s\n", test_str); \
	}

#define TEST(test_str, x) { \
		if (!x) { \
			TEST_ERR(test_str); \
		} else { \
			TEST_PASS(test_str); \
		} \
	}
#endif /* end of include guard: TEST_H */
