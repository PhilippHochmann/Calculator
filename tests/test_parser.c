#include <stdio.h>
#include <math.h>

#include "test_parser.h"
#include "../src/parsing/context.h"
#include "../src/parsing/node.h"
#include "../src/parsing/parser.h"
#include "../src/arithmetics/arith_context.h"

// To check if parsed tree evaluates to expected value
struct ValueTest {
    char *input;
    double result; 
};

// To check if parser returns expected error on malformed inputs
struct ErrorTest {
    char *input;
    ParserError result;
};

static const size_t NUM_VALUE_CASES = 49;
static struct ValueTest valueTests[] = {
    // 1. Basic prefix, infix, postfix
    { "2+3",  5 },
    { "2-3", -1 },
    { "2*3",  6 },
    { "4/2",  2 },
    { "2^3",  8 },
    { "-3",  -3 },
    { "+99", 99 },
    { "4!",  24 },
    { "3%",   0.03 },
    // 2. Precedence and parentheses
    { "1+2*3+4",      11 },
    { "1+2*(3+4)",    15 },
    { " ( 9.0 *  0)",  0 },
    // 3. Associativity
    { "1-2-3",             -4 },
    { "1-2-3 - ((1-2)-3)", 0 },
    { "2^2^3",             256 },
    { "2^2^3 - 2^(2^3)",   0 },
    // 4. Functions
    // 4.1. Constants
    { "pi",        3.141592653 },
    { "pi + 2",    5.141592653 },
    { "3+pi",      6.141592653 },
    { "pi2" ,      6.283185307 },
    { "pi(2)",     6.283185307 },
    { "pi()",      3.141592653 },
    { "pi() + 2",  5.141592653 },
    { "3+pi()",    6.141592653 },
    { "pi()2" ,    6.283185307 },
    { "pi()(2)",   6.283185307 },
    { "pi()e",     8.539734222 },
    { "sum",       0 },
    { "sum + 2",   2 },
    { "3+sum",     3 },
    { "sum2" ,     0 },
    { "sum(2)",    2 },
    { "sum()",     0 },
    { "sum() + 2", 2 },
    { "3+sum()",   3 },
    { "sum()2" ,   0 },
    { "sum()(2)",  0 },
    // 4.2. Unary functions
    { "sin(2)",      0.909297426 },
    { "sin(2)*3",    2.727892280 },
    { "sin(-2)%*3", -0.027278922 },
    { "sin2",        0.909297426 },
    { "sin2*3",      2.727892280 },
    { "sin-2%*3",   -0.027278922 },
    // 4.3. Binary functions and dynamic arity
    { "log(2 64, 1+1)", 7 },
    { "sum(1,2,3)",     6 },
    { "prod(2,3,4)-4!", 0 },
    // 5. Going wild
    { "5 .5sin2+5pi5", 80.81305990681 },
    { "--(1+sum(ld--8, --1%+--1%, 2 .2))%+1", 1.0442 },
    { "-sqrt(abs(--2!!*--sum(-1+.2-.2+2, 2^2^3-255, -sum(.1, .9), 1+2)*--2!!))", -4 },
};

static const size_t NUM_ERROR_CASES = 9;
static struct ErrorTest errorTests[] = {
    { "",          PERR_EMPTY },
    { "()",        PERR_EMPTY },
    { "x+",        PERR_MISSING_OPERAND },
    { "sin",       PERR_FUNCTION_WRONG_ARITY },
    { "sin(x, y)", PERR_FUNCTION_WRONG_ARITY },
    { "sin,",      PERR_UNEXPECTED_DELIMITER },
    { "-(1,2)",    PERR_UNEXPECTED_DELIMITER },
    { "(x",        PERR_EXCESS_OPENING_PARENTHESIS },
    { "x)",        PERR_EXCESS_CLOSING_PARENTHESIS }
};

static const double EPSILON = 0.00000001;
bool almost_equals(double a, double b)
{
    return (fabs(a - b) < EPSILON);
}

bool parser_test()
{
    Node *node = NULL;

    // Perform value tests
    for (size_t i = 0; i < NUM_VALUE_CASES; i++)
    {
        if (parse_input(g_ctx, valueTests[i].input, &node) != PERR_SUCCESS)
        {
            printf("[0] Parser Error in '%s'\n", valueTests[i].input);
            return false;
        }
        if (!almost_equals(arith_eval(node), valueTests[i].result))
        {
            printf("[0] Unexpected result in '%s'\n", valueTests[i].input);
            goto error;
        }
        free_tree(node);
    }

    // Perform error tests
    for (size_t i = 0; i < NUM_ERROR_CASES; i++)
    {
        if (parse_input(g_ctx, errorTests[i].input, &node) != errorTests[i].result)
        {
            printf("[0] Unexpected error type in '%s'\n", errorTests[i].input);
            goto error;
        }
    }

    return true;

    error:
    free_tree(node);
    return false;
}

Test get_parser_test()
{
    return (Test){
        parser_test,
        NUM_ERROR_CASES + NUM_VALUE_CASES,
        "Parser"
    };
}