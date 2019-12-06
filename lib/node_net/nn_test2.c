
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "log/log.h"
#include "dpool/dpool.h"

#include "node_net/types.h"
#include "node_net/pkt.h"

#include "node_net/router.h"
#include "node_net/node.h"

#define CHAN_SERVER 0
#define CHAN_CONN_HANDLERS 1
#define CHAN_MAIN_CHANNEL 2


static int dpool_free_cb(void *dpool, void *buf)
{
    L(LDEBUG, "dpool_free_cb called");
    struct dpool_buf *dpool_buf = buf;
    free(dpool_buf->data);
    return dpool_ret_buf(dpool, dpool_buf);
}

void *node_writer(struct nn_node *n, void *pdata)
{
    L(LDEBUG, "Enter node_writer\n");

    int i = 0;

    struct dpool *dpool = pdata;
    struct dpool_buf *dpool_buf;
    //struct buf *buf;
    struct nn_pkt *pkt;

    for(;;){

        L(LDEBUG, "before tx");
        node_wait(n, NN_TX_READY);
        L(LDEBUG, "tx");

        /* send packets */
        if((dpool_buf=dpool_get_buf(dpool))){

            dpool_buf->data = malloc(7);
            strcpy(dpool_buf->data, "okokok");

            /* build packet */
            PCHK(LDEBUG, pkt, pkt_init(n, CHAN_MAIN_CHANNEL, 0, dpool_buf,
                        sizeof(*dpool_buf), dpool, dpool_free_cb));
            assert(pkt);

            if(!node_tx(n, pkt)){
                usleep(100000);
                //pkt_set_state(pkt, PKT_STATE_CANCELLED);
                //printf("!!! sent\n");
            }else{
                pkt_free(pkt);
            }

        }else{
            // no buffer available */
            usleep(100);
        }

    }

    return NULL;
}

void *node_reader(struct nn_node *n, void *pdata)
{
    struct nn_pkt *pkt;
    //struct buf *buf;
    char *buf;
    enum nn_state state;
    struct dpool_buf *dpool_buf;

    printf("Enter node_reader\n");

    for(;;){

        L(LDEBUG, "before rx");
        node_wait(n, NN_RX_READY);
        L(LDEBUG, "rx");

        /* check state */
        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            // cleanup if need to
            node_set_state(n, NN_STATE_DONE);
            return NULL;
        }

        if(!node_rx(n, &pkt)){

            /* incoming packet */
            dpool_buf = pkt_get_data(pkt);
            buf = dpool_buf->data;

            //L(LNOTICE, "Got buf->i=%d", buf->i);
            L(LNOTICE, "Got buf=%s", buf);
            //free(buf->str);
            pkt_free(pkt);
        }
        //sched_yield();
        //usleep(1000);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    struct dpool *dpool;
    //struct buf *buf;
    char *buf;

    struct nn_router *router;
    struct nn_node *node0;
    struct nn_node *node1;

    dpool = dpool_create(sizeof(buf), 300, 0);

    router = router_init();
    router_add_chan(router, 0);
    router_add_chan(router, 1);
    router_add_chan(router, 2);

    node0 = node_init(NN_NODE_TYPE_THREAD, 0, node_writer, dpool);
    router_add_node(router, node0);
    router_add_to_chan(router, CHAN_MAIN_CHANNEL, node0);

    node1 = node_init(NN_NODE_TYPE_THREAD, 0, node_reader, dpool);
    router_add_node(router, node1);
    router_add_to_chan(router, CHAN_MAIN_CHANNEL, node1);

    router_set_state(router, NN_STATE_RUNNING);
    node_set_state(node0, NN_STATE_RUNNING);
    node_set_state(node1, NN_STATE_RUNNING);

    while(1){
        usleep(10000000);
        //printf("!!! spin\n");
    }

    return 0;
}
