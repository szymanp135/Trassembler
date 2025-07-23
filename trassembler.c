/*
 *	Trassembler - Tranzystron 16000 assembler by Paweł Szymański
 *	Developed: 29.06.2025 - 23.07.2025
 *
 *	This software is meant to translate assembly code in given file
 *	to Tranzystron 16000 machine code.
 *
 *	Error codes are located in 'error.h' file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "file.h"
#include "string.h"
#include "key-words.h"
#include "variable-tree.h"
#include "token.h"
#include "error.h"

#define MEMORY_SIZE 0x10000

#define SHOW_VARIABLES 1

/* Evaluates expression in given string. It is very important to have every
 symbol spaced out with at least one blank character.
 Returns 0 if succeeded and non-zero otherwise

 Arguments:
 * string - string with expression to be evaluated
 * result - output int with result of expression
 * machine_pointer - machine pointer
 * root - variable tree root node
 * care_about_result - decides whether errors such as non-existent variables
   should end execution of function with error
 */
int evaluate_expression(char* string, int* result, int machine_pointer,
	struct variable_tree_node* root, int care_about_result) {

	int i = 0, j = 0, k = 0;
	int value = 0;
	int res = 0, len = 0;
	int result_int = 0;
	int tokens[MAX_ARG_LENGTH] = { 0 };
	char word[MAX_ARG_LENGTH] = { 0 };

	/* Check if there is a blank space at the end; if there is not then add */
	len = strlen(string);
	if(string[len-1] != ' ' || string[len-1] != '\t') {
		string[len+1] = 0;
		string[len] = ' ';
	}

	/* Parse and tokenize string */
	while(*string) {
		/* If blank character is found finish reading word and turn it into
		 * token */
		if(*string == ' ' || *string == '\t') {
			res = tokenize_word(word, &value, root);
			/* Look for errors */
			if(res) {
				/* Add machine pointer as intended */
				if(res == 2) {
					value += machine_pointer;
					tokens[i++] = value;
				}
				/* Increment inappropriate args count if don't care about result */
				else if (res == -2 && !care_about_result)
					k++;
				else if(res < 0)
					return -1;
			}
			/* If succeeded in tokenization then add token to token array */
			else
				tokens[i++] = value;

			/* Clear word and reset j */
			memset(word, 0, j + 1);
			j = 0;
		}
		/* Write string word into arg */
		else
			word[j++] = *string;

		string++;
	}

	/* No expression (empty arg) */
	if(!i) {
		/* Found only invalid tokens but don't care about result */
		if (!care_about_result && k)
			return 80;
		/* No args */
		return 78;
	}

	/* Change tokens order to RPN */
	res = shunting_yard(tokens, &i);
	if(res)
		return 1;

	/* Evaluate expression */
	res = evaluate_rpn(tokens, i, &result_int);
	if(res)
		return 2;
	/* If overflow occurred return error */
	if(result_int > 0x10000)
		return 79;

	/* Write result */
	*result = (short)result_int;
	return 0;
}

/* Processes dotword keywords.
 Returns 0 if succeeded and non-zero otherwise.

 Arguments:
 * kw - keyword structure with keyword info
 * root - variable tree root node
 * line_num - number of current translating line
 * reflect_on_machine - decides whether modifying machine should be allowed
 * machine_pointer - machine code pointer
 * machine - machine code array
 */
int process_keyword_dotword(struct keyword* kw, struct variable_tree_node* root,
	int line_num, int reflect_on_machine, int* machine_pointer,
	short* machine) {

	int i = 0, len = 0;
	int res = 0;
	int value = 0;
	char arg[MAX_ARG_LENGTH];
	char string_arg[MAX_ARG_LENGTH];

	switch (kw->keyword) {
		case org: { /* Set origin of upcoming code */
			/* Read argument value */
			res = evaluate_expression(kw->arg_ptr + res, &value,
				*machine_pointer, root, 1);
			if(res) {
				if(res == 79) /* Detect overflow */
					fprintf(stderr, "\nError: Expression value overflow"
					 " @ line: %d\n", line_num);
				else
					fprintf(stderr, "\nError: Wrong argument @ line: "
						"%d\n",	line_num);
				return ERR_ARGINVALID;
			}
			/* Clip value */
			value &= 0x0000ffff;
			/* Check if origin is moved backwards */
			if(value < *machine_pointer) {
				fprintf(stderr, "\nError: Origin moved backwards @ line:"
					" %d\n", line_num);
				return ERR_ARGINVALID;
			}
			/* Set machine pointer to org value */
			*machine_pointer = value;
			if(SHOW_VARIABLES & !reflect_on_machine)
				printf("%-10s: 0x%04x\n", "ORIGIN", *machine_pointer);
		} break;

		case str: { /* Create new string */
			/* Read variable name */
			res = read_word(kw->arg_ptr, arg);
			/* Check if read anything */
			if(res <= 0) {
				fprintf(stderr, "\nError: No argument @ line: %d\n",
					line_num);
				return ERR_NOARG;
			}
			/* Read string */
			res = read_string(kw->arg_ptr + res, string_arg);
			/* Check if read anything */
			if(res) {
				fprintf(stderr, "\nError: Invalid argument @ line: %d\n",
					line_num);
				return ERR_ARGINVALID;
			}
			/* Save string position to memory */
			if(!reflect_on_machine) {
				if(variable_tree_add_value(root, arg, *machine_pointer)) {
					fprintf(stderr, "\nError: Variable already exists @ "
						"line: %d\n", line_num);
					return ERR_MULTIVARDEF;
				}
			}

			len = strlen(string_arg);
			/* Write string to memory */
			if(reflect_on_machine) {
				for(i = 0; i <= len && *machine_pointer < MEMORY_SIZE; i++) {
					machine[(*machine_pointer)++] = (unsigned char)string_arg[i];
				}
			}
			/* Otherwise just move pointer */
			else
				*machine_pointer += len + 1;

			if(SHOW_VARIABLES && !reflect_on_machine)
				printf("%-10s: %s = %s\n", "STRING", arg, string_arg);
		} break;

		case var: { /* Create new variable */
			/* if machine should be modified then break */
			if(reflect_on_machine)
				break;
			/* Read variable name */
			res = read_word(kw->arg_ptr, arg);
			/* Check if read anything */
			if(res <= 0) {
				fprintf(stderr, "\nError: No argument @ line: %d\n",
					line_num);
				return ERR_NOARG;
			}
			/* Evaluate variable value */
			res = evaluate_expression(kw->arg_ptr + res, &value,
				*machine_pointer, root, 1);
			if(res) {
				if(res == 79) /* Detect overflow */
					fprintf(stderr, "\nError: Expression value overflow "
						"@ line: %d\n", line_num);
				else
					fprintf(stderr, "E\nrror: Wrong argument @ line: "
						"%d\n",	line_num);
				return ERR_ARGINVALID;
			}
			/* Add variable to the tree */
			if(variable_tree_add_value(root, arg, value)) {
				fprintf(stderr, "\nError: Variable already exists @ line: "
					"%d\n", line_num);
				return ERR_MULTIVARDEF;
			}
			if(SHOW_VARIABLES)
				printf("%-10s: %s = 0x%04x\n", "VARIABLE", arg,
					value & 0xffff);
		} break;

		case lab: { /* Create new label */
			/* if machine should be modified then break */
			if(reflect_on_machine)
				break;
			/* Read label name */
			res = read_word(kw->arg_ptr, arg);
			/* Check if there is an argument */
			if(res <= 0)
				break;
			/* Add new label to variable tree */
			if(variable_tree_add_value(root, arg, *machine_pointer)) {
				fprintf(stderr, "\nError: Label already exists @ line: "
					"%d\n", line_num);
				return ERR_MULTIVARDEF;
			}
			if(SHOW_VARIABLES)
				printf("%-10s: %s = 0x%04x\n", "LABEL", arg,
					*machine_pointer);
		} break;

		default: {
			fprintf(stderr, "\nError: Wrong dotword keyword @ line: %d\n",
				line_num);
			return ERR_WRONGKEYWORD;
		}
	}

	return 0;
}

/* Reads code and processes only appropriate dotwords which do not have any
 impact on machine code.
 Returns zero if succeeded and non-zero otherwise

 Arguments:
 * code - string with code; has to be allocated on heap (to be resizable)
 * code_size - size of code string
 * machine_pointer - stores position in code
 * root - variable tree root node
 */
int preprocess_code(char* code, int* code_size, int* machine_pointer,
	struct variable_tree_node* root) {

	char line[MAX_LINE_LENGTH+1];
	int code_pointer = 0;
	int res = 0;
	int value = 0;
	int line_num = 0;
	struct keyword kw;

	while(code_pointer < MEMORY_SIZE && code[code_pointer]) {
		/* Read line */
		memset(line, 0, sizeof(line));
		if(read_line(code, &code_pointer, line)) {
			fprintf(stderr, "\nError reading line @ line: %d\n",
				line_num++);
			return ERR_CANTREAD;
		}
		line_num++;

		/* Parse keyword */
		if((res = parse_keyword(line, &kw))) {
			if(res == -1) /* Empty line */
				continue;
			fprintf(stderr, "\nError: Wrong keyword @ line: %d\n",
				line_num);
			free(code);
			return ERR_WRONGKEYWORD;
		}

		if(kw.is_dotword) {
			if((res = process_keyword_dotword(&kw, root, line_num,
				0, machine_pointer, 0))) {
				free(code);
				return res;
			}
		}
		else {
			/* Increment machine pointer */
			(*machine_pointer)++;
			/* Check if valid argument is present */
			if(strlen(kw.arg_ptr) > 0) {
				/* Evaluate argument value */
				res = evaluate_expression(kw.arg_ptr, &value,
					*machine_pointer, root, 0);
				if(res) {
					if(res == 78) /* Skip if no arg present */
						continue;
					else if(res == 79) /* Overflow */
						fprintf(stderr, "\nError: Expression value "
							"overflow @ line: %d\n", line_num);
					else if (res == 80) {
						/* Increment machine pointer because there is an argument */
						/* (just invalid) */
						(*machine_pointer)++;
						continue;
					}
					else
						fprintf(stderr, "\nError: Wrong argument @ line: "
							"%d\n", line_num);
					free(code);
					return ERR_ARGINVALID;
				}
				/* Increment machine pointer if valid argument present */
				(*machine_pointer)++;
			}
		}
	}

	return 0;
}

/**/
int translate(char* filename, short* machine, int* machine_pointer,
	struct variable_tree_node* root) {

	/* Text code related */
	char* code;
	int code_size;

	/* Text code reading related */
	char line[MAX_LINE_LENGTH+1];
	int code_pointer = 0;
	int res = 0;
	int value = 0;
	int line_num = 0;
	struct keyword kw;

	/* Read file and check if read correctly */
	if(!(code = read_file(filename, &code_size))) {
		fprintf(stderr, "\nError reading file %s\n", filename);
		return ERR_CANTREAD;
	}

	/* Preprocess code */
	if((res = preprocess_code(code, &code_size, machine_pointer, root)))
		return res;
	printf("\nPreprocessed \"%s\"\n", filename);

	/* Reset machine pointer after preprocessing file */
	*machine_pointer = 0;

	while(code_pointer < MEMORY_SIZE && code[code_pointer]) {
		/* Read line */
		memset(line, 0, sizeof(line));
		if(read_line(code, &code_pointer, line)) {
			fprintf(stderr, "\nError reading line @ line: %d\n",
				line_num++);
			return ERR_CANTREAD;
		}
		line_num++;

		/* Parse keyword */
		if((res = parse_keyword(line, &kw))) {
			if(res == -1) /* Empty line */
				continue;
			fprintf(stderr, "\nError: Wrong keyword @ line: %d\n",
				line_num);
			free(code);
			return ERR_WRONGKEYWORD;
		}

		if(kw.is_dotword) {
			if((res = process_keyword_dotword(&kw, root, line_num,
				1, machine_pointer, machine))) {
				free(code);
				return res;
			}
		}
		else {
			/* Write instruction op-code to machine code array */
			machine[(*machine_pointer)++] = kw.keyword;
			/* Try to evaluate argument if present */
			if(strlen(kw.arg_ptr) > 0) {
				/* Evaluate argument value */
				res = evaluate_expression(kw.arg_ptr, &value,
					*machine_pointer, root, 1);
				if(res) {
					if(res == 78) /* Skip if no arg present */
						continue;
					else if(res == 79) /*  */
						fprintf(stderr, "\nError: Expression value "
							"overflow @ line: %d\n", line_num);
					else
						fprintf(stderr, "\nError: Wrong argument @ line: "
							"%d\n", line_num);
					free(code);
					return ERR_ARGINVALID;
				}
				/* Write arg value to machine code array */
				machine[(*machine_pointer)++] = (short)value;
			}
		}
	}

	if(*machine_pointer >= MEMORY_SIZE) {
		fprintf(stderr, "\nError: Not enough memory\n");
		free(code);
		return ERR_CODEOVERFLOW;
	}

	free(code);
	return 0;
}

int main(int argc, char *argv[]) {

	char* filename;
	char* output_filename;
	short machine[MEMORY_SIZE] = { 0 };
	int machine_pointer = 0;
	int result = 0;
	struct variable_tree_node tree = { 0 };

	/* Print welcome message */
	printf("Trassembler v0.6 [04.07.2025]\nAssembler for Tranzystron "
		"16000 by Pawel Szymanski\n");

	/* Check whether file with code has been provided */
	if(argc < 2) {
		printf("Usage: %s <filename> <optional output filename>\n",
			argv[0]);
		return 0x10;
	}
	/* Get input file name */
	filename = argv[1];
	/* Get output file name */
	if(argc >= 3)
		output_filename = argv[2];
	else
		output_filename = "a.out";

	// print information about maximum argument length
	printf("\nMaximum argument length is: %d\n\n", MAX_ARG_LENGTH);

	if(SHOW_VARIABLES)
		printf("Defined variables:\n");

	/* Translate code */
	result = translate(filename, machine, &machine_pointer, &tree);
	variable_tree_dispose(&tree);

	/* If didn't succeed then return */
	if(result)
		return result;

	/* Write contents of machine to file */
	if(write_code_to_file(output_filename, machine, machine_pointer)) {
		fprintf(stderr, "\nError writing code to file %s\n",
			output_filename);
		return ERR_CANTWRITE;
	}
	printf("\nWritten code to file: \"%s\"\n", output_filename);

	/* Print info about successful translation and dump machine code */
	printf("\nSuccessfully translated code!\n\nMachine code:");
	print_mem(machine, machine_pointer);
	printf("\nSuccessfully translated code!\n\n");

	return result;
}
