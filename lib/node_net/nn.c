
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "router.h"
#include "node.h"
#include "grp.h"

#include "nn.h"


struct nn_router *nn_router_init(void)
{
    struct nn_router *rt;

    PCHK(LWARN, rt, router_init());

    return rt;
}

int nn_router_free(struct nn_router *rt)
{
    void *iter;
    int r;
    struct nn_node *n;

    //lock router

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

    ICHK(LWARN, r, router_free(rt));
    return r;
}

int nn_router_run(struct nn_router *rt)
{
    int r;

    ICHK(LWARN, r, router_run(rt));
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

int nn_node_free(struct nn_node *n)
{
    void *iter = NULL;
    int r;

    struct nn_router *rt;

    /* remove pointers from routers */
   // while((rt=node_routers_iter(n, &iter))){
   //     router_rem_memb(rt, n);
   // }

   // /* remove pointers from groups */
   // iter = NULL;
   // while((g=node_grps_iter(n, &iter))){
   //     grp_rem_memb(g, n);
   // }

    ICHK(LWARN, r, node_free(n));

    return r;
}

int nn_node_run(struct nn_node *n)
{
    int r;

    ICHK(LWARN, r, node_start(n));
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
    //    ICHK(LWARN, r, node_rem_from_grp(n, g));
    //    // unlock n
    //    /* remove pointers from nodes to this grp */
    //}

    //unlock_grp();

    ICHK(LWARN, r, grp_free(g));

    return r;
}

int nn_add_node_to_router(struct nn_node *n, struct nn_router *rt)
{
    int r;

    ICHK(LWARN, r, node_add_to_router(n, rt));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, router_add_memb(rt, n));
    if(r){
        int rr;
        ICHK(LWARN, rr, node_rem_from_router(n, rt));
        if(rr){
        }
    }

err:
    return r;
}

int nn_rem_node_from_router(struct nn_node *n, struct nn_router *rt)
{
    int r;

    ICHK(LWARN, r, router_rem_memb(rt, n));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, node_rem_from_router(n, rt));
    if(r){
        goto err;
    }

err:
    return r;
}


int nn_add_node_to_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    ICHK(LWARN, r, node_add_to_grp(n, g));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, grp_add_memb(g, n));
    if(r){
        int rr;
        ICHK(LWARN, rr, node_rem_from_grp(n, g));
        goto err;
    }

err:
    return r;
}

int nn_rem_node_from_grp(struct nn_node *n, struct nn_grp *g)
{
    int r;

    ICHK(LWARN, r, grp_rem_memb(g, n));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, node_rem_from_grp(n, g));
    if(r){
        goto err;
    }

err:
    return r;

}

int nn_router_set_cmd_cb(struct nn_router *rt, io_cmd_req_cb_t cb)
{
    int r;

    ICHK(LWARN, r, router_set_cmd_cb(rt, cb));
    return r;
}

int nn_router_add_cmd_req(struct nn_router *rt, struct nn_cmd *cmd)
{
    int r;

    r = router_add_cmd(rt, cmd);
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
