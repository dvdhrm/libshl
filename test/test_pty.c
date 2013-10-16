/*
 * SHL - PTY Tests
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

#include "test_common.h"

static const char sndmsg[] = "message\n";
static const char rcvmsg[] = "message\r\n";

static void run_child(void)
{
	int r, l;
	char buf[512];

	r = read(0, buf, sizeof(buf));
	ck_assert(r == strlen(sndmsg));
	ck_assert(!strncmp(buf, sndmsg, r));

	l = write(1, buf, r);
	ck_assert(l == r);
}

static void pty_cb(struct shl_pty *pty, char *u8, size_t len, void *data)
{
	static int i;

	ck_assert(len == strlen(rcvmsg));
	ck_assert(!strncmp(u8, rcvmsg, len));
}

static void run_parent(int bridge, struct shl_pty *pty)
{
	shl_pty_write(pty, sndmsg, strlen(sndmsg));
	shl_pty_dispatch(pty);

	shl_pty_bridge_dispatch(bridge, -1);
}

START_TEST(test_pty_setup)
{
	int bridge, r;
	struct shl_pty *pty;
	pid_t pid;

	bridge = shl_pty_bridge_new();
	ck_assert(bridge >= 0);

	pid = shl_pty_open(&pty, pty_cb, NULL, 80, 25);
	ck_assert(pid >= 0);

	if (!pid) {
		/* child */
		ck_assert(pty == NULL);
		run_child();
		exit(0);
	}

	/* parent */
	ck_assert(pty != NULL);
	ck_assert(shl_pty_is_open(pty));
	ck_assert(shl_pty_get_child(pty) == pid);
	ck_assert(shl_pty_get_fd(pty) >= 0);

	r = shl_pty_bridge_add(bridge, pty);
	ck_assert(r >= 0);

	run_parent(bridge, pty);

	ck_assert(pid == wait(&r));
	ck_assert(!r);

	shl_pty_bridge_remove(bridge, pty);

	shl_pty_close(pty);
	ck_assert(!shl_pty_is_open(pty));
	shl_pty_unref(pty);

	shl_pty_bridge_free(bridge);
}
END_TEST

TEST_DEFINE_CASE(setup)
	TEST(test_pty_setup)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(pty,
		TEST_CASE(setup),
		TEST_END
	)
)
