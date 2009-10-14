
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#include "sys/thread.h"

#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "router.h"
#include "node.h"
#include "link.h"
#include "grp.h"

#include "nn.h"

/* serialize the calls that change the relationships between the router's,
 * grps, node's */
static mutex_t rel_mutex = PTHREAD_MUTEX_INITIALIZER;

void rel_lock()
{
    mutex_lock(&rel_mutex);
}

void rel_unlock()
{
    mutex_unlock(&rel_mutex);
}

struct nn_router *nn_router_init(void)
{
    struct nn_router *rt;

    PCHK(LWARN, rt, router_init());

    return rt;
}


int nn_router_run(struct nn_router *rt)
{
    int r;

    //router_lock(rt);

    ICHK(LWARN, r, router_run(rt));

    //router_unlock(rt);

    return r;
}

struct nn_node *nn_node_init(enum nn_node_driver type, enum nn_node_attr attr,
        void *code, void *pdata)
{
    struct nn_node *n;

    PCHK(LWARN, n, node_init(type, attr, code, pdata));

    return n;
}

//int nn_node_setname(struct nn_node *n, char *name);

int nn_node_run(struct nn_node *n)
{
    int r;

    //node_lock(n);

    ICHK(LWARN, r, node_start(n));

    //node_unlock(n);

    return r;
}

struct nn_grp *nn_grp_init(int id)
{
    struct nn_grp *g;

    PCHK(LWARN, g, grp_init(id));

    return g;
}

int nn_grp_free(struct nn_grp *g)
{
    void *iter;
    int r;
    struct nn_node *node;

    // lock grp

    //while((n=grp_nodes_iter(g, &iter))){
    //    // unlock grp
    //    // lock n
    //    // lock grp
    //    // make sure still pointed at, grp_ismemb(g, n);
    //    ICHK(LWARN, r, node_quit_grp(n, g));
    //    // unlock n
    //    /* remove pointers from nodes to this grp */
    //}

    //unlock_grp();

    //rel_lock();

    ICHK(LWARN, r, grp_free(g));

    //rel_unlock();

    return r;
}

int nn_join_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    //rel_lock();
    //grp_lock(g);
    //node_lock(n);

    ICHK(LWARN, r, node_join_grp(n, g));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, grp_add_memb(g, n));
    if(r){
        int rr;
        ICHK(LWARN, rr, node_quit_grp(n, g));
        goto err;
    }

err:
    //node_unlock(n);
    //grp_unlock(g);
    //rel_unlock();

    return r;
}

int nn_quit_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    //rel_lock();
    //grp_lock(g);
    //node_lock(n);

    ICHK(LWARN, r, grp_rem_memb(g, n));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, node_quit_grp(n, g));
    if(r){
        goto err;
    }

err:
    //node_unlock(n);
    //grp_unlock(g);
    //rel_unlock();

    return r;

}

int nn_router_set_cmd_cb(struct nn_router *rt, io_cmd_req_cb_t cb)
{
    int r;

    //router_lock(rt);

    ICHK(LWARN, r, router_set_cmd_cb(rt, cb));

    //router_unlock(rt);

    return r;
}

int nn_router_add_cmd_req(struct nn_router *rt, struct nn_cmd *cmd)
{
    int r;

    //router_lock(rt);

    r = link_router_tx_cmd(rt, cmd);

    //router_unlock(rt);

    return r;
}

//int nn_router_set_data_cb(struct nn_router *rt, io_data_req_cb_t cb)
//{
//    int r;
//
//    ICHK(LWARN, r, router_set_data_cb(rt, cb));
//    return r;
//}

int nn_router_add_data_req(struct nn_router *rt, struct nn_io_data *data)
{
    int r;

    //r = router_add_data_req(rt, data);
    return r;
}

//route_route()

/*
int nn_link_node(struct nn_node *from, struct nn_node *to)
{
    nn_node_add_out_link(from);
    nn_node_add_in_link(to);
}

int nn_unlink_node(struct nn_node *from, struct nn_node *to)
{
}
*/


int nn_link(struct nn_node *n, struct nn_router *rt)
{
    int r;
    struct nn_link *l;

    l = link_init();

    /* lock router and link */
    router_lock(rt);
    link_lock(l);

    /* set router side pointers */
    router_link(rt, l);
    link_set_router(l, rt);

    /* unlock router and link */
    link_unlock(l);
    router_unlock(rt);

    /* lock node and link */
    node_lock(n);
    link_lock(l);

    /* set node side pointers */
    node_link(n, l);
    link_set_node(l, rt);

    /* unlock node and link */
    node_unlock(n);
    link_unlock(l);

err:

    return r;
}


static int _node_link_unlink(struct nn_node *n, struct nn_link *l)
{
    int r = 0;

    node_unlink(n, l);
    link_set_node(l, NULL);

    return r;
}

static int _router_link_unlink(struct nn_router *rt, struct nn_link *l)
{
    int r = 0;

    router_unlink(rt, l);
    link_set_node(l, NULL);

    return r;
}

#include <assert.h>

int nn_unlink(struct nn_node *n, struct nn_router *rt)
{
    void *iter;
    int r;
    struct nn_link *l;

    assert(1 == 0);

    router_lock(rt);

    iter = NULL;
    while((l=router_link_iter(rt, &iter))){
        link_lock(l);
        /* router and link now locked */

        _router_link_unlink(rt, l);

        link_unlock(l);
        router_unlock(rt);
        /* router and link now unlocked */

        node_lock(n);
        link_lock(l);
        /* node and link now locked */

        _node_link_unlink(n, l);

        link_unlock(l);
        node_unlock(n);
        /* node and link now unlocked */

        link_free(l);

        router_lock(rt);
    }

err:
    router_unlock(rt);

    return r;
}

#include <assert.h>
int nn_node_free(struct nn_node *n)
{
    void *iter = NULL;
    int r;
    struct nn_link *l;
    struct nn_router *rt;
    bool f = false; // free or not

    node_lock(n);

    /* remove in link */
    while((l=node_link_iter(n, &iter))){

        link_lock(l);

        _node_link_unlink(n, l);

        /* check if router pointer is NULL */
        rt = link_get_router(l);
        if(!rt){
            f = true;
        }
        link_unlock(l);
        node_unlock(n);

        if(f){
            assert(1 == 0);
            link_free(l);
            f = false;
        }

        /* router side */
       // rt = link_get_router(l);
       // if(NULL){
       // router_lock(rt);
       // link_lock(l);

       // _router_link_unlink(rt, l);

       // link_unlock(l);
       // router_unlock(rt);


        node_lock(n);
    }

    node_unlock(n);

   // /* remove pointers from groups */
   // iter = NULL;
   // while((g=node_grps_iter(n, &iter))){
   //     grp_rem_memb(g, n);
   // }


    ICHK(LWARN, r, node_free(n));

    //rel_unlock();

    return r;
}

int nn_router_free(struct nn_router *rt)
{
    void *iter = NULL;
    int r;
    struct nn_node *n;
    struct nn_link *l;
    bool f = false; // free or not

    router_lock(rt);

    /* remove in link */
    while((l=router_link_iter(rt, &iter))){
        printf("p %p\n", l);

        link_lock(l);

        _router_link_unlink(rt, l);

        /* check if router pointer is NULL */
        n = link_get_node(l);
        if(!n){
            assert(1 == 0);
            f = true;
        }
        link_unlock(l);
        router_unlock(rt);

        if(f){
            assert(1 == 0);
            link_free(l);
            f = false;
        }

        /* router side */
       // rt = link_get_router(l);
       // if(NULL){
       // router_lock(rt);
       // link_lock(l);

       // _router_link_unlink(rt, l);

       // link_unlock(l);
       // router_unlock(rt);


        router_lock(rt);
    }

    router_unlock(rt);

    //if(n->router_links){
    //    // rem and free first
    //    iter = NULL;
    //    while(nr=ll_next(n->router_links, &iter)){
    //        ICHK(LWARN, r, ll_rem(n->router_links, nr));
    //        free(nr);
    //    }
    //    ICHK(LWARN, r, ll_free(n->router_links));
    //    if(r) fail++;
    //}
    //
    //router_lock(rt);

    //while((l=router_nodes_iter(rt, &iter))){
    //link_unlink(struct nn_node *n, struct nn_router *rt)
    /* remove pointers from nodes */
   // iter = NULL;
   // while((n=router_nodes_iter(rt, &iter))){
   //     // unlock router
   //     // shit change state
   //     // lock node
   //     // lock router
   //     node_rem_from_router(n, rt);
   //     // unlock node
   // }

    //rel_lock();

    ICHK(LWARN, r, router_free(rt));

    //rel_unlock();

    return r;
}
