
/* a simple test program for node_net, for now 
 * create nodes/routers/conn's, change their states, send pkt's to them, 
 * and cleanup in loop. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#include "util/log.h"
#include "util/dpool.h"

#include "node_net/types.h"
#include "node_net/pkt.h"

#include "node_net/router.h"
#include "node_net/node.h"
#include "node_net/conn.h"

#define ok(x){ \
    assert(x); \
}

struct buf {
    int i;
    char var[1024 * 128];
};

#define CHAN_NO 3

#define CHAN_SERVER 0
#define CHAN_CONN_HANDLERS 1
#define CHAN_MAIN_CHANNEL 2

struct nn_chan *g[CHAN_NO];

//static int recv_global = 0;
//static int send_global = 0;
//static int try_send_global = 0;

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

void *thread1(struct nn_node *n, void *pdata)
{
    int i = 0;
    struct dpool *dpool = pdata;
    struct dpool_buf *dpool_buf;
    struct buf *buf;
    struct nn_pkt *pkt;
    enum nn_state state;

    sleep(1);
    for(;;){

        i++;
        /* do state */
        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            // cleanup if need to
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
                printf("!!! sent\n");
            }else{
                assert(0); // slam
                //pkt_free(pkt);
            }

        }
        //    while(pkt_get_state(pkt) != PKT_STATE_N_RX){
        //        printf("state: %d\n", pkt_get_state(pkt));
        //        usleep(1);
        //    }
        //    printf("state: %d\n", pkt_get_state(pkt));
        //    //pkt_free(pkt);
        //    exit(1);
        //    sleep(1);


        printf("!!! sleeping\n");
        usleep(20000);
    }

    return NULL;
}

void *thread0(struct nn_node *n, void *pdata)
{
    //struct dpool *dpool = pdata;
    struct nn_pkt *pkt;
    struct buf *buf;
    enum nn_state state;
    struct dpool_buf *dpool_buf;

    for(;;){

        //node_wait(n);

        /* check state */
        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            // cleanup if need to
            return NULL;
        }

        while(!node_rx(n, &pkt)){

            /* incoming packet */
            dpool_buf = pkt_get_data(pkt);
            buf = dpool_buf->data;

            L(LNOTICE, "Got buf->i=%d", buf->i);
            pkt_free(pkt);
        }
        //sched_yield();
        usleep(1000);
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


int main(int argc, char **argv)
{
    int i;
    struct nn_router *rt[1];

    struct nn_node *n[1024];
    struct nn_conn *cn[1024];

    //struct nn_pkt *pkt;
    struct dpool *dpool;
    struct buf *buf;
    //struct dpool_buf *dpool_buf;
    struct  router_status rt_status;
    struct  node_status n_status[1024];


    int times;
    for(times=0; times < 100000; times++){
        dpool = dpool_create(sizeof(*buf), 10000, 0);

        rt[0] = router_init();
        ok(rt[0]);

        for(i=0; i < CHAN_NO; i++){
            router_add_chan(rt[0], i);
        }

        n[0] = node_init(NN_NODE_TYPE_THREAD, 0, thread1, dpool);
        cn[0] = conn_conn(n[0], rt[0]);
        conn_join_chan(cn[0], CHAN_SERVER);

        n[1] = node_init(NN_NODE_TYPE_THREAD, 0, thread1, dpool);
        cn[1] = conn_conn(n[1], rt[0]);
        conn_join_chan(cn[1], CHAN_SERVER);

#define NODE_NO 3
        for(i=2; i < NODE_NO; i++){

            n[i] = node_init(NN_NODE_TYPE_THREAD, 0, thread0, dpool);
            cn[i] = conn_conn(n[i], rt[0]);

            conn_join_chan(cn[i], CHAN_CONN_HANDLERS);
            conn_join_chan(cn[i], CHAN_MAIN_CHANNEL);

            //conn_conn(n[i], rt[0], CHAN_MAIN_CHANNEL); /* valgrind */
        }

        router_set_state(rt[0], NN_STATE_RUNNING);

        for(i=NODE_NO-1; i >= 0; i--){
            node_set_state(n[i], NN_STATE_RUNNING);
        }

        while(1){
            usleep(1000000);
        }
       // assert(0);
        //usleep(1000000);
       // sleep(3);

        router_set_state(rt[0], NN_STATE_PAUSED);

        for(i=NODE_NO-1; i >= 0; i--){
            node_set_state(n[i], NN_STATE_PAUSED);
        }

        sleep(1);

        conn_quit_chan(cn[0], CHAN_SERVER);
        //conn_unconn(n[0], rt[0]);
        node_unconn(n[0], cn[0]);
        node_set_state(n[0], NN_STATE_SHUTDOWN);

        conn_quit_chan(cn[1], CHAN_SERVER);
        //conn_unconn(n[1], rt[0]);
        node_unconn(n[1], cn[1]);
        node_set_state(n[1], NN_STATE_SHUTDOWN);


        for(i=2; i < NODE_NO; i++){
            conn_quit_chan(cn[i], CHAN_CONN_HANDLERS);
            conn_quit_chan(cn[i], CHAN_MAIN_CHANNEL);
            //conn_unconn(n[i], rt[0]);
            node_unconn(n[i], cn[i]);
            node_set_state(n[i], NN_STATE_SHUTDOWN);
        }


        router_set_state(rt[0], NN_STATE_SHUTDOWN);

       // for(i=0; i < CHAN_NO; i++){
       //     router_rem_chan(rt[0], i);
       // }


        for(i=0; i < NODE_NO; i++){
            node_get_status(n[i], &n_status[i]);
            node_clean(n[i]);
            //abort();
        }

        router_get_status(rt[0], &rt_status);

        router_clean(rt[0]);

        dpool_free(dpool);

        printf("Router rx_pkts_total: %d\n", rt_status.rx_pkts_total);
        printf("Router tx_pkts_total: %d\n", rt_status.tx_pkts_total);

        for(i=0; i < NODE_NO; i++){
            printf("Node %d rx_pkts_total: %d\n", i, n_status[i].rx_pkts_total);
            printf("Node %d tx_pkts_total: %d\n", i, n_status[i].tx_pkts_total);
        }
    }

        //printf("Router rx_pkts_no: %d\n", status.rx_pkts_no);
        //printf("Router tx_pkts_no: %d\n", status.tx_pkts_no);

        //printf("!!! try sent: %d\n", try_send_global);
        //printf("!!! sent: %d\n", send_global);
        //printf("!!! receive: %d\n", recv_global);


        //exit(0);

#if 0

        /* send a pkt from the router */
        /*
        for(i=0; i < 100; i++){
            dpool_buf = dpool_get_buf(dpool);
            buf = dpool_buf->data;
            buf->i = i;
            pkt = pkt_init(c++, dpool_buf, sizeof(dpool_buf), dpool, 1,
                    NN_SENDTO_ALL, 0);
            while(nn_router_tx_pkt(rt[0], n[0], pkt)){
                usleep(1000);
            }
        }
        */
        //while(1){
            //sleep(3);
        //}
            sleep(10000);

        /* pause everything */
        for(i=0; i < 3; i++){
            node_set_state(n[i], NN_STATE_PAUSED);
        }
        router_set_state(rt[0], NN_STATE_PAUSED);

        /* run everything */
        for(i=0; i < 3; i++){
            node_set_state(n[i], NN_STATE_RUNNING);
        }
        router_set_state(rt[0], NN_STATE_RUNNING);


        for(i=0; i < 3; i++){
            /* unconn not needed but ok */
            //conn_unconn(n[i], rt[0]);
            node_set_state(n[i], NN_STATE_SHUTDOWN);
        }

#endif

    return 0;
}

