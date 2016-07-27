#ifndef ERROR_H
#define ERROR_H


/*
 * Check if calling realloc will be successful or not. 
 *
 * Return:
 *   - ptr to where realloc starts on success
 * If unsuccessful, realloc will return NULL and will exit with 1
 */
void *realloc_err(void *ptr, size_t size);

/*
 * Check if calling malloc will be successful or not. 
 *
 */
void malloc_err(void *ptr, size_t size);

/*
 * Check if calling strncat will be successful or not. 
 *
 * Return:
 *   - ptr to dest
 * If unsuccessful strncat_err will return print an error to stderr exit with 1
 *
 */
void strncat_err(char **dest, char *src, size_t n);

void write_err(int fildes, const void *buf, size_t nbytes);

size_t read_err(int fildes, void *buf, size_t nbytes);

#endif
	