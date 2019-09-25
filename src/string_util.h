#include <stdbool.h>
#include "parsing/context.h"
#include "parsing/node.h"

bool begins_with(char *prefix, char *string);
void print_tree_visual(ParsingContext *ctx, Node *node);
size_t tree_inline(ParsingContext *ctx, Node *node, char *buffer, size_t buffer_size, bool color);