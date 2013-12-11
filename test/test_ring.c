/*
 * SHL - Ring Tests
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

#include "test_common.h"

START_TEST(test_ring_setup)
{
	static const char buf[8192];
	struct shl_ring r;
	size_t l;
	struct iovec vec[2];
	int s;

	memset(&r, 0, sizeof(r));

	l = shl_ring_peek(&r, vec);
	ck_assert(l == 0);

	s = shl_ring_push(&r, buf, 2048);
	ck_assert(!s);
	ck_assert(shl_ring_get_size(&r) == 2048);

	l = shl_ring_peek(&r, vec);
	ck_assert(l == 1);
	ck_assert(vec[0].iov_len == 2048);
	ck_assert(!memcmp(vec[0].iov_base, buf, vec[0].iov_len));
	ck_assert(shl_ring_get_size(&r) == 2048);

	shl_ring_pull(&r, 2048);
	ck_assert(shl_ring_get_size(&r) == 0);

	l = shl_ring_peek(&r, vec);
	ck_assert(l == 0);
	ck_assert(shl_ring_get_size(&r) == 0);

	s = shl_ring_push(&r, buf, 2048);
	ck_assert(!s);
	ck_assert(shl_ring_get_size(&r) == 2048);

	l = shl_ring_peek(&r, vec);
	ck_assert(l == 1);
	ck_assert(vec[0].iov_len == 2048);
	ck_assert(!memcmp(vec[0].iov_base, buf, vec[0].iov_len));
	ck_assert(shl_ring_get_size(&r) == 2048);

	s = shl_ring_push(&r, buf, 1);
	ck_assert(!s);
	ck_assert(shl_ring_get_size(&r) == 2049);

	l = shl_ring_peek(&r, vec);
	ck_assert(l == 2);
	ck_assert(vec[0].iov_len == 2048);
	ck_assert(vec[1].iov_len == 1);
	ck_assert(!memcmp(vec[0].iov_base, buf, vec[0].iov_len));
	ck_assert(!memcmp(vec[1].iov_base, buf, vec[1].iov_len));
	ck_assert(shl_ring_get_size(&r) == 2049);

	shl_ring_pull(&r, 2048);
	ck_assert(shl_ring_get_size(&r) == 1);

	l = shl_ring_peek(&r, vec);
	ck_assert(l == 1);
	ck_assert(vec[0].iov_len == 1);
	ck_assert(!memcmp(vec[0].iov_base, buf, vec[0].iov_len));
	ck_assert(shl_ring_get_size(&r) == 1);

	shl_ring_pull(&r, 1);
	ck_assert(shl_ring_get_size(&r) == 0);

	s = shl_ring_push(&r, buf, 2048);
	ck_assert(!s);
	ck_assert(shl_ring_get_size(&r) == 2048);

	s = shl_ring_push(&r, buf, 2049);
	ck_assert(!s);
	ck_assert(shl_ring_get_size(&r) == 4097);

	l = shl_ring_peek(&r, vec);
	ck_assert(l == 1);
	ck_assert(vec[0].iov_len == 4097);
	ck_assert(!memcmp(vec[0].iov_base, buf, vec[0].iov_len));
	ck_assert(shl_ring_get_size(&r) == 4097);

	shl_ring_pull(&r, 1);
	ck_assert(shl_ring_get_size(&r) == 4096);

	s = shl_ring_push(&r, buf, 4096);
	ck_assert(!s);
	ck_assert(shl_ring_get_size(&r) == 8192);

	l = shl_ring_peek(&r, vec);
	ck_assert(l == 2);
	ck_assert(vec[0].iov_len == 8191);
	ck_assert(vec[1].iov_len == 1);
	ck_assert(!memcmp(vec[0].iov_base, buf, vec[0].iov_len));
	ck_assert(!memcmp(vec[1].iov_base, buf, vec[1].iov_len));
	ck_assert(shl_ring_get_size(&r) == 8192);

	shl_ring_clear(&r);
	ck_assert(shl_ring_get_size(&r) == 0);
}
END_TEST

TEST_DEFINE_CASE(setup)
	TEST(test_ring_setup)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(ring,
		TEST_CASE(setup),
		TEST_END
	)
)
