/*
 * File-oriented functions (reading and writing)
 */

#pragma once

#include <unistd.h>
#include <fcntl.h>

#define BLOCK_SIZE 4096

/* Writes machine code to file in little-endian format

 Arguments:
 * file_name - name of file to which machine code will be written
 * machine - array with machine code
 * machine_length - length of machine code
 */
int write_code_to_file(char* file_name, short* machine, int machine_length) {

	int i = 0, j = 0, k = 0;
	ssize_t res = 0;
	int fd;
	char buffer[BLOCK_SIZE];

	/* Open file */
	fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	/* Write data to file */
	while(i < machine_length) {
		/* Get block of machine code to prevent writing to file byte by byte */
		for(j = 0; j < BLOCK_SIZE/2 && i < machine_length; ++j, ++i) {
			buffer[2*j] = machine[i] & 0xff;
			buffer[2*j + 1] = (machine[i] >> 8) & 0xff;
		}
		/* Write code to file */
		for(k = 0; k < 2*j; k += res) {
			if((res = write(fd, buffer, 2*j)) < 0) {
				close(fd);
				return res;
			}
		}
	}

	close(fd);
	return 0;
}

/* Reads code from provided file and stores allocated string size in file_size.
 Returns NULL if couldn't read file; returned pointer shall be freed after
 usage.

 Arguments:
 * file_name - string with name of file to be read
 * file_size - output variable with size of allocated memory
 */
char* read_file(char* file_name, int* file_size) {

	int fd = 0;
	char* buffer = 0;
	char* tmp_buffer;
	int pointer = 0;
	ssize_t bytes_read;

	/* Open file */
	if((fd = open(file_name, O_RDONLY)) < 0)
		return NULL;

	*file_size = 0;
	while(!(*file_size - pointer)) {
		/* If buffer is out of space expand it */
		/* Add 1 to pointer to always provide space for terminating NULL byte */
		if(pointer + 1 >= *file_size) {
			*file_size += BLOCK_SIZE;
			tmp_buffer = realloc(buffer, *file_size);

			/* If realloc fails free buffer and return NULL */
			if(!tmp_buffer) {
				if(buffer) free(buffer);
				close(fd);
				return NULL;
			}

			/* Set all bytes of new block to 0 */
			memset(tmp_buffer + *file_size - BLOCK_SIZE, 0, BLOCK_SIZE);

			buffer = tmp_buffer;
		}

		/* Read file data */
		do {
			bytes_read = read(fd, buffer + pointer,
				*file_size - pointer);
			pointer += (int)bytes_read;
		} while(bytes_read > 0);
	}

	close(fd);

	return buffer;
}