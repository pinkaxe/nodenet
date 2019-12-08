
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

//#define CHAN_SERVER 0
//#define CHAN_CONN_HANDLERS 1
//#define CHAN_MAIN_CHANNEL 2

struct nn_chan *chan0;

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

        sleep(2);
        L(LDEBUG, "before tx");
        node_wait(n, NN_TX_READY);
        L(LDEBUG, "tx");

        /* send packets */
        if((dpool_buf=dpool_get_buf(dpool))){

            dpool_buf->data = malloc(7);
            strcpy(dpool_buf->data, "okokok");

            /* build packet */
            PCHK(LDEBUG, pkt, pkt_init(n, chan0, 0, dpool_buf,
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
            L(LNOTICE, "!!!!! Got buf=%s", buf);
            //free(buf->str);
            pkt_free(pkt);
        }
        //sched_yield();
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

static int g_port = 4848;

void *tcpserver(struct nn_node *n, void *pdata)
{
    struct dpool *dpool = pdata;
    struct dpool_buf *dpool_buf;
    //struct buf *buf;
    struct nn_pkt *pkt;
    enum nn_state state;
    int r;

    int port = g_port++;
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

            char buf[256];
            int c;

            int flags = fcntl(cfd, F_GETFL, 0);
            fcntl(cfd, F_SETFL, flags | O_NONBLOCK);

            while(1){
                // get incoming on socket and write to channel
                c = read(cfd, buf, sizeof(buf-1));
                if(c > 0){
                    buf[c] = '\0';
                    printf("received: %s (%d)\n", buf, c);
                    if((dpool_buf=dpool_get_buf(dpool))){
                        /* */
                        //buf = dpool_buf_get_datap(dpool_buf);
                        //buf = dpool_buf->data;
                        /* set buf */
                        //buf->i = cfd;
                        dpool_buf->data = malloc(c);
                        strcpy(dpool_buf->data, buf);

                        /* build packet */
                        PCHK(LWARN, pkt, pkt_init(n, chan0, 0, dpool_buf,
                                    sizeof(*dpool_buf), dpool, dpool_free_cb));
                        assert(pkt);

                        if(node_tx(n, pkt)){
                            L(LWARN, "node_tx failed");
                            // fail
                            pkt_free(pkt);
                            close(fd);
                        }
                        printf("6\n");
                    }
                }else{
                    //printf("nothing\n");
                }

                // get incoming on channel and write to socket
                if(node_rx(n, &pkt) == 0){
                    /* incoming packet */
                    dpool_buf = pkt_get_data(pkt);

                    //L(LNOTICE, "Got buf->i=%d", buf->i);
                    L(LNOTICE, "Got buf=%s", dpool_buf->data);
                    c = write(cfd, dpool_buf->data, sizeof(dpool_buf->data));
    //EAGAIN
                    //free(buf->str);
                    pkt_free(pkt);

                }else{
                    L(LDEBUG, "nothing");
                }
                usleep(1000000);
            }
        }


        sched_yield();
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

    chan0 = chan_init();
    router_add_chan(router, chan0);
    //router_add_chan(router, 1);
    //router_add_chan(router, 2);

/*
    node0 = node_init(NN_NODE_TYPE_THREAD, 0, node_writer, dpool);
    router_add_node(router, node0);
    router_add_to_chan(router, CHAN_MAIN_CHANNEL, node0);

    node1 = node_init(NN_NODE_TYPE_THREAD, 0, node_reader, dpool);
    router_add_node(router, node1);
    router_add_to_chan(router, CHAN_MAIN_CHANNEL, node1);
*/

    router_set_state(router, NN_STATE_RUNNING);
 //   node_set_state(node0, NN_STATE_RUNNING);
 //   node_set_state(node1, NN_STATE_RUNNING);

    struct nn_node *tcpserver_node;
    tcpserver_node = node_init(NN_NODE_TYPE_THREAD, 0, tcpserver, dpool);
    router_add_node(router, tcpserver_node);
    router_add_to_chan(router, chan0, tcpserver_node);

    node_set_state(tcpserver_node, NN_STATE_RUNNING);

    struct nn_node *tcpserver_node2;
    tcpserver_node2 = node_init(NN_NODE_TYPE_THREAD, 0, tcpserver, dpool);
    router_add_node(router, tcpserver_node2);
    router_add_to_chan(router, chan0, tcpserver_node2);

    node_set_state(tcpserver_node2, NN_STATE_RUNNING);

    struct nn_node *tcpserver_node3;
    tcpserver_node3 = node_init(NN_NODE_TYPE_THREAD, 0, tcpserver, dpool);
    router_add_node(router, tcpserver_node3);
    router_add_to_chan(router, chan0, tcpserver_node3);

    node_set_state(tcpserver_node3, NN_STATE_RUNNING);

    while(1){
        usleep(10000000);
        //printf("!!! spin\n");
    }

    return 0;
}

/*
int mainx(int argc, char **argv)
{
    struct nn_router *router;
    struct nn_node *node0;
    struct nn_node *node1;

    router = router_init();

    struct node_thread {
        void *code;
        void *data;
    }

    struct node_remote{
        int port;
    }
    struct node_remote remote_data = {port: 100};

    node_remote_init(remote_data);

    node0 = node_init(NN_NODE_TYPE_THREAD, 0, node_writer, dpool);

    node1 = node_init(NN_NODE_TYPE_NET_LISTENER, 0, node_writer, dpool);
    node2 = node_init(NN_NODE_TYPE_NET_LISTENER, 0, node_writer, dpool);

    node2 = node_init(NN_NODE_TYPE_NET_CONNECTOR, 0, node_writer, dpool);

    chan0 = chan_init(CHAN_BROADCAST | CHAN_BLOCKING);

    node_join(node0, chan0);
    node_join(node1, chan0);

    node_set_state(node0, NN_STATE_RUNNING);
    node_set_state(node1, NN_STATE_RUNNING);

    router_set_state(router, NN_STATE_RUNNING);

    return 0;
}
*/

