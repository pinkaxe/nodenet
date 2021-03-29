#ifndef UTIL_LOG_H__
#define UTIL_LOG_H__

#include<errno.h>

void dlog(int level, const char *filename, int line,
                const char *funcname, const char *format, ...);

#define	LEMERG	0	/* system is unusable */
#define	LALERT	1	/* action must be taken immediately */
#define	LCRIT	2	/* critical conditions */
#define	LERR	    3	/* error conditions */
#define	LWARN    4	/* warning conditions */
#define	LNOTICE  5	/* normal but significant condition */
#define	LINFO    6	/* informational */
#define	LDEBUG   7	/* debug-level messages */

#define log0(level, mesg) \
	dlog(level, __FILE__, __LINE__, __FUNCTION__, mesg) 

#define log1(level, mesg, arg1) \
	dlog(level, __FILE__, __LINE__, __FUNCTION__, mesg, arg1) 

#define log2(level, mesg, arg1, arg2) \
	dlog(level, __FILE__, __LINE__, __FUNCTION__, mesg, arg1, arg2) 

#define log3(level, mesg, arg1, arg2, arg3) \
	dlog(level, __FILE__, __LINE__, __FUNCTION__, mesg, arg1, arg2, arg3) 

#define log4(level, mesg, arg1, arg2, arg3, arg4) \
	dlog(level, __FILE__, __LINE__, __FUNCTION__, mesg, arg1, arg2, arg3, arg4) 

#define L(level, ...) dlog(level, __FILE__, __LINE__, __FUNCTION__,  __VA_ARGS__)

//#define L(level, ...)

#define ICHK(level, lexp, rexp) \
    if((lexp = (rexp))){ \
        L(level, "%s fail, r=%d(%s)", #rexp, lexp, strerror(lexp)); \
    } 


#define PCHK(level, lexp, rexp) \
    if(!(lexp = (rexp))){ \
        L(level, "%s fail, err=%d(%s)", #rexp, errno, strerror(errno)); \
    }


#endif

