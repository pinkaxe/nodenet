
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "log/log.h"

#include "nodenet/types.h"

#include "nodenet/router.h"
#include "nodenet/node.h"


void *node_writer(struct nn_node *n, void *pdata)
{
    L(LDEBUG, "Enter node_writer\n");

    struct nn_chan *chan0 = pdata;
    enum nn_state state;

    for(;;){

        usleep(1);
/*
        L(LDEBUG, "before tx");
        node_wait(n, NN_TX_READY);
        L(LDEBUG, "tx");
*/

        /* check state */
        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            // cleanup if need to
            node_set_state(n, NN_STATE_DONE);
            return NULL;
        }

        chan_add_data(chan0, n, "okokjack", strlen("okokjack") + 1);

    }

    return NULL;
}

void *node_reader(struct nn_node *n, void *pdata)
{
    printf("Enter node_reader\n");
    struct nn_chan *chan0 = pdata;
    enum nn_state state;

    node_block_rx(n, false);

    for(;;){
        usleep(1);

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

        // get incoming on channel
        char *data = chan_get_data(chan0, n);
        if(data){
            L(LNOTICE, "!!!!! Got buf=%s", data);
            free(data);
        }else{
            //L(LDEBUG, "nothing");
        }

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
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

static int g_port = 4848;

void *tcpserver(struct nn_node *n, void *pdata)
{
    struct nn_chan *chan0 = pdata;
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

            // unblock rx when we get a connection
            node_block_rx(n, false);

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
                    chan_add_data(chan0, n, buf, strlen(buf));
                }else{
                    //printf("nothing\n");
                }

                // get incoming on channel and write to socket
                char *data = chan_get_data(chan0, n);
                if(data){
                    c = write(cfd, data, strlen(data));
                    free(data);
                    //EAGAIN
                }else{
                    //L(LDEBUG, "nothing");
                }
                usleep(1);
            }
        }

    }

    return NULL;
}


int main(int argc, char **argv)
{
    struct nn_router *router;
    router = router_init();

    struct nn_chan *chan0;
    chan0 = chan_init(1024 * 1024); // zero-copy buffered ...
    router_add_chan(router, chan0);
    router_set_state(router, NN_STATE_RUNNING);

    //router_add_chan(router, 1);
    //router_add_chan(router, 2);

    struct nn_node *node0;
    node0 = node_init(NN_NODE_TYPE_THREAD, 0, node_writer, chan0);
    router_add_node(router, node0);
    router_add_to_chan(router, chan0, node0);
    node_set_state(node0, NN_STATE_RUNNING);

    struct nn_node *node1;
    node1 = node_init(NN_NODE_TYPE_THREAD, 0, node_reader, chan0);
    router_add_node(router, node1);
    router_add_to_chan(router, chan0, node1);
    node_set_state(node1, NN_STATE_RUNNING);

    struct nn_node *tcpserver_node;
    tcpserver_node = node_init(NN_NODE_TYPE_THREAD, 0, tcpserver, chan0);
    router_add_node(router, tcpserver_node);
    router_add_to_chan(router, chan0, tcpserver_node);
    node_set_state(tcpserver_node, NN_STATE_RUNNING);

    struct nn_node *tcpserver_node2;
    tcpserver_node2 = node_init(NN_NODE_TYPE_THREAD, 0, tcpserver, chan0);
    router_add_node(router, tcpserver_node2);
    router_add_to_chan(router, chan0, tcpserver_node2);
    node_set_state(tcpserver_node2, NN_STATE_RUNNING);

    struct nn_node *tcpserver_node3;
    tcpserver_node3 = node_init(NN_NODE_TYPE_THREAD, 0, tcpserver, chan0);
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
int main(int argc, char **argv)
{
    chan = nn_channel_init(NN_CHANNEL_BLOCKING);

    node0 = nn_node_init(NN_NODE_TYPE_SERVER, NULL, NULL);
    node1 = nn_node_init(NN_NODE_TYPE_SERVER, NULL, NULL);

    nn_node_join(node0, chan);
    nn_node_join(node1, chan);

    return 0;
}
*/

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

