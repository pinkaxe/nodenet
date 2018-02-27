#ifndef UTIL_LINK_H__
#define UTIL_LINK_H__

enum link_state {
    LINK_STATE_ALIVE = 0x01,
    LINK_STATE_DEAD
};

struct link;

struct link *link_init();

/* call both free functions for l to be free'd */
int link_free_from(struct link *l);
int link_free_to(struct link *l);

void *link_get_from(struct link *l);
void *link_get_to(struct link *l);
enum link_state link_get_state(struct link *l);

int link_set_from(struct link *l, void *from);
int link_set_to(struct link *l, void *to);
int link_set_state(struct link *l, enum link_state state);

#endif
