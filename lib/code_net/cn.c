
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "router.h"
#include "elem.h"
#include "grp.h"

#include "cn.h"


struct cn_router *cn_router_init(void)
{
    struct cn_router *rt;

    PCHK(LWARN, rt, router_init());
    return rt;
}

int cn_router_free(struct cn_router *rt)
{
    int r;

    ICHK(LWARN, r, router_free(rt));
    return r;
}

int cn_router_run(struct cn_router *rt)
{
    int r;

    ICHK(LWARN, r, router_run(rt));
    return r;
}

struct cn_elem *cn_elem_init(enum cn_elem_type type, enum cn_elem_attr attr,
        void *code, void *pdata)
{
    struct cn_elem *e;

    PCHK(LWARN, e, elem_init(type, attr, code, pdata));
    return e;
}

//int cn_elem_setname(struct cn_elem *e, char *name);

int cn_elem_free(struct cn_elem *e)
{
    int r;

    ICHK(LWARN, r, elem_free(e));
    return r;
}

int cn_elem_run(struct cn_elem *e)
{
    int r;

    ICHK(LWARN, r, elem_start(e));
    return r;
}

struct cn_grp *cn_grp_init(int id)
{
    struct cn_grp *g;

    PCHK(LWARN, g, grp_init(id));
    return g;
}

int cn_grp_free(struct cn_grp *g)
{
    int r;

    ICHK(LWARN, r, grp_free(g));
    return r;
}

int cn_add_elem_to_router(struct cn_elem *e, struct cn_router *rt)
{
    int r;

    ICHK(LWARN, r, elem_add_to_router(e, rt));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, router_add_memb(rt, e));
    if(r){
        int rr;
        ICHK(LWARN, rr, elem_rem_from_router(e, rt));
        if(rr){
        }
    }

err:
    return r;
}

int cn_rem_elem_from_router(struct cn_elem *e, struct cn_router *rt)
{
    int r;

    ICHK(LWARN, r, router_rem_memb(rt, e));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, elem_rem_from_router(e, rt));
    if(r){
        goto err;
    }

err:
    return r;
}


int cn_add_elem_to_grp(struct cn_elem *e, struct cn_grp *g)
{
    int r;

    ICHK(LWARN, r, elem_add_to_grp(e, g));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, grp_add_memb(g, e));
    if(r){
        int rr;
        ICHK(LWARN, rr, elem_rem_from_grp(e, g));
        goto err;
    }

err:
    return r;
}

int cn_rem_elem_from_grp(struct cn_elem *e, struct cn_grp *g)
{
    int r;

    ICHK(LWARN, r, grp_rem_memb(g, e));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, elem_rem_from_grp(e, g));
    if(r){
        goto err;
    }

err:
    return r;

}

int cn_router_set_cmd_cb(struct cn_router *rt, io_cmd_req_cb_t cb)
{
    int r;

    ICHK(LWARN, r, router_set_cmd_cb(rt, cb));
    return r;
}

int cn_router_add_cmd_req(struct cn_router *rt, struct cn_cmd *cmd)
{
    int r;

    r = router_add_cmd(rt, cmd);
    return r;
}

//int cn_router_set_data_cb(struct cn_router *rt, io_data_req_cb_t cb)
//{
//    int r;
//
//    ICHK(LWARN, r, router_set_data_cb(rt, cb));
//    return r;
//}

int cn_router_add_data_req(struct cn_router *rt, struct cn_io_data *data)
{
    int r;

    //r = router_add_data_req(rt, data);
    return r;
}

//route_route()

/*
int cn_link_elem(struct cn_elem *from, struct cn_elem *to)
{
    cn_elem_add_out_link(from);
    cn_elem_add_in_link(to);
}

int cn_unlink_elem(struct cn_elem *from, struct cn_elem *to)
{
}
*/
