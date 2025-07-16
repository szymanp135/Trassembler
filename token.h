//
// Created by pawel on 02.07.25.
//

#pragma once

#ifndef TOKEN_H
#define TOKEN_H

#endif //TOKEN_H

#include <stdlib.h>
#include <stdio.h>
#include "variable-tree.h"

#define TOKEN_ERROR 			-1
#define TOKEN					0x60000000
#define TOKEN_ADD				TOKEN + 0x01
#define TOKEN_SUBTRACT			TOKEN + 0x02
#define TOKEN_MULTIPLY			TOKEN + 0x03
#define TOKEN_DIVIDE			TOKEN + 0x04
#define TOKEN_AND				TOKEN + 0x05
#define TOKEN_OR				TOKEN + 0x06
#define TOKEN_XOR				TOKEN + 0x07
#define TOKEN_NOT				TOKEN + 0x08
#define TOKEN_SHIFT_LEFT		TOKEN + 0x09
#define TOKEN_SHIFT_RIGHT		TOKEN + 0x0a
#define TOKEN_PARENTHESIS		TOKEN + 0x10
#define TOKEN_PARENTHESIS_LEFT	TOKEN + 0x10
#define TOKEN_PARENTHESIS_RIGHT	TOKEN + 0x11

const unsigned int TOKEN_PRECEDENCE[] ={ 0,
	 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 0, 0, 0, 0, 0,
	-1,-1 };

/* Converts string to int
 Returns 0 if succeeded; Otherwise non-zero value.

 Arguments:
 * string - string to be converted
 * number - output int that has been translated
 */
int string2int(char* string, int* number) {

	char c;
	int base;
	*number = 0;

	/* Determine base */
	switch (*string) {
		case '%': base = 2; string++; break;
		case '$': base = 16; string++; break;
		default: base = 10; break;
	}

	/* Convert number */
	while((c = *string++)) {

		switch(base) {
			case 2: {
				if(!(c >= '0' && c <= '1'))
					return -1;
				*number <<= 1; /* Multiply number by base (by shifting) */
				*number |= c - '0'; /* Add bit */
			} break;
			case 10: {
				if(!(c >= '0' && c <= '9'))
					return -1;
				*number *= 10;
				*number += c - '0';
			} break;
			case 16: {
				*number *= 16;
				if(c >= '0' && c <= '9')
					*number += c - '0';
				else if (c >= 'a' && c <= 'f')
					*number += c - 'a' + 10;
				else
					return -1;
			} break;
		}
	}

	return 0;
}

/* Converts string variable into int number.
 Returns 0 if succeeded and non-zero otherwise.

 Arguments:
 * string - variable name
 * number - output value of variable
 * root - variable tree root node
 */
int variable2int(char* string, int* number,
	struct variable_tree_node* root) {

	struct variable_tree_node* node = NULL;

	/* Search for node */
	node = variable_tree_search(root, string);
	/* If node does not exists return error */
	if(!node)
		return -1;

	/* Set number to node value */
	*number = node->value;

	return 0;
}

/* Converts word into token. Values and variables are translated into their
 values, operators are converted info operator tokens. List of operator
 tokens is situated in 'token.h' file on top.
 Returns 0 if succeeded; 2 if result value needs addition of current machine
 pointer; 1 if word is NULL; -1 if couldn't convert number; -2 if couldn't
 convert variable; -3 if no value provided

 Arguments:
 * word - word to be tokenized
 * value - output value of token
 * root - variable tree root node
 */
int tokenize_word(char* word, int* value, struct variable_tree_node* root) {

	int res = 0xff;
	*value = 0;

	if(!*word)
		return 1;

	switch (*word) {
		/* Operators */
		case '+': *value = TOKEN_ADD; return 0;
		case '-': *value = TOKEN_SUBTRACT; return 0;
		case '*': *value = TOKEN_MULTIPLY; return 0;
		case '/': *value = TOKEN_DIVIDE; return 0;
		case '&': *value = TOKEN_AND; return 0;
		case '|': *value = TOKEN_OR; return 0;
		case '^': *value = TOKEN_XOR; return 0;
		case '~': *value = TOKEN_NOT; return 0;
		case '<': *value = TOKEN_SHIFT_LEFT; return 0;
		case '>': *value = TOKEN_SHIFT_RIGHT; return 0;
		case '(': *value = TOKEN_PARENTHESIS_LEFT; return 0;
		case ')': *value = TOKEN_PARENTHESIS_RIGHT; return 0;
		case '@': *value = -1; return 2; /* 'here' alias */

		default: break;
	}

	/* Convert number */
	if((*word >= '0' && *word <= '9') || *word == '%' || *word == '$')
		res = string2int(word, value);

	/* Convert variable into value it is corresponding to */
	else {
		res = variable2int(word, value, root);
		/* If variable not found return error */
		if(res) {
			return -2;
		}
	}

	/* If couldn't translate number return error */
	if(res)
		return -1;

	return 0;
}

/* Changes tokens order from infix to postfix (RPN).
 Algorithm stolen from: https://en.wikipedia.org/wiki/Shunting_yard_algorithm
 Returns 0 if succeeded and non-zero otherwise.

 Arguments:
 * tokens - array of tokens to be ordered. Ordered array is written here
 * tokens_length - length of tokens array (num of tokens in array). Ordered
   array length is written here
 */
int shunting_yard(int* tokens, int* tokens_length) {

	int i, j, k;
	int token;
	int* ordered_tokens = 0;
	int* operator_stack = 0;

	ordered_tokens = malloc(sizeof(int) * *tokens_length);
	operator_stack = malloc(sizeof(int) * *tokens_length);
	if(!ordered_tokens || !operator_stack)
		return -1;

	/* Main algorithm loop */
	/* i is tokens iterator; j is ordered_tokens queue pointer;
	 * k is operator_stack stack pointer */
	for(i = 0, j = 0, k = 0; i < *tokens_length; i++) {
		token = tokens[i];
		/* Token is a number */
		if(token < TOKEN) {
			ordered_tokens[j++] = token;
			continue;
		}
		/* Token is an operator */
		if(token < TOKEN_PARENTHESIS) {
			while(k > 0 && operator_stack[k-1] != TOKEN_PARENTHESIS_LEFT &&
				TOKEN_PRECEDENCE[operator_stack[k-1] - TOKEN] >=
				TOKEN_PRECEDENCE[token - TOKEN]) {

				ordered_tokens[j++] = operator_stack[--k];
			}
			operator_stack[k++] = token;
			continue;
		}
		/* Token is a left parenthesis */
		if(token == TOKEN_PARENTHESIS_LEFT) {
			operator_stack[k++] = TOKEN_PARENTHESIS_LEFT;
			continue;
		}
		/* Token is a right parenthesis */
		if(token == TOKEN_PARENTHESIS_RIGHT) {
			/* Pop operators from stack and move them to output queue */
			while(k > 0 && operator_stack[k-1] != TOKEN_PARENTHESIS_LEFT) {
				ordered_tokens[j++] = operator_stack[--k];
			}
			/* Return is stack is empty (no left parenthesis has been found) */
			if(!k) {
				free(ordered_tokens);
				free(operator_stack);
				return 1;
			}
			k--; /* Pop left parenthesis and discard it */
		}
	}

	/* Pop all remaining operators and put them into queue */
	while(k > 0) {
		/* If left parenthesis is present then parenthesis are wrong */
		if(operator_stack[k-1] == TOKEN_PARENTHESIS_LEFT) {
			free(ordered_tokens);
			free(operator_stack);
			return 1;
		}
		ordered_tokens[j++] = operator_stack[--k];
	}

	/* Write ordered tokens to output array */
	*tokens_length = j;
	for (i = 0; i < *tokens_length; i++) {
		tokens[i] = ordered_tokens[i];
	}

	free(ordered_tokens);
	free(operator_stack);
	return 0;
}

/* Evaluates expression represented as tokens in RPN order.
 Returns 0 if succeeded and non-zero otherwise.

 Arguments:
 * tokens - array of tokens in RPN order
 * tokens_length - length of tokens array
 * value - output of result value
 */
int evaluate_rpn(int* tokens, int tokens_length, int* value) {

	int i, sp = 0;
	int* stack = 0;

	/* Create stack */
	stack = malloc(sizeof(int) * tokens_length);
	if(!stack)
		return 1;

	/* Evaluate value of expression */
	for(i = 0; i < tokens_length; i++) {
		/* Push number to stack */
		if(tokens[i] < TOKEN) {
			stack[sp++] = tokens[i];
		}
		/* Execute operators */
		else {
			/* NOT being a unary operator has to be resolved separately */
			if(tokens[i] == TOKEN_NOT) {
				/* If not enough arguments on stack return error */
				if(sp < 1) {
					free(stack);
					return 1;
				}
				stack[sp-1] = ~stack[sp-1];
			}
			/* All other operators */
			else {
				/* If not enough arguments on stack return error */
				if(sp < 2) {
					free(stack);
					return 2;
				}
				switch (tokens[i]) {
					case TOKEN_ADD:
						stack[sp-2] = stack[sp-2] + stack[sp-1]; break;
					case TOKEN_SUBTRACT:
						stack[sp-2] = stack[sp-2] - stack[sp-1]; break;
					case TOKEN_MULTIPLY:
						stack[sp-2] = stack[sp-2] * stack[sp-1]; break;
					case TOKEN_DIVIDE:
						stack[sp-2] = stack[sp-2] / stack[sp-1]; break;
					case TOKEN_AND:
						stack[sp-2] = stack[sp-2] & stack[sp-1]; break;
					case TOKEN_OR:
						stack[sp-2] = stack[sp-2] | stack[sp-1]; break;
					case TOKEN_XOR:
						stack[sp-2] = stack[sp-2] ^ stack[sp-1]; break;
					case TOKEN_SHIFT_LEFT:
						stack[sp-2] = stack[sp-2] << stack[sp-1]; break;
					case TOKEN_SHIFT_RIGHT:
						stack[sp-2] = stack[sp-2] >> stack[sp-1]; break;
					default:
						free(stack);
						return 1;
				}
				/* Decrement stack pointer */
				sp--;
			}
		}
	}

	/* If not every value has been taken into account then error occured */
	if(sp > 1) {
		free(stack);
		return 3;
	}

	/* Write result into value */
	*value = stack[sp-1];

	free(stack);
	return 0;
}
