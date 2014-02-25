/*
 * SHL - Valgrind Verification
 *
 * Copyright (c) 2012-2014 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

/* Dummy which just leaks memory. Used to verify valgrind memcheck. */

#include "test_common.h"

START_TEST(test_valgrind)
{
	void *p;

	p = malloc(0x100);
	ck_assert(!!p);
}
END_TEST

TEST_DEFINE_CASE(misc)
	TEST(test_valgrind)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(valgrind,
		TEST_CASE(misc),
		TEST_END
	)
)
