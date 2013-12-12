/*
 * SHL - Ring Tests
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

#include "test_common.h"

START_TEST(test_buf_setup)
{
	static const char buf[8192];
	struct shl_buf b;
	int r;

	shl_zero(b);

	r = shl_buf_push(&b, buf, 4096);
	ck_assert(!r);
	ck_assert(shl_buf_get_size(&b) == 4096);
	ck_assert(!memcmp(shl_buf_get_data(&b), buf, shl_buf_get_size(&b)));

	r = shl_buf_push(&b, buf, 4096);
	ck_assert(!r);
	ck_assert(shl_buf_get_size(&b) == 8192);
	ck_assert(!memcmp(shl_buf_get_data(&b), buf, shl_buf_get_size(&b)));

	shl_buf_pull(&b, 4096);
	ck_assert(shl_buf_get_size(&b) == 4096);
	ck_assert(!memcmp(shl_buf_get_data(&b), buf, shl_buf_get_size(&b)));

	shl_buf_flush(&b);
	ck_assert(shl_buf_get_size(&b) == 0);

	shl_buf_clear(&b);
}
END_TEST

TEST_DEFINE_CASE(setup)
	TEST(test_buf_setup)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(buf,
		TEST_CASE(setup),
		TEST_END
	)
)
