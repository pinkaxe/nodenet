
#include<stdlib.h>

struct ll {
	void *start;
	void *end;
};

struct link{
	void *prev;
	void *next;
};

struct test {
	struct link link;
	int x;
};

void *ll_add(void *old, void *new) 
{
	struct link *old_link = old;
	struct link *new_link = new;

	if(old){
		old_link->prev = new;
		new_link->next = old;
	}else{
		new_link->prev = NULL;
		new_link->next = NULL;
	}
	return new;
	//test.link.next = &test2;

}

void ll_del(void *item)
{
	struct link *item_link = item;
	struct link *prev = item_link->prev;
	struct link *next = item_link->next;

	if(prev)
		prev->next = NULL;
	if(next)
		next->prev = NULL;

	if(prev && next){
		prev->next = next;
		next->prev = prev;
	}
}

void *ll_next(void *curr)
{
	struct link *link = curr;
	return link->next;
}

#define ll_foreach(start, curr, track) \
	for(curr=start,track=curr->link.next; \
			curr && ((track=curr->link.next) || 1) ; \
			curr=track)

int main()
{
	struct test *start;
	struct test *curr, *track, *new;

	int j;
	int i;
	for(;;){
		start = NULL;
		for(i=0; i < 10; i++){
			new = malloc(sizeof *new);
			new->x = i;
			start = ll_add(start, new);
		}

		ll_foreach(start, curr, track){	
			printf("- %d\n", curr->x);
			ll_del(curr);
			printf("1\n");
			free(curr);
		}
	}


	//test2.link.prev = &test;
	//test.link.next = &test2;

	
	return 0;
}

/*
struct ll *ll_init(size_t size)
{
	struct ll *ll;

	if(!(ll = malloc(sizeof *ll))){
		return NULL;
	}

	ll->p = NULL;
	ll->n = NULL;

	return ll;
}

int ll_insert()
{
}


int ll_remove()
{
}
*/
