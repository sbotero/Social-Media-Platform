#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>


void malloc_err(void **ptr, size_t size){
	if((*ptr = malloc(size)) == NULL){
		perror("malloc");
		exit(1);
	}
}

void strncat_err(char **dest, char *src, size_t n){
	if ((strncat(*dest, src, n)) != *dest){
        fprintf(stderr, "The right amount of characters for the dashed line \
            was not copied into the buffer.\n");
        exit(1);
    }
}

void write_err(int fildes, const void *buf, size_t nbytes){
	int bytes_written = write(fildes, buf, nbytes);
	if(bytes_written < 0){
		perror("Write");
		exit(1);
	}
}

size_t read_err(int fildes, void *buf, size_t nbytes){
	int bytes_read = read(fildes, buf, nbytes);
	if (bytes_read < 0) {
		perror("read");
		exit(1);
	}

	return bytes_read;
}

