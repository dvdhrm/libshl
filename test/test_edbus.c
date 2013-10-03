/*
 * SHL - edbus tests
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

#include "test_common.h"

static void test_dbus_verify_cb(DBusPendingCall *pending,
				void *data)
{
	DBusMessage *m;
	int *ack = data;

	m = dbus_pending_call_steal_reply(pending);
	ck_assert(!!m);
	ck_assert(dbus_message_get_type(m) == DBUS_MESSAGE_TYPE_METHOD_RETURN);

	dbus_message_unref(m);
	*ack = 1;
}

START_TEST(test_edbus_verify)
{
	struct shl_edbus *ctx;
	const char *t;
	DBusPendingCall *pending;
	DBusConnection *c;
	DBusMessage *m;
	int efd, r, ack;
	bool b;

	efd = shl_edbus_new_bus(DBUS_BUS_SYSTEM, &c, &ctx);
	ck_assert(efd >= 0);

	r = shl_edbus_dispatch(ctx, 100);
	ck_assert(!r);

	m = dbus_message_new_method_call(DBUS_SERVICE_DBUS,
					 DBUS_PATH_DBUS,
					 DBUS_INTERFACE_DBUS,
					 "GetNameOwner");
	ck_assert(!!m);

	t = "org.freedesktop.login1";
	b = dbus_message_append_args(m,
				     DBUS_TYPE_STRING, &t,
				     DBUS_TYPE_INVALID);
	ck_assert(b);

	b = dbus_connection_send_with_reply(c, m, &pending, -1);
	ck_assert(b);

	b = dbus_pending_call_set_notify(pending, test_dbus_verify_cb,
					 &ack, NULL);
	ck_assert(b);

	dbus_message_unref(m);

	ack = 0;
	r = shl_edbus_dispatch(ctx, 200);
	ck_assert(!r);
	ck_assert(ack);

	dbus_pending_call_cancel(pending);
	dbus_pending_call_unref(pending);

	shl_edbus_free(ctx);
}
END_TEST

TEST_DEFINE_CASE(verify)
	TEST(test_edbus_verify)
TEST_END_CASE

int main(int argc, char **argv)
{
	if (sd_booted() <= 0)
		return 77;

	return test_run_suite(TEST_SUITE(edbus,
					 TEST_CASE(verify),
					 TEST_END));
}
