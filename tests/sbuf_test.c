#include<stdio.h>
#include<stdarg.h>
#include<string.h>

#include "util/log.h"
#include "util/sbuf.h"
int main(int argc, char **argv)
{
	int e;
	int i;
	struct sbuf *b, *b2;
	for(;;){
		if(!(b = sbuf_init(0, &e))){
			log1(LERR, "sbuf_init failed: %d", e);
		}

		if(!(b2 = sbuf_init(128, &e))){
			log1(LERR, "sbuf_init failed: %d", e);
		}

		for(i=0; i < 100; i++){
			sbuf_append(b, "love");
			sbuf_append(b, "ya");
			sbuf_append(b, "free ");
		}
		for(i=0; i < 100; i++){
			sbuf_append(b2, "1");
			sbuf_append(b2, "22");
			sbuf_append(b2, "333");
		}

		printf("find: %p\n", sbuf_find("I am free", "I am"));
		printf("find: %p\n", sbuf_find("I am free", "free"));
		printf("find: %p\n", sbuf_find("I am free", "bla"));
		printf("find: %p\n", sbuf_find("I am free", "freedom"));

		sbuf_replace(b2, "1", "x", 1000);
		sbuf_replace(b2, "22", "yyy", 1000);
		sbuf_replace(b2, "333", "z", 1000);

		sbuf_replace(b, "loveyafree", "love you forever and ever", 100000);
		sbuf_replace(b, "and ever", "", 100000);
		sbuf_replace(b, "  ", " ", 100000);
		sbuf_append(b, "bloody %s %d", "sunny", 4);
		/*
		sbuf_replace(b, "bloo", "moo", 100000);
		*/
		sbuf_replace(b, "xxxxx", "freddy", 100000);
		sbuf_insert(b, 0, "freddy: %s", "888");

		printf("%s:%d|\n2: %s:%d|\n", sbuf_get(b), (int)
				strlen(sbuf_get(b)), sbuf_get(b2), (int)
				strlen(sbuf_get(b2)));
		sbuf_free(b);
		sbuf_free(b2);
	}	
	return 0;
}
