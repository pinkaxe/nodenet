#include<stdio.h>
#include<stdarg.h>
#include<string.h>

#include "util/log.h"
#include "util/sbuf.h"
int main(int argc, char **argv)
{
	int n;
	int i;
	struct sbuf *b, *b2;

	for(;;){
		if(!(b = sbuf_init(0, &n))){
			log1(LERR, "sbuf_init failed: %d", n);
		}

		if(!(b2 = sbuf_init(128, &n))){
			log1(LERR, "sbuf_init failed: %d", n);
		}


		for(i=0; i < 100; i++){
			sbuf_append(b, "one");
			sbuf_append(b, "two");
			sbuf_append(b, "three");
		}

		for(i=0; i < 100; i++){
			sbuf_append(b2, "1");
			sbuf_append(b2, "22");
			sbuf_append(b2, "333");
		}

		printf("find: %p\n", sbuf_find("I am free", "I am"));
		printf("find: %p\n", sbuf_find("I am free", "am"));
		printf("find: %p\n", sbuf_find("I am free", "free"));
		printf("find: %p\n", sbuf_find("I am free", "bla"));
		printf("find: %p\n", sbuf_find("I am free", "freedom"));

		sbuf_replace(b2, "1", "x", 1000);
		sbuf_replace(b2, "22", "yyy", 1000);
		sbuf_replace(b2, "333", "z", 1000);

		sbuf_replace(b, "onetwothree", "one two three", 100000);
		sbuf_replace(b, "and ever", "", 100000);
		sbuf_replace(b, "  ", " ", 100000);
		sbuf_append(b, "bloody %s %d", "sunny", 4);
		/*
		sbuf_replace(b, "bloo", "moo", 100000);
		*/
		sbuf_replace(b, "xxxxx", "zipzip", 100000);
		sbuf_insert(b, 0, "zipzip: %s", "888");

		printf("%s:%d|\n2: %s:%d|\n", sbuf_get(b), (int)
				strlen(sbuf_get(b)), sbuf_get(b2), (int)
				strlen(sbuf_get(b2)));
		sbuf_free(b);
		sbuf_free(b2);
	}
	return 0;
}
