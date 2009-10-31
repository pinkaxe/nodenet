
/* a simple test program for node_net, for now 
 * create nodes/routers/grps, change there states, send pkt's to them, 
 * and cleanup in loop. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "util/log.h"

#include "node_net/types.h"
#include "node_net/pkt.h"
#include "node_net/nn.h"

#define ok(x){ \
    assert(x); \
}


int input_node(struct nn_node *n, void *buf, int len, void *pdata)
{
    L(LNOTICE, "Process buffer %", __FUNCTION__);
    return 0;
}

int process_node(struct nn_node *n, void *buf, int len, void *pdata)
{
    L(LNOTICE, "Process buffer %", __FUNCTION__);
    return 0;
}

int output_node(struct nn_node *n, void *buf, int len, void *pdata)
{

    L(LNOTICE, "Process buffer %", __FUNCTION__);
    //n->tx_pkt(n, );
    //nn_node_tx_pkt(n, buf, len, destiny);
    //nn_node_tx(n, );
    return 0;
}

#define GRPS_NO 3
int main(int argc, char **argv)
{
    int i, c;
    struct nn_router *rt[1];
    struct nn_node *n[1024];
    struct nn_grp *g[GRPS_NO];
    struct nn_pkt *pkt;

    while(1){

        rt[0] = nn_router_init();
        ok(rt[0]);

        for(i=0; i < GRPS_NO; i++){
            g[i] = nn_grp_init(i);
            ok(g[i]);
        }

        /* create input nodes */
        for(i=0; i < 100; i++){
            n[i] = nn_node_init(NN_NODE_TYPE_THREAD, NN_NODE_ATTR_NO_INPUT,
                    input_node, NULL);
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


        /* send a pkt to the router */
        for(i=0; i < 100; i++){
            pkt = pkt_init(c++, NULL, 0, 1, NN_SENDTO_ALL, 0);
            while(nn_router_tx_pkt(rt[0], n[i], pkt)){
                usleep(160000);
            }
        }
        sleep(2);

        /* unpause everything */
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

        printf("loop done\n");
        usleep(100000);
    }
    return 0;
}

