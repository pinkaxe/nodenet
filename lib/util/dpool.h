#ifndef DPOOL_H
#define DPOOL_H

struct dpool_buf {
    int id;
    void *data;
};

enum dpool_opt {
    DPOOL_OPT_ALLOC
};

enum {
    ENONEAVAIL = 0xFF
}ERRNUM;

struct dpool *dpool_create(int bufsize, int max_no, int opt);
void dpool_free(struct dpool *h);

struct dpool_buf *dpool_get_buf(struct dpool *h);
int dpool_ret_buf(struct dpool *h, struct dpool_buf *buf);

#endif
