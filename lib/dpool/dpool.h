#ifndef UTIL_DPOOL_H__
#define UTIL_DPOOL_H__

struct dpool_buf {
    int id;
    void *data;
    size_t len;
    int ref_cnt;
};

enum dpool_opt {
    DPOOL_OPT_ALLOC
};

enum {
    ENONEAVAIL = 0xFF
}ERRNUM;

struct dpool *dpool_create(size_t bufsize, size_t max_no, int opt);
void dpool_free(struct dpool *h);

struct dpool_buf *dpool_get_buf(struct dpool *h);
struct dpool_buf *dpool_get_filled_buf(struct dpool *h);
int dpool_ret_buf(struct dpool *h, struct dpool_buf *buf);

//void *dpool_buf_get_data(struct dpool_buf *h);

#endif
