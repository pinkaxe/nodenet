
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "pkt.h"
#include "conn.h"

#include "node.h"
#include "node_io.h"
#include "node_drivers/node_driver.h"

static int node_isok(struct nn_node *n);

struct nn_node {

    DBG_STRUCT_START

    enum nn_node_driver type; /* thread, process etc. */
    enum nn_node_attr attr;
    enum nn_state state;

    /* rel */
    struct ll *conn;      /* all conn connected to via conn's */
    struct ll *grp_conns;     /* all groups connected to via conn's */

    /* funcp's to communicate with this node type driver */
    struct node_driver_ops *ops;

    void *code;  /* pointer to object depending on type */

    void *pdata; /* private passthru data */

    /* maximum time a node exe is allowed to run on one buffer processing */
    uint32_t max_exe_usec;

    mutex_t mutex;
    cond_t cond;

    DBG_STRUCT_END
};


/* for nn_node->grp_conns ll */
struct nn_node_grp {
    struct nn_grp *grp;
};


/** node type **/

struct nn_node *node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata)
{
    int r;
    struct nn_node *n;

    PCHK(LWARN, n, calloc(1, sizeof(*n)));
    if(!n){
        goto err;
    }
    DBG_STRUCT_INIT(n);

    n->ops = node_driver_get_ops(n->type);
    assert(n->ops);

    PCHK(LWARN, n->grp_conns, ll_init());
    if(!n->grp_conns){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    PCHK(LWARN, n->conn, ll_init());
    if(!n->conn){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    n->type = type;
    n->attr = attr;
    n->code = code;
    n->pdata = pdata;

    // FIXME: err checking
    mutex_init(&n->mutex, NULL);
    cond_init(&n->cond, NULL);

    n->state = NN_STATE_PAUSED;

    node_isok(n);
    ICHK(LWARN, r, node_io_run(n));

err:
    return n;
}

int node_free(struct nn_node *n)
{
    struct ll_iter *iter;
    int fail = 0;
    int r = 0;

    struct nn_node_grp *ng;

    node_isok(n);

    mutex_lock(&n->mutex);

    if(n->conn){
        ICHK(LWARN, r, ll_free(n->conn));
        if(r) fail++;
    }

    if(n->grp_conns){
        iter = ll_iter_init(n->grp_conns);

        while(!ll_iter_next(iter, (void **)&ng)){
            ICHK(LWARN, r, ll_rem(n->grp_conns, ng));
            free(ng);
        }
        ICHK(LWARN, r, ll_free(n->grp_conns));
        if(r) fail++;

        ll_iter_free(iter);

    }

    mutex_unlock(&n->mutex);

    cond_destroy(&n->cond);
    mutex_destroy(&n->mutex);

    free(n);

    return fail;
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

int node_cond_wait(struct nn_node *n)
{
    cond_wait(&n->cond, &n->mutex);
    return 0;
}

int node_cond_broadcast(struct nn_node *n)
{
    cond_broadcast(&n->cond);
    return 0;
}

int node_conn(struct nn_node *n, struct nn_conn *cn)
{
    int r = 1;

    ICHK(LWARN, r, ll_add_front(n->conn, (void **)&cn));

    return r;
}

int node_unconn(struct nn_node *n, struct nn_conn *cn)
{
    int r;

    node_isok(n);

    ICHK(LWARN, r, ll_rem(n->conn, cn));

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
    ICHK(LWARN, r, ll_add_front(n->grp_conns, (void **)&eg));

err:
    return r;
}

int node_quit_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    node_isok(n);

    ICHK(LWARN, r, ll_rem(n->grp_conns, g));
    return 0;
}


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

int node_set_state(struct nn_node *n, enum nn_state state)
{
    n->state = state;
    return 0;
}

enum nn_state node_get_state(struct nn_node *n)
{
    return n->state;
}

/* iter */

struct node_conn_iter *node_conn_iter_init(struct nn_node *rt)
{
    return (struct node_conn_iter *)ll_iter_init(rt->conn);
}

int node_conn_iter_free(struct node_conn_iter *iter)
{
    return ll_iter_free((struct ll_iter *)iter);
}

int node_conn_iter_next(struct node_conn_iter *iter, struct nn_conn **cn)
{
    return ll_iter_next((struct ll_iter *)iter, (void **)cn);
}



/** debug functions **/

static int node_isok(struct nn_node *n)
{

    DBG_STRUCT_ISOK(n);

    assert(n->type >= 0 && n->type < 128);
    assert(n->attr >= 0 && n->attr < 128);
    assert(n->code);

    //node_router_isok(n);
    //node_grp_isok(n);
    //node_print(n);

    return 0;
}

int node_print(struct nn_node *n)
{
    int c;
    //void *iter;

    c = 0;

    NODE_CONN_ITER_PRE

    printf("node->conn\t");
    printf("n=%p, cn:%p, rt=%p\n", n, cn, conn_get_router(cn));

    NODE_CONN_ITER_POST

    return 0;
}

#if 0
/* check that the conn's it points to points back */
static int node_router_isok(struct nn_node *n)
{
    int r = 0;
    void *iter;

    iter = NULL;
   // while((cn=ll_iter_next(n->conn, &iter))){
   //     /* make sure we are a member */
   //     //r = router_isconn(rt->conn, n);
   //     //r = router_print(rt->conn);
   //     //assert(r == 0);
   //     //if(r){
   //     //    break;
   //     //}
   // }

    return r;
}

/* check that the grp's it points to points back */
static int node_grp_isok(struct nn_node *n)
{
    int r = 0;
    void *iter;

    iter = NULL;
   // while((g=ll_iter_next(n->grp_conns, &iter))){
   //     r = grp_ismemb(g->grp, n);
   //     assert(r == 0);
   //     //r = grp_print(g->grp);
   //     if(r){
   //         break;
   //     }
   // }

    return r;
}

#endif


/** debug functions ends **/


