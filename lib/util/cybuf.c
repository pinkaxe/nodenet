#include<stdlib.h>
#include<assert.h>

struct cybuf {
    int head, tail, len;
    void **pp;
    void (*get_cb)(void *p);
};

struct cybuf *cybuf_init(int len)
{
    struct cybuf *h;

    h = calloc(1, sizeof(struct cybuf));
    if(!h){
        goto end;
    }

    h->len = len;
    h->pp = calloc(len, sizeof(void *));
    if(!h->pp){
        cybuf_free(h);
        h = NULL;
        goto end;
    }

end:
    return h;
}

int cybuf_free(struct cybuf *h)
{
    if(h->pp){
        free(h->pp);
    }

    if(h){
        free(h);
    }

    return 1;
}

int cybuf_add(struct cybuf *h, void *item) 
{
    assert(h);  
    
    h->pp[h->head] = item;

    if(++h->head >= h->len){
        h->head = 0;
    }

    return 0;
}

void *cybuf_get(struct cybuf *h) 
{
    void *r = NULL;

    r = h->pp[h->tail];

    if(++h->tail >= h->len){
        h->tail = 0;
    }

    return r;
}

void *cybuf_set_get_cb(struct cybuf *h, void (*get_cb)(void *p)) 
{
    h->get_cb = get_cb;
}

int main(int argc, char **argv)
{
    int i;
    struct cybuf *h;
    char *str0 = "abc";
    char *str1 = "---";

    while(1){
        h = cybuf_init(6);
        for(i=0; i < 2; i++){
            cybuf_add(h, str0);     
            cybuf_add(h, str1);     
            cybuf_add(h, str0);     
            cybuf_add(h, str1);     
            cybuf_add(h, str0);     
            cybuf_add(h, str1);     
            printf("** %s\n", (char *)cybuf_get(h));
            printf("** %s\n", (char *)cybuf_get(h));
            printf("** %s\n", (char *)cybuf_get(h));
            printf("** %s\n", (char *)cybuf_get(h));
            printf("** %s\n", (char *)cybuf_get(h));
            printf("** %s\n", (char *)cybuf_get(h));
        }
        cybuf_free(h);
    }

    return 0;
}

