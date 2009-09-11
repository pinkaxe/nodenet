#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

void *xmalloc(size_t size)
{
	void *ret;
	if(!(ret=malloc(size))){
		printf("malloc error: %s\n", strerror(errno));
		exit(-1);
	}
	return ret;
}

void *xcalloc(size_t count, size_t size)
{
	return calloc(count, size);
}

void *xrealloc(void *ptr, size_t size)
{
	void *ret;
	if(!(ret=realloc(ptr, size))){
		printf("realloc error: %s\n", strerror(errno));
		exit(-1);
	}
	return ret;
}

void xfree(void *ptr)
{
	free(ptr);
}

