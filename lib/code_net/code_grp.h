#ifndef __CODE_GRP_H__
#define __CODE_GRP_H__

struct code_grp {
    struct ll *memb;
    int memb_no;
};

struct code_grp *code_grp_init(void);
void code_grp_free(struct code_grp *code_grp);

int code_grp_add_memb(struct code_grp grp, void *memb);
int code_grp_rem_memb(struct code_grp grp, void *memb);

struct code_grp *code_grp_init()
{
    ll_create();
}

void code_grp_free(struct code_grp *code_grp)
{
}

int code_grp_add_memb(struct code_grp grp, void *memb)
{
    ll_add();
}

int code_grp_rem_memb(struct code_grp grp, void *memb)
{
    ll_rem();
}

#endif
