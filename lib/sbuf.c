#include<stdint.h>
#include<stddef.h>
#include<stdlib.h>
#include<string.h>

#include "sbuf.h"
#include "xstdlib.h"

struct sbuf {
	char *start;
	char *end; /* point to \0 */
	size_t  size; /* total allocated space */
};



struct sbuf *sbuf_init(size_t size)
{
	struct sbuf *b;

	if(!(b = malloc(sizeof *b))){
		return NULL;
	}

	if(!size) size = 1;
	if(!(b->start = malloc(size))){
		free(b);
		return NULL;
	}

	*b->start = '\0';
	b->end = b->start;
	b->size = size;

	return b;
}

int sbuf_grow(struct sbuf *b, size_t size)
{
	void *new;
	ptrdiff_t offset = b->end - b->start;
	if(!(new = realloc(b->start, size))){
		return 1;
	}
	b->start = new;
	b->end = b->start + offset;
	b->size = size;
	return 0;
}


int sbuf_append(struct sbuf *b, const char *buf, size_t len)
{
	size_t need; 

	if(!len) len = strlen(buf);
	need = b->end - b->start + len + 1;

	if(need > len){
		if(sbuf_grow(b, need)){
			return 1;
		}
	}

	strcpy(b->end, buf);
	b->end += len;

	return 0;
}
			

char *sbuf_find(const char *buf, const char *str)
{
	const char *needle = str;
	const char *s = buf;
	const char *ss; /* saved start of match */

	while(*s){
		ss = s;
		needle = str; 
		while(*s && *s++ == *needle++)
			;
		needle--;
		if(*needle == '\0'){
			return (char *)ss;
		}
	}		
	return NULL;
}

int sbuf_replace(struct sbuf *b, const char *old, const char *new, int num)
{
	int i, res = 0;
	char *match;
	char *buf = b->start;
	size_t newl = strlen(new);
	size_t oldl = strlen(old);
	int ldiff = newl - oldl;

	if(ldiff == 0){
		for(i=0; i < num; i++){
			if(!(match = sbuf_find(buf, old))){
				break;
			}
			res++;
			memcpy(match, new, newl);
			buf = match + newl;
		}
	}else{
		char *newstartp, *oldstart, *oldstartp;
		char *oldend;
		size_t oldsize;

		oldstartp = b->start;
		oldstart = b->start;
		oldend =  b->end;
		oldsize = b->size;

		if(!(b->start = malloc(oldend - oldstart))){
			b->start = oldstart;
			return -1;
		}
		newstartp = b->start;
		b->end = b->start;


		for(i=0; i < num; i++){

			if(!(match = sbuf_find(oldstartp, old))){
				break;
			}

			memcpy(newstartp, oldstartp, match - oldstartp);
			newstartp += match - oldstartp;
			b->end += match - oldstartp;
			oldstartp += match - oldstartp;

			if(ldiff > 0){
				sbuf_grow(b, b->size + ldiff);
				b->size += ldiff;
			}
			memcpy(newstartp, new, newl);
			newstartp += newl;
			b->end += newl;
			oldstartp += oldl;

			res++;
		}

		if(res){
			free(oldstart);
			*b->end = '\0';
		}else{
			free(b->start);
			b->start = oldstart;
			b->end = oldend;
			b->size = oldsize;
		}

	}
	return res;
}

int xsbuf_replace(struct sbuf *b, const char *old, const char *new, int num)
{
	int i, res = 0;
	size_t need;
	char *match;
	char *buf = b->start;
	size_t newl = strlen(new);
	size_t oldl = strlen(old);
	int ldiff = newl - oldl;

	for(i=0; i < num; i++){
		if(!(match = sbuf_find(buf, old))){
			break;
		}
		res++;
		if(ldiff != 0){
			need = b->end - b->start + ldiff;
			sbuf_grow(b, need);
			if(!(match = sbuf_find(buf, old))){
				break;
			}
			printf("len: %d\n", b->end - (match + oldl));
			memmove(match + newl, match + oldl, b->end - (match +
						oldl));
			b->end += ldiff; 
		}
		memcpy(match, new, newl);
		buf = match + newl;
	}
	return res;
}

char *sbuf_get(struct sbuf *b)
{
	return b->start;
}

void sbuf_free(struct sbuf *b)
{
	if(b->start) free(b->start);
	if(b) free(b);
}
