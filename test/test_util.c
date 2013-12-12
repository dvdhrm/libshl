/*
 * SHL - Utility Tests
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

#include "test_common.h"

START_TEST(test_util_atoi_ctoi)
{
	ck_assert(shl_ctoi('0', 0) == -EINVAL);
	ck_assert(shl_ctoi('f', 0) == -EINVAL);

	ck_assert(shl_ctoi('0', 10) == 0);
	ck_assert(shl_ctoi('1', 10) == 1);
	ck_assert(shl_ctoi('8', 10) == 8);
	ck_assert(shl_ctoi('9', 10) == 9);
	ck_assert(shl_ctoi('a', 10) == -EINVAL);
	ck_assert(shl_ctoi('$', 10) == -EINVAL);
	ck_assert(shl_ctoi('\0', 10) == -EINVAL);

	ck_assert(shl_ctoi('7', 8) == 7);
	ck_assert(shl_ctoi('8', 8) == -EINVAL);

	ck_assert(shl_ctoi('a', 11) == 10);
	ck_assert(shl_ctoi('f', 16) == 15);
	ck_assert(shl_ctoi('z', 36) == 35);
	ck_assert(shl_ctoi('z', 35) == -EINVAL);
	ck_assert(shl_ctoi('z', 37) == 35);
}
END_TEST

#define TEST_ATOI_RAW(_suffix, _type, _max, _str, _val, _base, _r, _off) \
	({ \
		_type v; \
		const char *off = NULL; \
		ck_assert_int_eq(shl_atoi_ ## _suffix ((_str), (_base), &off, &v), (_r)); \
		ck_assert(off != NULL); \
		ck_assert_ptr_eq((void*)off, (void*)&(_str)[(_off)]); \
		if ((_r) >= 0) \
			ck_assert_int_eq(v, (_val)); \
		else if ((_r) == -ERANGE) \
			ck_assert_int_eq(v, (_max)); \
	})

#define TEST_ATOI(_str, _val, _base, _r, _off) \
	TEST_ATOI_RAW(ull, unsigned long long, ULLONG_MAX, (_str), (_val), (_base), (_r), (_off))

START_TEST(test_util_atoi_base)
{
	TEST_ATOI("0", 0, 10, 0, 1);
	TEST_ATOI("999", 999, 10, 0, 3);
	TEST_ATOI("ff", 0xff, 16, 0, 2);
	TEST_ATOI("ffg", 0xff, 16, 0, 2);
	TEST_ATOI("ffg", 0, 10, 0, 0);
	TEST_ATOI("0", 0, 37, -EINVAL, 0);

	TEST_ATOI("0x", 0, 0, 0, 1);
	TEST_ATOI("0x1", 1, 0, 0, 3);
	TEST_ATOI("0xg", 0, 0, 0, 1);
	TEST_ATOI("0", 0, 0, 0, 1);
	TEST_ATOI("08", 8, 0, 0, 2);
	TEST_ATOI("008", 0, 0, 0, 2);
	TEST_ATOI("07", 7, 0, 0, 2);
	TEST_ATOI("017", 15, 0, 0, 3);

	TEST_ATOI("0x10", 0, 10, 0, 1);
	TEST_ATOI("4294967296", 4294967296ULL, 10, 0, 10);
	TEST_ATOI("4294967296", 0x4294967296ULL, 16, 0, 10);
	TEST_ATOI("18446744073709551615", 18446744073709551615ULL, 10, 0, 20);
	TEST_ATOI("18446744073709551615", 0, 11, -ERANGE, 20);
	TEST_ATOI("184467440737095516150", 0, 36, -ERANGE, 21);
}
END_TEST

#define TEST_ATOI_UL(_str, _val, _base, _r, _off) \
	TEST_ATOI_RAW(ul, unsigned long, ULONG_MAX, (_str), (_val), (_base), (_r), (_off))
#define TEST_ATOI_U(_str, _val, _base, _r, _off) \
	TEST_ATOI_RAW(u, unsigned int, UINT_MAX, (_str), (_val), (_base), (_r), (_off))

START_TEST(test_util_atoi_types)
{
	TEST_ATOI_UL("0", 0, 10, 0, 1);
	TEST_ATOI_UL("999", 999, 10, 0, 3);
	TEST_ATOI_UL("ff", 0xff, 16, 0, 2);
	TEST_ATOI_UL("ffg", 0xff, 16, 0, 2);
	TEST_ATOI_UL("ffg", 0, 10, 0, 0);
	TEST_ATOI_UL("0", 0, 37, -EINVAL, 0);

	TEST_ATOI_U("0", 0, 10, 0, 1);
	TEST_ATOI_U("999", 999, 10, 0, 3);
	TEST_ATOI_U("ff", 0xff, 16, 0, 2);
	TEST_ATOI_U("ffg", 0xff, 16, 0, 2);
	TEST_ATOI_U("ffg", 0, 10, 0, 0);
	TEST_ATOI_U("0", 0, 37, -EINVAL, 0);

	TEST_ATOI_U("0x", 0, 0, 0, 1);
	TEST_ATOI_U("0x1", 1, 0, 0, 3);
	TEST_ATOI_U("0xg", 0, 0, 0, 1);
	TEST_ATOI_U("0", 0, 0, 0, 1);
	TEST_ATOI_U("08", 8, 0, 0, 2);
	TEST_ATOI_U("008", 0, 0, 0, 2);
	TEST_ATOI_U("07", 7, 0, 0, 2);
	TEST_ATOI_U("017", 15, 0, 0, 3);

	TEST_ATOI_UL("18446744073709551615",
		     18446744073709551615ULL,
		     10,
		     (ULONG_MAX >= UINT64_MAX) ? 0 : -ERANGE,
		     20);

	TEST_ATOI_U("65535",
		    65535ULL,
		    10,
		    (UINT_MAX >= UINT16_MAX) ? 0 : -ERANGE,
		    5);

	TEST_ATOI_U("4294967295",
		    4294967295ULL,
		    10,
		    (UINT_MAX >= UINT32_MAX) ? 0 : -ERANGE,
		    10);

	TEST_ATOI_U("18446744073709551615",
		    18446744073709551615ULL,
		    10,
		    (UINT_MAX >= UINT64_MAX) ? 0 : -ERANGE,
		    20);
}
END_TEST

TEST_DEFINE_CASE(atoi)
	TEST(test_util_atoi_ctoi)
	TEST(test_util_atoi_base)
	TEST(test_util_atoi_types)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(util,
		TEST_CASE(atoi),
		TEST_END
	)
)
