
/* a simple test program for node_net, for now 
 * create nodes/routers/grps, change their states, send pkt's to them, 
 * and cleanup in loop. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#include "util/log.h"
#include "util/dpool.h"

#include "node_net/types.h"
#include "node_net/pkt.h"

#include "node_net/router.h"
#include "node_net/node.h"
#include "node_net/conn.h"
#include "node_net/grp.h"

#define ok(x){ \
    assert(x); \
}

struct buf {
    int i;
};


static int dpool_free_cb(void *dpool, void *dpool_buf)
{
    printf("!!!!!!!!!!dpool_free_cb\n");
    return dpool_ret_buf(dpool, dpool_buf);
}

void *thread1(struct nn_node *n, void *pdata)
{
    int i = 0;
    struct dpool *dpool = pdata;
    struct dpool_buf *dpool_buf;
    struct buf *buf;
    struct nn_pkt *pkt;
    enum nn_state state;

#if 0
        /* build internal packet */
    for(;;){
        pkt = pkt_init(n, 1, NN_SENDTO_ALL, 0);
        if(pkt){
           // buf = pkt->data
            b = pkt_get_data(pkt);
            b->i = i++;

            /* add to outgoing tx buffers */
            while(nn_node_add_tx_pkt(n, pkt)){
                usleep(10000);
            }
        }
    }

    // send data
    send_data(n, buf, 1, NN_SENDTO_ALL, 0);
#endif
    //node_set_refcnt_cb(struct nn_node *n, void (*cb)(struct nn_node *n, int));


    for(;;){

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

            while(node_tx(n, dpool_buf, sizeof(*dpool_buf), dpool, 1,
                        NN_SENDTO_ALL, 0, dpool_free_cb)){
                usleep(10000);
            }
        }
        usleep(100000);

        /*
        while(1){
            node_rx(n, &dpool_buf, &len, &pdata);

            buf = dpool_buf->data;
            L(LINFO, "Got buf->i=%d", buf->i);

            dpool_ret_buf(dpool, dpool_buf);
        }
        */

        /* receive packets */
        while(!node_get_rx_pkt(n, &pkt)){
            struct dpool_buf *dpool_buf = pkt_get_data(pkt);
            buf = dpool_buf->data;
            L(LINFO, "Got buf->i=%d", buf->i);
            /* when done call pkt_free */
            //dpool_ret_buf(dpool, dpool_buf);
            pkt_free(pkt);
        }
        //usleep(10000);
        sched_yield();
    }

    return NULL;
}

void *thread0(struct nn_node *n, void *pdata)
{
    struct dpool *dpool = pdata;
    struct nn_pkt *pkt;
    struct buf *buf;
    enum nn_state state;

    for(;;){
        /* check state */
        state = node_do_state(n);
        if(state == NN_STATE_SHUTDOWN){
            // cleanup if need to
            return NULL;
        }

        while(!node_get_rx_pkt(n, &pkt)){
            /* incoming packet */
            struct dpool_buf *dpool_buf = pkt_get_data(pkt);
            buf = dpool_buf->data;
            L(LINFO, "Got buf->i=%d", buf->i);
            /* when done call pkt_free */
            //dpool_ret_buf(dpool, dpool_buf);
            pkt_free(pkt);
        }

        sched_yield();
    }

    return NULL;
}

#define GRPS_NO 3

int main(int argc, char **argv)
{
    int i;
    struct nn_router *rt[1];
    struct nn_node *n[1024];
    struct nn_grp *g[GRPS_NO];
    //struct nn_pkt *pkt;
    struct dpool *dpool;
    struct buf *buf;
    //struct dpool_buf *dpool_buf;

    while(1){

        dpool = dpool_create(sizeof(*buf), 100, 0);

        rt[0] = router_init();
        ok(rt[0]);
        abort();

        for(i=0; i < GRPS_NO; i++){
            g[i] = grp_init(i);
            ok(g[i]);
        }

        /* create input nodes */
        n[0] = node_init(NN_NODE_TYPE_THREAD, NN_NODE_ATTR_NO_INPUT,
                thread1, dpool);
        conn_conn(n[0], rt[0]);
        n[1] = node_init(NN_NODE_TYPE_THREAD, NN_NODE_ATTR_NO_INPUT,
                thread0, dpool);
        conn_conn(n[1], rt[0]);
       // for(i=1; i < 1; i++){
       //     n[i] = nn_node_init(NN_NODE_TYPE_THREAD, NN_NODE_ATTR_NO_INPUT,
       //             thread0, dpool);
       //     /* connect to router */
       //     conn_conn(n[i], rt[0]);
       //     //nn_join_grp(n[i], g[1]);
       //     ok(n[i]);
       // }

        /* set everthing state to running */
        for(i=0; i < 2; i++){
            node_set_state(n[i], NN_STATE_RUNNING);
        }
        router_set_state(rt[0], NN_STATE_RUNNING);

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
            //sleep(2);
        //}
            sleep(3);

        /* pause everything */
        for(i=0; i < 2; i++){
            node_set_state(n[i], NN_STATE_PAUSED);
        }
        router_set_state(rt[0], NN_STATE_PAUSED);

        /* run everything */
        for(i=0; i < 2; i++){
            node_set_state(n[i], NN_STATE_RUNNING);
        }
        router_set_state(rt[0], NN_STATE_RUNNING);


        for(i=0; i < 2; i++){
            /* unconn not needed but ok */
            //conn_unconn(n[i], rt[0]);
            node_set_state(n[i], NN_STATE_SHUTDOWN);
        }

        grp_free(g[0]);
        grp_free(g[1]);
        grp_free(g[2]);

        router_set_state(rt[0], NN_STATE_SHUTDOWN);

        for(i=0; i < 2; i++){
            node_clean(n[i]);
        }

        router_clean(rt[0]);

        dpool_free(dpool);

        printf("loop done\n");
        usleep(2000);
    }
    return 0;
}

#if 0
void *worker_thread(struct nn_node *n, void *arg)
{
    struct dpool *dpool = pdata;
    struct dpool_buf *dpool_buf;
    struct buf *buf;
    struct nn_pkt *pkt;

    while(!nn_node_get_rx_pkt(n, &pkt)){
        struct dpool_buf *dpool_buf = pkt_get_data(pkt);
        buf = dpool_buf->data;
        L(LINFO, "Got buf->fd=%d", buf->fd);
        /* when done call pkt_free */
        close(buf->fd);
        dpool_ret_buf(dpool, dpool_buf);
        pkt_free(pkt);
    }
    usleep(10000);
}

void *server_thread(struct nn_node *n, void *arg)
{
	struct server_handle *sh;
	struct   sockaddr_in sin;
	int tr = 1;

	NULL_TEST(sh = malloc(sizeof *sh));
	sh->port = port;
	sh->max = max;
	sh->tpoolh = thread_pool_init(max, _server_worker);
	sh->ph = ph;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	SOCK_TEST(sh->fd = socket(AF_INET, SOCK_STREAM, 0));
	SOCK_TEST(bind(sh->fd, (struct sockaddr *) &sin, sizeof(sin)));
	SOCK_TEST(setsockopt(sh->fd, SOL_SOCKET,SO_REUSEADDR, &tr, sizeof(int)));
	SOCK_TEST(listen(sh->fd, max) == -1);

	//PTHREAD_TEST(pthread_create(&tid, NULL, _server_thread, sh));
    
	struct   sockaddr_in pin;
	int 	 cfd;
	socklen_t addrlen;

	sh = sh_arg;


    addrlen = sizeof(pin); 

	for(;;){
		SOCK_TEST(cfd = accept(sh->fd, 
			(struct sockaddr *)  &pin, &addrlen)); 

		DLOG2(LOG_INFO, "Login from: %s:%d", 
			inet_ntoa(pin.sin_addr),ntohs(pin.sin_port));


        for(;;){
            if((dpool_buf=dpool_get_buf(dpool))){
                /* */
                //buf = dpool_buf_get_datap(dpool_buf);
                buf = dpool_buf->data;
                /* set buf */
                buf->fd = cfd;

                /* build internal packet */
                pkt = pkt_init(n, dpool_buf, sizeof(dpool_buf), dpool, 1,
                        NN_SENDTO_ALL, 0);

                /* add to outgoing tx buffers */
                while(nn_node_add_tx_pkt(n, pkt)){
                    usleep(10000);
                }
            }
            if(thread_pool_add_work(sh->tpoolh, workp, _server_conn_res)){	
                close(workp->fd); 
                free(workp);
                workp = NULL;
            }
	}
	return NULL;

	return sh;	
}


void *server(struct nn_node *n, void *pdata)
{
    int i = 0;
    struct dpool *dpool = pdata;
    struct dpool_buf *dpool_buf;
    struct buf *buf;
    struct nn_pkt *pkt;

    for(;;){
        if((dpool_buf=dpool_get_buf(dpool))){
            /* */
            //buf = dpool_buf_get_datap(dpool_buf);
            buf = dpool_buf->data;
            /* set buf */
            buf->i = i++;

            /* build internal packet */
            pkt = pkt_init(n, dpool_buf, sizeof(dpool_buf), dpool, 1,
                    NN_SENDTO_ALL, 0);

            /* add to outgoing tx buffers */
            while(nn_node_add_tx_pkt(n, pkt)){
                usleep(10000);
            }
        }
        sleep(1);

        while(!nn_node_get_rx_pkt(n, &pkt)){
            struct dpool_buf *dpool_buf = pkt_get_data(pkt);
            buf = dpool_buf->data;
            L(LINFO, "Got buf->i=%d", buf->i);
            /* when done call pkt_free */
            dpool_ret_buf(dpool, dpool_buf);
            pkt_free(pkt);
        }
        usleep(10000);
    }

    return NULL;
}
#endif
