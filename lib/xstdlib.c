#include<stdlib.h>
#include<stdio.h>

void *xmalloc(size_t size)
{
	return malloc(size);
}

void *xcalloc(size_t count, size_t size)
{
	return calloc(count, size);
}

void xfree(void *ptr)
{
	free(ptr);
}

