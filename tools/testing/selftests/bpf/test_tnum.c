// SPDX-License-Identifier: GPL-2.0-only
/* test_tnum.c: Selftests for tnum_scast function
 *
 * This program tests the tnum_scast function
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "tnum.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

struct tnum tnum_scast(struct tnum a, u8 size)
{
	u64 s = size * 8 - 1;
	u64 sign_mask;
	u64 value_mask;
	u64 new_value, new_mask;

	if (size >= 8) {
		return a;
	}

	sign_mask = 1ULL << s;
	value_mask = (1ULL << (s + 1)) - 1;

	new_value = a.value & value_mask;
	new_mask = a.mask & value_mask;

	if (a.mask & sign_mask) {
		new_mask |= ~value_mask;
	} else if (a.value & sign_mask) {
		new_value |= ~value_mask;
	}

	return TNUM(new_value, new_mask);
}

struct tnum_test_case {
	const char *description;
	struct tnum input;
	u8 size;
	struct tnum expected;
};

static int test_tnum_scast(void)
{
	int i, err = 0;
	struct tnum result;

	/* Define test cases */
	struct tnum_test_case tests[] = {
		/* 8-bit tests */
		{
			.description = "Known positive value (8-bit)",
			.input = TNUM(0x7F, 0x00),  // 127 in decimal
			.size = 1,
			.expected = TNUM(0x000000000000007F, 0x0000000000000000),
		},
		{
			.description = "Known negative value (8-bit)",
			.input = TNUM(0xFF, 0x00),  // -1 in 8-bit signed
			.size = 1,
			.expected = TNUM(0xFFFFFFFFFFFFFFFF, 0x0000000000000000),
		},
		{
			.description = "Unknown sign bit (8-bit)",
			.input = TNUM(0x7F, 0x80),  // Value 127, sign bit unknown
			.size = 1,
			.expected = TNUM(0x000000000000007F, 0xFFFFFFFFFFFFFF80),
		},
		{
			.description = "Completely unknown value (8-bit)",
			.input = TNUM(0x00, 0xFF),  // All bits unknown
			.size = 1,
			.expected = TNUM(0x0000000000000000, 0xFFFFFFFFFFFFFFFF),
		},
		/* 16-bit tests */
		{
			.description = "Known positive value (16-bit)",
			.input = TNUM(0x7FFF, 0x0000),
			.size = 2,
			.expected = TNUM(0x0000000000007FFF, 0x0000000000000000),
		},
		{
			.description = "Known negative value (16-bit)",
			.input = TNUM(0xFFFF, 0x0000),  // -1 in 16-bit signed
			.size = 2,
			.expected = TNUM(0xFFFFFFFFFFFFFFFF, 0x0000000000000000),
		},
		{
			.description = "Unknown sign bit (16-bit)",
			.input = TNUM(0x7FFF, 0x8000),
			.size = 2,
			.expected = TNUM(0x0000000000007FFF, 0xFFFFFFFFFFFF8000),
		},
		{
			.description = "Completely unknown value (16-bit)",
			.input = TNUM(0x0000, 0xFFFF),
			.size = 2,
			.expected = TNUM(0x0000000000000000, 0xFFFFFFFFFFFFFFFF),
		},
		/* 32-bit tests */
		{
			.description = "Known positive value (32-bit)",
			.input = TNUM(0x7FFFFFFF, 0x00000000),
			.size = 4,
			.expected = TNUM(0x000000007FFFFFFF, 0x0000000000000000),
		},
		{
			.description = "Known negative value (32-bit)",
			.input = TNUM(0xFFFFFFFF, 0x00000000),  // -1 in 32-bit signed
			.size = 4,
			.expected = TNUM(0xFFFFFFFFFFFFFFFF, 0x0000000000000000),
		},
		{
			.description = "Unknown sign bit (32-bit)",
			.input = TNUM(0x7FFFFFFF, 0x80000000),
			.size = 4,
			.expected = TNUM(0x000000007FFFFFFF, 0xFFFFFFFF80000000),
		},
		{
			.description = "Completely unknown value (32-bit)",
			.input = TNUM(0x00000000, 0xFFFFFFFF),
			.size = 4,
			.expected = TNUM(0x0000000000000000, 0xFFFFFFFFFFFFFFFF),
		},
		/* 64-bit tests */
		{
			.description = "Known positive value (64-bit)",
			.input = TNUM(0x7FFFFFFFFFFFFFFF, 0x0000000000000000),
			.size = 8,
			.expected = TNUM(0x7FFFFFFFFFFFFFFF, 0x0000000000000000),
		},
		{
			.description = "Known negative value (64-bit)",
			.input = TNUM(0xFFFFFFFFFFFFFFFF, 0x0000000000000000),
			.size = 8,
			.expected = TNUM(0xFFFFFFFFFFFFFFFF, 0x0000000000000000),
		},
		{
			.description = "Unknown sign bit (64-bit)",
			.input = TNUM(0x7FFFFFFFFFFFFFFF, 0x8000000000000000ULL),
			.size = 8,
			.expected = TNUM(0x7FFFFFFFFFFFFFFF, 0x8000000000000000ULL),
		},
		{
			.description = "Completely unknown value (64-bit)",
			.input = TNUM(0x0000000000000000, 0xFFFFFFFFFFFFFFFF),
			.size = 8,
			.expected = TNUM(0x0000000000000000, 0xFFFFFFFFFFFFFFFF),
		},
	};

	printf("Running tnum_scast tests...\n\n");

	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		struct tnum_test_case *t = &tests[i];

		result = tnum_scast(t->input, t->size);

		printf("Test %d (%s, size=%d bytes):\n", i + 1, t->description, t->size);
		printf("  Input:    value=0x%016llx, mask=0x%016llx\n",
				t->input.value, t->input.mask);
		printf("  Expected: value=0x%016llx, mask=0x%016llx\n",
				t->expected.value, t->expected.mask);
		printf("  Result:   value=0x%016llx, mask=0x%016llx\n",
				result.value, result.mask);

		if (memcmp(&result, &t->expected, sizeof(struct tnum)) != 0) {
			printf("  Fail.\n\n");
			err = 1;
		} else {
			printf("  Pass.\n\n");
		}
	}

	if (err)
		printf("Some tnum_scast tests failed.\n");
	else
		printf("All tnum_scast tests passed successfully.\n");

	return err;
}

int main(int argc, char **argv)
{
	int err = 0;

	err |= test_tnum_scast();

	return err;
}

