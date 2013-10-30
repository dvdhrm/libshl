/*
 * SHL - Ring buffer
 *
 * Copyright (c) 2011-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain
 */

/*
 * Ring buffer
 */

#ifndef SHL_RING_H
#define SHL_RING_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

struct shl_ring {
	char *buf;
	size_t size;
	size_t start;
	size_t end;
};

int shl_ring_push(struct shl_ring *r, const char *u8, size_t len);
size_t shl_ring_peek(struct shl_ring *r, struct iovec *vec);
void shl_ring_pop(struct shl_ring *r, size_t len);
void shl_ring_clear(struct shl_ring *r);

#endif  /* SHL_RING_H */
