#ifndef __XSTDLIB__H__
#define __XSTDLIB__H__

#undef malloc
#define malloc xmalloc
void *xmalloc(size_t size);

#undef calloc
#define calloc xcalloc
void *xcalloc(size_t count, size_t size);

#undef free 
#define free xfree
void xfree(void *ptr);

#endif

