#pragma once
#include <stdarg.h>
#include <stdbool.h>
#include "operator.h"

struct ParsingContext;

typedef bool (*TryParseHandler)(char *in, void *out);
typedef size_t (*ToStringHandler)(void *in, size_t buffer_size, char *out);
typedef bool (*EqualsHandler)(struct ParsingContext *ctx, void *a, void *b);

typedef struct ParsingContext
{
    size_t value_size;         // e.g. sizeof(bool) for propositional logic, sizeof(double) for arithmetic
    size_t recommended_str_len;     // Needed to let parsing know how much data to allocate for stringed value
    size_t num_ops;            // Current count of operators 
    size_t max_ops;            // Maximum count of operators (limited by buffer size)
    TryParseHandler try_parse; // Used to detect a literal token
    ToStringHandler to_string; // Used to print trees
    EqualsHandler equals;      // Used in node_equals, tree_equals
    Operator *glue_op;         // Points to op in operators
    Operator *operators;       // On heap!
} ParsingContext;

ParsingContext get_context(
    size_t value_size,
    size_t recommended_str_len,
    size_t max_ops,
    TryParseHandler try_parse,
    ToStringHandler to_string,
    EqualsHandler handler);
void free_context(ParsingContext *ctx);
bool ctx_add_ops(ParsingContext *ctx, size_t count, ...);
int ctx_add_op(ParsingContext *ctx, Operator op);
bool ctx_set_glue_op(ParsingContext *ctx, Operator *op);
void ctx_remove_glue_op(ParsingContext *ctx);
Operator *ctx_lookup_op(ParsingContext *ctx, char *name, OpPlacement placement);
Operator *ctx_lookup_function(ParsingContext *ctx, char *name, size_t arity);