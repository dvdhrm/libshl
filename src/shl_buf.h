/*
 * SHL - Dynamic buffer
 *
 * Copyright (c) 2011-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain
 */

/*
 * Dynamic buffer
 */

#ifndef SHL_BUF_H
#define SHL_BUF_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "shl_macro.h"
#include "shl_util.h"

struct shl_buf {
	uint8_t *buf;
	size_t size;
	size_t used;
};

static inline void shl_buf_flush(struct shl_buf *b)
{
	b->used = 0;
}

static inline void shl_buf_clear(struct shl_buf *b)
{
	free(b->buf);
	shl_zero(*b);
}

static inline void *shl_buf_get_data(struct shl_buf *b)
{
	return b->buf;
}

static inline size_t shl_buf_get_size(struct shl_buf *b)
{
	return b->used;
}

static inline int shl_buf_push(struct shl_buf *b, const void *d, size_t l)
{
	size_t nlen;

	if (!l)
		return 0;

	nlen = b->used + l;
	if (nlen <= b->used)
		return -ENOMEM;

	if (!shl_greedy_realloc((void**)&b->buf, &b->size, nlen))
		return -ENOMEM;

	memcpy(&b->buf[b->used], d, l);
	b->used += l;

	return 0;
}

static inline void shl_buf_pop(struct shl_buf *b, size_t l)
{
	b->used -= shl_min(b->used, l);
}

static inline void shl_buf_pull(struct shl_buf *b, size_t l)
{
	l = shl_min(b->used, l);
	memmove(b->buf, &b->buf[l], b->used - l);
	b->used -= l;
}

#endif  /* SHL_BUF_H */
