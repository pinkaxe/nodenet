
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "sys/thread.h"

#include "util/log.h"
#include "util/dbg.h"
#include "util/que.h"
#include "util/ll.h"

#include "types.h"
#include "cmd.h"
#include "link.h"

#include "node.h"
#include "node_drivers/node_driver.h"


struct nn_node {

    DBG_STRUCT_START

    enum nn_node_driver type; /* thread, process etc. */
    enum nn_node_attr attr;

    /* rel */
    struct ll *grp_links;     /* all groups linkected to via link's */
    struct ll *router_links;      /* all router_links linkected to via link's */

    /* funcp's to communicate with this node type */
    struct node_driver_ops *ops;

    void *code;  /* pointer to object depending on type */

    void *pdata; /* private passthru data */

    /* maximum time a node exe is allowed to run on one buffer processing */
    uint32_t max_exe_usec;

    mutex_t mutex;

    DBG_STRUCT_END
};


/* for nn_node->grp_links ll */
struct nn_node_grp {
    struct nn_grp *grp;
};

/* for nn_node->router_links ll */
struct nn_node_router {
    struct nn_link *link;
};


/** node type **/

struct nn_node *node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata)
{
    int r;
    int err;
    struct nn_node *n;

    PCHK(LWARN, n, calloc(1, sizeof(*n)));
    if(!n){
        goto err;
    }
    DBG_STRUCT_INIT(n);

    n->ops = node_driver_get_ops(n->type);
    assert(n->ops);

    /*
    PCHK(LWARN, n->in_data, que_init(8));
    if(!n->in_data){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    PCHK(LWARN, n->in_cmd, que_init(8));
    if(!n->in_cmd){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }
    */

    PCHK(LWARN, n->grp_links, ll_init());
    if(!n->grp_links){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    PCHK(LWARN, n->router_links, ll_init());
    if(!n->router_links){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    n->type = type;
    n->attr = attr;
    n->code = code;
    n->pdata = pdata;

    // FIXME: err checking
    mutex_init(&n->mutex, NULL);

    node_isok(n);

err:
    return n;
}

int node_free(struct nn_node *n)
{
    void *iter;
    int fail = 0;
    int r = 0;

    struct nn_node_router *nr;
    struct nn_node_grp *ng;

    node_isok(n);

    mutex_lock(&n->mutex);

    if(n->router_links){
        // rem and free first
        iter = NULL;
        while(nr=ll_next(n->router_links, &iter)){
            ICHK(LWARN, r, ll_rem(n->router_links, nr));
            free(nr);
        }
        ICHK(LWARN, r, ll_free(n->router_links));
        if(r) fail++;
    }

    if(n->grp_links){
        iter = NULL;
        while(ng=ll_next(n->grp_links, &iter)){
            ICHK(LWARN, r, ll_rem(n->grp_links, ng));
            free(ng);
        }
        ICHK(LWARN, r, ll_free(n->grp_links));
        if(r) fail++;
    }

    /*
    if(n->in_data){
        ICHK(LWARN, r, que_free(n->in_data));
        if(r) fail++;
    }

    if(n->in_cmd){
        ICHK(LWARN, r, que_free(n->in_cmd));
        if(r) fail++;
    }
    */

    mutex_unlock(&n->mutex);
    mutex_destroy(&n->mutex);

    free(n);

    return fail;
}


int node_start(struct nn_node *n)
{
    int r;

    ICHK(LWARN, r, node_io_run(n));

    return r;
}

int node_lock(struct nn_node *n)
{
    mutex_lock(&n->mutex);
    return 0;
}

int node_unlock(struct nn_node *n)
{
    mutex_unlock(&n->mutex);
    return 0;
}

/* the stuff ??? */

int node_conn(struct nn_node *n, struct nn_link *l)
{
    int r = 1;
    struct nn_node_router *en;

    PCHK(LWARN, en, malloc(sizeof(*en)));
    if(!en){
        goto err;
    }

    en->link = l;
    ICHK(LWARN, r, ll_add_front(n->router_links, (void **)&en));

err:
    return r;
}

int node_dconn(struct nn_node *n, struct nn_link *l)
{
    int r;

    node_isok(n);

    ICHK(LWARN, r, ll_rem(n->router_links, l));
    //TODO:
    return 0;
}

int node_join_grp(struct nn_node *n, struct nn_grp *g)
{
    int r = 1;
    struct nn_node_grp *eg;

    PCHK(LWARN, eg, malloc(sizeof(*eg)));
    if(!eg){
        goto err;
    }

    eg->grp = g;
    ICHK(LWARN, r, ll_add_front(n->grp_links, (void **)&eg));

err:
    return r;
}

int node_quit_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    node_isok(n);

    ICHK(LWARN, r, ll_rem(n->grp_links, g));
    return 0;
}

/*

int node_link(struct nn_node *from, struct nn_node *to)
{
    int r;
    int i;
    struct nn_node *curr;
    struct nn_node_link *link;
    void *start, *track;

    node_isok(from);
    node_isok(to);

    PCHK(LWARN, link, malloc(sizeof(*link)));
    if(!link){
        printf("cunt\rt");
        exit(-1);
        //LOG1(
    }

    link->node = to;
    PCHK(LWARN, r, ll_add_front(from->out_links_llh, (void **)&link));

    // link back
    PCHK(LWARN, link, malloc(sizeof(*link)));
    if(!link){
        printf("cunt\rt");
        exit(-1);
        //LOG1(
    }

    ll_add_front(to->in_links_llh, (void **)&link);
}

int node_unlink(struct nn_node *from, struct nn_node *to)
{
}
*/

int node_get_type(struct nn_node *n)
{
    return n->type;
}

int node_get_attr(struct nn_node *n)
{
    return n->attr;
}

void *node_get_pdatap(struct nn_node *n)
{
    return n->pdata;
}

void *node_get_codep(struct nn_node *n)
{
    return n->code;
}

/*
int  node_add_cmd(struct nn_node *n, struct nn_cmd *cmd)
{
    return que_add(n->in_cmd, cmd);
}

int node_add_data(struct nn_node *n, void *data, uint32_t data_no)
{
}

void *node_get_cmd(struct nn_node *n, struct timespec *ts)
{
    struct nn_cmd *c;

    c = que_get(n->in_cmd, ts);

    return c;
}

void *node_get_buf(struct nn_node *n, struct timespec *ts)
{
    void *buf;

    buf = que_get(n->in_data, ts);
    return buf;
}
*/


/*
int node_write_in_cmd(struct nn_node *n, enum nn_cmd_cmd cmd, void *pdata)
{
    struct nn_cmd *c = malloc(sizeof(*c));

    c->id = cmd;
    c->pdata = pdata;

    return que_add(n->in_cmd, c);
}

void *node_write_in_buf(struct nn_node *n)
{
}

*/




/** debug functions **/

/* check that the link's it points to points back */
int node_router_isok(struct nn_node *n)
{
    void *track;
    struct nn_node_router *rt;
    int r = 0;
    void *iter;

    iter = NULL;
    while((rt=ll_next(n->router_links, &iter))){
        /* make sure we are a member */
        r = router_isconn(rt->link, n);
        //r = router_print(rt->link);
        assert(r == 0);
        if(r){
            break;
        }
    }

    return r;
}

/* check that the grp's it points to points back */
int node_grp_isok(struct nn_node *n)
{
    void *track;
    struct nn_node_grp *g;
    int r = 0;
    void *iter;

    iter = NULL;
    while((g=ll_next(n->grp_links, &iter))){
        r = grp_ismemb(g->grp, n);
        assert(r == 0);
        //r = grp_print(g->grp);
        if(r){
            break;
        }
    }

    return r;
}

int node_isok(struct nn_node *n)
{

    DBG_STRUCT_ISOK(n);

    printf("n->type: %d", n->type);
    assert(n->type >= 0 && n->type < 128);
    assert(n->attr >= 0 && n->attr < 128);
    assert(n->code);
    //assert(n->out_links_llh);
    //assert(n->in_links_llh);
    //assert(n->grp_links);
    //assert(n->router_links);
    //assert(n->in_data);
    //assert(n->in_cmd);
    //assert(n->out_data);
    //assert(n->out_cmd);

    //node_router_isok(n);
    //node_grp_isok(n);
    //node_print(n);

    return 0;
}

int node_print(struct nn_node *n)
{
    void *track;
    struct nn_node_router *rt;
    struct nn_node_grp *g;
    int r = 0;
    int c;
    void *iter;

    puts("\rt-- node->link --\rt");

    c = 0;

    iter = NULL;
    while((rt=ll_next(n->router_links, &iter))){
        printf("p:%p\rt", rt->link);
        c++;
    }

    puts("\rt-- node->grp --\rt");

    c = 0;
    iter = NULL;
    while((g=ll_next(n->grp_links, &iter))){
        printf("p:%p\rt", g->grp);
        c++;
    }

    printf("total: %d\rt\rt", c);

    return 0;
}

/** debug functions ends **/
