/*
 * Small utility library
 */

#pragma once

#define PRINT_MEM_LINE_LEN 8

/* Prints contents of machine in pretty way

 Arguments:
 * machine - array with machine code
 * machine_length - length of machine code
 */
void print_mem(short* machine, int machine_length) {

	int i = 0, j = 0, k = 0;
	int last_val = 0x7fffffff;
	int last_line_same = 0, prev_last_line_same = 0;
	int wordbuf[PRINT_MEM_LINE_LEN];

	machine_length &= 0x0001ffff; /* Clip machine pointer */

	/* Iterate through machine code */
	while(i < machine_length) {

		/* Print info that words are repeating */
		if(last_line_same && !prev_last_line_same)
			printf("\n**** (%04x)", last_val);

		prev_last_line_same = last_line_same;

		/* Print line (8 words) */
		last_line_same = 1;
		for(j = 0, k = 0; j < PRINT_MEM_LINE_LEN; ++j, ++i) {

			/* Save machine values to buffer */
			wordbuf[j] = machine[i] & 0x0000ffff;

			/* Check if values are the same as last character in previous line.
			 * This is to skip lines with the same words because there is no
			 * need to print them */
			if(machine[i] == last_val && last_line_same)
				continue;

			/* If values differ then print those skipped ones */
			last_line_same = 0;
			if(k < PRINT_MEM_LINE_LEN) {
				/* Print line number (current line address) */
				printf("\n%04x:", i - j);
				for(; k < j; ++k) {
					printf(" %04x", wordbuf[k]);
				}
				k = PRINT_MEM_LINE_LEN; /* Set k = 8 to not enter loop twice */
			}

			/* Print current machine value */
			printf(" %04x", wordbuf[j]);
		}

		/* If line is the same then skip other processing */
		if(last_line_same)
			continue;

		/* Save last character in line */
		last_val = machine[i - 1];

		/* Print fancy character display on the right of byte dump */
		printf("\t| ");
		for(k = 0; k < 2*PRINT_MEM_LINE_LEN; ++k) {
			j = ((short*)wordbuf)[k] & 0x000000ff;
			/* Print dot if character is not printable */
			if(j < 0x20 || j > 0x7e)
				printf(".");
			/* Else print proper character */
			else
				printf("%c", j);
		}
		printf(" |");
	}

	printf("\n");
}

/* Converts string characters to lowercase; does not touch characters in quotes

 Arguments:
 * string - string to be converted
 */
void to_lower(char *string) {

	char quote = 0;
	int in_quotes = 0;

	while (*string) {
		/* Check if found quote */
		if ((*string == '\'' || *string == '"') && !in_quotes) {
			quote = *string;
			in_quotes = 1;
		}
		else if(*string == quote && in_quotes)
			in_quotes = 0;

		if(*string >= 'A' && *string <= 'Z' && !in_quotes) {
			*string += 'a' - 'A';
		}
		string++;
	}
}