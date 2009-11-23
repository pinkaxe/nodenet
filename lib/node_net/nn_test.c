
/* a simple test program for node_net, for now 
 * create nodes/routers/conn's, change their states, send pkt's to them, 
 * and cleanup in loop. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "sys/thread.h"
#include "util/log.h"
#include "util/dpool.h"

#include "node_net/types.h"
#include "node_net/pkt.h"

#include "node_net/router.h"
#include "node_net/node.h"

#define ok(x){ \
    assert(x); \
}

struct buf {
    int i;
    char var[1024];
};

#define CHAN_NO 3

#define CHAN_SERVER 0
#define CHAN_CONN_HANDLERS 1
#define CHAN_MAIN_CHANNEL 2

struct nn_chan *g[CHAN_NO];


static int dpool_free_cb(void *dpool, void *dpool_buf)
{
    L(LNOTICE, "dpool_free_cb called");
    return dpool_ret_buf(dpool, dpool_buf);
}

static int free_cb(void *none, void *buf)
{
    L(LDEBUG, "free_cb called");
    free(buf);
    return 0;
}

void *thread0(struct nn_node *n, void *pdata)
{
    int i = 0;
    struct dpool *dpool = pdata;
    struct dpool_buf *dpool_buf;
    struct buf *buf;
    struct nn_pkt *pkt;
    enum nn_state state;


    for(;;){

        i++;
        /* do state */
        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            // cleanup if need to
            node_set_state(n, NN_STATE_FINISHED);
            return NULL;
        }

        /* send packets */
        if((dpool_buf=dpool_get_buf(dpool))){
            /* */
            //buf = dpool_buf_get_datap(dpool_buf);
            buf = dpool_buf->data;
            /* set buf */
            buf->i = i++;

            /* build packet */
            PCHK(LWARN, pkt, pkt_init(n, CHAN_MAIN_CHANNEL, 0, dpool_buf,
                        sizeof(*dpool_buf), dpool, dpool_free_cb));
            assert(pkt);

            //try_send_global++;
            if(!node_tx(n, pkt)){
                //usleep(10000);
                //pkt_set_state(pkt, PKT_STATE_CANCELLED);
                //printf("!!! sent\n");
            }else{
                //assert(0); // slam
                pkt_free(pkt);
                usleep(10000);
            }

        }

        sched_yield();
        //usleep(1000);
    }

    return NULL;
}

void *thread1(struct nn_node *n, void *pdata)
{
    //struct dpool *dpool = pdata;
    struct nn_pkt *pkt;
    struct buf *buf;
    enum nn_state state;
    struct dpool_buf *dpool_buf;

    for(;;){

        node_wait(n);

        /* check state */
        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            // cleanup if need to
            node_set_state(n, NN_STATE_FINISHED);
            return NULL;
        }

        if(!node_rx(n, &pkt)){

            /* incoming packet */
            dpool_buf = pkt_get_data(pkt);
            buf = dpool_buf->data;

            L(LNOTICE, "Got buf->i=%d", buf->i);
            pkt_free(pkt);
        }
        sched_yield();
        //usleep(1000);
    }

    return NULL;
}

#define SOCK_TEST(call) \
    if((call) == -1){ \
        L(LERR, "%s:%d:%s Error: %s\n", \
            __FILE__, __LINE__, #call, strerror(errno)); \
    }

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>



void *server(struct nn_node *n, void *pdata)
{
    struct dpool *dpool = pdata;
    struct dpool_buf *dpool_buf;
    struct buf *buf;
    struct nn_pkt *pkt;
    enum nn_state state;
    int r;


    int port = 4848;
    int max = 10;
    int tr = 1;
    struct   sockaddr_in sin;
    struct   sockaddr_in cin;
    socklen_t addrlen;
    int fd, cfd;

    /* start listening server */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    SOCK_TEST(fd = socket(AF_INET, SOCK_STREAM, 0));
    SOCK_TEST(bind(fd, (struct sockaddr *) &sin, sizeof(sin)));
    SOCK_TEST(setsockopt(fd, SOL_SOCKET,SO_REUSEADDR, &tr, sizeof(int)));
    SOCK_TEST(listen(fd, max) == -1);

    int flags;
    if (-1 == (flags = fcntl(fd, F_GETFL, 0))){
        flags = 0;
    }
    ICHK(LWARN, r, fcntl(fd, F_SETFL, flags | O_NONBLOCK));

    addrlen = sizeof(cin);

    for(;;){

        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            // cleanup if need to
            close(fd);
            return NULL;
        }

        cfd = accept(fd, (struct sockaddr *)  &cin, &addrlen);
        if(cfd == -1){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                // nothing available
            }
        }else{
            L(LINFO, "Connection from: %s:%d",
                    inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));

            /* send fd to worker thread */
            if((dpool_buf=dpool_get_buf(dpool))){
                /* */
                //buf = dpool_buf_get_datap(dpool_buf);
                buf = dpool_buf->data;
                /* set buf */
                buf->i = cfd;

                /* build packet */
                PCHK(LWARN, pkt, pkt_init(n, CHAN_CONN_HANDLERS, 1, dpool_buf,
                            sizeof(*dpool_buf), dpool, dpool_free_cb));
                assert(pkt);

                if(node_tx(n, pkt)){
                    // fail
                    pkt_free(pkt);
                    close(fd);
                }
            }
        }


        sched_yield();
    }

    return NULL;
}


void *connection(struct nn_node *n, void *pdata)
{
    //struct dpool *dpool = pdata;
    struct dpool_buf *dpool_buf;
    enum nn_state state;
    size_t c;
    struct nn_pkt *pkt;
    int buf_len;
    struct buf *buf_req;
    int fd = 0;
    char *buf_in = NULL;
    char *buf_out = NULL;


    for(;;){

        /* set how many to allow from which group */
        node_set_rx_cnt(n, CHAN_CONN_HANDLERS, 1); /* get one conn first, get
                                                     dec each time a buffer is rx */
        node_set_rx_cnt(n, CHAN_MAIN_CHANNEL, 0);

        while(node_rx(n, &pkt)){
            state = node_do_state(n);
            if(state == NN_STATE_SHUTDOWN){
                // cleanup if need to
                return NULL;
            }
            usleep(100000);
        }

        node_set_rx_cnt(n, CHAN_MAIN_CHANNEL, -2); /* allow unlimited */

        dpool_buf = pkt_get_data(pkt);
        buf_req = dpool_buf->data;
        fd = buf_req->i;

        L(LINFO, "Starting fd=%d", fd);
        /* don't need the packet anymore */
        pkt_free(pkt);

        int flags;
        int r;
        if (-1 == (flags = fcntl(fd, F_GETFL, 0))){
            flags = 0;
        }
        ICHK(LWARN, r, fcntl(fd, F_SETFL, flags | O_NONBLOCK));

        for(;;){

            state = node_do_state(n);
            if(state == NN_STATE_SHUTDOWN){
                // cleanup if need to
                close(fd);
                if(buf_in) free(buf_in);
                return NULL;
            }

            if(!buf_in){
                buf_in = malloc(1024);
            }

            if((c=read(fd, buf_in, 1024)) && c != -1){
                /* */
                //buf_len = dpool_buf->data_len;
                buf_len = c;
                buf_in[c] = '\0';

                printf("!!!!! tx %s(%d)\n", buf_in, (int) c);
                if(!strncmp(buf_in, "quit", 4)){
                    break;
                }

                PCHK(LWARN, pkt, pkt_init(n, CHAN_MAIN_CHANNEL, 0, buf_in, buf_len,
                            NULL, free_cb));
                assert(pkt);

                while(node_tx(n, pkt)){
                    usleep(10000);
                }
                buf_in = NULL;
            }

            /* receive packets and write to client */
            while(!node_rx(n, &pkt)){
                //struct dpool_buf *dpool_buf = pkt_get_data(pkt);
                buf_out = pkt_get_data(pkt);
                printf("!!!! writing: %s\n", buf_out);
                write(fd, "resp: ", strlen("resp: "));
                write(fd, buf_out, strlen(buf_out));
                /* when done call pkt_free */
                pkt_free(pkt);
            }

            sched_yield();
        }

        close(fd);
        fd = 0;
    }

    return NULL;
}

static void *main_thread(void *none);

int main(int argc, char **argv)
{
    thread_t tid;

    thread_create(&tid, NULL, main_thread, NULL);

    while(1){
        usleep(10000000);
        //printf("!!! spin\n");
    }

    return 0;
}


static void *main_thread(void *none)
{
    int i;
    struct nn_router *rt[1];

    struct nn_node *n[1024];
    struct nn_node *n1[1024];

    struct dpool *dpool;
    struct buf *buf;
    struct  router_status rt_status;
    struct  node_status n_status[1024];


    int times;
    int thread0_no = 0;
    int thread1_no = 0;
    int thread0_no_max = 7;
    int thread1_no_max = 9;

    for(times=0; times < 100000; times++){

        dpool = dpool_create(sizeof(*buf), 3000, 0);

        thread0_no++;
        if(thread0_no > thread0_no_max){
            thread0_no = 1;
        }

        thread1_no++;
        if(thread1_no > thread1_no_max){
            thread1_no = 1;
        }

        rt[0] = router_init();
        ok(rt[0]);

        for(i=0; i < CHAN_NO; i++){
            router_add_chan(rt[0], i);
        }

        // node 0
        for(i=0; i < thread0_no; i++){
            n[i] = node_init(NN_NODE_TYPE_THREAD, 0, thread0, dpool);

            router_add_node(rt[0], n[i]);
            router_add_to_chan(rt[0], CHAN_SERVER, n[i]);
        }


        // node 1
        for(i=0; i < thread1_no; i++){
            n1[i] = node_init(NN_NODE_TYPE_THREAD, 0, thread1, dpool);

            router_add_node(rt[0], n1[i]);
            router_add_to_chan(rt[0], CHAN_MAIN_CHANNEL, n1[i]);
        }

        router_set_state(rt[0], NN_STATE_RUNNING);

        for(i=0; i < thread0_no; i++){
            node_set_state(n[i], NN_STATE_RUNNING);
        }

        for(i=0; i < thread1_no; i++){
            node_set_state(n1[i], NN_STATE_RUNNING);
        }

        //usleep(10000000);
        usleep(3000000);

        for(i=0; i < thread0_no; i++){
            node_set_state(n[i], NN_STATE_PAUSED);
        }

        for(i=0; i < thread1_no; i++){
            node_set_state(n1[i], NN_STATE_PAUSED);
        }

        router_set_state(rt[0], NN_STATE_PAUSED);


        /* print status */
        router_get_status(rt[0], &rt_status);
        printf("Router rx_pkts_total: %d\n", rt_status.rx_pkts_total);
        printf("Router tx_pkts_total: %d\n", rt_status.tx_pkts_total);


        for(i=0; i < thread0_no; i++){
            //router_rem_from_chan(rt[0], CHAN_SERVER, n[i]);
            router_rem_node(rt[0], n[i]);
            node_get_status(n[i], &n_status[i]);
            printf("Node 0.%d rx_pkts_total: %d\n", i, n_status[i].rx_pkts_total);
            printf("Node 0.%d tx_pkts_total: %d\n", i, n_status[i].tx_pkts_total);
            node_set_state(n[i], NN_STATE_SHUTDOWN);
        }

        for(i=0; i < thread1_no; i++){
            //router_rem_from_chan(rt[0], CHAN_MAIN_CHANNEL, n1[i]);
            router_rem_node(rt[0], n1[i]);
            node_get_status(n1[i], &n_status[i]);
            printf("Node 1.%d rx_pkts_total: %d\n", i, n_status[i].rx_pkts_total);
            printf("Node 1.%d tx_pkts_total: %d\n", i, n_status[i].tx_pkts_total);
            node_set_state(n1[i], NN_STATE_SHUTDOWN);
        }
        printf("Times: %d, thread_0_no=%d, thread1_no=%d\n", times, thread0_no, thread1_no);

        usleep(2000000);

        router_set_state(rt[0], NN_STATE_SHUTDOWN);

        for(i=0; i < thread0_no; i++){
            node_free(n[i]);
        }

        for(i=0; i < thread1_no; i++){
            node_free(n1[i]);
        }

        router_free(rt[0]);

        //usleep(3000000);

        dpool_free(dpool);

    }

    return 0;
}

