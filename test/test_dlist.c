/*
 * SHL - DList Tests
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain.
 */

#include "test_common.h"

static struct shl_dlist list;

struct node {
	char huge_padding[16384];
	uint8_t v;
	char paaaaaadding[16384];
	struct shl_dlist list;
	char more_padding[32768];
};

static struct node o[] = {
	{ .v = 0 },
	{ .v = 1 },
	{ .v = 2 },
	{ .v = 3 },
	{ .v = 4 },
	{ .v = 5 },
	{ .v = 6 },
	{ .v = 7 },
};

START_TEST(test_dlist_setup)
{
	int num, i;

	/* list is zeroed (global variable), so it cannot be empty */
	ck_assert(!shl_dlist_empty(&list));

	shl_dlist_init(&list);
	ck_assert(shl_dlist_empty(&list));

	shl_dlist_link(&list, &o[0].list);
	shl_dlist_link(&list, &o[1].list);
	shl_dlist_link(&list, &o[2].list);
	shl_dlist_link(&list, &o[3].list);
	shl_dlist_link(&list, &o[4].list);
	shl_dlist_link(&list, &o[5].list);
	shl_dlist_link(&list, &o[6].list);
	shl_dlist_link(&list, &o[7].list);

	num = 0;
	while (!shl_dlist_empty(&list)) {
		++num;
		shl_dlist_unlink(shl_dlist_last(&list));
	}
	ck_assert(num == 8);

	for (i = 0; i < 8; ++i) {
		ck_assert(!o[i].list.next);
		ck_assert(!o[i].list.prev);
	}
}
END_TEST

TEST_DEFINE_CASE(setup)
	TEST(test_dlist_setup)
TEST_END_CASE

START_TEST(test_dlist_iter)
{
	int num, i;
	struct shl_dlist *iter, *tmp;

	shl_dlist_init(&list);

	shl_dlist_link(&list, &o[0].list);
	shl_dlist_link_tail(&list, &o[1].list);
	shl_dlist_link_tail(&list, &o[2].list);
	shl_dlist_link(&list, &o[3].list);
	shl_dlist_link(&list, &o[4].list);
	shl_dlist_link_tail(&list, &o[5].list);
	shl_dlist_link(&list, &o[6].list);
	shl_dlist_link_tail(&list, &o[7].list);

	iter = shl_dlist_first(&list);
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 6);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 4);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 3);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 0);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 1);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 2);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 5);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 7);
	iter = iter->next;
	ck_assert(iter == &list);

	num = 0;
	shl_dlist_for_each_safe(iter, tmp, &list) {
		if (num++ == 2)
			shl_dlist_unlink(iter);
	}

	iter = shl_dlist_first(&list);
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 6);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 4);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 0);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 1);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 2);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 5);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 7);
	iter = iter->next;
	ck_assert(iter == &list);

	num = 0;
	shl_dlist_for_each_reverse_safe(iter, tmp, &list) {
		if (num++ == 2)
			shl_dlist_unlink(iter);
	}

	iter = shl_dlist_first(&list);
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 6);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 4);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 0);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 1);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 5);
	iter = iter->next;
	ck_assert(shl_dlist_entry(iter, struct node, list)->v == 7);
	iter = iter->next;
	ck_assert(iter == &list);

	num = 0;
	shl_dlist_for_each_but_one(iter, &o[0].list, &list)
		++num;
	ck_assert(num == 5);

	num = 0;
	shl_dlist_for_each_but_one(iter, &o[0].list, &list)
		if (num++ == 1)
			ck_assert(shl_dlist_entry(iter, struct node, list)->v == 5);
		else if (num++ == 2)
			ck_assert(shl_dlist_entry(iter, struct node, list)->v == 7);

	num = 0;
	shl_dlist_for_each_reverse_but_one(iter, &o[0].list, &list)
		if (num++ == 1)
			ck_assert(shl_dlist_entry(iter, struct node, list)->v == 6);
		else if (num++ == 2)
			ck_assert(shl_dlist_entry(iter, struct node, list)->v == 7);
}
END_TEST

TEST_DEFINE_CASE(iter)
	TEST(test_dlist_iter)
TEST_END_CASE

TEST_DEFINE(
	TEST_SUITE(dlist,
		TEST_CASE(setup),
		TEST_CASE(iter),
		TEST_END
	)
)
