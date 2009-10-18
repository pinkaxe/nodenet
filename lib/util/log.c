
#include<syslog.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdarg.h>

#include "log.h"

/* define vsyslog for ansi, IMPROVE: for serious ansi compilers */
void vsyslog(int priority, const char *format, va_list ap);

void dlog(int level, const char *filename, int line,
                const char *funcname, const char *format, ...)
{
    va_list args;

    openlog("x", LOG_PERROR | LOG_NDELAY | LOG_PID, LOG_USER);
    setlogmask(LOG_UPTO(LOG_INFO));

    va_start(args, format);
    level = LOG_WARNING;
    //syslog(level, "%s:%d:%s->", filename, line, funcname);
    //vsyslog(level, format, args);

	if(level <= LOG_ERR){
		printf("Error: |");
        printf("%s:%d:%s->", filename, line, funcname);
		vprintf(format, args);
		puts("|");
	    va_end(args);
		exit(EXIT_FAILURE);
	}

    va_end(args);

}


