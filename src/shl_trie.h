/*
 * SHL - crit-bit trie
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain
 */

/*
 * Crit-bit Trie
 * This is a fast implementation of crit-bit tries. It is used to store
 * string->data associastions. It supports fast inserts, removals and lookups.
 * Furthermore, fast prefix-searches are supported.
 */

#ifndef SHL_TRIE_H
#define SHL_TRIE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Trie Head
 * Embed this structure in your objects where you want the trie head to reside.
 * You can either initialize the memory to 0 or use shl_trie_zero() to do that.
 */
struct shl_trie {
	void *root;
};

/*
 * Zero out a Trie
 * You must clear a trie head to zero before using it. Either do that via
 * memset(.., 0, ..) or use this helper.
 * You must not call this helper on an non-empty initialized trie. If the trie
 * is empty, it is safe to call this.
 *
 * To destroy a trie, simply remove all elements from it. In this case, the
 * trie has no more allocated memory and can simply be removed. If you cannot
 * guarantee that all elements are removed, use shl_trie_clear().
 */
static inline void shl_trie_zero(struct shl_trie *trie)
{
	trie->root = NULL;
}

/*
 * Clear a Trie
 * This traverses the given trie and calls @free_cb() on each node. Note that it
 * does not provide the key-name to the callback. This is because keys are not
 * stored in the trie itself but the caller is required to allocate memory for
 * them. Therefore, it is very likely that the key is stored in @data itself and
 * no reason to pass it here.
 * You can pass a context @ctx which is passed through untouched to the
 * callbacks.
 *
 * You must not perform any trie operation from within the callback! The trie is
 * in inconsistent state during traversal. Any insert/remove/lookup will fail or
 * even end up in an endless loop!
 *
 * Once you cleared the trie, there is no more memory allocated. You're free to
 * destroy it or start inserting elements again. No need to call shl_trie_zero()
 * again.
 */
void shl_trie_clear(struct shl_trie *trie,
		    void (*free_cb) (const uint8_t *key, void *data, void *ctx),
		    void *ctx);

/* Same as shl_trie_clear() but with "const char" as key argument. */
static inline void shl_trie_clear_str(struct shl_trie *trie,
				      void (*free_cb) (const char *key,
				                       void *data,
				                       void *ctx),
				      void *ctx)
{
	return shl_trie_clear(trie,
			      (void(*)(const uint8_t*, void*, void*))free_cb,
			      ctx);
}

/*
 * Lookup an element
 * This searches the trie for a string @key. @keylen is the string-length of
 * @key. Note that @key must be zero-terminated. The length is only needed for
 * performance reasons. You may use strlen().
 *
 * Returns false if the element couldn't be found. Otherwise, true is returned
 * and the stored pointer is returned in @out.
 */
bool shl_trie_lookup(struct shl_trie *trie, const uint8_t *key, size_t keylen,
		     void **out);

/* Same as shl_trie_lookup() but calls strlen() on the given string so you
 * don't need to know the string-length. */
static inline bool shl_trie_lookup_str(struct shl_trie *trie, const char *str,
				       void **out)
{
	return shl_trie_lookup(trie, (const uint8_t*)str, strlen(str), out);
}

#define SHL_TRIE_OVERWRITE true
#define SHL_TRIE_NO_OVERWRITE false

/*
 * Insert an element
 * Insert a new element into the trie. The key is a zero-terminated string @key
 * (with string-length @keylen). @data is stored together with the key.
 * This function fails with -EALREADY if the key is already present. Use
 * SHL_TRIE_OVERWRITE as @overwrite argument to make this function silently
 * overwrite any present entry.
 * Returns 0 on success, -ENOMEM if out of memory.
 *
 * Note that the storage of @key must be valid as long as the element is stored
 * in the trie. The key is _not_ copied into the trie. The caller must dup it or
 * store it in the context saved as @data.
 */
int shl_trie_insert(struct shl_trie *trie, const uint8_t *key, size_t keylen,
		    void *data, bool overwrite);

/* Same as shl_trie_insert() but uses strlen() to get the key-length. */
static inline int shl_trie_insert_str(struct shl_trie *trie, const char *str,
				      void *data, bool overwrite)
{
	return shl_trie_insert(trie, (const uint8_t*)str, strlen(str), data,
			       overwrite);
}

/*
 * Remove an element
 * Search the trie for @key (with key-length @keylen). If not found, return
 * false. Otherwise, return true and store the key in @key_out and data in
 * @data_out.
 * Neither the key nor data are touched by the trie. The caller has to free them
 * if they were allocated on the heap.
 *
 * You can set @key_out or/and @data_out to NULL to avoid returning the given
 * pointer.
 */
bool shl_trie_remove(struct shl_trie *trie, const uint8_t *key, size_t keylen,
		     const uint8_t **key_out, void **data_out);

/* Same as shl_trie_remove() but with "const char" as key type. */
static inline bool shl_trie_remove_str(struct shl_trie *trie, const char *str,
				       const char **key_out, void **data_out)
{
	return shl_trie_remove(trie, (const uint8_t*)str, strlen(str),
			       (const uint8_t**)key_out, data_out);
}

/*
 * Visit matching elements
 * This traverses the trie and visits elements with the given prefix @prefix
 * (with string-length @plen). If @prefix is NULL, @plen is ignored and the
 * whole trie is visited.
 * For each matching element, the callback do_cb() (if non-NULL) is called. You
 * can pass a context @ctx to the callbacks. It is left untouched by this code.
 *
 * You must not call any trie function from within the callbacks. The trie
 * itself is in an inconsistent state during the callback (to track state).
 *
 * Note that prefix-search is _fast_. In fact, all elements with a common prefix
 * share the same sub-tree. So all this function does is find this sub-tree and
 * then traverse it.
 */
void shl_trie_visit(struct shl_trie *trie, const uint8_t *prefix, size_t plen,
		    void (*do_cb) (const uint8_t *key, void *data, void *ctx),
		    void *ctx);

/* Same as shl_trie_visit() but with "const char" as key type. */
static inline void shl_trie_visit_str(struct shl_trie *trie,
				      const char *prefix,
				      void (*do_cb) (const char *key,
				                     void *data, void *ctx),
				      void *ctx)
{
	return shl_trie_visit(trie, (const uint8_t*)prefix,
			      prefix ? strlen(prefix) : 0,
			      (void(*)(const uint8_t*, void*, void*))do_cb,
			      ctx);
}

#endif  /* SHL_TRIE_H */
