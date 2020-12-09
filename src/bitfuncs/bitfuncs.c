// Generic and platform independent bitwise functions library
// Copyright (C) 2020, Artem Mikheev <c@renbou.ru>.
// Licensed under the Apache License, Version 2.0.

#include "bitfuncs/bitfuncs.h"

#define test_gcc(major, minor, patch) __GNUC__ > (major) || \
              (__GNUC__ == (major) && (__GNUC_MINOR__ > (minor) || \
                                 (__GNUC_MINOR__ == (minor) && \
                                  __GNUC_PATCHLEVEL__ >= (patch))))


#define clz_code(bits, power_of_two) \
	uint##bits##_t n = (bits);		 \
	for (int i = (power_of_two) - 1; i >= 0; i--) { \
		uint##bits##_t cur = ((uint##bits##_t)1) << i; \
		uint##bits##_t higher = x >> cur; \
		if (higher != 0) { \
			n -= cur; \
			x = higher; \
		} \
	} \
	return n - x;

unsigned uint8_clz(uint8_t x) {
	clz_code(8, 3)
}

unsigned uint16_clz(uint16_t x) {
	clz_code(16, 4)
}

unsigned uint32_clz(uint32_t x) {
#if test_gcc(3, 4, 0) || __clang_major__ > 5
	return x != 0 ? __builtin_clz(x) : 32;
#else
	clz_code(32, 5)
#endif
}

unsigned uint64_clz(uint64_t x) {
#if test_gcc(3, 4, 0) || __clang_major__ > 5
	return x != 0 ? __builtin_clzll(x) : 64;
#else
	clz_code(64, 6)
#endif
}
