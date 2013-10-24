/*
 * SHL - LLog Tests
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

#include "test_common.h"

START_TEST(test_llog_setup)
{
	llog_dnotice(log_llog, NULL, "test");
}
END_TEST

TEST_DEFINE_CASE(setup)
	TEST(test_llog_setup)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(llog,
		TEST_CASE(setup),
		TEST_END
	)
)
