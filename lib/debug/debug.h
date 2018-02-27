#ifndef UTIL_DEBUG_H__
#define UTIL_DEBUG_H__

#define ASSERT(call) \
	if(!(call)){ \
		log3(LERR, "%s:%d:%s Assertion failed\n", \
			__FILE__, __LINE__, #call); \
	}

#define NULL_TEST(call) \
	if((call) == NULL){ \
		log4(LERR, "%s:%d:%s Error: %s\n", \
			__FILE__, __LINE__, #call, strerror(errno)); \
	}

#define PTHREAD_TEST(call) \
	{ \
	int ret; \
	if((ret=(call)) != 0){ \
		log4(LERR, "%s:%d:%s Error: %s\n", \
			__FILE__, __LINE__, #call, strerror(ret)); \
	} \
	}

#define SOCK_TEST(call) \
	if((call) == -1){ \
		log4(LERR, "%s:%d:%s Error: %s\n", \
			__FILE__, __LINE__, #call, strerror(errno)); \
	}


#endif

