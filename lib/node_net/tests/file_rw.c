
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "util/log.h"

#include "node_net/types.h"
#include "node_net/pkt.h"

#include "node_net/router.h"
#include "node_net/node.h"
#include "node_net/conn.h"

#define CHAN_NO 2

#define CHAN_RW 0
#define CHAN_W 1

#define BUF_LEN 4096

struct nn_chan *chan[CHAN_NO];

struct buf {
    unsigned char *data[BUF_LEN];
    int len;
};

static int free_cb(void *none, void *buf)
{
    L(LDEBUG, "free_cb called");
    free(buf);
    return 0;
}


void *reader_writer(struct nn_node *n, void *pdata)
{
    struct buf *buf;
    struct nn_pkt *pkt;
    enum nn_state state;
    int fd;

    fd = open("x", O_RDONLY);
    if(fd < 2){
        printf("%s\n", strerror(errno));
        exit(1);
    }

   //long sz = sysconf(_SC_PAGESIZE);

    for(;;){

        /* do state */
        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            // cleanup if need to
            close(fd);
            return NULL;
        }

        //if(i++ < 12800000){
            /* send packets */
            if((buf=malloc(sizeof(*buf)))){

                buf->len = read(fd, buf->data, BUF_LEN);
                if(buf->len > 0){
                    /* build packet */
                    PCHK(LWARN, pkt, pkt_init(n, CHAN_W, 0, buf,
                                sizeof(*buf), NULL, free_cb));
                    assert(pkt);

                    if(node_tx(n, pkt)){
                        assert(0);
                    }
                }

            }
        //}

        usleep(1000);
    }

}

void *writer(struct nn_node *n, void *pdata)
{
    struct buf *buf;
    struct nn_pkt *pkt;
    enum nn_state state;

    int fd;

    fd = open("node.dump", O_WRONLY | O_CREAT);
    assert(fd > 2);

    for(;;){

        node_wait(n);

        /* do state */
        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            // cleanup if need to
            close(fd);
            return NULL;
        }

        while(!node_rx(n, &pkt)){
            /* incoming packet */
            buf = pkt_get_data(pkt);
            write(fd, buf->data, buf->len);
            //L(LINFO, "Got buf->i=%d", buf->i);
            //recv_global++;
            /* when done call pkt_free */
            //dpool_ret_buf(dpool, dpool_buf);
            pkt_free(pkt);
        }
        //sched_yield();
        usleep(1000);
    }
}

void *writer2(struct nn_node *n, void *pdata)
{
    struct buf *buf;
    struct nn_pkt *pkt;
    enum nn_state state;
    int c;
    int fd;

    fd = open("node2.dump", O_WRONLY | O_CREAT);
    assert(fd > 2);

    for(;;){

        node_wait(n);

        /* do state */
        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            // cleanup if need to
            close(fd);
            return NULL;
        }

        while(!node_rx(n, &pkt)){
            /* incoming packet */
            buf = pkt_get_data(pkt);
            c = write(fd, buf->data, buf->len);
            if(c <= 0){
                L(LINFO, "Got buf->i=%d", buf->i);
            }
            pkt_free(pkt);
        }
        //sched_yield();
        usleep(1000);
    }
}


int main(int argc, char **argv)
{
    int i;
    struct nn_router *rt[1];

    struct nn_node *n[1024];
    struct nn_conn *cn[1024];

    struct  router_status rt_status;
    struct  node_status n_status[1024];

    while(1){

        rt[0] = router_init();

        for(i=0; i < CHAN_NO; i++){
            router_add_chan(rt[0], i);
        }

        n[0] = node_init(NN_NODE_TYPE_THREAD, 0, reader_writer, NULL);
        cn[0] = conn_conn(n[0], rt[0]);
        conn_join_chan(cn[0], CHAN_RW);

        n[1] = node_init(NN_NODE_TYPE_THREAD, 0, writer, NULL);
        cn[1] = conn_conn(n[1], rt[0]);
        conn_join_chan(cn[1], CHAN_W);

        n[2] = node_init(NN_NODE_TYPE_THREAD, 0, writer2, NULL);
        cn[2] = conn_conn(n[2], rt[0]);
        conn_join_chan(cn[2], CHAN_W);

#define NODE_NO 3
#if 0
        for(i=2; i < NODE_NO; i++){

            n[i] = node_init(NN_NODE_TYPE_THREAD, 0, thread0, dpool);
            cn[i] = conn_conn(n[i], rt[0]);

            conn_join_chan(cn[i], CHAN_CONN_HANDLERS);
            conn_join_chan(cn[i], CHAN_MAIN_CHANNEL);

            //conn_conn(n[i], rt[0], CHAN_MAIN_CHANNEL); /* valgrind */
        }
#endif

        router_set_state(rt[0], NN_STATE_RUNNING);

        //for(i=NODE_NO-1; i >= 0; i--){
        for(i=0; i < NODE_NO; i++){
            node_set_state(n[i], NN_STATE_RUNNING);
        }

        sleep(2);

        //router_set_state(rt[0], NN_STATE_PAUSED);

        //for(i=NODE_NO-1; i >= 0; i--){
        //    node_set_state(n[i], NN_STATE_PAUSED);
        //}

        //sleep(1);

#if 0
        for(i=2; i < NODE_NO; i++){
            conn_quit_chan(cn[i], CHAN_CONN_HANDLERS);
            conn_quit_chan(cn[i], CHAN_MAIN_CHANNEL);
            cn[i] = conn_unconn(n[i], rt[0]);
            node_set_state(n[i], NN_STATE_SHUTDOWN);
        }

        sleep(1);
#endif

        conn_quit_chan(cn[0], CHAN_RW);
        node_set_state(n[0], NN_STATE_SHUTDOWN);

        conn_quit_chan(cn[1], CHAN_W);
        node_set_state(n[1], NN_STATE_SHUTDOWN);

        conn_quit_chan(cn[2], CHAN_W);
        node_set_state(n[2], NN_STATE_SHUTDOWN);

        router_set_state(rt[0], NN_STATE_SHUTDOWN);

       // for(i=0; i < CHAN_NO; i++){
       //     router_rem_chan(rt[0], i);
       // }

        for(i=0; i < NODE_NO; i++){
            node_get_status(n[i], &n_status[i]);
            node_free(n[i]);
        }

        router_get_status(rt[0], &rt_status);

        router_clean(rt[0]);


        printf("Router rx_pkts_total: %d\n", rt_status.rx_pkts_total);
        printf("Router tx_pkts_total: %d\n", rt_status.tx_pkts_total);

        for(i=0; i < NODE_NO; i++){
            printf("Node %d rx_pkts_total: %d\n", i,
                    n_status[i].rx_pkts_total);
            printf("Node %d tx_pkts_total: %d\n", i,
                    n_status[i].tx_pkts_total);
        }

        //printf("Router rx_pkts_no: %d\n", status.rx_pkts_no);
        //printf("Router tx_pkts_no: %d\n", status.tx_pkts_no);

        return 0;
    }
}
