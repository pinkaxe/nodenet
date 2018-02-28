
#ifndef UTIL_DBG_H__
#define UTIL_DBG_H__

#define _DEBUG_
#ifdef _DEBUG_

#define DBG_STRUCT_START unsigned short dbg_struct_start_magic;
#define DBG_STRUCT_END unsigned short dbg_struct_end_magic;

#define DBG_STRUCT_INIT(s) \
    s->dbg_struct_start_magic = 0x1839; \
    s->dbg_struct_end_magic = 0x8742;

#define DBG_STRUCT_ISOK(s) \
    assert(s->dbg_struct_start_magic == 0x1839); \
    assert(s->dbg_struct_end_magic == 0x8742);

#else

#define DBG_STRUCT_START
#define DBG_STRUCT_END
#define DBG_STRUCT_INIT(s)
#define DBG_STRUCT_ISOK(s)

#endif
#endif // UTIL_DBG_H__
