
/* a simple test program for node_net, for now 
 * create nodes/routers/grps, change their states, send pkt's to them, 
 * and cleanup in loop. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "util/log.h"
#include "util/dpool.h"

#include "node_net/types.h"
#include "node_net/pkt.h"
#include "node_net/nn.h"

#define ok(x){ \
    assert(x); \
}

struct mesg {
    int i;
};

void *thread1(struct nn_node *n, void *pdata)
{
    struct dpool *dpool = pdata;
    struct dpool_buf *dpool_buf;
    struct mesg *mesg;
    struct nn_pkt *pkt;
    printf("xxx %p\n", pdata);

    for(;;){
        while(!(dpool_buf=dpool_get_buf(dpool))){
            usleep(1000);
        }

        mesg = dpool_buf->data;
        mesg->i = 876;

        pkt = pkt_init(0, dpool_buf, sizeof(dpool_buf), dpool, 1,
                NN_SENDTO_ALL, 0);

        nn_node_add_tx_pkt(n, pkt);

        sleep(1);
    }

    return NULL;
}

void *thread0(struct nn_node *n, void *arg)
{
    struct nn_pkt *pkt;
    struct mesg *mesg;

    for(;;){
     //   pkt = nn_node_get_rx_pkt(n);
     //   //pkt_get_data_len(pkt);

     //   if(pkt){
     //       mesg = pkt_get_data(pkt);
     //       /* when done call pkt_free */
     //       dpool_ret_buf(mesg);
     //       pkt_free(pkt);

     //       /* build new packet and send */
     //       pkt = pkt_init(0, dpool_buf, sizeof(dpool_buf), dpool, 1,
     //               NN_SENDTO_ALL, 0);
     //       node_add_tx_pkt(pkt);
     //   }

     //   node_cond_wait(n, ts);
     sleep(1);

    }

    return NULL;
}

#if 0
int input_node(struct nn_node *n, void *buf, int len, void *pdata)
{
    struct dpool *dpool = pdata;
    struct dpool_buf *dpool_buf = buf;
    struct mesg *mesg  = dpool_buf->data;

    L(LNOTICE, "Process buffer %d, %s", mesg->i, __FUNCTION__);

    dpool_ret_buf(dpool, dpool_buf);

    return 0;
}


struct nn_pkt *node0_get_pkt(void *pdata)
{
    struct nn_pkt *pkt;

    dpool_buf = dpool_get_buf(dpool);
    mesg = dpool_buf->data;
    mesg->i = i;

    pkt = pkt_init(c++, dpool_buf, sizeof(dpool_buf), dpool, 1,
            NN_SENDTO_ALL, 0);
    return pkt;
}
#endif

#define GRPS_NO 3

int main(int argc, char **argv)
{
    int i, c;
    struct nn_router *rt[1];
    struct nn_node *n[1024];
    struct nn_grp *g[GRPS_NO];
    struct nn_pkt *pkt;
    struct dpool *dpool;
    struct mesg *mesg;
    struct dpool_buf *dpool_buf;

    while(1){

        dpool = dpool_create(sizeof(*mesg), 100, 0);

        rt[0] = nn_router_init();
        ok(rt[0]);

        for(i=0; i < GRPS_NO; i++){
            g[i] = nn_grp_init(i);
            ok(g[i]);
        }

        /* create input nodes */
        n[0] = nn_node_init(NN_NODE_TYPE_THREAD, NN_NODE_ATTR_NO_INPUT,
                thread1, dpool);
        printf("xxx1 %p\n", dpool);
        for(i=1; i < 100; i++){
            n[i] = nn_node_init(NN_NODE_TYPE_THREAD, NN_NODE_ATTR_NO_INPUT,
                    thread0, dpool);
            /* connect to router */
            nn_conn(n[i], rt[0]);
            //nn_join_grp(n[i], g[1]);
            ok(n[i]);
        }

        /* set everthing state to running */
        for(i=0; i < 100; i++){
            nn_node_set_state(n[i], NN_STATE_RUNNING);
        }
        nn_router_set_state(rt[0], NN_STATE_RUNNING);

        /* send a pkt from the router */
        /*
        for(i=0; i < 100; i++){
            dpool_buf = dpool_get_buf(dpool);
            mesg = dpool_buf->data;
            mesg->i = i;
            pkt = pkt_init(c++, dpool_buf, sizeof(dpool_buf), dpool, 1,
                    NN_SENDTO_ALL, 0);
            while(nn_router_tx_pkt(rt[0], n[0], pkt)){
                usleep(1000);
            }
        }
        */
        while(1){
            sleep(5);
        }

        /* pause everything */
        for(i=0; i < 100; i++){
            nn_node_set_state(n[i], NN_STATE_PAUSED);
        }
        nn_router_set_state(rt[0], NN_STATE_PAUSED);

        /* run everything */
        for(i=0; i < 100; i++){
            nn_node_set_state(n[i], NN_STATE_RUNNING);
        }
        nn_router_set_state(rt[0], NN_STATE_RUNNING);


        for(i=0; i < 100; i++){
            /* unconn not needed but ok */
            //nn_unconn(n[i], rt[0]);
            nn_node_free(n[i]);
        }

        nn_grp_free(g[0]);
        nn_grp_free(g[1]);
        nn_grp_free(g[2]);

        nn_router_free(rt[0]);

        for(i=0; i < 100; i++){
            nn_node_clean(n[i]);
        }

        nn_router_clean(rt[0]);

        dpool_free(dpool);

        printf("loop done\n");
        usleep(100000);
    }
    return 0;
}

