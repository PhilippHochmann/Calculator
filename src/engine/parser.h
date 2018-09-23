#pragma once
#include "context.h"

typedef enum {
    // No error
    PERR_SUCCESS,
    // Not initialized
    PERR_NOT_INIT,
    // NULL-Pointer in arguments
    PERR_ARGS_MALFORMED,
    // Checked at pre-processing
    PERR_CTX_MALFORMED,
    // Token buffer overflow
    PERR_MAX_TOKENS_EXCEEDED,
    // op-stack or node-stack overflow
    PERR_STACK_EXCEEDED,
    // Only without glue-op
    PERR_UNEXPECTED_SUBEXPRESSION,
    // There are opening parenthesis on op-stack after all tokens are processed
    PERR_UNEXPECTED_OPENING_PARENTHESIS,
    // Mismatch: no opening parenthesis found
    PERR_UNEXPECTED_CLOSING_PARENTHESIS,
    // Delimiter too soon (e.g. 1+,)
    PERR_UNEXPECTED_DELIMITER,
    // Too many children
    PERR_MISSING_OPERATOR,
    // Could not pop 'arity' amount of nodes
    PERR_MISSING_OPERAND,
    // Arity too high (maybe dynamic)
    PERR_EXCEEDED_MAX_CHILDREN,
    // Function of wrong arity
    PERR_FUNCTION_WRONG_ARITY,
    // Not a single node
    PERR_EMPTY,
} ParserError;

void init_parser();
void uninit_parser();
ParserError parse_node(ParsingContext *ctx, char *input, Node **res);
