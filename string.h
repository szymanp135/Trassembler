/*
 * String processing library
 */

#pragma once

#include <string.h>
#include "key-words.h"

#define MAX_ARG_LENGTH 256
#define MAX_LINE_LENGTH 512

struct keyword {
	short keyword;
	int is_dotword;

	char* arg_ptr;
};

/* Parses line and finds keyword.
 Keyword is found in such manner that all preceding blank spaces are skipped
 and then first 3 characters are read (dot '.' is omitted if present).
 Returns 0 if found; returns -1 if line is blank. Other values mean error

 Arguments:
 * line - string with keyword
 * kw - structure with parsed keyword and dotword information
 */
int parse_keyword(char* line, struct keyword* kw) {

	int i = 0, j = 0;
	char keyword[4] = { 0 };

	/* Clear kw */
	memset(kw, 0, sizeof(*kw));

	/* Skip all blank characters */
	for(i = 0; line[i] == ' ' || line[i] == '\t'; i++);
	/* Return if line is blank/comment */
	if(!line[i] || line[i] == ';')
		return -1;
	/* Mark dotword */
	if(line[i] == '.') {
		kw->is_dotword = 1;
		i++;
	}
	/* Read keyword */
	for(j = 0; j < KEYWORD_LENGTH - 1; j++, i++) {
		keyword[j] = line[i];
	}

	/* Find keyword code */
	for(j = 0; j < KEY_WORDS; ++j) {
		if(!strcmp(keyword, key_words[j]))
			break;
	}

	/* Haven't found any matching keyword */
	if(j >= KEY_WORDS) {
		return 1;
	}

	/* Assign and return */
	if(j < INSTRUCTIONS)
		kw->keyword = (short)(unsigned char)instr_code[j];
	else
		kw->keyword = (short)(unsigned char)dot_code[j - INSTRUCTIONS];
	kw->arg_ptr = line + i;
	return 0;
}

/* Reads line from code
 Returns 0 if succeeded

 Arguments:
 * code - string with code
 * code_pointer - pointer to currently read char in code
 * line - read line output string
 */
int read_line(char* code, int* code_pointer, char* line) {

	int i = 0;
	int in_quotes = 0;
	char quote = 0;

	/* Read line of code till comment or end of line or file */
	while(code[*code_pointer] != 0 && code[*code_pointer] != '\n' &&
		(code[*code_pointer] != ';' || in_quotes) && i < MAX_LINE_LENGTH) {

		if((code[*code_pointer] == '"' || code[*code_pointer] == '\'') &&
			!in_quotes) {
			quote = code[*code_pointer];
			in_quotes = 1;
		}
		else if(code[*code_pointer] == quote && in_quotes) {
			in_quotes = 0;
		}

		line[i++] = code[(*code_pointer)++];
	}
	/* Read through comment to skip it */
	while(code[*code_pointer] != 0 && code[*code_pointer] != '\n')
		(*code_pointer)++;
	(*code_pointer)++;
	line[i] = 0;
	to_lower(line);

	return 0;
}

/* Skip all preceding blank characters and read one word
 Returns number of characters read.

 Arguments:
 * string - text in which word is looked for
 * arg - string in which found word is stored
 */
int read_word(char* string, char* arg) {

	int i, j;
	/* Skip blank spaces */
	for(i = 0; string[i] == ' ' || string[i] == '\t'; i++);
	/* Read word */
	for(j = 0; j < MAX_ARG_LENGTH && string[i] != 0 && string[i] != ';' &&
		string[i] != ' ' && string[i] != '\t' && string[i] != '\n'; j++, i++) {
		arg[j] = string[i];
	}
	/* Set final byte to terminating one */
	arg[j] = 0;
	return i;
}

/* Skip all preceding blank characters and read string (stuff in quotes)
 Returns 0 if succeeded and non-zero otherwise.

 Arguments:
 * string - text in which string is looked for
 * arg - string in which found string is stored
 */
int read_string(char* string, char* arg) {

	int i, j;
	char quote = 0;

	/* Skip blank spaces */
	for(i = 0; string[i] == ' ' || string[i] == '\t'; i++);
	/* Set read quote type */
	quote = string[i++];
	/* Check if actually a quote was read */
	if(quote != '"' && quote != '\'')
		return 1;
	/* Read string */
	for(j = 0; j < MAX_ARG_LENGTH && string[i] != 0 && string[i] != quote;
		++j, ++i){
		arg[j] = string[i];
	}
	/* Set final byte to terminating one */
	arg[j] = 0;

	/* Haven't found closing quote - return error */
	if(string[i] != quote)
		return 1;

	return 0;
}