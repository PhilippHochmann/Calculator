#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "iterator.h"

// To save a little bit of space, rule out portions of the ASCII table that will
// never occur in strings inserted into the trie.
#define TRIE_START_CHAR '!'
#define TRIE_END_CHAR   ('|' + 1) // Exclusive bound
#define TRIE_MAX_ITERATOR_DEPTH 30

#define TRIE_ADD_ELEM(trie, str, type, expr) (*(type*)trie_add_str(trie, str) = (expr))

typedef struct TrieNode TrieNode;

typedef struct
{
    size_t elem_size;
    size_t count;
    TrieNode *first_node;
} Trie;

typedef struct
{
    Iterator base;
    const Trie *trie;
    char curr_str[TRIE_MAX_ITERATOR_DEPTH];
    const TrieNode *nodes[TRIE_MAX_ITERATOR_DEPTH];
} TrieIterator;

Trie trie_create(size_t elem_size);
void trie_destroy(Trie *trie);
void *trie_add_str(Trie *trie, const char *string);
void trie_remove_str(Trie *trie, const char *string);
bool trie_contains(const Trie *trie, const char *string, void **out_data);
size_t trie_longest_prefix(const Trie *trie, const char *string, void **out_data);
size_t trie_count(const Trie *trie);

TrieIterator trie_get_iterator(const Trie *trie);
const char *trie_get_current_string(const TrieIterator *iterator);
