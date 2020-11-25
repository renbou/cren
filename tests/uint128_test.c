#include <stdlib.h>
#include <stdio.h>
#include <integers/uint128.h>

int main() {
	puts("--- uint128 library testing ---");
	puts("[1] Creation, parsing and get_lower/get_higher tests");

	uint128_t test1_a = uint128_create(0x8899aabbccddeeffull, 0x0011223344556677ull);
	if (!(uint128_get_higher(test1_a) == 0x8899aabbccddeeffull && uint128_get_lower(test1_a) == 0x0011223344556677ull)) {
		printf(
			"!ERROR! Problem with uint128_create or one of uint128_get_higher/uint128_get_lower:\n"
   			"\tLower bits during creation have been set to 0x0011223344556677, but are now 0x%llx\n"
	  		"\tHigher bits during creation have been set to 0x8899aabbccddeeff, but are now 0x%llx",
			uint128_get_lower(test1_a), uint128_get_higher(test1_a));
		exit(-1);
	}

	uint128_t test1_b = uint128_value(0xdeadd00dcafebeefull);
	if (!(uint128_get_higher(test1_b) == 0 && uint128_get_lower(test1_b) == 0xdeadd00dcafebeefull)) {
		printf(
			"!ERROR! Problem with uint128_value or one of uint128_get_higher/uint128_get_lower:\n"
			"\tLower bits during creation have been set to 0xdeadd00dcafebeef, but are now 0x%llx\n"
			"\tHigher bits during creation have been set to 0, but are now 0x%llx",
			uint128_get_lower(test1_b), uint128_get_higher(test1_b));
		exit(-1);
	}

	uint128_t test1_c = uint128_parse("340282366920938463463374607431768211455");
	if (!(uint128_get_higher(test1_c) == 18446744073709551615ull && uint128_get_lower(test1_c) == 18446744073709551615ull)) {
		printf(
			"!ERROR! Problem with uint128_parse or one of uint128_get_higher/uint128_get_lower:\n"
			"\tLower bits were supposed to be 18446744073709551615, but are actually %llu\n"
			"\tHigher bits were supposed to be 18446744073709551615, but are actually %llu",
			uint128_get_lower(test1_c), uint128_get_higher(test1_c));
		exit(-1);
	}

	uint128_t test1_d = uint128_parse("0000000000000000340282366920938463463374607431768211455");
	if (!(uint128_get_higher(test1_d) == 18446744073709551615ull && uint128_get_lower(test1_d) == 18446744073709551615ull)) {
		printf(
			"!ERROR! Problem with uint128_parse or one of uint128_get_higher/uint128_get_lower:\n"
			"\tLower bits were supposed to be 18446744073709551615, but are actually %llu\n"
			"\tHigher bits were supposed to be 18446744073709551615, but are actually %llu",
			uint128_get_lower(test1_d), uint128_get_higher(test1_d));
		exit(-1);
	}

	uint128_t test1_e = uint128_parse("0x   0123");
	if (!(uint128_get_higher(test1_e) == 0 && uint128_get_lower(test1_e) == 0)) {
		printf(
			"!ERROR! Problem with uint128_parse or one of uint128_get_higher/uint128_get_lower:\n"
			"\tLower bits were supposed to be 0, but are actually %llu\n"
			"\tHigher bits were supposed to be 0, but are actually %llu",
			uint128_get_lower(test1_e), uint128_get_higher(test1_e));
		exit(-1);
	}

	uint128_t test1_f = uint128_parse(NULL);
	if (!(uint128_get_higher(test1_f) == 0 && uint128_get_lower(test1_f) == 0)) {
		printf(
			"!ERROR! Problem with uint128_parse or one of uint128_get_higher/uint128_get_lower:\n"
			"\tLower bits were supposed to be 0, but are actually %llu\n"
			"\tHigher bits were supposed to be 0, but are actually %llu",
			uint128_get_lower(test1_f), uint128_get_higher(test1_f));
		exit(-1);
	}

	uint128_t test1_g = uint128_parse("0x11112233445566778899AABBCCDDEEFF");
	if (!(uint128_get_higher(test1_g) == 0x1111223344556677ull && uint128_get_lower(test1_g) == 0x8899AABBCCDDEEFFull)) {
		printf(
			"!ERROR! Problem with uint128_parse or one of uint128_get_higher/uint128_get_lower:\n"
			"\tLower bits were supposed to be 0x8899AABBCCDDEEFF, but are actually %llx\n"
			"\tHigher bits were supposed to be 0x0011223344556677, but are actually %llx",
			uint128_get_lower(test1_g), uint128_get_higher(test1_g));
		exit(-1);
	}

	uint128_t test1_h = uint128_parse("0xFF11112233445566778899AABBCCDDEEFF");
	if (!(uint128_get_higher(test1_h) == 0xFFFFFFFFFFFFFFFFull && uint128_get_lower(test1_h) == 0xFFFFFFFFFFFFFFFFull)) {
		printf(
			"!ERROR! Problem with uint128_parse or one of uint128_get_higher/uint128_get_lower:\n"
			"\tLower bits were supposed to be 0xFFFFFFFFFFFFFFFF, but are actually %llx\n"
			"\tHigher bits were supposed to be 0xFFFFFFFFFFFFFFFF, but are actually %llx",
			uint128_get_lower(test1_h), uint128_get_higher(test1_h));
		exit(-1);
	}

	uint128_t test1_i = uint128_parse("0b11011110101011011011111011101111110010101111111010111010101111101101000000001101110111101110110110111010110111011010110111101110");
	if (!(uint128_get_higher(test1_i) == 0xDEADBEEFCAFEBABE && uint128_get_lower(test1_i) == 0xD00DDEEDBADDADEE)) {
		printf(
			"!ERROR! Problem with uint128_parse or one of uint128_get_higher/uint128_get_lower:\n"
			"\tLower bits were supposed to be 0xDEADBEEFCAFEBABE, but are actually %llx\n"
			"\tHigher bits were supposed to be 0xD00DDEEDBADDADEE, but are actually %llx",
			uint128_get_lower(test1_i), uint128_get_higher(test1_i));
		exit(-1);
	}

	puts("[\\1] Test block has been passed!");

	return 0;
}
