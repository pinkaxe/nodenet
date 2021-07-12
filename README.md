
* [Introduction](#introduction)

* [Build](#build)

* [Architecture and code](#architecture-and-code)

* [External libraries](#external-libraries)

* [API usage](#api-usage)


## Introduction

This is infrastruture to facilitate communication between different
programs/components. The implemenation is in c but the end-points could be
written in other languages if the proper wrappers are implemented.  It doesn't
assume a specific protocol for the communication and is very general and can be
used anywhere where different pieces of code have to communicate with each
other.

There are very few dependencies so it could be used on systems with limited
resources. Main dependency is on https://github.com/pinkaxe/cragbag.git which
contains a a few c helper libraries that doesn't have external dependencies.


## Build

Install dependencies for building, eg. for Debian/Ubuntu,

    sudo apt-get install -y autoconf automake build-essential libtool

Get the submodules it depends on,

    git submodule update --init

Then build everything with,

    autoreconf --install --force && \
    ./configure --prefix=$PWD/out && \
    make -j$(nproc) && \
    make install

This should build and install the libraries in out/lib and test programs in
out/bin. The main test is lib/nodenet/nn_test2.


## Architecture and code

The code is located in lib/nodenet.

Abbreviations are used in the code,

    n -> node
    rt -> router
    chan -> channel
    pkt -> pkt
    h -> local handle

A short discussion of the parts and files follows,

a) Router: This gets request from nodes and routes to the appropriate
node/s. One could replace the router functionality with your own
callbacks to implement different architectures.

    lib/nodenet/router.c

b) Node: For each node there is a node driver that then handles communication
with the user part.

    lib/nodenet/node.c
    lib/nodenet/node_drivers/node_driver.h

c) Channnel: Nodes and routers communicates through a channel.  This is
done so one thread never has to lock a router and node simultaneously which will
cause a deadlock. The router always locks, router and then channel and does its
job while the node always locks the node and channel and does it job.  For example
when a node gets free'd it locks itself and the channel, sets a flag in the
channel but doesn't clean it up, only itself. Next time the router connects
to the channel it cleans it up. The channel also contains all the buffers
for the channel.

    lib/nodenet/channel.c


d) Packet: packet that holds content sent between nodes and routers. This can
optionally be used but a different protocol can also be used.

    lib/nodenet/pkt.c

## External libraries

https://github.com/pinkaxe/cragbag.git

Throughout the system this library gets used for for things like queues and
linked lists etc. See lib/cragbag/que/que.c and lib/cragbag/ll/ll.c.  Another
part that is often used is for data pools, see lib/cragbag/dpool/dpool.c. There
are tests in the same directories that shows the example usage.


## API usage

Basic API usage can be seen in the file lib/nodenet/nn_test.c.  This is all
done in one process but usually the nodes and routers would be running in
different processes and devices. The user doesn't have to worry about any
locking. This API could be used to build higher level API that applications can
use.
