// extended precision integers library for C
// Copyright (C) 2020, Artem Mikheev <c@renbou.ru>.
// Licensed under the Apache License, Version 2.0.
#ifndef CREN_INTEGERS_UINT128_H
#define CREN_INTEGERS_UINT128_H

/***** uint128.h *****
 * This header defines the unsigned 128-bit type as well as some mathematical operations with that type.
 * It also defines functions to help initialize such integers in various ways and convert them to strings.
 * WARNING! Not all of the defined operations here might be implemented at the current time, so please check
 *			the function in question for "to do"s before using it
 **/

#include <stdint.h>

// Determine endianness in order to correctly make the struct with the same order as actual integers
#define CREN_INTS_LITTLE_ENDIAN 0
#define CREN_INTS_BIG_ENDIAN 1
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
    defined(__BIG_ENDIAN__) || \
    defined(__ARMEB__) || \
    defined(__THUMBEB__) || \
    defined(__AARCH64EB__) || \
    defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
#define ENDIANNESS CREN_INTS_BIG_ENDIAN
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
    defined(__LITTLE_ENDIAN__) || \
    defined(__ARMEL__) || \
    defined(__THUMBEL__) || \
    defined(__AARCH64EL__) || \
    defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
#define ENDIANNESS CREN_INTS_LITTLE_ENDIAN
#else
#error "I don't know what architecture this is!"
#endif


#if !defined(__SIZEOF_INT128__)
#define COMPILER_INT128_AVAILABLE 1
#else
#define COMPILER_INT128_AVAILABLE 0
#endif

#define SIZEOF_INT128 16
// Maximum needed decimal characters to represent a 128-bit integer
#define INT128_DECIMAL_SIZE 39

/* Struct defining a 128-bit unsigned integer
 * Basically simply represents two 64-bit uints, the higher and lower parts */
typedef struct i_uint128_t {
#if ENDIANNESS == CREN_INTS_LITTLE_ENDIAN
	uint64_t lo; // the lower 64 bits
	uint64_t hi; // the higher 64 bits
#else
	uint64_t hi; // the higher 64 bits
	uint64_t lo; // the lower 64 bits
#endif
} i_uint128_t;

/* If the 128-bit int is implemented by GCC, then use it instead of this */
#if COMPILER_INT128_AVAILABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
typedef unsigned __int128 uint128_t;
#pragma GCC diagnostic pop
#else
typedef i_uint128_t uint128_t;
#endif

/// Creation and parsing functions

/* Creates a 128-bit uint from two 64-bit uints */
uint128_t uint128_create(const uint64_t hi, const uint64_t lo);

/* Creates a 128-bit uint from one 64-bit uint*/
uint128_t uint128_value(const uint64_t a);

/* Parses a 128-bit uint from a string
 * Supports formats:
 * 0x[0-9a-f] - parses the integer as a hex string
 * 0o[0-7] - parses the integer as an octal string
 * 0b[0-1] - parses the integer as a binary string
 * [0-9] - parses the integer as a decimal string
 * It isn't this library problem to strip and filter the string, so you should handle that separately.
 * If the parsed value is greater than the maximum value of a 128 unsigned integer (2^128-1), then it returns
 * a uint128 with the max value.
 * If the string can't be parsed, then returns 0.
 */
uint128_t uint128_parse(const char *string);

/// Conversion functions

/* Gets the lower 64 bits of the 128-bit integer */
uint64_t uint128_get_lower(const uint128_t a);

/* Gets the higher 64 bits of the 128-bit integer */
uint64_t uint128_get_higher(const uint128_t a);

/* Converts the 128-bit uint to a string, storing it in the string argument, base can be one of from 2 to 36
 * string - where to store the result, this should be enough to fit any string-representation of an uint128,
 * 				   so 128 chars (maximum 128 chars if binary)
 * returns pointer to the resulting string, if something has gone wrong during conversion it returns NULL
 */
const char * uint128_to_string(const uint128_t a, char * const string, const unsigned int base);

/// Bitwise operations

/* Shift a 128-bit uint to the left by shift bits */
uint128_t uint128_shift_left(const uint128_t a, const unsigned int shift);

/* Shift a 128-bit uint to the right by shift bits */
uint128_t uint128_shift_right(const uint128_t a, const unsigned int shift);

/* Bitwise or of two 128-bit uints */
uint128_t uint128_or(const uint128_t a, const uint128_t b);

/* Bitwise or of an 128-bit uint with a 64-bit uint */
uint128_t uint128_or_uint64(const uint128_t a, const uint64_t b);

/* Bitwise xor of two 128-bit uints */
uint128_t uint128_xor(const uint128_t a, const uint128_t b);

/* Bitwise xor of an 128-bit uint with a 64-bit uint */
uint128_t uint128_xor_uint64(const uint128_t a, const uint64_t b);

/* Bitwise xor of two 128-bit uints */
uint128_t uint128_and(const uint128_t a, const uint128_t b);

/* Bitwise xor of an 128-bit uint with a 64-bit uint */
uint128_t uint128_and_uint64(const uint128_t a, const uint64_t b);

/// Comparison

/* Compares two 128-bit uints, returning 1 if the two are equal */
int uint128_equ(const uint128_t a, const uint128_t b);

/* Compares two 128-bit uints, returning 1 if a < b */
int uint128_lt(const uint128_t a, const uint128_t b);

/* Compares two 128-bit uints, returning 1 if a <= b */
int uint128_lte(const uint128_t a, const uint128_t b);

/* Compares two 128-bit uints, returning 1 if a > b */
int uint128_gt(const uint128_t a, const uint128_t b);

/* Compares two 128-bit uints, returning 1 if a >= b */
int uint128_gte(const uint128_t a, const uint128_t b);

/// Arithmetic

/* Adds two 128-bit uints together */
uint128_t uint128_add(const uint128_t a, const uint128_t b);

/* Adds a 64-bit uint to a 128-bit uint */
uint128_t uint128_add_uint64(const uint128_t a, const uint64_t b);

/* Subs the second 128-bit uint from the first one */
uint128_t uint128_subtract(const uint128_t a, const uint128_t b);

/* Subtracts a 64-bit uint to a 128-bit uint */
uint128_t uint128_subtract_uint64(const uint128_t a, const uint64_t b);

/* Multiplies two uint64's and creates a 128-bit uint with the result */
uint128_t uint64_multiply(const uint64_t a, const uint64_t b);

/* Multiplies the two 128-bit uints */
uint128_t uint128_multiply(const uint128_t a, const uint128_t b);

/* Multiplies a 128-bit uint by a 64-bit uint */
uint128_t uint128_multiply_uint64(const uint128_t a, const uint64_t b);

// Struct defining the return type of divrem function which divides a uint128 by uint128,
// calculating the quotient and remainder in the process
typedef struct uint128_divrem_result {
	uint128_t quotient, remainder;
} uint128_divrem_result;

/* Divides first 128-bit uint by second one, returns the quotient and remainder in struct */
uint128_divrem_result uint128_divrem(const uint128_t a, const uint128_t b);

/* Divides the first 128-bit uint by the second one */
uint128_t uint128_divide(const uint128_t a, const uint128_t b);

/* Computes the remainder from dividing the first 128-bit uint by the second one */
uint128_t uint128_mod(const uint128_t a, const uint128_t b);

/* Divides a 128-bit uint by a 64-bit uint */
uint128_t uint128_divide_uint64(const uint128_t a, const uint64_t b);

/* Computes the remainder from dividing the first 128-bit uint by a 64-bit uint */
uint64_t uint128_mod_uint64(const uint128_t a, const uint64_t b);

/* Increments the 128-bit integer */
uint128_t uint128_increment(const uint128_t a);

/* Decrements the 128-bit integer */
uint128_t uint128_decrement(const uint128_t a);

#endif //CREN_INTEGERS_UINT128_H
