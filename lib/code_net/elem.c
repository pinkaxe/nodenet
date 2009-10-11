
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

#include "elem.h"
#include "elem_types/elem_type.h"


struct cn_elem {
    DBG_STRUCT_START
    enum cn_elem_type type; /* thread, process etc. */
    enum cn_elem_attr attr;
    void *code;  /* pointer to object depending on type */

    /* pointers for relationships */
    struct ll *grp_llh;             /* links to all groups this elem belongs to */
    struct ll *router_llh;               /* links to all groups this elem belongs to */

    /* bufs */
    struct que *in_data_queh;         /* input for code elem */
    struct que *in_cmd_queh;          /* input cmds for code elem */

    struct que *out_data_queh;        /* output to other code elem's */
    struct que *out_cmd_queh;         /* input cmds for code elem */

    void *pdata; /* private passthru data */

    /* maximum time a element is allowed to exe on one buffer processing */
    uint32_t max_exe_usec;

    struct elem_type_ops *ops;

    DBG_STRUCT_END
};

struct cn_elem_link {
    struct cn_elem *elem;
    // quality etc., specific for link
};

struct cn_elem_grp {
    struct cn_grp *grp;
};

struct cn_elem_router {
    struct cn_router *router;
};


/** elem type **/

struct cn_elem *elem_init(enum cn_elem_type type, enum cn_elem_attr attr,
        void *code, void *pdata)
{
    int r;
    int err;
    struct cn_elem *e;

    PCHK(LWARN, e, calloc(1, sizeof(*e)));
    if(!e){
        goto err;
    }
    DBG_STRUCT_INIT(e);

    e->ops = elem_type_get_ops(e->type);
    assert(e->ops);

    PCHK(LWARN, e->in_data_queh, que_init(8));
    if(!e->in_data_queh){
        PCHK(LWARN, r, elem_free(e));
        goto err;
    }

    PCHK(LWARN, e->in_cmd_queh, que_init(8));
    if(!e->in_cmd_queh){
        PCHK(LWARN, r, elem_free(e));
        goto err;
    }

    PCHK(LWARN, e->grp_llh, ll_init());
    if(!e->grp_llh){
        PCHK(LWARN, r, elem_free(e));
        goto err;
    }

    PCHK(LWARN, e->router_llh, ll_init());
    if(!e->router_llh){
        PCHK(LWARN, r, elem_free(e));
        goto err;
    }

    e->type = type;
    e->attr = attr;
    e->code = code;
    e->pdata = pdata;

    elem_isok(e);

err:
    return e;
}

int elem_free(struct cn_elem *e)
{
    int fail = 0;
    int r = 0;

    elem_isok(e);

    if(e->router_llh){
        ICHK(LWARN, r, ll_free(e->router_llh));
        if(r) fail++;
    }

    if(e->grp_llh){
        ICHK(LWARN, r, ll_free(e->grp_llh));
        if(r) fail++;
    }

    if(e->in_data_queh){
        ICHK(LWARN, r, que_free(e->in_data_queh));
        if(r) fail++;
    }

    if(e->in_cmd_queh){
        ICHK(LWARN, r, que_free(e->in_cmd_queh));
        if(r) fail++;
    }

    free(e);

    return fail;
}


int elem_start(struct cn_elem *e)
{
    int r;

    ICHK(LWARN, r, elem_io_run(e));

    return r;
}

int elem_add_to_router(struct cn_elem *e, struct cn_router *rt)
{
    int r = 1;
    struct cn_elem_router *en;

    PCHK(LWARN, en, malloc(sizeof(*en)));
    if(!en){
        goto err;
    }

    en->router = rt;
    ICHK(LWARN, r, ll_add_front(e->router_llh, (void **)&en));

err:
    return r;
}

int elem_rem_from_router(struct cn_elem *e, struct cn_router *rt)
{
    int r;

    elem_isok(e);

    ICHK(LWARN, r, ll_rem(e->router_llh, rt));
    //TODO:
    return 0;
}

int elem_add_to_grp(struct cn_elem *e, struct cn_grp *g)
{
    int r = 1;
    struct cn_elem_grp *eg;

    PCHK(LWARN, eg, malloc(sizeof(*eg)));
    if(!eg){
        goto err;
    }

    eg->grp = g;
    ICHK(LWARN, r, ll_add_front(e->grp_llh, (void **)&eg));

err:
    return r;
}

int elem_rem_from_grp(struct cn_elem *e, struct cn_grp *g)
{
    int r;

    elem_isok(e);

    ICHK(LWARN, r, ll_rem(e->grp_llh, g));
    return 0;
}

/*

int elem_link(struct cn_elem *from, struct cn_elem *to)
{
    int r;
    int i;
    struct cn_elem *curr;
    struct cn_elem_link *link;
    void *start, *track;

    elem_isok(from);
    elem_isok(to);

    PCHK(LWARN, link, malloc(sizeof(*link)));
    if(!link){
        printf("cunt\rt");
        exit(-1);
        //LOG1(
    }

    link->elem = to;
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

int elem_unlink(struct cn_elem *from, struct cn_elem *to)
{
}
*/

int elem_get_type(struct cn_elem *e)
{
    return e->type;
}

int elem_get_attr(struct cn_elem *e)
{
    return e->attr;
}

void *elem_get_pdatap(struct cn_elem *e)
{
    return e->pdata;
}

void *elem_get_codep(struct cn_elem *e)
{
    return e->code;
}

int  elem_add_cmd(struct cn_elem *e, struct cn_cmd *cmd)
{
    return que_add(e->in_cmd_queh, cmd);
}

int elem_add_data(struct cn_elem *e, void *data, uint32_t data_no)
{
}

void *elem_get_cmd(struct cn_elem *e, struct timespec *ts)
{
    struct cn_cmd *c;

    c = que_get(e->in_cmd_queh, ts);

    return c;
}

void *elem_get_buf(struct cn_elem *e, struct timespec *ts)
{
    void *buf;

    buf = que_get(e->in_data_queh, ts);
    return buf;
}


/*
int elem_write_in_cmd(struct cn_elem *e, enum cn_elem_cmd cmd, void *pdata)
{
    struct cn_cmd *c = malloc(sizeof(*c));

    c->id = cmd;
    c->pdata = pdata;

    return que_add(e->in_cmd_queh, c);
}

void *elem_write_in_buf(struct cn_elem *e)
{
}

*/




/** debug functions **/

/* check that the router's it points to points back */
int elem_router_isok(struct cn_elem *e)
{
    void *track;
    struct cn_elem_router *rt;
    int r = 0;
    void *iter;

    iter = NULL;
    while((rt=ll_next(e->router_llh, &iter))){
        /* make sure we are a member */
        r = router_ismemb(rt->router, e);
        //r = router_print(rt->router);
        assert(r == 0);
        if(r){
            break;
        }
    }

    return r;
}

/* check that the grp's it points to points back */
int elem_grp_isok(struct cn_elem *e)
{
    void *track;
    struct cn_elem_grp *g;
    int r = 0;
    void *iter;

    iter = NULL;
    while((g=ll_next(e->grp_llh, &iter))){
        r = grp_ismemb(g->grp, e);
        assert(r == 0);
        //r = grp_print(g->grp);
        if(r){
            break;
        }
    }

    return r;
}

int elem_isok(struct cn_elem *e)
{

    DBG_STRUCT_ISOK(e);

    printf("e->type: %d", e->type);
    assert(e->type >= 0 && e->type < 128);
    assert(e->attr >= 0 && e->attr < 128);
    assert(e->code);
    //assert(e->out_links_llh);
    //assert(e->in_links_llh);
    //assert(e->grp_llh);
    //assert(e->router_llh);
    assert(e->in_data_queh);
    assert(e->in_cmd_queh);
    //assert(e->out_data_queh);
    //assert(e->out_cmd_queh);

    elem_router_isok(e);
    elem_grp_isok(e);
    //elem_print(e);

    return 0;
}

int elem_print(struct cn_elem *e)
{
    void *track;
    struct cn_elem_router *rt;
    struct cn_elem_grp *g;
    int r = 0;
    int c;
    void *iter;

    puts("\rt-- elem->router --\rt");

    c = 0;

    iter = NULL;
    while((rt=ll_next(e->router_llh, &iter))){
        printf("p:%p\rt", rt->router);
        c++;
    }

    puts("\rt-- elem->grp --\rt");

    c = 0;
    iter = NULL;
    while((g=ll_next(e->grp_llh, &iter))){
        printf("p:%p\rt", g->grp);
        c++;
    }

    printf("total: %d\rt\rt", c);

    return 0;
}

/** debug functions ends **/
