
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

struct cn_grp *cn_grp_init(int id)
{
    return grp_init(id);
}

int cn_grp_free(struct cn_grp *g)
{
    return grp_free(g);
}

/*
int cn_add_elem_to_net(struct cn_elem *e, struct cn_net *n)
{
    cn_elem_add_to_net();
    cn_net_add_memb();
}

int cn_rem_elem_from_net(struct cn_elem *e, struct cn_net *n)
{
    cn_net_rm_memb();
    cn_elem_rem_from_net();
}

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
