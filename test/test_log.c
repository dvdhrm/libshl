/*
 * SHL - DList Tests
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

#include "test_common.h"

START_TEST(test_log_setup)
{
	log_notice("test");
}
END_TEST

TEST_DEFINE_CASE(setup)
	TEST(test_log_setup)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(log,
		TEST_CASE(setup),
		TEST_END
	)
)
