
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"
#include "util/dbg.h"
#include "util/que.h"
#include "util/ll.h"

#include "types.h"
#include "cmd.h"

#include "node.h"
#include "node_types/node_type.h"


struct nn_node {
    DBG_STRUCT_START
    enum nn_node_type type; /* thread, process etc. */
    enum nn_node_attr attr;
    void *code;  /* pointer to object depending on type */

    /* pointers for relationships */
    struct ll *grp_llh;             /* links to all groups this node belongs to */
    struct ll *router_llh;               /* links to all groups this node belongs to */

    /* bufs */
    struct que *in_data_queh;         /* input for code node */
    struct que *in_cmd_queh;          /* input cmds for code node */

    struct que *out_data_queh;        /* output to other code node's */
    struct que *out_cmd_queh;         /* input cmds for code node */

    void *pdata; /* private passthru data */

    /* maximum time a nodeent is allowed to exe on one buffer processing */
    uint32_t max_exe_usec;

    struct node_type_ops *ops;

    DBG_STRUCT_END
};

struct nn_node_link {
    struct nn_node *node;
    // quality etc., specific for link
};

struct nn_node_grp {
    struct nn_grp *grp;
};

struct nn_node_router {
    struct nn_router *router;
};


/** node type **/

struct nn_node *node_init(enum nn_node_type type, enum nn_node_attr attr,
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

    n->ops = node_type_get_ops(n->type);
    assert(n->ops);

    PCHK(LWARN, n->in_data_queh, que_init(8));
    if(!n->in_data_queh){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    PCHK(LWARN, n->in_cmd_queh, que_init(8));
    if(!n->in_cmd_queh){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    PCHK(LWARN, n->grp_llh, ll_init());
    if(!n->grp_llh){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    PCHK(LWARN, n->router_llh, ll_init());
    if(!n->router_llh){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    n->type = type;
    n->attr = attr;
    n->code = code;
    n->pdata = pdata;

    node_isok(n);

err:
    return n;
}

int node_free(struct nn_node *n)
{
    int fail = 0;
    int r = 0;

    node_isok(n);

    if(n->router_llh){
        ICHK(LWARN, r, ll_free(n->router_llh));
        if(r) fail++;
    }

    if(n->grp_llh){
        ICHK(LWARN, r, ll_free(n->grp_llh));
        if(r) fail++;
    }

    if(n->in_data_queh){
        ICHK(LWARN, r, que_free(n->in_data_queh));
        if(r) fail++;
    }

    if(n->in_cmd_queh){
        ICHK(LWARN, r, que_free(n->in_cmd_queh));
        if(r) fail++;
    }

    free(n);

    return fail;
}


int node_start(struct nn_node *n)
{
    int r;

    ICHK(LWARN, r, node_io_run(n));

    return r;
}

int node_add_to_router(struct nn_node *n, struct nn_router *rt)
{
    int r = 1;
    struct nn_node_router *en;

    PCHK(LWARN, en, malloc(sizeof(*en)));
    if(!en){
        goto err;
    }

    en->router = rt;
    ICHK(LWARN, r, ll_add_front(n->router_llh, (void **)&en));

err:
    return r;
}

int node_rem_from_router(struct nn_node *n, struct nn_router *rt)
{
    int r;

    node_isok(n);

    ICHK(LWARN, r, ll_rem(n->router_llh, rt));
    //TODO:
    return 0;
}

int node_add_to_grp(struct nn_node *n, struct nn_grp *g)
{
    int r = 1;
    struct nn_node_grp *eg;

    PCHK(LWARN, eg, malloc(sizeof(*eg)));
    if(!eg){
        goto err;
    }

    eg->grp = g;
    ICHK(LWARN, r, ll_add_front(n->grp_llh, (void **)&eg));

err:
    return r;
}

int node_rem_from_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    node_isok(n);

    ICHK(LWARN, r, ll_rem(n->grp_llh, g));
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

int  node_add_cmd(struct nn_node *n, struct nn_cmd *cmd)
{
    return que_add(n->in_cmd_queh, cmd);
}

int node_add_data(struct nn_node *n, void *data, uint32_t data_no)
{
}

void *node_get_cmd(struct nn_node *n, struct timespec *ts)
{
    struct nn_cmd *c;

    c = que_get(n->in_cmd_queh, ts);

    return c;
}

void *node_get_buf(struct nn_node *n, struct timespec *ts)
{
    void *buf;

    buf = que_get(n->in_data_queh, ts);
    return buf;
}


/*
int node_write_in_cmd(struct nn_node *n, enum nn_node_cmd cmd, void *pdata)
{
    struct nn_cmd *c = malloc(sizeof(*c));

    c->id = cmd;
    c->pdata = pdata;

    return que_add(n->in_cmd_queh, c);
}

void *node_write_in_buf(struct nn_node *n)
{
}

*/




/** debug functions **/

/* check that the router's it points to points back */
int node_router_isok(struct nn_node *n)
{
    void *track;
    struct nn_node_router *rt;
    int r = 0;
    void *iter;

    iter = NULL;
    while((rt=ll_next(n->router_llh, &iter))){
        /* make sure we are a member */
        r = router_ismemb(rt->router, n);
        //r = router_print(rt->router);
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
    while((g=ll_next(n->grp_llh, &iter))){
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
    //assert(n->grp_llh);
    //assert(n->router_llh);
    assert(n->in_data_queh);
    assert(n->in_cmd_queh);
    //assert(n->out_data_queh);
    //assert(n->out_cmd_queh);

    node_router_isok(n);
    node_grp_isok(n);
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

    puts("\rt-- node->router --\rt");

    c = 0;

    iter = NULL;
    while((rt=ll_next(n->router_llh, &iter))){
        printf("p:%p\rt", rt->router);
        c++;
    }

    puts("\rt-- node->grp --\rt");

    c = 0;
    iter = NULL;
    while((g=ll_next(n->grp_llh, &iter))){
        printf("p:%p\rt", g->grp);
        c++;
    }

    printf("total: %d\rt\rt", c);

    return 0;
}

/** debug functions ends **/
