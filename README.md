
sudo apt-get install -y \
  autoconf automake build-essential git libtool \
    libsqlite3-dev

1) Build

2) Basic Idea

3) Basic architecture

4) Code layout

5) Usage


## 1) Build

Build with,

    autoreconf --install --force
    ./configure
    make

There are test programs in the lib/* directories. The main test for the node
net is lib/node_net/nn_test2.


## 2) Basic Idea

Very thin layer to abstract communication between different programs/components
local and remote. Originally I thought of having a management computer with a
lot of avr chips on the board communicating over i2c but it is very general and
can be used for example for:

- Distributed computing(clusters etc.)
- Software pipelines
- It can simulates a computer network in software that could be taken
further.
- Anywhere where different systems have to communicate with each other.


## Basic architecture

There are a few basic parts to the system which makes it possible
for the user of the system to create code nodes which could be
threads/remote processes etc. and then communicate between them.

1) Essential parts:

a) Router: This gets request from nodes and routes to the appropriate
node/s. One could replace the router functionality with your own
callbacks.

    files - lib/node_net/router_io.c, lib/node_net/router.c


b) Node: For each node there is a thread that runs that handles the io for
it.  From this thread it gets passed to the specific node_driver which
then handles communication with the user part.

    files - lib/node_net/node_io.c, lib/node_net/node.c


c) Connection: A node and router never communicates directly but go
through a connection.  This is done so one thread never has to lock a
router and node simultaneously which will cause a deadlock. The router
always locks, router and then conn and does it job while the node always
locks the node and conn and does it job.  For example when a node gets
free'd it locks itself and the connection, sets a flag in the connection
but doesn't clean it up, only itself. Next time the router connects to the
connection it cleans it up.  The connection also contains all the buffers
for the connection.

    files - lib/node_net/conn.c, lib/node_net/conn_io.c(not yet)


d) Groups: Nodes can belong to multiple groups and according to that the
router will then route. A node can for example request to route a buffer
to a node in a specific group.

    files - lib/node_net/grp.c


e) Node drivers: For each kind of communication to a system outside
one have to implement a driver that passes the buffers to and from
them. This will be different for different types of nodes. eg. thread or
some socket communication of a legacy system.

    files - lib/node_net/node_drivers/*

2) Optional parts:

Command: to send commands between nodes. This can optionally be used
but a different protocol can also be used.

    files - lib/node_net/pkt.c

3) lib/util usage:

Throughout the system lib/util/que.c and lib/util/ll.c(linked list) gets
used.  For example, each router have a linked list of pointers to all it's
connections,  each connection have que's of pointers to the buffers that
come through them.  etc. These structures can be seen in the respective
c files. All the buffers get passed by pointers for speed.


## Code layout

A lot of abbreviations is used in the code.

    n -> node
    rt -> router
    g -> group
    cn -> connection
    pkt -> pkt
    h -> local handle


Files:

#### lib/util

    - this contains useful util functions, like que(queue), ll(linked
    list), dpool(data pool), bitmap etc. This can be used separately for
    different projects and not dependent on anything else here.

#### lib/util/tests

    - basic test for the utils

#### lib/sys

    - functions to override for different systems, for now it just wraps
    libpthread so one can put another threading system under it.

#### lib/wrap

    - wrap some system functions for debugging(malloc, calloc etc.)
    keeping the stdlib interface. These should be able to be included in
    any other file where the headers gets used to debug other programs.

#### lib/node_net

- this is the main library keeping track of the net layout and doing
all the work of transferring the buffers between nodes' and routers.
It uses the libraries mentioned above.

- there are a few layers to this:

a) the data objects that gets used for storage and retrieval

- lib/node_net/router.c
- lib/node_net/node.c
- lib/node_net/link.c
- lib/node_net/grp.c
- lib/node_net/pkt.c

b) layer that take care of the io between the nodes/routers.

- lib/node_net/router_io.c
- lib/node_net/node_io.c

c) public api

- lib/node_net/nn.c

d) node drivers that is in the directory node_drivers and do the
interfacing to the external world by copying/passing buffers between
node_io and the node implementation. There is only a thread
implementation at the moment. I plan a improve ment to this so 
the drivers run in separate processes and not influence the rest
of the system if there is bugs.

- lib/node_net/node_drivers/thread.c


## 5) Usage

Basic api usage can be seen in the file lib/node_net/nn_test.c. It uses the
public api in lib/node_net/nn.h to create nodes/router/groups and connect them
up.  From this thread command buffers can be sent to nodes and routers directly
but usually it will be handled from nodes. The user must only use the functions
in nn.h and pkt.h. The other files is internal.

For each initiated router a thread gets created(lib/node_net/router_io.c), for
each initiated node a thread gets created(lib/node_net/node_io.c) and for each
connection between a node and router, a connection object gets created. The
other files just create api's so router_io.c, node_io.c and the public api. nn.c
can do it's work.

The user don't have to worry about any locking.


