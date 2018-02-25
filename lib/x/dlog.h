#ifndef __DLOG_H
#define __DLOG_H

void dlog(int level, const char *filename, int line,
                const char *funcname, const char *format, ...);

#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */

#define DLOG0(level, mesg) \
	dlog(level, __FILE__, __LINE__, __FUNCTION__, mesg) 

#define DLOG1(level, mesg, arg1) \
	dlog(level, __FILE__, __LINE__, __FUNCTION__, mesg, arg1) 

#define DLOG2(level, mesg, arg1, arg2) \
	dlog(level, __FILE__, __LINE__, __FUNCTION__, mesg, arg1, arg2) 

#define DLOG3(level, mesg, arg1, arg2, arg3) \
	dlog(level, __FILE__, __LINE__, __FUNCTION__, mesg, arg1, arg2, arg3) 

#define DLOG4(level, mesg, arg1, arg2, arg3, arg4) \
	dlog(level, __FILE__, __LINE__, __FUNCTION__, mesg, arg1, arg2, arg3, arg4) 

#endif

