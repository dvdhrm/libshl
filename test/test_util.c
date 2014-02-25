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

static void test_cat(const char *a, const char *b, const char *ab)
{
	char *x;

	x = shl_strcat(a, b);
	ck_assert(!!x);
	ck_assert(!strcmp(x, ab));
	free(x);
}

#define TEST_CAT(_a, _b) test_cat((_a), (_b), (_a _b));

START_TEST(test_util_str_cat)
{
	TEST_CAT("a", "b");
	TEST_CAT("", "b");
	TEST_CAT("a", "");
	TEST_CAT("STH", "MORE");
	test_cat(NULL, "b", "b");
	test_cat("a", NULL, "a");
	test_cat(NULL, NULL, "");
}
END_TEST

#define TEST_JOIN(_res, ...) ({ \
		char *x; \
		x = shl_strjoin(__VA_ARGS__); \
		ck_assert(!!x); \
		ck_assert(!strcmp(x, (_res))); \
		free(x); \
	})

START_TEST(test_util_str_join)
{
	TEST_JOIN("", NULL, NULL);
	TEST_JOIN("a", "a", NULL);
	TEST_JOIN("abc", "a", "b", "c", NULL);
	TEST_JOIN("abc", "", "a", "", "b", "", "c", "", NULL);
	TEST_JOIN("aasdfb", "a", "asdf", "b", NULL);
	TEST_JOIN("asdfb", "asdf", "b", NULL);
}
END_TEST

START_TEST(test_util_str_startswith)
{
	ck_assert(!!shl_startswith("", ""));
	ck_assert(!!shl_startswith("asdf", "asdf"));
	ck_assert(!!shl_startswith("asdf", "asd"));
	ck_assert(!!shl_startswith("asdf", ""));
	ck_assert(!shl_startswith("asdf", "asdfg"));
	ck_assert(!shl_startswith("asdf", " asdf"));
	ck_assert(!shl_startswith(" asdf", "asdf"));
}
END_TEST

START_TEST(test_util_str_qstr)
{
	int r;
	char **strv, *t;

	r = shl_qstr_tokenize("", &strv);
	ck_assert(r == 0);
	ck_assert(strv && !*strv);
	shl_strv_free(strv);

	r = shl_qstr_tokenize(NULL, &strv);
	ck_assert(r == 0);
	ck_assert(strv && !*strv);
	shl_strv_free(strv);

	r = shl_qstr_tokenize("foo", &strv);
	ck_assert(r == 1);
	ck_assert(strv && !strcmp(*strv, "foo") && !strv[1]);
	shl_strv_free(strv);

	r = shl_qstr_tokenize("''", &strv);
	ck_assert(r == 1);
	ck_assert(strv && !strcmp(*strv, "") && !strv[1]);
	shl_strv_free(strv);

	r = shl_qstr_tokenize("foo bar", &strv);
	ck_assert(r == 2);
	ck_assert(strv &&
		  !strcmp(strv[0], "foo") &&
		  !strcmp(strv[1], "bar") &&
		  !strv[2]);
	shl_strv_free(strv);

	r = shl_qstr_tokenize("more foo bar entries", &strv);
	ck_assert(r == 4);
	ck_assert(strv &&
		  !strcmp(strv[0], "more") &&
		  !strcmp(strv[1], "foo") &&
		  !strcmp(strv[2], "bar") &&
		  !strcmp(strv[3], "entries") &&
		  !strv[4]);
	shl_strv_free(strv);

	r = shl_qstr_tokenize("\"mo\"re f''oo bar'' 'entries'", &strv);
	ck_assert(r == 4);
	ck_assert(strv &&
		  !strcmp(strv[0], "more") &&
		  !strcmp(strv[1], "foo") &&
		  !strcmp(strv[2], "bar") &&
		  !strcmp(strv[3], "entries") &&
		  !strv[4]);
	shl_strv_free(strv);

	r = shl_qstr_tokenize("\"\\\"'mo\"re f'\"'oo bar'' 'en\\ntries'\\", &strv);
	ck_assert(r == 4);
	ck_assert(strv &&
		  !strcmp(strv[0], "\"'more") &&
		  !strcmp(strv[1], "f\"oo") &&
		  !strcmp(strv[2], "bar") &&
		  !strcmp(strv[3], "en\ntries\\") &&
		  !strv[4]);
	shl_strv_free(strv);

	r = shl_qstr_join((char*[]){ NULL }, &t);
	ck_assert(r == 0);
	ck_assert(!strcmp(t, ""));
	free(t);

	r = shl_qstr_join((char*[]){ "asdf", NULL }, &t);
	ck_assert(r == 4);
	ck_assert(!strcmp(t, "asdf"));
	free(t);

	r = shl_qstr_join((char*[]){ "as df", "buhu", "  ", "yeha\\", NULL }, &t);
	ck_assert(r == 24);
	ck_assert_str_eq(t, "\"as df\" buhu \"  \" yeha\\\\");
	free(t);

	r = shl_qstr_join((char*[]){ "as\tdf", "buhu", "\n", NULL }, &t);
	ck_assert_int_eq(r, 16);
	ck_assert_str_eq(t, "\"as\tdf\" buhu \"\n\"");
	free(t);
}
END_TEST

TEST_DEFINE_CASE(str)
	TEST(test_util_str_cat)
	TEST(test_util_str_join)
	TEST(test_util_str_startswith)
	TEST(test_util_str_qstr)
TEST_END_CASE

START_TEST(test_misc_greedy_alloc)
{
	static uint64_t zero[1000 * 1000];
	uint64_t *arr;
	size_t cnt;

	arr = NULL;
	cnt = 0;

	ck_assert_ptr_ne(SHL_GREEDY_REALLOC0_T(arr, cnt, 10), NULL);
	ck_assert_int_ge(cnt, 10);
	ck_assert(!memcmp(arr, zero, 10 * sizeof(*arr)));

	ck_assert_ptr_ne(SHL_GREEDY_REALLOC0_T(arr, cnt, 100), NULL);
	ck_assert_int_ge(cnt, 100);
	ck_assert(!memcmp(arr, zero, 100 * sizeof(*arr)));

	ck_assert_ptr_ne(SHL_GREEDY_REALLOC0_T(arr, cnt, 1000), NULL);
	ck_assert_int_ge(cnt, 1000);
	ck_assert(!memcmp(arr, zero, 1000 * sizeof(*arr)));

	ck_assert_ptr_ne(SHL_GREEDY_REALLOC0_T(arr, cnt, 10000), NULL);
	ck_assert_int_ge(cnt, 10000);
	ck_assert(!memcmp(arr, zero, 10000 * sizeof(*arr)));

	free(arr);
}
END_TEST

TEST_DEFINE_CASE(misc)
	TEST(test_misc_greedy_alloc)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(util,
		TEST_CASE(atoi),
		TEST_CASE(str),
		TEST_CASE(misc),
		TEST_END
	)
)
