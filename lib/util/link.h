#ifndef NN_LINK_H__
#define NN_LINK_H__

struct nn_link *link_init();

/* call both free functions for l to be free'd */
int link_free_from(struct nn_link *l);
int link_free_to(struct nn_link *l);

void *link_get_from(struct nn_link *l);
void *link_get_to(struct nn_link *l);
enum nn_link_state link_get_state(struct nn_link *l);

int link_set_from(struct nn_link *l, void *from);
int link_set_to(struct nn_link *l, void *to);
int link_set_state(struct nn_link *l, enum nn_link_state state);

#endif
