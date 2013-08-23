/*
 * SHL - crit-bit trie
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain
 */

/*
 * Crit-bit Trie
 * This implements crit-bit tries. Our layout is very simple. A trie is simply a
 * pointer to the root node. NULL if the trie is empty. A trie consists of nodes
 * and entries. An entry is a leaf, a node is a branch.
 * Pointers to nodes and entries are "void*" with an alignment of at least 4.
 * The lower 2 bits are used as markers. The LSB distinguishes nodes from
 * entries. The second least bit is used during traversal and is only temporary.
 *
 * So before we deref a pointer, we always have to check whether it's a node or
 * entry first. We need to clear the LSB and then cast to the correct type.
 * If it's an entry, it's a trie leaf. It contains a data pointer and a pointer
 * to the key. The key itself is not stored in the trie, it has to be
 * user-allocated and managed.
 *
 * If a pointer is a node, we got to a binary branch. A node does not contain
 * any key, but only path information. A node has two void* pointers for left
 * and right child. They are never NULL! They can point to either a node or an
 * entry.
 * Additionally, every node contains prefix information.
 *
 * In a crit-bit trie, every sub-trie shares a common prefix. The root node of
 * every sub-trie contains information at which bit the left and right child's
 * prefix differs (via ->byte and ->otherbits). ->byte is the byte position of
 * the first difference between any key of the left trie and right trie.
 * ->otherbits is a bitmask with the most significant bit-position that differs
 * not set. So ->byte and ->otherbits can be used to identify the exact bit at
 * which the prefixes of the left and right trie start to differ.
 */

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "shl_trie.h"

struct shl_trie_node {
	void *childs[2];
	size_t byte;
	uint8_t otherbits;
};

/* We use a struct to simplify iterations. But it must be equivalent to a
 * simple pointer so tell gcc to pack it (just to be safe). */
struct shl_trie_entry {
	uint8_t *key;
} __attribute__ ((__packed__));

/* We use the LSBs of pointers to distinguish nodes from entries and store
 * traversal information. Hence, we need at least a 4-byte alignment.
 * posix_memalign requires at least sizeof(void*) so use this macro to get a
 * valid posix_memalign() alignment parameter.
 *
 * The LSB of pointers is used to distinguish nodes from entries. The second bit
 * is used during trie traversal to store state. */
#define SHL_TRIE_ALIGNMENT ((sizeof(void*) > 4) ? sizeof(void*) : 4)

/* Helper to use during insert to verify user-allocated objects have the
 * correct alignment */
static inline int shl_check_alignment(const void *ptr)
{
	unsigned long addr;

	addr = (unsigned long)ptr;
	if (addr & 0x3UL)
		return -EFAULT;

	return 0;
}

/* return true if @ptr is a node, otherwise it's an entry */
static inline bool shl_trie_is_node(void *ptr)
{
	unsigned long addr;

	/* we use the LSB to mark nodes */
	addr = (unsigned long)ptr;
	return !!(addr & 0x1UL);
}

/* casts @ptr to a node; caller must guarantee that @ptr is a node */
static inline struct shl_trie_node *shl_trie_get_node(void *ptr)
{
	unsigned long addr;

	/* remove LSB markers and return node */
	addr = (unsigned long)ptr;
	addr &= ~(0x3UL);
	return (struct shl_trie_node*)addr;
}

/* casts @ptr to an entry; caller must guarantee that @ptr is an entry */
static inline struct shl_trie_entry *shl_trie_get_entry(void *ptr)
{
	unsigned long addr;

	/* Entries have the LSB marker not set, but may have the visited
	 * marker set. */
	addr = (unsigned long)ptr;
	addr &= ~(0x2UL);
	return (struct shl_trie_entry*)addr;
}

/* cast @node into a node pointer for use in trie objects */
static inline void *shl_trie_make_node(struct shl_trie_node *node)
{
	unsigned long addr;

	/* add the LSB marker for nodes */
	addr = (unsigned long)node;
	addr |= 0x1UL;
	return (void*)addr;
}

/* cast @entry into an entry pointer for use in trie objects */
static inline void *shl_trie_make_entry(struct shl_trie_entry *entry)
{
	/* no markers for entries, just cast them */
	return entry;
}

/* return whether the node was already visited during traversal */
static inline bool shl_trie_is_visited(void *ptr)
{
	unsigned long addr;

	/* the second LSB is used to mark visited nodes */
	addr = (unsigned long)ptr;
	return !!(addr & 0x2UL);
}

/* clear the visited marker */
static inline void *shl_trie_clear_visited(void *ptr)
{
	unsigned long addr;

	/* the second LSB is used to mark visited nodes */
	addr = (unsigned long)ptr;
	addr &= ~(0x2UL);
	return (void*)addr;
}

/* mark as visited */
static inline void *shl_trie_mark_visited(void *ptr)
{
	unsigned long addr;

	/* the second LSB is used to mark visited nodes */
	addr = (unsigned long)ptr;
	addr |= 0x2UL;
	return (void*)addr;
}

/* allocate objects with alignment restrictions */
static inline int shl_trie_alloc(void **out, size_t size)
{
	int ret;

	ret = posix_memalign(out, SHL_TRIE_ALIGNMENT, size);
	return ret ? -ENOMEM : 0;
}

/* allocate node with alignment restrictions */
static inline int shl_trie_alloc_node(struct shl_trie_node **out)
{
	return shl_trie_alloc((void**)out, sizeof(**out));
}

bool shl_trie_lookup(struct shl_trie *trie, const uint8_t *key, size_t keylen,
		     uint8_t ***out)
{
	struct shl_trie_node *node;
	uint8_t c;
	uint16_t direction;
	void *iter;

	iter = trie->root;

	/* empty tries cannot contain @key */
	if (!iter)
		return false;

	/* find closest match; iterate until we get an entry */
	while (shl_trie_is_node(iter)) {
		node = shl_trie_get_node(iter);

		/* We assume every key is followed by an infinite string of
		 * binary zeros. That is, shorter strings are always the left
		 * child, longer strings the right. */
		if (node->byte < keylen)
			c = key[node->byte];
		else
			c = 0;

		/* Get direction. (q->otherbits | c) is either a full 8bit mask
		 * or a mask with 1 bit unset. If we add 1 to a full mask, we
		 * get 0x100. If we add 1 to as mask with only 1 bit unset, we
		 * get 0xff. Hence, a right-shift will give us either 0 or 1,
		 * which is the direction we go to. */
		direction = (1 + (uint16_t)(node->otherbits | c)) >> 8;

		iter = node->childs[direction];
	}

	/* Got an entry, but no clue whether it's correct. Test it! */
	if (strcmp((const char*)key,
		   (const char*)shl_trie_get_entry(iter)->key))
		return false;

	if (out)
		*out = &shl_trie_get_entry(iter)->key;
	return true;
}

int shl_trie_insert(struct shl_trie *trie, uint8_t **rkey, size_t keylen,
		    uint8_t ***out)
{
	const uint8_t *key = *rkey;
	struct shl_trie_entry *entry;
	struct shl_trie_node *node, *new;
	int ret;
	uint8_t c, newotherbits;
	uint16_t newdirection, direction;
	size_t newbyte;
	void *iter, **where;

	ret = shl_check_alignment(rkey);
	if (ret < 0)
		return ret;

	iter = trie->root;

	/* Empty trie? Insert it as root. */
	if (!iter) {
		trie->root = shl_trie_make_entry((struct shl_trie_entry*)rkey);
		return 0;
	}

	/* iterate until we get an entry, see shl_trie_lookup() */
	while (shl_trie_is_node(iter)) {
		node = shl_trie_get_node(iter);

		if (node->byte < keylen)
			c = key[node->byte];
		else
			c = 0;

		direction = (1 + (uint16_t)(node->otherbits | c)) >> 8;
		iter = node->childs[direction];
	}

	entry = shl_trie_get_entry(iter);

	/* We found the closest entry match @entry. Now find the first bit that
	 * differs between @entry and @key. Both keys are 0-terminated, so we
	 * can safely iterate @keylen. If @entry->key is shorter than @keylen,
	 * they differ in the 0-terminating byte. */

	for (newbyte = 0; newbyte < keylen; ++newbyte) {
		if (entry->key[newbyte] != key[newbyte]) {
			newotherbits = entry->key[newbyte] ^ key[newbyte];
			goto insert_new;
		}
	}

	/* The first @keylen bytes are equal, but maybe @entry->key is longer
	 * than @key. */
	if (entry->key[newbyte]) {
		newotherbits = entry->key[newbyte];
		goto insert_new;
	}

	if (out)
		*out = &entry->key;
	return -EALREADY;

insert_new:
	/* Here we know that @key is not present in our trie. Furthermore,
	 * @newotherbits contain the XOR on the first differing byte between the
	 * closest match and @key. Use some bit-magic to turn this into a 8-bit
	 * bitmask with only the most significant differing bit _not_ set. */
	newotherbits |= newotherbits >> 1;
	newotherbits |= newotherbits >> 2;
	newotherbits |= newotherbits >> 4;
	newotherbits = (newotherbits & ~(newotherbits >> 1)) ^ 0xff;

	/* Calculate direction of the existing trie suffix. That is,
	 * @newdirection is where the existing trie will hang off once we insert
	 * a new node. Our new entry for @key will go into the other
	 * direction. */
	c = entry->key[newbyte];
	newdirection = (1 + (uint16_t)(newotherbits | c)) >> 8;

	/* allocate new node */
	ret = shl_trie_alloc_node(&new);
	if (ret)
		return ret;

	/* get entry pointer */
	entry = shl_trie_make_entry((struct shl_trie_entry*)rkey);

	/* fill in node */
	new->byte = newbyte;
	new->otherbits = newotherbits;
	new->childs[newdirection ^ 1] = shl_trie_make_entry(entry);

	/* So earlier we searched for the closest match in the existing trie,
	 * measured by number of prefix-changes. However, our new key might not
	 * share the same prefixes as the existing entries. We did calculate a
	 * proper byte/bit difference, but now we need to find the place where
	 * to insert it.
	 * This isn't required for our searching algorithm, but it is required
	 * for the lexicographic order and prefix-searches.
	 * That is, we now perform a prefix-search to find the place where to
	 * insert our new entry. */

	where = &trie->root; /* guaranteed to be non-NULL */
	while (shl_trie_is_node(*where)) {
		node = shl_trie_get_node(*where);

		/* if the node does not share a common prefix, we're done */
		if (node->byte > newbyte)
			break;
		if (node->byte == newbyte && node->otherbits > newotherbits)
			break;

		/* node shares the prefix, go one node down */
		if (node->byte < keylen)
			c = key[node->byte];
		else
			c = 0;

		direction = (1 + (uint16_t)(node->otherbits | c)) >> 8;
		where = &node->childs[direction];
	}

	/* insert our node at the given position */
	new->childs[newdirection] = *where;
	*where = shl_trie_make_node(new);

	if (out)
		*out = &entry->key;
	return 0;
}

bool shl_trie_remove(struct shl_trie *trie, const uint8_t *key, size_t keylen,
		     uint8_t ***out)
{
	struct shl_trie_node *node;
	struct shl_trie_entry *entry;
	uint8_t c;
	uint16_t direction;
	void **where, **grandparent;

	/* empty tries cannot contain @key */
	if (!trie->root)
		return false;

	grandparent = NULL;
	where = &trie->root;
	while (shl_trie_is_node(*where)) {
		grandparent = where;
		node = shl_trie_get_node(*where);

		if (node->byte < keylen)
			c = key[node->byte];
		else
			c = 0;

		direction = (1 + (uint16_t)(node->otherbits | c)) >> 8;
		where = &node->childs[direction];
	}

	entry = shl_trie_get_entry(*where);

	/* Got an entry, but no clue whether it's correct. Test it! */
	if (strcmp((const char*)key,
		   (const char*)entry->key))
		return false;

	/* unlink entry from trie */
	if (!grandparent) {
		trie->root = NULL;
	} else {
		*grandparent = node->childs[direction ^ 1];
		free(node);
	}

	/* save entry so caller can access it */
	if (out)
		*out = &entry->key;

	return true;
}

enum shl_trie_traverse_flags {
	SHL_TRIE_FREE		= 0x1,
	SHL_TRIE_SKIP_ROOT	= 0x2,
};

static void shl_trie_traverse(void *sub_trie,
			      void (*do_cb) (uint8_t **key, void *ctx),
			      void *ctx, enum shl_trie_traverse_flags flags)
{
	struct shl_trie_node *node, *parent;
	struct shl_trie_entry *entry;
	void *iter;

	/*
	 * This is a bit tricky. We have no parent pointers in nodes, so we
	 * cannot easily traverse the trie. The two obvious methods are
	 * backtracking or recursive traversal. Both require huge amounts of
	 * stack-memory.
	 * So instead, we make use of the two child pointers in each node. Once
	 * we go down one child, we drop the pointer and instead store the
	 * parent pointer. We set the second LSB bit to mark a node as visited.
	 * We clear it once we go back to the parent and reset the child
	 * pointers again.
	 * This allows us to traverse the trie without backtracking.
	 *
	 * We probably need some benchmark here. Our context is only one pointer
	 * per recursion and the deepest recursion is only the longest key-len
	 * (this also requires "2 * key-len" entries in the trie!). Thus, the
	 * recursive approach will probably be faster and still within a
	 * limited stack memory range.
	 * Nevertheless, you must take into account that malicious input may
	 * exploit this! A typical user-space stack is 2MB, so if you allow huge
	 * user-controlled input strings, a user can trigger stack-overflows
	 * with the recursive approach. This iterative approach is safe against
	 * such attacks.
	 */

	parent = NULL;
	iter = sub_trie;
	while (iter) {
		/* If we got to an entry, we use the callback to let the caller
		 * visit the entry.
		 * The next iteration should continue with the parent. If there
		 * was no parent, @parent is NULL and we're fine. If there was a
		 * parent, @parent points to the parent node. Since the parent
		 * is always a node, there is no need to adjust the @parent
		 * pointer as nodes store them in their context temporarily.
		 * Instead, we save @iter in @parent, so the parent node can
		 * correctly reset the node pointer. */
		if (!shl_trie_is_node(iter)) {
			entry = shl_trie_get_entry(iter);
			if (do_cb)
				do_cb(&entry->key, ctx);

			iter = parent;
			/* @entry may be freed, but we never deref it */
			parent = shl_trie_make_entry(entry);
			continue;
		}

		/* @iter is a node! */
		node = shl_trie_get_node(iter);

		/* Test whether we already visited the right child. In this
		 * case, we're done with the node. We set @iter to the parent
		 * (which we stored in the child pointer), then reset the child
		 * pointer and continue with the parent. */
		if (shl_trie_is_visited(node->childs[1])) {
			iter = shl_trie_clear_visited(node->childs[1]);

			if (flags & SHL_TRIE_FREE)
				free(node);
			else
				node->childs[1] = shl_trie_clear_visited(parent);

			/* @node may be freed, but we never deref it */
			parent = shl_trie_make_node(node);
			continue;
		}

		/* Left child is already visited. Traverse the right child, but
		 * mark it as visited before so once we get back we know about
		 * it. Also set the @parent pointer, so we _can_ actually get
		 * back to this node. It was already set when we visited the
		 * left child, but it could have been reset while visiting the
		 * grand-childs.
		 * Note that the left node sets @parent to itself before
		 * returning to us. We use it to reset the left child node
		 * before recursing into the right one. */
		if (shl_trie_is_visited(node->childs[0])) {
			iter = node->childs[1];
			node->childs[1] = node->childs[0];
			node->childs[0] = shl_trie_clear_visited(parent);
			parent = shl_trie_make_node(node);
			continue;
		}

		/* Node is unvisited. We have a valid parent pointer, store it
		 * in childs[0]. Then traverse into childs[0] and set our node
		 * as the new parent.
		 * Mark the left child as visited, so once we get back we know
		 * about it. */
		iter = node->childs[0];
		node->childs[0] = shl_trie_mark_visited(parent);
		parent = shl_trie_make_node(node);
	}
}

void shl_trie_clear(struct shl_trie *trie,
		    void (*free_cb) (uint8_t **key, void *ctx),
		    void *ctx)
{
	shl_trie_traverse(trie->root, free_cb, ctx, SHL_TRIE_FREE);
	trie->root = NULL;
}

void shl_trie_visit(struct shl_trie *trie, const uint8_t *prefix, size_t plen,
		    void (*do_cb) (uint8_t **key, void *ctx),
		    void *ctx)
{
	void *iter, *top;
	struct shl_trie_node *node;
	struct shl_trie_entry *entry;
	uint8_t c;
	uint16_t direction;
	size_t i;

	/* empty tries are boring */
	if (!trie->root)
		return;

	/* if no prefix given, traverse the whole trie */
	if (!prefix || !plen)
		return shl_trie_traverse(trie, do_cb, ctx, 0);

	/* If a prefix is given, we search the trie for the longest common
	 * prefix and remember it as @top. We continue to find the best match so
	 * we can verify the prefixes actually match. If not, we have no
	 * matching entry in the whole trie. Otherwise, we simply traverse the
	 * given sub-tree. */
	iter = trie->root;
	top = iter;
	while (shl_trie_is_node(iter)) {
		node = shl_trie_get_node(iter);

		if (node->byte < plen)
			c = prefix[node->byte];
		else
			c = 0;

		direction = (1 + (uint16_t)(node->otherbits | c)) >> 8;
		iter = node->childs[direction];

		/* update @top only if the prefix is still in common */
		if (node->byte < plen)
			top = iter;
	}

	entry = shl_trie_get_entry(iter);

	/* if the prefix doesn't match, we have no matching entries */
	for (i = 0; i < plen; ++i)
		if (prefix[i] != entry->key[i])
			return;

	return shl_trie_traverse(top, do_cb, ctx, 0);
}
