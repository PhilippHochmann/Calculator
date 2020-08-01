#include <string.h>
#include "trie.h"

TrieNode *malloc_trienode(size_t elem_size)
{
    return calloc(1, sizeof(TrieNode) + elem_size);
}

Trie trie_create(size_t elem_size)
{
    return (Trie){
        .first_node = malloc_trienode(elem_size),
        .elem_size  = elem_size
    };
}

void destroy_rec(TrieNode *node)
{
    for (unsigned char i = 0; i < END_CHAR - START_CHAR; i++)
    {
        if (node->next[i] != NULL) destroy_rec(node->next[i]);
    }
    free(node);
}

void trie_destroy(Trie *trie)
{
    destroy_rec(trie->first_node);
}

void trie_add_str(Trie *trie, char *string, void *data)
{
    TrieNode *curr = trie->first_node;
    for (size_t i = 0; string[i] != '\0'; i++)
    {
        if (string[i] < START_CHAR || string[i] > END_CHAR)
        {
            return;
        }

        unsigned char index = string[i] - START_CHAR;
        if (curr->next[index] == NULL)
        {
            curr->next[index] = malloc_trienode(trie->elem_size);
        }
        curr = curr->next[index];
    }

    // curr is end node of inserted string
    curr->is_leaf = true;
    if (data != NULL) memcpy((void*)curr->data, data, trie->elem_size);
}

bool is_node_empty(TrieNode *node)
{
    for (unsigned char i = 0; i < END_CHAR - START_CHAR; i++)
    {
        if (node->next[i] != NULL) return false;
    }
    return true;
}

bool remove_rec(TrieNode *node, size_t depth, char *string)
{
    if (depth == strlen(string))
    {
        node->is_leaf = false;
    }
    else
    {
        if (string[depth] < START_CHAR || string[depth] > END_CHAR) return false;
        unsigned char index = string[depth] - START_CHAR;
        if (node->next[index] != NULL)
        {
            if (remove_rec(node->next[index], depth + 1, string))
            {
                node->next[index] = NULL;
            }
        }
    }

    // Never free the first trie node
    if (is_node_empty(node) && depth != 0)
    {
        free(node);
        return true;
    }
    else
    {
        return false;
    }
}

void trie_remove_str(Trie *trie, char *string)
{
    remove_rec(trie->first_node, 0, string);
}

bool trie_contains(Trie *trie, char *string, void *out_data)
{
    return trie_longest_prefix(trie, string, out_data) == strlen(string);
}

size_t trie_longest_prefix(Trie *trie, char *string, void *out_data)
{
    size_t res = 0;
    TrieNode *curr = trie->first_node;
    if (out_data != NULL) memcpy(out_data, curr->data, trie->elem_size);

    for (size_t i = 0; string[i] != '\0'; i++)
    {
        // Char in string that can not be in trie
        if (string[i] < START_CHAR || string[i] > END_CHAR) return res;

        unsigned char index = string[i] - START_CHAR;
        if (curr->next[index] != NULL)
        {
            curr = curr->next[index];
        }
        else
        {
            return res;
        }

        if (curr->is_leaf)
        {
            res = i + 1;
            if (out_data != NULL) memcpy(out_data, curr->data, trie->elem_size);
        }
    }

    return res;
}