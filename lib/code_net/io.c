
int cn_io_write_buf(struct cn_net *net, struct ce_elem *from,
        void *buf, struct code_buf_prop *prop, int len);
int cn_io_write_ctrl(struct cn_net *net, struct ce_elem *from,
        void *buf, struct code_buf_prop *prop, int len);