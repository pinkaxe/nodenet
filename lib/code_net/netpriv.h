#ifndef CODE_NET_NETPRIV_H__
#define CODE_NET_NETPRIV_H__

struct cn_net_memb {
    struct cn_elem *memb;
};

struct cn_net {
    struct ll *memb;
    struct que *cmd_req;
    struct que *data_req;
    io_cmd_req_cb_t io_cmd_req_cb;
    io_data_req_cb_t io_data_req_cb;
};

#endif
