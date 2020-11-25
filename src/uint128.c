// extended precision integers library for C
// Copyright (C) 2020, Artem Mikheev <c@renbou.ru>.
// Licensed under the Apache License, Version 2.0.

#include <stddef.h>
#include <string.h>
#include "integers/uint128.h"

#if COMPILER_INT128_AVAILABLE
static uint128_t UINT128_ZERO = 0;
static uint128_t UINT128_MAX = ((uint128_t)(0xffffffffffffffffull) << 64) | 0xffffffffffffffffull;
#else
static uint128_t UINT128_ZERO = {.hi = 0, .lo = 0};
static uint128_t UINT128_MAX = {.hi = 0xffffffffffffffffull, .lo = 0xffffffffffffffffull};
#endif

/// Creation, parsing

uint128_t uint128_create(const uint64_t hi, const uint64_t lo) {
#if COMPILER_INT128_AVAILABLE
	return ((uint128_t)(hi) << 64) + lo;
#else
	return (uint128_t){.hi = hi, .lo = lo};
#endif
}

uint128_t uint128_value(const uint64_t a) {
#if COMPILER_INT128_AVAILABLE
	return (uint128_t)(a);
#else
	return (uint128_t){.hi = 0, .lo = a};
#endif
}

/* Parse one character into a decimal number and return -1 if we fail */
int64_t parse_decimal_digit(const char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	return -1;
}

/* Recursively parses a string of dcimal characters into a 128-bit uint
 * If the parsed value is too big returns the maximum-value-uint128
 * If the string can't be parsed as decimal returns a 0
 * !The string passed into this function must not start with zeroes, skip them first using find_first_non_zero
 */
uint128_t parse_from_decimal(const char *string, uint128_t previous_value, const size_t num_digits) {
	if (string == NULL || string[0] == '\0')
		return previous_value;

	if (num_digits + 1 > INT128_DECIMAL_SIZE) {
		return UINT128_MAX;
	}

	int64_t current_digit = parse_decimal_digit(string[0]);
	if (current_digit < 0)
		return UINT128_ZERO;

	uint128_t next = uint128_add_uint64(uint128_multiply_uint64(previous_value, 10), (uint64_t)current_digit);

	// if the first decimal digit isn't equal to the current, then we must've overflown
	if (uint128_get_lower(next) % 10 != (uint64_t)current_digit)
		return UINT128_MAX;

	return parse_from_decimal(string + 1, next,num_digits + 1);
}

/* Parse one character into a hex number and return -1 if we fail */
int64_t parse_hex_digit(const char c) {
	if (c >= 'a' && c <= 'f')
		return (c - 'a') + 10;
	if (c >= 'A' && c <= 'F')
		return (c - 'A') + 10;
	return parse_decimal_digit(c);
}

/* Recursively parses a string of hex characters into a 128-bit uint
 * If the parsed value is too big returns the maximum-value-uint128
 * If the string can't be parsed as hex returns a 0
 * !The string passed into this function must not start with zeroes, skip them first using find_first_non_zero
 */
uint128_t parse_from_hex(const char *string, uint128_t previous_value, const size_t num_digits) {
	if (string == NULL || string[0] == '\0')
		return previous_value;

	if (num_digits + 1 > 2 * SIZEOF_INT128) {
		return UINT128_MAX;
	}

	int64_t current_digit = parse_hex_digit(string[0]);
	if (current_digit < 0)
		return UINT128_ZERO;
	return parse_from_hex(string + 1,
						  uint128_or_uint64(uint128_shift_left(previous_value, 4), (uint64_t)current_digit),
						  num_digits + 1);
}

/* Parse one character into an octal number and return -1 if we fail */
int64_t parse_octal_digit(const char c) {
	if (c >= '0' && c <= '7')
		return c - '0';
	return -1;
}

/* Recursively parses a string of octal characters into a 128-bit uint
 * If the parsed value is too big returns the maximum-value-uint128
 * If the string can't be parsed as octal returns a 0
 * !The string passed into this function must not start with zeroes, skip them first using find_first_non_zero
 */
uint128_t parse_from_octal(const char *string, uint128_t previous_value, const size_t num_digits) {
	if (string == NULL || string[0] == '\0')
		return previous_value;

	if (num_digits + 1 > 4 * SIZEOF_INT128) {
		return UINT128_MAX;
	}

	int64_t current_digit = parse_octal_digit(string[0]);
	if (current_digit < 0)
		return UINT128_ZERO;
	return parse_from_octal(string + 1,
						  uint128_or_uint64(uint128_shift_left(previous_value, 3), (uint64_t)current_digit),
						  num_digits + 1);
}

/* Parse one character into an binary number and return -1 if we fail */
int64_t parse_binary_digit(const char c) {
	if (c == '0' || c == '1')
		return c - '0';
	return -1;
}

/* Recursively parses a string of binary characters into a 128-bit uint
 * If the parsed value is too big returns the maximum-value-uint128
 * If the string can't be parsed as hex returns a 0
 * !The string passed into this function must not start with zeroes, skip them first using find_first_non_zero
 */
uint128_t parse_from_binary(const char *string, uint128_t previous_value, const size_t num_digits) {
	if (string == NULL || string[0] == '\0')
		return previous_value;

	if (num_digits + 1 > 8 * SIZEOF_INT128) {
		return UINT128_MAX;
	}

	int64_t current_digit = parse_binary_digit(string[0]);
	if (current_digit < 0)
		return UINT128_ZERO;
	return parse_from_binary(string + 1,
						  uint128_or_uint64(uint128_shift_left(previous_value, 1), (uint64_t)current_digit),
						  num_digits + 1);
}

/* Function that skips characters of the supposedly number string until it finds a supposedly non-zero value */
const char * find_first_non_zero(const char *string) {
	if (string == NULL)
		return NULL;
	while(*string == '0')
		string++;
	return string;
}

uint128_t uint128_parse(const char *string) {
	if (string == NULL) {
		return_zero:
		return UINT128_ZERO;
	}
	const size_t string_length = strlen(string);
	if (string_length == 0) {
		goto return_zero;
	}

	// Determine the type of string we will be parsing
	// hex
	if (string_length > 2 && string[0] == '0' && (string[1] == 'x' || string[1] == 'X')) {
		return parse_from_hex(find_first_non_zero(string + 2), UINT128_ZERO, 0);
	}
	// octal
	if (string_length > 2 && string[0] == '0' && string[1] == 'o') {
		return parse_from_octal(find_first_non_zero(string + 2), UINT128_ZERO, 0);
	}
	// binary
	if (string_length > 2 && string[0] == '0' && string[1] == 'b') {
		return parse_from_binary(find_first_non_zero(string + 2), UINT128_ZERO, 0);
	}
	// decimal
	return parse_from_decimal(find_first_non_zero(string), UINT128_ZERO, 0);
}

/// Lower and higher bits of 128-bit ints

uint64_t uint128_get_lower(const uint128_t a) {
#if COMPILER_INT128_AVAILABLE
	return (uint64_t)(a);
#else
	return a.lo;
#endif
}

uint64_t uint128_get_higher(const uint128_t a) {
#if COMPILER_INT128_AVAILABLE
	return (uint64_t)(a >> 64);
#else
	return a.hi;
#endif
}

/// Bitwise operations

uint128_t uint128_shift_left(const uint128_t a, const unsigned int shift) {
#if COMPILER_INT128_AVAILABLE
	return (a << shift);
#else
	return (shift < 64) ?
		   		// Set hi to the shift of hi and the value of the left part that has shifted
		   		// If the shift is 0, 64-shift wouldn't do anything, which is why we split it into two shifts
				(uint128_t){.hi = (a.hi << shift) | ((a.lo >> 1) >> (63 - shift)), .lo = a.lo << shift} :
				// If we have shifted more than 64 bits to the left, then only the lower bits will be left,
				// and if we shift 128+, then the number will just be a zero
		   		(shift < 128) ? (uint128_t){.hi = a.lo << (shift - 64), .lo = 0} : UINT128_ZERO;
#endif
}


uint128_t uint128_shift_right(const uint128_t a, const unsigned int shift) {
#if COMPILER_INT128_AVAILABLE
	return (a >> shift);
#else
	// Like with left shift but reversed
	return (shift < 64) ?
		   (uint128_t){.hi = a.hi >> shift, .lo = (a.lo >> shift) || ((a.hi << 1) << (63 - shift))} :
		   (shift < 128) ? (uint128_t){.hi = 0,  .lo = a.hi >> (shift - 64)} : UINT128_ZERO;
#endif
}

uint128_t uint128_or(const uint128_t a, const uint128_t b) {
#if COMPILER_INT128_AVAILABLE
	return a | b;
#else
	return (uint128_t){.hi = a.hi | b.hi, .lo = a.lo | b.lo};
#endif
}

uint128_t uint128_or_uint64(const uint128_t a, const uint64_t b) {
#if COMPILER_INT128_AVAILABLE
	return a | b;
#else
	return (uint128_t){.hi = a.hi, .lo = a.lo | b};
#endif
}

/* Struct defining a result of an operation along with a carry "bit" which resulted from that operation */
typedef struct uint64_with_carry {
	const uint64_t value;
	const int carry;
} uint64_with_carry;

/// Addition

/* Adds two uint64's and adds the previous carry, as well as calculates the new carry */
uint64_with_carry uint64_add_with_carry(const uint64_t a, const uint64_t b, const int carry) {
	const uint64_t result_without_carry = a + b;
	// Find out if the first addition resulted in a carry
	const int new_carry1 = result_without_carry < a;
	const uint64_t result = result_without_carry + carry;
	// Find out if adding the previous carry has resulted in a carry
	const int new_carry2 = result < result_without_carry;
	return (uint64_with_carry){.value = result, .carry = new_carry1 | new_carry2};
}

uint128_t uint128_add(const uint128_t a, const uint128_t b) {
#if COMPILER_INT128_AVAILABLE
	return a + b;
#else
	uint64_with_carry lo = uint64_add_with_carry(a.lo, b.lo, 0);
	uint64_with_carry hi = uint64_add_with_carry(a.hi, b.hi, lo.carry);
	return (uint128_t){.hi = hi.value, .lo = lo.value};
#endif
}

uint128_t uint128_add_uint64(const uint128_t a, const uint64_t b) {
#if COMPILER_INT128_AVAILABLE
	return a + b;
#else
	uint64_with_carry lo = uint64_add_with_carry(a.lo, b, 0);
	// in order to not have to call the carry add function again, we can simply add the carry from the previous result,
	// since b doesn't have bits higher than the 64'th
	return (uint128_t){.hi = a.hi + lo.carry, .lo = lo.value};
#endif
}

/// Subtraction

/* Subtracts two uint64's and subs the previous carry, as well as calculates the new carry */
uint64_with_carry uint64_sub_with_carry(const uint64_t a, const uint64_t b, const int carry) {
	const uint64_t result_without_carry = a - b;
	// Find out if the first subtraction resulted in a carry
	const int new_carry1 = result_without_carry > a;
	const uint64_t result = result_without_carry - carry;
	// Find out if the second addition resulted in a carry
	const int new_carry2 = result > result_without_carry;
	return (uint64_with_carry){.value = result, .carry = new_carry1 | new_carry2};
}

uint128_t uint128_subtract(const uint128_t a, const uint128_t b) {
#if COMPILER_INT128_AVAILABLE
	return a - b;
#else
	uint64_with_carry lo = uint64_sub_with_carry(a.lo, b.lo, 0);
	uint64_with_carry hi = uint64_sub_with_carry(a.hi, b.hi, lo.carry);
	return (uint128_t){.hi = hi.value, .lo = lo.value};
#endif
}

uint128_t uint128_subtract_uint64(const uint128_t a, const uint64_t b) {
#if COMPILER_INT128_AVAILABLE
	return a - b;
#else
	uint64_with_carry lo = uint64_sub_with_carry(a.lo, b, 0);
	// in order to not have to call the carry sub function again, we can simply sub the carry from the previous result,
	// since b doesn't have bits higher than the 64'th
	return (uint128_t){.hi = a.hi - lo.carry, .lo = lo.value};
#endif
}

/// Multiplication

uint128_t uint64_multiply(const uint64_t a, const uint64_t b) {
#if COMPILER_INT128_AVAILABLE
	return ((uint128_t)a) * b;
#else
	const uint64_t a_lo = a & 0xffffffff;
	const uint64_t a_hi = a >> 32;
	const uint64_t b_lo = b & 0xffffffff;
	const uint64_t b_hi = b >> 32;

	// Multiply the different parts of the 64 bit numbers in order to correctly identify carry's
	const uint64_t part0 = a_lo * b_lo;
	const uint64_t part1 = a_hi * b_lo;
	const uint64_t part2 = a_lo * b_hi;
	const uint64_t part3 = a_hi * b_hi;

	// Identify what will carry over into the upper bits of the 128-bit integer
	const uint64_t lower_parts_carry = part1 + (part0 >> 32);
	// This will also tell us what has carried into the upper 32 bits of the lower 64 bits of the 128-bit integer
	const uint64_t upper_parts_carry = part2 + (lower_parts_carry & 0xffffffff);

	// combine all the results into lower and higher bits
	const uint64_t result_lo = (upper_parts_carry << 32) | (part0 & 0xffffffff);
	const uint64_t result_hi = part3 + (upper_parts_carry >> 32) + (lower_parts_carry >> 32);

	return (uint128_t){.hi = result_hi, .lo = result_lo};
#endif
}

uint128_t uint128_multiply(const uint128_t a, const uint128_t b) {
#if COMPILER_INT128_AVAILABLE
	return a * b;
#else
	// Multiply the lower bits properly, and then simply multiply the parts that can be in our higher bits
	uint128_t lo_multiplication_result = uint64_multiply(a.lo, b.lo);
	uint128_t result = {.hi = lo_multiplication_result.hi + (a.lo * b.hi) + (a.hi * b.lo),
					   	  .lo = lo_multiplication_result.lo};
	return result;
#endif
}

uint128_t uint128_multiply_uint64(const uint128_t a, const uint64_t b) {
#if COMPILER_INT128_AVAILABLE
	return a * (uint128_t)(b);
#else
	// Multiply the lower bits properly, and then simply multiply the parts that can be in our higher bits
	uint128_t lo_multiplication_result = uint64_multiply(a.lo, b);
	uint128_t result = {.hi = lo_multiplication_result.hi + (a.hi * b),
		.lo = lo_multiplication_result.lo};
	return result;
#endif
}

uint128_t uint128_increment(const uint128_t a) {
#if COMPILER_INT128_AVAILABLE
	return a + 1;
#else
	const uint64_t new_lo = a.lo + 1;
	const unsigned int carry = new_lo < a.lo;
	return (uint128_t){.hi = a.hi + carry, .lo = new_lo};
#endif
}

uint128_t uint128_decrement(const uint128_t a) {
#if COMPILER_INT128_AVAILABLE
	return a - 1;
#else
	const uint64_t new_lo = a.lo - 1;
	const unsigned int carry = new_lo > a.lo;
	return (uint128_t){.hi = a.hi - carry, .lo = new_lo};
#endif
}
