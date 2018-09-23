#pragma once
#include <stdbool.h>

#include "operator.h"

typedef bool (*TryParseHandler)(char *in, void *out);
typedef void (*ToStringHandler)(void *in, char *out, size_t buff_size);
typedef bool (*EqualsHandler)(void *a, void *b);

typedef struct {

    size_t value_size; // e.g. sizeof(bool) for propositional logic, sizeof(double) for arithmetic
    size_t min_str_len; // Needed to let external functions know how much data to allocate for stringed value
    
    int num_ops;
    int max_ops;
        
    TryParseHandler try_parse;
    ToStringHandler to_string;
    EqualsHandler equals;
    
    Operator *glue_op;
    Operator *operators; // On heap!

} ParsingContext;

ParsingContext get_context(
    size_t val_size,
    size_t min_strbuf_length,
    int max_ops,
    TryParseHandler try_parse,
    ToStringHandler to_string,
    EqualsHandler handler);
    
int add_op(ParsingContext *ctx, Operator op);
bool set_glue_op(ParsingContext *ctx, Operator *op);
void remove_glue_op(ParsingContext *ctx);
