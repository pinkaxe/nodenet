#ifndef __LOG_H__
#define __LOG_H__

void dlog(int level, const char *filename, int line,
                const char *funcname, const char *format, ...);

#define	emerg	0	/* system is unusable */
#define	alert	1	/* action must be taken immediately */
#define	crit	2	/* critical conditions */
#define	err	3	/* error conditions */
#define	warn    4	/* warning conditions */
#define	notice  5	/* normal but significant condition */
#define	info    6	/* informational */
#define	debug   7	/* debug-level messages */

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

#endif

