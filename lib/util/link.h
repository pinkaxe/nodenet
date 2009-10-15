#ifndef UTIL_LINK_H__
#define UTIL_LINK_H__

enum link_state {
    LINK_STATE_ALIVE = 0x01,
    LINK_STATE_DEAD
};

struct link;

struct link *link_init();

/* call both free functions for cn to be free'd */
int link_free_from(struct link *cn);
int link_free_to(struct link *cn);

void *link_get_from(struct link *cn);
void *link_get_to(struct link *cn);
enum link_state link_get_state(struct link *cn);

int link_set_from(struct link *cn, void *from);
int link_set_to(struct link *cn, void *to);
int link_set_state(struct link *cn, enum link_state state);

#endif
