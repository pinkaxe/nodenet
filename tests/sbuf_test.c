#include<stdio.h>
#include<string.h>

#include "../lib/sbuf.h"

int main(int argc, char **argv)
{
	int i;
	struct sbuf *b, *b2;
	for(;;){
		b = sbuf_init(0);
		b2 = sbuf_init(128);
		for(i=0; i < 100; i++){
			sbuf_append(b, "love", 0);
			sbuf_append(b, "ya", 0);
			sbuf_append(b, "free ", 0);
		}
		for(i=0; i < 100; i++){
			sbuf_append(b2, "1", 0);
			sbuf_append(b2, "22", 0);
			sbuf_append(b2, "333", 0);
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
		sbuf_append(b, "bloody", 0);
		sbuf_replace(b, "bloo", "moo", 100000);
		sbuf_replace(b, "xxxxx", "freddy", 100000);

		printf("%s:%d|\n2: %s:%d|\n", sbuf_get(b), (int)
				strlen(sbuf_get(b)), sbuf_get(b2), (int)
				strlen(sbuf_get(b2)));
		sbuf_free(b);
		sbuf_free(b2);
	}	
	return 0;
}
