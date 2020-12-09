// extended precision integers library for C
// Copyright (C) 2020, Artem Mikheev <c@renbou.ru>.
// Licensed under the Apache License, Version 2.0.

#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "integers/uint128.h"
#include "bitfuncs/bitfuncs.h"

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

/* Iteratively parses a string of dcimal characters into a 128-bit uint
 * If the parsed value is too big returns the maximum-value-uint128
 * If the string can't be parsed as decimal returns a 0
 * !The string passed into this function must not start with zeroes, skip them first using find_first_non_zero
 */
uint128_t parse_from_decimal(const char *string) {
	if (string == NULL)
		return UINT128_ZERO;

	uint128_t value = UINT128_ZERO;
	for (size_t num_digits = 0; *string; string++, num_digits++) {
		// find out how many digits we can have at maximum, divide with round-up
		if (num_digits + 1 > INT128_DECIMAL_SIZE)
			return UINT128_MAX;

		int64_t current_digit = parse_decimal_digit(*string);
		if (current_digit < 0)
			return UINT128_ZERO;

		value = uint128_add_uint64(uint128_multiply_uint64(value, 10), (uint64_t)current_digit);

		// if the first decimal digit isn't equal to the current, then we must've overflown
		if (((uint128_get_lower(value) % 10 +
				 ((uint128_get_higher(value) % 10) * 6)) % 10) != (uint64_t)current_digit)
			return UINT128_MAX;
	}
	return value;
}

// typedef for all functions that parse one digit in some base
typedef int64_t(*parse_digit)(const char);

/* Function which can parse all strings which are in a base which is a power of 2 (ex: binary, octal, hex)
 * The difference between this and all other bases is that it doesn't need to check for an overflow after
 * modifying the current iteration's integer, it only needs to check length (since all bases like these
 * will fit perfectly).
 *
 * If the parsed value is too big returns the maximum-value-uint128
 * If the string can't be parsed as hex returns a 0
 * !The string passed into this function must not start with zeroes, skip them first using find_first_non_zero
 */
uint128_t parse_from_power_of_2(parse_digit digit_parser, uint64_t digit_bits, const char *string) {
	if (string == NULL)
		return UINT128_ZERO;

	uint128_t value = UINT128_ZERO;
	for (size_t num_digits = 0; *string; string++, num_digits++) {
		// find out how many digits we can have at maximum, divide with round-up
		if (num_digits + 1 > (SIZEOF_INT128 * 8 + digit_bits - 1) / digit_bits)
			return UINT128_MAX;

		int64_t current_digit = digit_parser(*string);
		if (current_digit < 0)
			return UINT128_ZERO;

		value = uint128_or_uint64(uint128_shift_left(value, digit_bits), (uint64_t)current_digit);
	}

	return value;
}

/* Parse one character into a hex number and return -1 if we fail */
int64_t parse_hex_digit(const char c) {
	if (c >= 'a' && c <= 'f')
		return (c - 'a') + 10;
	if (c >= 'A' && c <= 'F')
		return (c - 'A') + 10;
	return parse_decimal_digit(c);
}

uint128_t parse_from_hex(const char *string) {
	return parse_from_power_of_2(parse_hex_digit, 4, string);
}

/* Parse one character into an octal number and return -1 if we fail */
int64_t parse_octal_digit(const char c) {
	if (c >= '0' && c <= '7')
		return c - '0';
	return -1;
}

uint128_t parse_from_octal(const char *string) {
	return parse_from_power_of_2(parse_octal_digit, 3, string);
}

/* Parse one character into an binary number and return -1 if we fail */
int64_t parse_binary_digit(const char c) {
	if (c == '0' || c == '1')
		return c - '0';
	return -1;
}

uint128_t parse_from_binary(const char *string) {
	return parse_from_power_of_2(parse_binary_digit, 1, string);
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
	if (string_length > 2 && string[0] == '0') {
		switch(string[1]) {
			case 'x':
			case 'X':
				return parse_from_hex(find_first_non_zero(string + 2));
			case 'o':
			case 'O':
				return parse_from_octal(find_first_non_zero(string + 2));
			case 'b':
			case 'B':
				return parse_from_binary(find_first_non_zero(string + 2));

		}
	}
	// decimal
	return parse_from_decimal(find_first_non_zero(string));
}

/// Conversion functions

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

//const char * uint128_to_string(const uint128_t a, char * const string, const unsigned int base) {
//	if (base < 2 || base > 36)
//		return NULL;
//
//	if (uint128_equ(a, UINT128_ZERO)) {
//		string[0] = '0';
//	} else {
//		while (!uint128_equ(a, UINT128_ZERO)) {
//
//		}
//	}
//}

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
		   (uint128_t){.hi = a.hi >> shift, .lo = (a.lo >> shift) | ((a.hi << 1) << (63 - shift))} :
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

uint128_t uint128_xor(const uint128_t a, const uint128_t b) {
#if COMPILER_INT128_AVAILABLE
	return a ^ b;
#else
	return (uint128_t){.hi = a.hi ^ b.hi, .lo = a.lo ^ b.lo};
#endif
}

/* Bitwise xor of an 128-bit uint with a 64-bit uint */
uint128_t uint128_xor_uint64(const uint128_t a, const uint64_t b) {
#if COMPILER_INT128_AVAILABLE
	return a ^ b;
#else
	return (uint128_t){.hi = a.hi, .lo = a.lo ^ b};
#endif
}

uint128_t uint128_and(const uint128_t a, const uint128_t b) {
#if COMPILER_INT128_AVAILABLE
	return a & b;
#else
	return (uint128_t){.hi = a.hi & b.hi, .lo = a.lo & b.lo};
#endif
}

/* Bitwise xor of an 128-bit uint with a 64-bit uint */
uint128_t uint128_and_uint64(const uint128_t a, const uint64_t b) {
#if COMPILER_INT128_AVAILABLE
	return a & b;
#else
	return (uint128_t){.hi = a.hi, .lo = a.lo & b};
#endif
}

/// Comparison

int uint128_equ(const uint128_t a, const uint128_t b) {
#if COMPILER_INT128_AVAILABLE
	return a == b;
#else
	return a.hi == b.hi && a.lo == b.lo;
#endif
}

int uint128_lt(const uint128_t a, const uint128_t b) {
#if COMPILER_INT128_AVAILABLE
	return a < b;
#else
	return (a.hi < b.hi) || ((a.hi == b.hi) && (a.lo < b.lo));
#endif
}

int uint128_lte(const uint128_t a, const uint128_t b) {
	return !uint128_lt(b, a);
}

int uint128_gt(const uint128_t a, const uint128_t b) {
	return uint128_lt(b, a);
}

int uint128_gte(const uint128_t a, const uint128_t b) {
	return !uint128_lt(a, b);
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

/* full multiplication of two 64-bit uints into a 128-bit uint */
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

/// Division
// The division algorithm here is the optimized division by reciprocal, given in gmplib.org/~tege/division-paper.pdf
// (Improved division by invariant integers)

#define small_reciprocal_for_table(divisor_top_9_bits) \
	(uint16_t)(0x7fd00 / (0x100 | (uint8_t)(divisor_top_9_bits)))
#define TWO_RECIPROCALS(x) \
	small_reciprocal_for_table((x)), small_reciprocal_for_table((x) + 1)
#define FOUR_RECIPROCALS(x) \
	TWO_RECIPROCALS((x)), TWO_RECIPROCALS((x) + 2)
#define EIGHT_RECIPROCALS(x) \
	FOUR_RECIPROCALS((x)), FOUR_RECIPROCALS((x) + 4)
#define SIXTEEN_RECIPROCALS(x) \
	EIGHT_RECIPROCALS((x)), EIGHT_RECIPROCALS((x) + 8)
#define THIRTYTWO_RECIPROCALS(x) \
	SIXTEEN_RECIPROCALS((x)), SIXTEEN_RECIPROCALS((x) + 16)
#define SIXTYFOUR_RECIPROCALS(x) \
	THIRTYTWO_RECIPROCALS((x)), THIRTYTWO_RECIPROCALS((x) + 32)
#define ALL_RECIPROCALS() \
	SIXTYFOUR_RECIPROCALS(0 * 64), SIXTYFOUR_RECIPROCALS(1 * 64), SIXTYFOUR_RECIPROCALS(2 * 64), SIXTYFOUR_RECIPROCALS(3 * 64)

const uint16_t small_reciprocal_table[] = {ALL_RECIPROCALS()};

// Helper defines here since we need these a lot in this part, because we will use this division even
// if uint128 is present in compiler, cause this is more optimized currently
#define gethi(a) uint128_get_higher(a)
#define getlo(a) uint128_get_lower(a)

// Reciprocal-computing algorithm based on Newton's method, described in the GMPlib paper
uint64_t reciprocal_128_by_64(const uint64_t divisor) {
	const uint64_t divisor_least_sig_bit = divisor & 1;
	const uint64_t divisor_top_9_bits = divisor >> 55; // round-down
	const uint64_t divisor_top_40_bits = (divisor >> 24) + 1; // round-down
	const uint64_t divisor_top_63_bits = (divisor >> 1) + divisor_least_sig_bit; // round-up

	const uint32_t v0 = small_reciprocal_table[divisor_top_9_bits - 256]; // table lookup of the top bits, iteration 0

	const uint64_t v1 = (v0 << 11) - (uint32_t)(v0 * v0 * divisor_top_40_bits >> 40) - 1; // iteration 1
	const uint64_t v2 = (v1 << 13) + (v1 * (0x1000000000000000ull - v1 * divisor_top_40_bits) >> 47); // iteration 2

	const uint64_t e = ((v2 >> 1) & (0 - divisor_least_sig_bit)) - v2 * divisor_top_63_bits;
	const uint64_t v3 = (gethi(uint64_multiply(v2, e)) >> 1) + (v2 << 31); // iteration 3
	const uint64_t v4 = v3 - gethi(uint128_add_uint64(uint64_multiply(v3, divisor), divisor)) - divisor; // iteration 4
	return v4;
}

// Reciprocal algorithm based on the previous one for computing a reciprocal of a 128-bit uint over 196 bits
uint64_t reciprocal_196_by_128(const uint128_t divisor) {
	uint64_t v = reciprocal_128_by_64(gethi(divisor));
	uint64_t p = gethi(divisor) * v + getlo(divisor);
	if (p < getlo(divisor)) {
		v--;
		if (p >= gethi(divisor)) {
			v--;
			p -= gethi(divisor);
		}
		p -= gethi(divisor);
	}

	const uint128_t t = uint64_multiply(v, getlo(divisor));
	p += gethi(t);
	if (p < gethi(t)) {
		v--;
		if (p >= gethi(divisor)) {
			if (p > gethi(divisor) || getlo(t) >= getlo(divisor))
				v--;
		}
	}
	return v;
}

// Struct defining the result of dividing a 128-bit uint by a 64-bit uint
typedef struct uint128_div_uint64_result {
	uint64_t quotient, remainder;
} uint128_div_uint64_result;

// Helper functions to increment/decrement the higher 64-bit part of the 128-bit uint
uint128_t uint128_increment_higher(uint128_t a) {
#if COMPILER_INT128_AVAILABLE
	return a + ((uint128_t)(1) << 64);
#else
	return (uint128_t){.hi = a.hi + 1, .lo = a.lo};
#endif
}

uint128_t uint128_decrement_higher(uint128_t a) {
#if COMPILER_INT128_AVAILABLE
	return a - ((uint128_t)(1) << 64);
#else
	return (uint128_t){.hi = a.hi - 1, .lo = a.lo};
#endif
}

// Algorithm div_2by1 from the paper
uint128_div_uint64_result divrem_uint128_by_uint64(const uint128_t a, const uint64_t divisor, const uint64_t reciprocal) {
	uint128_t quotient_guess = uint64_multiply(reciprocal, gethi(a));
	quotient_guess = uint128_add(quotient_guess, a);
	quotient_guess = uint128_increment_higher(quotient_guess);

	uint64_t remainder_guess = getlo(a) - gethi(quotient_guess) * divisor;
	if (remainder_guess > getlo(quotient_guess)) {
		quotient_guess = uint128_decrement_higher(quotient_guess);
		remainder_guess += divisor;
	}
	if (remainder_guess >= divisor) {
		quotient_guess = uint128_increment_higher(quotient_guess);
		remainder_guess -= divisor;
	}

	return (uint128_div_uint64_result){.quotient = gethi(quotient_guess), .remainder = remainder_guess};
}

// Struct defining the result of dividing a 196-bit uint by a 128-bit uint
typedef struct uint196_div_uint128_result {
	uint64_t quotient;
	uint128_t remainder;
} uint196_div_uint128_result;

// Algorithm div_3by2 from the paper
uint196_div_uint128_result divrem_uint196_by_uint128(const uint64_t a2, const uint64_t a1, const uint64_t a0,
													 const uint128_t divisor, const uint64_t reciprocal) {
	uint128_t quotient_guess = uint64_multiply(reciprocal, a2);
	quotient_guess = uint128_add(quotient_guess, uint128_create(a2, a1));

	uint64_t remainder_higher = a1 - gethi(quotient_guess) * gethi(divisor);
	uint128_t temporary = uint64_multiply(getlo(divisor), gethi(quotient_guess));

	uint128_t remainder_guess = uint128_subtract(
		uint128_subtract(uint128_create(remainder_higher, a0), temporary), divisor);
	remainder_higher = gethi(remainder_guess);
	quotient_guess = uint128_increment_higher(quotient_guess);

	if (remainder_higher >= getlo(quotient_guess)) {
		quotient_guess = uint128_decrement_higher(quotient_guess);
		remainder_guess = uint128_add(remainder_guess, divisor);
	}
	if (uint128_gte(remainder_guess, divisor)) {
		quotient_guess = uint128_increment_higher(quotient_guess);
		remainder_guess = uint128_subtract(remainder_guess, divisor);
	}

	return (uint196_div_uint128_result){.quotient = gethi(quotient_guess), .remainder = remainder_guess};
}

// Use the previous funtions/algorithms for school-like division
uint128_divrem_result uint128_divrem(const uint128_t a, const uint128_t b) {
#if COMPILER_INT128_AVAILABLE
	return (uint128_divrem_result){.quotient = a / b, .remainder = a % b};
#else
	if (gethi(b) == 0) {
		assert(getlo(b) != 0);	// dividing by 0

		const unsigned left_shift = uint64_clz(getlo(b));
		const unsigned right_shift = (64 - left_shift) % 64;
		const uint64_t right_mask = ((uint64_t)(left_shift == 0)) - 1;

		const uint64_t divisor = getlo(b) << left_shift;
		const uint64_t dividend_lower = getlo(a) << left_shift;
		const uint64_t dividend_higher = (gethi(a) << left_shift) | ((getlo(a) >> right_shift) & right_mask);
		const uint64_t dividend_extra = (gethi(a) >> right_shift) & right_mask;

		const uint64_t reciprocal = reciprocal_128_by_64(divisor);
		const uint128_div_uint64_result result_higher = divrem_uint128_by_uint64(
				uint128_create(dividend_extra, dividend_higher), divisor, reciprocal
			);
		const uint128_div_uint64_result result_lower = divrem_uint128_by_uint64(
				uint128_create(result_higher.remainder, dividend_lower), divisor, reciprocal
			);
		return (uint128_divrem_result){.quotient = uint128_create(result_higher.quotient, result_lower.quotient),
								 .remainder = uint128_create(0, result_lower.remainder >> left_shift)};
	}

	if (gethi(b) > gethi(a)) {
		return (uint128_divrem_result){.quotient = UINT128_ZERO, .remainder = a};
	}

	const unsigned left_shift = uint64_clz(gethi(b));
	// if the divisor has no 0-bits on the left, then the quotient is either 1 or 0
	if (left_shift == 0) {
		const unsigned quotient = ((unsigned)(gethi(b) < gethi(a))) | ((unsigned)(getlo(b) <= getlo(a)));
		return (uint128_divrem_result){.quotient = uint128_create(0, quotient),
								 .remainder = uint128_subtract(a, quotient ? b : UINT128_ZERO)};
	}

	const unsigned right_shift = 64 - left_shift;
	const uint64_t divisor_lower = getlo(b) << left_shift;
	const uint64_t divisor_higher = (gethi(b) << left_shift) | (getlo(b) >> right_shift);
	const uint64_t dividend_lower = getlo(a) << left_shift;
	const uint64_t dividend_higher = (gethi(a) << left_shift) | (getlo(a) >> right_shift);
	const uint64_t dividend_extra = gethi(a) >> right_shift;

	const uint128_t divisor = uint128_create(divisor_higher, divisor_lower);

	const uint64_t reciprocal = reciprocal_196_by_128(divisor);
	const uint196_div_uint128_result result = divrem_uint196_by_uint128(dividend_extra, dividend_higher, dividend_lower,
			divisor, reciprocal
		);

	return (uint128_divrem_result){.quotient = uint128_create(0, result.quotient),
								.remainder = uint128_shift_right(result.remainder, left_shift)};
#endif
}

uint128_t uint128_divide(const uint128_t a, const uint128_t b) {
#if COMPILER_INT128_AVAILABLE
	return a / b;
#else
	return uint128_divrem(a, b).quotient;
#endif
}

uint128_t uint128_mod(const uint128_t a, const uint128_t b) {
#if COMPILER_INT128_AVAILABLE
	return a / b;
#else
	return uint128_divrem(a, b).remainder;
#endif
}

uint128_t uint128_divide_uint64(const uint128_t a, const uint64_t b) {
#if COMPILER_INT128_AVAILABLE
	return a / b;
#else
	return uint128_divrem(a, (uint128_t){.hi = 0, .lo = b}).quotient;
#endif
}

uint64_t uint128_mod_uint64(const uint128_t a, const uint64_t b) {
#if COMPILER_INT128_AVAILABLE
	return a / b;
#else
	return uint128_divrem(a, (uint128_t){.hi = 0, .lo = b}).remainder.lo;
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
