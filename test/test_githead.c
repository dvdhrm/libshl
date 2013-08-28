/*
 * SHL - DList Tests
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

#include "test_common.h"

START_TEST(test_githead_verify)
{
	char *cmd;
	int r;

	r = asprintf(&cmd,
		     "test \"x`git describe`\" = \"x%s\" || exit 1 ; exit 0",
		     shl_githead);
	ck_assert(r >= 0);

	r = system(cmd);
	ck_assert_msg(!r, "system(%s) failed", cmd);

	free(cmd);
}
END_TEST

TEST_DEFINE_CASE(verify)
	TEST(test_githead_verify)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(githead,
		TEST_CASE(verify),
		TEST_END
	)
)
