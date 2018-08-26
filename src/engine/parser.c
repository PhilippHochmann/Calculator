#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include <stdio.h>
#include "constants.h"
#include "context.h"
#include "node.h"
#include "tokenizer.h"
#include "parser.h"

// Global vars while parsing
static bool initialized = false;
static ParsingContext *ctx;
static Node **node_stack = NULL;
static Operator **op_stack = NULL;
static int *arities = NULL;
static int num_nodes = 0;
static int num_ops = 0;
static ParserError error;

/*
Summary: Reserves memory for buffers used while parsing
*/
void init_parser()
{
	if (op_stack == NULL) op_stack = malloc(MAX_STACK_SIZE * sizeof(Operator*));
	if (node_stack == NULL) node_stack = malloc(MAX_STACK_SIZE * sizeof(Node*));
	if (arities == NULL) arities = malloc(MAX_STACK_SIZE * sizeof(int));
	initialized = true;
}

/*
Summary: Frees memory of buffers used while parsing
*/
void uninit_parser()
{
	if (op_stack != NULL) free(node_stack);
	if (node_stack != NULL) free(op_stack);
	if (arities != NULL) free(arities);
	initialized = false;
}

Operator* search_op(char *name, OpPlacement placement)
{
	for (int i = 0; i < ctx->num_ops; i++)
	{
		Operator *curr_op = &ctx->operators[i];
		
		if (curr_op->placement == placement
			&& strcmp(curr_op->name, name) == 0)
		{
			return curr_op;
		}
	}
	
	return NULL;
}

Operator* search_function(char *name, int arity)
{
	for (int i = 0; i < ctx->num_ops; i++)
	{
		Operator *curr_op = &ctx->operators[i];
		
		if (curr_op->placement == OP_PLACE_FUNCTION
			&& strcmp(curr_op->name, name) == 0
			&& curr_op->arity == arity)
		{
			return curr_op;
		}
	}
	
	return NULL;
}

bool node_push(Node *value)
{
	if (num_nodes == MAX_STACK_SIZE)
	{
		error = PERR_STACK_EXCEEDED;
		return false;
	}
	
	node_stack[num_nodes] = value;
	num_nodes++;
	
	return true;
}

bool node_pop(Node **out)
{
	if (num_nodes == 0)
	{
		error = PERR_MISSING_OPERAND;
		return false;
	}
	
	num_nodes--;
	*out = node_stack[num_nodes];
	
	return true;
}

bool op_pop_and_insert()
{
	if (num_ops == 0)
	{
		error = PERR_MISSING_OPERATOR;
		return false;
	}
	
	Operator *op = op_stack[num_ops - 1];
	bool is_function = (op != NULL && op->placement == OP_PLACE_FUNCTION);
	
	//if (is_function) printf("popped function! arity: %d ", arities[num_ops-1]);
	
	// Function overloading: Find function with suitable arity
	if (is_function)
	{
		if (op->arity != arities[num_ops - 1])
		{
			char *name = op->name;
			op = search_function(name, arities[num_ops - 1]);
			
			// Fallback: Find function of dynamic aritiy
			if (op == NULL)
			{
				op = search_function(name, DYNAMIC_ARITY);
			}
			
			if (op == NULL)
			{
				error = PERR_FUNCTION_WRONG_ARITY;
				return false;
			}
		}
	}
	
	if (op != NULL) // Construct operator-node and append children
	{
		Node *op_node = malloc(sizeof(Node));
		*op_node = get_operator_node(op);
		op_node->num_children = is_function ? arities[num_ops - 1] : op->arity;
		
		if (op_node->num_children > MAX_CHILDREN)
		{
			error = PERR_EXCEEDED_MAX_CHILDREN;
			return false;
		}
		
		for (int i = 0; i < op_node->num_children; i++)
		{
			if (!node_pop(&(op_node->children[op_node->num_children - i - 1]))) return false;
		}
		
		if (!node_push(op_node)) return false;
	}
	
	num_ops--;
	return true;
}

bool op_push(Operator *op)
{
	bool is_function = (op != NULL && op->placement == OP_PLACE_FUNCTION);
	
	if (op != NULL)
	{
		if (op->placement != OP_PLACE_PREFIX && !is_function)
		{
			while (
				num_ops > 0 &&
				op_stack[num_ops - 1] != NULL &&
				(op->precedence < op_stack[num_ops - 1]->precedence ||
					(op->precedence == op_stack[num_ops - 1]->precedence &&
					(op->assoc == OP_ASSOC_LEFT || op->assoc == OP_ASSOC_BOTH))))
			{
				if (!op_pop_and_insert()) return false;
			}
		}
	}
	
	if (num_ops == MAX_STACK_SIZE)
	{
		error = PERR_STACK_EXCEEDED;
		return false;
	}
	
	
	arities[num_ops] = (is_function ? 0 : -1);
	op_stack[num_ops] = op;
	num_ops++;
	return true;
}

/*
Summary: Parses string to abstract syntax tree with operators of given context
Returns: Error code to indicate whether string was parsed successfully or which error occured
*/
ParserError parse_node(ParsingContext *context, char *input, Node **res)
{
	// 0. Early outs
	if (context == NULL || input == NULL || res == NULL) return PERR_ARGS_MALFORMED;
	if (!initialized) return PERR_NOT_INIT;
	// Glue-op must be infix to "glue" two subtrees together
	if (context->glue_op != NULL && context->glue_op->placement != OP_PLACE_INFIX) return PERR_CTX_MALFORMED;
	
	// 1. Tokenize input
	int num_tokens = 0;
	char **tokens;
	char *keywords[context->num_ops];
	for (int i = 0; i < context->num_ops; i++)
	{
		keywords[i] = context->operators[i].name;
	}
	
	if (!tokenize(input, keywords, context->num_ops, &tokens, &num_tokens)) return PERR_MAX_TOKENS_EXCEEDED;

	// 2. Initialize data structures
	ctx = context;
	error = PERR_SUCCESS;
	num_ops = 0;
	num_nodes = 0;

	// 3. Process each token
	bool await_subexpression = true;
	for (int i = 0; i < num_tokens; i++)
	{
		if (error != PERR_SUCCESS) goto exit;
		
		char *token = tokens[i];
		size_t tok_len = strlen(token);
		
		// II. Does glue-op need to be inserted?
		if (!await_subexpression && ctx->glue_op != NULL)
		{
			if (!is_closing_parenthesis(token[0])
				&& !is_delimiter(token[0])
				&& search_op(token, OP_PLACE_INFIX) == NULL
				&& search_op(token, OP_PLACE_POSTFIX) == NULL)
			{
				if (!op_push(ctx->glue_op)) goto exit;
				await_subexpression = true;
			}
		}
		
		// III. Is token opening parenthesis?
		if (is_opening_parenthesis(token[0]))
		{
			if (!op_push(NULL)) goto exit;
			await_subexpression = true;
			continue;
		}

		// IV. Is token closing parenthesis or argument delimiter?
		if (is_closing_parenthesis(token[0]))
		{
			while (num_ops > 0 && op_stack[num_ops - 1] != NULL)
			{
				if (!op_pop_and_insert())
				{
					error = PERR_UNEXPECTED_CLOSING_PARENTHESIS;
					goto exit;
				}
			}
			
			if (num_ops > 0)
			{
				op_pop_and_insert();
			}
			else
			{
				error = PERR_UNEXPECTED_CLOSING_PARENTHESIS;
				goto exit;
			}
			
			bool empty_params = (i > 0 && is_opening_parenthesis(tokens[i - 1][0]));
			if (num_ops > 0 && arities[num_ops - 1] != -1 && !empty_params)
			{
				arities[num_ops - 1]++;
			}
			
			await_subexpression = false;
			
			continue;
		}
		
		if (is_delimiter(token[0]))
		{
			while (num_ops > 0 && op_stack[num_ops - 1] != NULL)
			{
				if (!op_pop_and_insert())
				{
					error = PERR_UNEXPECTED_DELIMITER;
					goto exit;
				}
			}
			
			// Increase operand counter
			if (num_ops > 1 && arities[num_ops - 2] != -1)
			{
				arities[num_ops - 2]++;
			}
			
			await_subexpression = true;
			
			continue;
		}
		// - - -
		
		// V. Is token operator?
		Operator *op = NULL;
		if (await_subexpression)
		{
			bool function_flag = false;
			op = search_op(token, OP_PLACE_FUNCTION);
			if (op != NULL) function_flag = true;
			
			if (op == NULL)
			{
				op = search_op(token, OP_PLACE_PREFIX);
			}
			
			if (op != NULL) // Function or prefix operator found (handled the same)
			{
				if (!op_push(op)) goto exit;
				await_subexpression = op->arity != 0; // Constants don't await subexpr.
				
				// Handle unary functions without parenthesis (e.g. sin2)
				if (function_flag)
				{
					if (i != num_tokens - 1)
					{
						if (!is_opening_parenthesis(tokens[i + 1][0]))
						{
							arities[num_ops - 1] = 1;
						}
					}
				}
				
				continue;
			}
		}
		else
		{
			op = search_op(token, OP_PLACE_INFIX);
			if (op != NULL) // Infix operator found
			{
				if (!op_push(op)) goto exit;
				await_subexpression = true;
				continue;
			}
			
			op = search_op(token, OP_PLACE_POSTFIX);
			if (op != NULL) // Postfix operator found
			{
				if (!op_push(op)) goto exit;
				await_subexpression = false;
				continue;
			}
			
			// We can fail here: no more tokens processable (no glue-op)
			error = PERR_UNEXPECTED_TOKEN;
			goto exit;
		}
		
		// VI. Token must be variable or constant (leaf)
		Node *node = malloc(sizeof(Node));
		void *constant = malloc(ctx->value_size);
		if (ctx->try_parse(token, constant)) // Is token constant?
		{
			*node = get_constant_node(constant);
		}
		else // Token must be variable
		{
			free(constant);
			char *name = malloc(tok_len * sizeof(char*) + 1);
			strcpy(name, token);
			*node = get_variable_node(name);
		}
		await_subexpression = false;
		
		if (!node_push(node)) goto exit;
	}
	
	// 5. Pop all remaining operators
	while (num_ops > 0)
	{
		if (op_stack[num_ops - 1] == NULL)
		{
			error = PERR_UNEXPECTED_OPENING_PARENTHESIS;
			goto exit;
		}
		if (!op_pop_and_insert()) goto exit;
	}
	
	// 6. Build result and return value
	switch (num_nodes)
	{
		case 0:
			error = PERR_EMPTY; // We haven't constructed a single node
			break;
		case 1:
			error = PERR_SUCCESS; // We successfully constructed a single AST
			*res = node_stack[0];
			break;
		default:
			error = PERR_MISSING_OPERATOR; // We have multiple ASTs (need glue-op)
	}
	
	exit:
	for (int i = 0; i < num_tokens; i++) free(tokens[i]);
	free(tokens);
	return error;
}
