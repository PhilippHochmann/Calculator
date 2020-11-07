#pragma once
#include <stdbool.h>
#include "matching.h"

// Special rule prefixes:
#define MATCHING_CONST_PREFIX        'c'
#define MATCHING_CONST_OR_VAR_PREFIX 'b'
#define MATCHING_OP_OR_VAR_PREFIX    'd'
#define MATCHING_OP_PREFIX           'o'
#define MATCHING_LITERAL_VAR_PREFIX  'l'

bool prefix_filter(const char *var, NodeList nodes, Matching *m);
bool exponent_even_filter(const char *var, NodeList nodes, Matching *m);
bool constant_derivative_filter(const char *var, NodeList nodes, Matching *m);