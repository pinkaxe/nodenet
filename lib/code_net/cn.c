
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#include "util/log.h"

#include "types.h"
#include "cmd.h"
#include "net.h"
#include "elem.h"
#include "grp.h"

#include "cn.h"


struct cn_net *cn_net_init(void)
{
    struct cn_net *n;

    PCHK(LWARN, n, net_init());
    return n;
}

int cn_net_free(struct cn_net *n)
{
    int r;

    ICHK(LWARN, r, net_free(n));
    return r;
}

int cn_net_run(struct cn_net *n)
{
    int r;

    ICHK(LWARN, r, io_route_run(n));
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

int cn_add_elem_to_net(struct cn_elem *e, struct cn_net *n)
{
    int r;

    ICHK(LWARN, r, elem_add_to_net(e, n));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, net_add_memb(n, e));
    if(r){
        int rr;
        ICHK(LWARN, rr, elem_rem_from_net(e, n));
        if(rr){
        }
    }

err:
    return r;
}

int cn_rem_elem_from_net(struct cn_elem *e, struct cn_net *n)
{
    int r;

    ICHK(LWARN, r, net_rem_memb(n, e));
    if(r){
        goto err;
    }

    ICHK(LWARN, r, elem_rem_from_net(e, n));
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

int cn_net_set_cmd_cb(struct cn_net *n, io_cmd_req_cb_t cb)
{
    int r;

    ICHK(LWARN, r, net_set_cmd_cb(n, cb));
    return r;
}

int cn_net_add_cmd_req(struct cn_net *n, struct cn_cmd *cmd)
{
    int r;

    r = net_add_cmd(n, cmd);
    return r;
}

//int cn_net_set_data_cb(struct cn_net *n, io_data_req_cb_t cb)
//{
//    int r;
//
//    ICHK(LWARN, r, net_set_data_cb(n, cb));
//    return r;
//}

int cn_net_add_data_req(struct cn_net *n, struct cn_io_data *data)
{
    int r;

    //r = net_add_data_req(n, data);
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
