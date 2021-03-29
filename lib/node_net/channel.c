
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include "log/log.h"
#include "que/que.h"
#include "ll/ll.h"

#include "channel.h"

struct chan_nodes_iter;

/*  */
struct nn_chan {
    int id;
    struct ll *nodes; /* all nodes in this chan, for router output */
};

/* each chan have a ll of nodes's it is nodesected to */
#define CHAN_NODES_ITER_PRE(chan) \
    { \
    assert(chan); \
    int done = 0; \
    struct chan_nodes_iter *iter = NULL; \
    struct nn_node *n; \
    iter = chan_nodes_iter_init(chan); \
    while(!done && !chan_nodes_iter_next(iter, &n)){ \

#define CHAN_NODES_ITER_POST(chan) \
    } \
    chan_nodes_iter_free(iter); \
    }

struct nn_chan *channel_init(void)
{
    int r;
    struct nn_chan *chan = NULL;

    PCHK(LWARN, chan, calloc(1, sizeof(*chan)));
    if(!chan){
        goto err;
    }
    //chan->id = id;

    PCHK(LWARN, chan->nodes, ll_init());
    if(!chan->nodes){
        PCHK(LWARN, r, channel_free(chan));
        chan = NULL;
        goto err;
    }

err:
    return chan;

}

int channel_free(struct nn_chan *chan)
{
    free(chan);
    return 0;
}

