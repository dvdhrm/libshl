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
#include <sys/uio.h>

struct shl_buf {
	char *buf;
	size_t size;
	size_t len;
};

/* Compute next higher power-of-2 of @v. Returns 4096 in case v is 0. */
static inline size_t shl_buf_pow2(size_t v)
{
	size_t i;

	if (!v)
		return 4096;

	--v;

	for (i = 1; i < 8 * sizeof(size_t); i *= 2)
		v |= v >> i;

	return ++v;
}

static inline int shl_buf_push(struct shl_buf *b, const char *d, size_t l)
{
	size_t nlen;
	char *buf;

	if (b->size - b->len < l) {
		nlen = shl_buf_pow2(b->len + l);
		if (nlen <= b->size)
			return -ENOMEM;

		buf = realloc(b->buf, nlen);
		if (!buf)
			return -ENOMEM;

		b->buf = buf;
		b->size = nlen;
	}

	memcpy(&b->buf[b->len], d, l);
	b->len += l;

	return 0;
}

static inline void shl_buf_pull(struct shl_buf *b, size_t l)
{
	if (l > b->len)
		l = b->len;

	memmove(b->buf, &b->buf[l], b->len - l);
	b->len -= l;
}

static inline void shl_buf_flush(struct shl_buf *b)
{
	b->len = 0;
}

static inline void shl_buf_clear(struct shl_buf *b)
{
	free(b->buf);
	memset(b, 0, sizeof(*b));
}

#endif  /* SHL_BUF_H */
