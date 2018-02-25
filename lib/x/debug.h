#ifndef __DEBUG_H
#define __DEBUG_H

#define ASSERT(call) \
	if(!(call)){ \
		DLOG3(LOG_ERR, "%s:%d:%s Assertion failed\n", \
			__FILE__, __LINE__, #call); \
	}

#define NULL_TEST(call) \
	if((call) == NULL){ \
		DLOG4(LOG_ERR, "%s:%d:%s Error: %s\n", \
			__FILE__, __LINE__, #call, strerror(errno)); \
	}

#define PTHREAD_TEST(call) \
	{ \
	int ret; \
	if((ret=call) != 0){ \
		DLOG4(LOG_ERR, "%s:%d:%s Error: %s\n", \
			__FILE__, __LINE__, #call, strerror(ret)); \
	} \
	}

#define SOCK_TEST(call) \
	if((call) == -1){ \
		DLOG4(LOG_ERR, "%s:%d:%s Error: %s\n", \
			__FILE__, __LINE__, #call, strerror(errno)); \
	}


#endif

