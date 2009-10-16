
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
#include "conn.h"

#include "node.h"
#include "node_drivers/node_driver.h"


struct nn_node {

    DBG_STRUCT_START

    enum nn_node_driver type; /* thread, process etc. */
    enum nn_node_attr attr;
    enum nn_state state;

    /* rel */
    struct ll *router_conns;      /* all router_conns connected to via conn's */
    struct ll *grp_conns;     /* all groups connected to via conn's */

    /* funcp's to communicate with this node type */
    struct node_driver_ops *ops;

    void *code;  /* pointer to object depending on type */

    void *pdata; /* private passthru data */

    /* maximum time a node exe is allowed to run on one buffer processing */
    uint32_t max_exe_usec;

    mutex_t mutex;

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
    int err;
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

    PCHK(LWARN, n->router_conns, ll_init());
    if(!n->router_conns){
        PCHK(LWARN, r, node_free(n));
        goto err;
    }

    n->type = type;
    n->attr = attr;
    n->code = code;
    n->pdata = pdata;

    // FIXME: err checking
    mutex_init(&n->mutex, NULL);

    n->state = NN_STATE_PAUSED;

    node_isok(n);

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

    if(n->router_conns){
        ICHK(LWARN, r, ll_free(n->router_conns));
        if(r) fail++;
    }

//    if(n->grp_conns){
//        iter = ll_iter_init();
//
//        while(ng=ll_next(n->grp_conns, &iter)){
//            ICHK(LWARN, r, ll_rem(n->grp_conns, ng));
//            free(ng);
//        }
//        ICHK(LWARN, r, ll_free(n->grp_conns));
//        if(r) fail++;
//
//        iter = ll_iter_init();
//
//    }

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

int node_conn(struct nn_node *n, struct nn_conn *cn)
{
    int r = 1;

    ICHK(LWARN, r, ll_add_front(n->router_conns, (void **)&cn));

err:
    return r;
}

int node_unconn(struct nn_node *n, struct nn_conn *cn)
{
    int r;

    node_isok(n);

    ICHK(LWARN, r, ll_rem(n->router_conns, cn));

    return 0;
}

struct nn_conn *node_conn_iter(struct nn_node *n, void **iter)
{
    int r = 0;
    struct nn_conn *cn;

  //  assert(n);

  //  cn = ll_next(n->router_conns, iter);

  //  if(cn){
  //      return cn;
  //  }else{
  //      return NULL;
  //  }
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

/* check that the conn's it points to points back */
int node_router_isok(struct nn_node *n)
{
    void *track;
    struct nn_conn *cn;
    int r = 0;
    void *iter;

    iter = NULL;
   // while((cn=ll_next(n->router_conns, &iter))){
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
int node_grp_isok(struct nn_node *n)
{
    void *track;
    struct nn_node_grp *g;
    int r = 0;
    void *iter;

    iter = NULL;
   // while((g=ll_next(n->grp_conns, &iter))){
   //     r = grp_ismemb(g->grp, n);
   //     assert(r == 0);
   //     //r = grp_print(g->grp);
   //     if(r){
   //         break;
   //     }
   // }

    return r;
}

int node_isok(struct nn_node *n)
{

    DBG_STRUCT_ISOK(n);

    //printf("n->type: %d", n->type);
    assert(n->type >= 0 && n->type < 128);
    assert(n->attr >= 0 && n->attr < 128);
    assert(n->code);
    //assert(n->out_conns_llh);
    //assert(n->in_conns_llh);
    //assert(n->grp_conns);
    //assert(n->router_conns);
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
    struct nn_conn *cn;
    struct nn_node_grp *g;
    int r = 0;
    int c;
    void *iter;

    puts("\n-- node->conn --\n");

    c = 0;

    iter = NULL;
    //while((cn=ll_next(n->router_conns, &iter))){
    //    printf("p:%p\n", cn);
    //    c++;
    //}

    puts("\n-- node->grp --\n");

    c = 0;
    iter = NULL;
    //while((g=ll_next(n->grp_conns, &iter))){
    //    printf("p:%p\n", g->grp);
    //    c++;
    //}

    printf("total: %d\n\n", c);

    return 0;
}

/** debug functions ends **/

