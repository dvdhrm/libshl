/*
 * SHL - Ring Tests
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

#include "test_common.h"

START_TEST(test_macro_stringify)
{
	ck_assert(!strcmp(SHL_STRINGIFY(sth), "sth"));
	ck_assert(!strcmp(SHL_STRINGIFY(SOME-TEST%FUNC()), "SOME-TEST%FUNC()"));
}
END_TEST

START_TEST(test_macro_concatenate)
{
	ck_assert(SHL_CONCATENATE(123, 456) == 123456);
	ck_assert(!strcmp(SHL_STRINGIFY(SHL_CONCATENATE(foo, bar)), "foobar"));
	ck_assert(strcmp(SHL_STRINGIFY(SHL_CONCATENATE(foo, bar)), "foo bar"));
}
END_TEST

/* unique function names */
_shl_unused_ static inline void SHL_CONCATENATE(prefix_, SHL_UNIQUE(foobar))(void) { }
_shl_unused_ static inline void SHL_CONCATENATE(prefix_, SHL_UNIQUE(foobar))(void) { }

START_TEST(test_macro_unique)
{
	ck_assert(strcmp(SHL_STRINGIFY(SHL_UNIQUE(foobar)), SHL_STRINGIFY(SHL_UNIQUE(foobar))));
}
END_TEST

START_TEST(test_macro_array_length)
{
	int a1[1], a2[128];
	int b = 5;
	int a3[b];

	ck_assert(SHL_ARRAY_LENGTH(a1) == 1);
	ck_assert(SHL_ARRAY_LENGTH(a2) == 128);
	ck_assert(SHL_ARRAY_LENGTH(a3) == b);
}
END_TEST

START_TEST(test_macro_container_of)
{
	struct foobar {
		int padding;
		void *entry;
	};
	void **sth = &((struct foobar*)0)->entry;

	ck_assert(sth != NULL);
	ck_assert(shl_container_of(sth, struct foobar, entry) == NULL);
}
END_TEST

START_TEST(test_macro_max)
{
	ck_assert(shl_max(5, 6) == 6);
	ck_assert(shl_max(-1, 10) == 10);
	ck_assert(shl_max((unsigned)-1, 10U) == UINT_MAX);
	ck_assert(shl_max_t(unsigned, ULLONG_MAX, 10) == UINT_MAX);
}
END_TEST

START_TEST(test_macro_min)
{
	ck_assert(shl_min(5, 6) == 5);
	ck_assert(shl_min(-1, 10) == -1);
	ck_assert(shl_min((unsigned)-1, 10U) == 10U);
	ck_assert(shl_min_t(signed, ULLONG_MAX, 10) == -1);
}
END_TEST

START_TEST(test_macro_clamp)
{
	ck_assert(shl_clamp(10, 0, 1) == 1);
	ck_assert(shl_clamp(1, -2, 2) == 1);
	ck_assert(shl_clamp(1, 2, 3) == 2);
}
END_TEST

START_TEST(test_macro_align_power2)
{
	ck_assert(SHL_ALIGN_POWER2(0) == 0);
	ck_assert(SHL_ALIGN_POWER2(1) == 1);
	ck_assert(SHL_ALIGN_POWER2(2) == 2);
	ck_assert(SHL_ALIGN_POWER2(3) == 4);
	ck_assert(SHL_ALIGN_POWER2(12) == 16);

	ck_assert(SHL_ALIGN_POWER2(SIZE_MAX) == 0);
	ck_assert(SHL_ALIGN_POWER2(SIZE_MAX / 2) == SIZE_MAX / 2 + 1);
}
END_TEST

START_TEST(test_macro_zero)
{
	int a1[5] = { 0, 1, 2, 3, 4 };
	int z[100] = { };
	struct foobar {
		int a;
		int b;
	} a2 = { 0xff, 0xff }, *a3 = &a2;

	ck_assert(memcmp(a1, z, sizeof(a1)));
	shl_zero(a1);
	ck_assert(!memcmp(a1, z, sizeof(a1)));

	ck_assert(memcmp(a3, z, sizeof(*a3)));
	shl_zero(*a3);
	ck_assert(!memcmp(a3, z, sizeof(*a3)));
}
END_TEST

START_TEST(test_macro_intptr)
{
	int v[] = {
		0, 1, 2, 3,
		-1, -2, -3,
		INT_MAX, INT_MIN,
		INT_MAX + 1U, INT_MIN + 1,
		INT_MAX - 1, INT_MIN - 1U,
	};
	size_t i;

	for (i = 0; i < SHL_ARRAY_LENGTH(v); ++i) {
		ck_assert(SHL_PTR_TO_INT(SHL_INT_TO_PTR(v[i])) == v[i]);
		ck_assert(SHL_PTR_TO_INT(SHL_UINT_TO_PTR(v[i])) == v[i]);
		ck_assert(SHL_PTR_TO_UINT(SHL_UINT_TO_PTR(v[i])) == v[i]);
		ck_assert(SHL_PTR_TO_UINT(SHL_INT_TO_PTR(v[i])) == v[i]);
	}

	ck_assert(SHL_U64_TO_PTR(SHL_PTR_TO_U64(&i)) == (void*)&i);
	ck_assert(SHL_S64_TO_PTR(SHL_PTR_TO_S64(&i)) == (void*)&i);
	ck_assert(SHL_S64_TO_PTR(SHL_PTR_TO_U64(&i)) == (void*)&i);
	ck_assert(SHL_U64_TO_PTR(SHL_PTR_TO_S64(&i)) == (void*)&i);
}
END_TEST

#define TEST_MULT(_suffix, _var, _a, _b, _r) \
	({ \
		_var = (_a); \
		ck_assert(!shl_mult_ ## _suffix (&_var, (_b))); \
		ck_assert_int_eq(_var, (_r)); \
	})

#define TEST_MULT_RANGE(_suffix, _var, _a, _b) \
	({ \
		_var = (_a); \
		ck_assert(shl_mult_ ## _suffix (&_var, (_b)) == -ERANGE); \
	})

START_TEST(test_util_misc_arithmetic)
{
	unsigned int vu;
	unsigned long vul;
	unsigned long long vull;
	uint8_t v8;
	uint16_t v16;
	uint32_t v32;
	uint64_t v64;

	TEST_MULT(u, vu, 0U, 0U, 0U);
	TEST_MULT(ul, vul, 0UL, 0UL, 0UL);
	TEST_MULT(ull, vull, 0ULL, 0ULL, 0ULL);
	TEST_MULT(u8, v8, 0ULL, 0ULL, 0ULL);
	TEST_MULT(u16, v16, 0ULL, 0ULL, 0ULL);
	TEST_MULT(u32, v32, 0ULL, 0ULL, 0ULL);
	TEST_MULT(u64, v64, 0ULL, 0ULL, 0ULL);

	TEST_MULT_RANGE(u, vu, 2U, UINT_MAX);
	TEST_MULT_RANGE(ul, vul, 2UL, ULONG_MAX);
	TEST_MULT_RANGE(ull, vull, 2ULL, ULLONG_MAX);
	TEST_MULT_RANGE(u8, v8, 2ULL, UINT8_MAX);
	TEST_MULT_RANGE(u16, v16, 2ULL, UINT16_MAX);
	TEST_MULT_RANGE(u32, v32, 2ULL, UINT32_MAX);
	TEST_MULT_RANGE(u64, v64, 2ULL, UINT64_MAX);

	TEST_MULT(u, vu, (UINT_MAX & ~0x3U) / 4U, 4, UINT_MAX & ~0x3U);
	TEST_MULT(ul, vul, (ULONG_MAX & ~0x3UL) / 4UL, 4, ULONG_MAX & ~0x3UL);
	TEST_MULT(ull, vull, (ULLONG_MAX & ~0x3ULL) / 4ULL, 4, ULLONG_MAX & ~0x3ULL);
	TEST_MULT(u8, v8, (UINT8_MAX & ~0x3ULL) / 4ULL, 4, UINT8_MAX & ~0x3ULL);
	TEST_MULT(u16, v16, (UINT16_MAX & ~0x3ULL) / 4ULL, 4, UINT16_MAX & ~0x3ULL);
	TEST_MULT(u32, v32, (UINT32_MAX & ~0x3ULL) / 4ULL, 4, UINT32_MAX & ~0x3ULL);
	TEST_MULT(u64, v64, (UINT64_MAX & ~0x3ULL) / 4ULL, 4, UINT64_MAX & ~0x3ULL);
}
END_TEST

TEST_DEFINE_CASE(misc)
	TEST(test_macro_stringify)
	TEST(test_macro_concatenate)
	TEST(test_macro_unique)
	TEST(test_macro_array_length)
	TEST(test_macro_container_of)
	TEST(test_macro_max)
	TEST(test_macro_min)
	TEST(test_macro_clamp)
	TEST(test_macro_align_power2)
	TEST(test_macro_zero)
	TEST(test_macro_intptr)
	TEST(test_util_misc_arithmetic)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(macro,
		TEST_CASE(misc),
		TEST_END
	)
)
