
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "util/log.h"
#include "util/dpool.h"
#include "util/que.h"
#include "util/ll2.h"
#include "util/dbg.h"

#include "types.h"
#include "io.h"
#include "elem.h"
#include "grp.h"
#include "net.h"
#include "run.h"


struct cn_elem {
    DBG_STRUCT_START
    enum cn_elem_type type; /* thread, process etc. */
    enum cn_elem_attr attr;
    void *code;  /* pointer to object depending on type */

    /* pointers for relationships */
    struct ll2 *out_links_llh;         /* links to other code_elem's */
    struct ll2 *in_links_llh;          /* links coming in from other code_elem's */
    struct ll2 *grp_llh;             /* links to all groups this elem belongs to */
    struct ll2 *net_llh;               /* links to all groups this elem belongs to */

    /* bufs */
    struct que *in_data_queh;         /* input for code elem */
    struct que *in_cmd_queh;          /* input cmds for code elem */

    struct que *out_data_queh;        /* output to other code elem's */
    struct que *out_cmd_queh;         /* input cmds for code elem */

    void *pdata; /* private passthru data */

    /* maximum time a element is allowed to exe on one buffer processing */
    uint32_t max_exe_usec;

    DBG_STRUCT_END
};

struct cn_elem_link {
    struct cn_elem *elem;
    // quality etc., specific for link
};

struct cn_elem_grp {
    struct cn_grp *grp;
};

struct cn_elem_net {
    struct cn_net *net;
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

    PCHK(LWARN, e->in_links_llh, ll2_init());
    if(!e->in_links_llh){
        PCHK(LWARN, r, elem_free(e));
        goto err;
    }

    PCHK(LWARN, e->grp_llh, ll2_init());
    if(!e->grp_llh){
        PCHK(LWARN, r, elem_free(e));
        goto err;
    }

    PCHK(LWARN, e->net_llh, ll2_init());
    if(!e->net_llh){
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

    if(e->net_llh){
        ICHK(LWARN, r, ll2_free(e->net_llh));
        if(r) fail++;
    }

    if(e->grp_llh){
        ICHK(LWARN, r, ll2_free(e->grp_llh));
        if(r) fail++;
    }

    if(e->in_links_llh){
        ICHK(LWARN, r, ll2_free(e->in_links_llh));
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
    /* we call run in a different file so it is possible to replaced it with a
     * version not using threads */
    ICHK(LWARN, r, run(e));

    return r;
}

int elem_add_to_net(struct cn_elem *e, struct cn_net *n)
{
    int r = 1;
    struct cn_elem_net *en;

    PCHK(LWARN, en, malloc(sizeof(*en)));
    if(!en){
        goto err;
    }

    en->net = n;
    ICHK(LWARN, r, ll2_add_front(e->net_llh, (void **)&en));

err:
    return r;
}

int elem_rem_from_net(struct cn_elem *e, struct cn_net *n)
{
    int r;

    elem_isok(e);

    ICHK(LWARN, r, ll2_rem(e->net_llh, n));
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
    ICHK(LWARN, r, ll2_add_front(e->grp_llh, (void **)&eg));

err:
    return r;
}

int elem_rem_from_grp(struct cn_elem *e, struct cn_grp *g)
{
    int r;

    elem_isok(e);

    ICHK(LWARN, r, ll2_rem(e->grp_llh, g));
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
        printf("cunt\n");
        exit(-1);
        //LOG1(
    }

    link->elem = to;
    PCHK(LWARN, r, ll2_add_front(from->out_links_llh, (void **)&link));

    // link back
    PCHK(LWARN, link, malloc(sizeof(*link)));
    if(!link){
        printf("cunt\n");
        exit(-1);
        //LOG1(
    }

    ll2_add_front(to->in_links_llh, (void **)&link);
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

int  send_cmd_to_elem(struct cn_elem *e, struct cn_io_cmd *cmd)
{
    //printf("!!! sending: %p\n", cmd);
    return que_add(e->in_cmd_queh, cmd);
}

int  send_data_to_elem(struct cn_elem *e, void *data, uint32_t data_no)
{
}


/*
 
int elem_write_in_cmd(struct cn_elem *e, enum cn_elem_cmd cmd, void *pdata)
{
    struct cn_io_cmd *c = malloc(sizeof(*c));

    c->id = cmd;
    c->pdata = pdata;

    return que_add(e->in_cmd_queh, c);
}

void *elem_write_in_buf(struct cn_elem *e)
{
}

*/

void *elem_read_in_cmd(struct cn_elem *e, struct timespec *ts)
{
    struct cn_io_cmd *c;

    c = que_get(e->in_cmd_queh, ts);
    //printf("!!! cheking: %p\n", c);
    //if(c){
    //    pdata = c->pdata;
    //    free(c);
    //}

    return c;
}


void *elem_read_in_buf(struct cn_elem *e, struct timespec *ts)
{
    void *buf;

    buf = que_get(e->in_data_queh, ts);
    return buf;
}


/** debug functions **/

/* check that the net's it points to points back */
int elem_net_isok(struct cn_elem *e)
{
    void *track;
    struct cn_elem_net *n;
    int r = 0;
    void *iter;

    iter = NULL;
    while((n=ll2_next(e->net_llh, &iter))){
        /* make sure we are a member */
        r = net_ismemb(n->net, e);
        //r = net_print(n->net);
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
    while((g=ll2_next(e->grp_llh, &iter))){
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
    //assert(e->net_llh);
    assert(e->in_data_queh);
    assert(e->in_cmd_queh);
    //assert(e->out_data_queh);
    //assert(e->out_cmd_queh);

    elem_net_isok(e);
    elem_grp_isok(e);
    //elem_print(e);

    return 0;
}

int elem_print(struct cn_elem *e)
{
    void *track;
    struct cn_elem_net *n;
    struct cn_elem_grp *g;
    int r = 0;
    int c;
    void *iter;

    puts("\n-- elem->net --\n");

    c = 0;

    iter = NULL;
    while((n=ll2_next(e->net_llh, &iter))){
        printf("p:%p\n", n->net);
        c++;
    }

    puts("\n-- elem->grp --\n");

    c = 0;
    iter = NULL;
    while((g=ll2_next(e->grp_llh, &iter))){
        printf("p:%p\n", g->grp);
        c++;
    }

    printf("total: %d\n\n", c);

    return 0;
}

/** debug functions ends **/
