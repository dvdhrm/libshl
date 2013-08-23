/*
 * SHL - Trie Tests
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

#include "test_common.h"

static char *s[] = {
	"/some/path",
	"/some/sub/path",
	"/some/path/extended",
	"/another/path",
	"/some/more",
	"/another/path/added",
	"/some/path/again",
	"/some",
	"/",
	"",
	"/more/paths",
	"relative/paths",
	"/absolute/again",
	"relative",
	"relative/paths/again",
	"another/relative/path",
	"/some/more/absolute/paths",
	"this/is/a/bit/longer/than/the/other/paths/without/any/proper/common/prefix/compared/to/the/others",
	".",
	"relative/path",
	"last/path",
	NULL
};

static char *u[] = {
	"/some/",
	"some",
	",",
	"/relative/paths",
	"some/path",
	"/some/path/",
	"/relative/path",
	"relativ",
	NULL
};

START_TEST(test_trie_setup_zero)
{
	struct shl_trie t = { .root = TEST_INVALID_PTR, };

	shl_trie_zero(&t);
	ck_assert(t.root == 0);

	shl_trie_clear(&t, NULL, NULL);
	ck_assert(t.root == 0);
}
END_TEST

START_TEST(test_trie_setup_nozero)
{
	struct shl_trie t = { .root = 0, };

	shl_trie_clear(&t, NULL, NULL);
	ck_assert(t.root == 0);
}
END_TEST

TEST_DEFINE_CASE(setup)
	TEST(test_trie_setup_zero)
	TEST(test_trie_setup_nozero)
TEST_END_CASE

START_TEST(test_trie_add_zero)
{
	struct shl_trie t = { .root = TEST_INVALID_PTR, };
	int ret;

	shl_trie_zero(&t);

	ret = shl_trie_insert_str(&t, &s[0], NULL);
	ck_assert(ret == 0);

	ret = shl_trie_insert_str(&t, &s[1], NULL);
	ck_assert(ret == 0);

	shl_trie_clear(&t, NULL, NULL);
	ck_assert(t.root == 0);
}
END_TEST

START_TEST(test_trie_add_nozero)
{
	struct shl_trie t = { .root = 0, };
	int ret;

	ret = shl_trie_insert_str(&t, &s[0], NULL);
	ck_assert(ret == 0);

	ret = shl_trie_insert_str(&t, &s[1], NULL);
	ck_assert(ret == 0);

	shl_trie_clear(&t, NULL, NULL);
	ck_assert(t.root == 0);
}
END_TEST

/*
 * Stress test insertions including the empty string.
 */
START_TEST(test_trie_add_many)
{
	struct shl_trie t = { .root = TEST_INVALID_PTR, };
	int ret, i;
	bool r;

	shl_trie_zero(&t);

	/* test empty trie */

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	/* verify inserts */

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	/* clear and test empty trie */

	shl_trie_clear(&t, NULL, NULL);
	ck_assert(t.root == 0);

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	/* insert again */

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}
}
END_TEST


TEST_DEFINE_CASE(add)
	TEST(test_trie_add_zero)
	TEST(test_trie_add_nozero)
	TEST(test_trie_add_many)
TEST_END_CASE

/*
 * Stress tests insertions and removals.
 */
START_TEST(test_trie_remove)
{
	struct shl_trie t = { .root = TEST_INVALID_PTR, };
	int ret, i;
	bool r;

	shl_trie_zero(&t);

	/* verify inserts */

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_remove_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	/* clear and test empty trie */

	shl_trie_clear(&t, NULL, NULL);
	ck_assert(t.root == 0);

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	/* insert again */

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; u[i]; ++i) {
		ret = shl_trie_insert_str(&t, &u[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_remove_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_remove_str(&t, u[i], NULL);
		ck_assert(r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_remove_str(&t, s[i], NULL);
		ck_assert(r);
	}

	ck_assert(t.root == NULL);
}
END_TEST

/*
 * Stress tests insertions and removals with overwrite.
 */
START_TEST(test_trie_remove_overwrite)
{
	struct shl_trie t = { .root = TEST_INVALID_PTR, };
	int ret, i;
	bool r;
	char **o;

	shl_trie_zero(&t);

	/* verify inserts */

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], &o);
		ck_assert(ret == -EALREADY);
		ck_assert(o == &s[i]);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_remove_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	/* clear and test empty trie */

	shl_trie_clear(&t, NULL, NULL);
	ck_assert(t.root == 0);

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	/* insert again */

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; u[i]; ++i) {
		ret = shl_trie_insert_str(&t, &u[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], &o);
		ck_assert(ret == -EALREADY);
		ck_assert(o == &s[i]);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_remove_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_remove_str(&t, u[i], NULL);
		ck_assert(r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_remove_str(&t, s[i], NULL);
		ck_assert(r);
	}

	ck_assert(t.root == NULL);
}
END_TEST

TEST_DEFINE_CASE(remove)
	TEST(test_trie_remove)
	TEST(test_trie_remove_overwrite)
TEST_END_CASE

static void test_trie_visit_some_cb(char **key, void *ctx)
{
	int *num = ctx;

	++*num;
	ck_assert_msg(strncmp(*key, "/some/", 6) == 0, "invalid trie visit %s", *key);
}

static void test_trie_visit_some2_cb(char **key, void *ctx)
{
	int *num = ctx;

	++*num;
	ck_assert_msg(strncmp(*key, "/some", 5) == 0, "invalid trie visit %s", *key);
}

static void test_trie_visit_some3_cb(char **key, void *ctx)
{
	int *num = ctx;

	++*num;
	ck_assert_msg(strncmp(*key, "/some/p", 7) == 0, "invalid trie visit %s", *key);
}

/*
 * Stress tests insertions and removals with overwrite.
 */
START_TEST(test_trie_visit)
{
	struct shl_trie t = { .root = TEST_INVALID_PTR, };
	int ret, i, num;
	bool r;
	char **o;

	shl_trie_zero(&t);

	/* verify inserts */

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], &o);
		ck_assert(ret == -EALREADY);
		ck_assert(o == &s[i]);
	}

	num = 0;
	shl_trie_visit_str(&t, "/some/", test_trie_visit_some_cb, &num);
	ck_assert(num == 6);

	num = 0;
	shl_trie_visit_str(&t, "/some", test_trie_visit_some2_cb, &num);
	ck_assert(num == 7);

	num = 0;
	shl_trie_visit_str(&t, "/some/p", test_trie_visit_some3_cb, &num);
	ck_assert(num == 3);

	num = 0;
	shl_trie_visit_str(&t, "~", test_trie_visit_some3_cb, &num);
	ck_assert(num == 0);

	/* clear and test empty trie */

	shl_trie_clear(&t, NULL, NULL);
	ck_assert(t.root == 0);

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	/* insert again */

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; u[i]; ++i) {
		ret = shl_trie_insert_str(&t, &u[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], &o);
		ck_assert(ret == -EALREADY);
		ck_assert(o == &s[i]);
	}

	for (i = 0; u[i]; ++i) {
		ret = shl_trie_insert_str(&t, &u[i], &o);
		ck_assert(ret == -EALREADY);
		ck_assert(o == &u[i]);
	}

	num = 0;
	shl_trie_visit_str(&t, "/some/", test_trie_visit_some_cb, &num);
	ck_assert(num == 8);

	num = 0;
	shl_trie_visit_str(&t, "/some", test_trie_visit_some2_cb, &num);
	ck_assert(num == 9);

	num = 0;
	shl_trie_visit_str(&t, "/some/p", test_trie_visit_some3_cb, &num);
	ck_assert(num == 4);

	num = 0;
	shl_trie_visit_str(&t, "~", test_trie_visit_some3_cb, &num);
	ck_assert(num == 0);

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_remove_str(&t, s[i], &o);
		ck_assert(r);
		ck_assert(o == &s[i]);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; s[i]; ++i) {
		ret = shl_trie_insert_str(&t, &s[i], NULL);
		ck_assert(ret == 0);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_remove_str(&t, u[i], &o);
		ck_assert(r);
		ck_assert(o == &u[i]);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_lookup_str(&t, s[i], NULL);
		ck_assert(r);
	}

	for (i = 0; u[i]; ++i) {
		r = shl_trie_lookup_str(&t, u[i], NULL);
		ck_assert(!r);
	}

	for (i = 0; s[i]; ++i) {
		r = shl_trie_remove_str(&t, s[i], NULL);
		ck_assert(r);
	}

	ck_assert(t.root == NULL);
}
END_TEST

TEST_DEFINE_CASE(visit)
	TEST(test_trie_visit)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(trie,
		TEST_CASE(setup),
		TEST_CASE(add),
		TEST_CASE(remove),
		TEST_CASE(visit),
		TEST_END
	)
)
