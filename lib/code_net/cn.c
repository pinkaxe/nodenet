
#include "types.h"
#include "net.h"
#include "elem.h"
#include "grp.h"

#include "cn.h"

struct cn_net *cn_net_init(void)
{
    return net_init();
}

int cn_net_free(struct cn_net *n)
{
    return net_free(n);
}

struct cn_elem *cn_elem_init(enum cn_elem_type type, enum cn_elem_attr attr,
        void *code, void *pdata)
{
    return elem_init(type, attr, code, pdata);
}

int cn_elem_free(struct cn_elem *e)
{
    return elem_free(e);
}

int cn_elem_run(struct cn_elem *e)
{
    return elem_start(e);
}

struct cn_grp *cn_grp_init(int id)
{
    return grp_init(id);
}

int cn_grp_free(struct cn_grp *g)
{
    return grp_free(g);
}

int cn_add_elem_to_net(struct cn_elem *e, struct cn_net *n)
{
    int r;

    printf("add: %p\n", n);
    r = elem_add_to_net(e, n);
    if(r){
        goto err;
    }

    r = net_add_memb(n, e);
    if(r){
        int rr;
        rr = elem_rem_from_net(e, n);
        printf("fuck\n");
        goto err;
    }
    printf("added\n");

err:
    return r;
}

int cn_rem_elem_from_net(struct cn_elem *e, struct cn_net *n)
{
    int r;

    r = net_rem_memb(n, e);
    if(r){
        goto err;
    }

    r = elem_rem_from_net(e, n);
    if(r){
        goto err;
    }

err:
    return r;
}

/*

int cn_add_elem_to_grp(struct cn_elem *e, struct cn_grp *g)
{
    cn_elem_add_to_grp();
    cn_grp_add_memb();
}

int cn_rem_elem_from_grp(struct cn_elem *e, struct cn_grp *g)
{
    cn_grp_rem_memb();
    cn_elem_rem_from_grp();
}

int cn_link_elem(struct cn_elem *from, struct cn_elem *to)
{
    cn_elem_add_out_link(from);
    cn_elem_add_in_link(to);
}

int cn_unlink_elem(struct cn_elem *from, struct cn_elem *to)
{
}
*/
