// Generic and platform independent bitwise functions library
// Copyright (C) 2020, Artem Mikheev <c@renbou.ru>.
// Licensed under the Apache License, Version 2.0.

#ifndef CREN_BITFUNCS_H
#define CREN_BITFUNCS_H

#include <stdint.h>

/**
* Count leading zeros in an 8-bit unsigned int
*/
unsigned uint8_clz(uint8_t x);

/**
 * Count leading zeros in a 16-bit unsigned int
 */
unsigned uint16_clz(uint16_t x);

/**
 * Count leading zeros in a 32-bit unsigned int
 */
unsigned uint32_clz(uint32_t x);

/**
 * Count leading zeros in a 64-bit unsigned int
 */
unsigned uint64_clz(uint64_t x);

#endif //CREN_BITFUNCS_H
