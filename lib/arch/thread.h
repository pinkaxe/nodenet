#ifndef __THREAD_H__
#define __THREAD_H__

#include<pthread.h>

typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;	
typedef	pthread_cond_t cond_t;	


#ifdef NDEBUG

#define dbg_thread(call) call

#else

#define dbg_thread(call) \
{ \
	int ret; \
	if((ret=(call)) != 0){ \
        printf("cunt\n"); \
		log4(LERR, "%s:%d:%s Error: %s\n", __FILE__, __LINE__, \
			#call, strerror(ret)); \
	} \
}

#endif

#define mutex_init(mutex, data) \
	dbg_thread(pthread_mutex_init(mutex, data))

#define cond_init(cond, data) \
	dbg_thread(pthread_cond_init(cond, data))

#define thread_create(tid, data, func, arg) \
	dbg_thread(pthread_create(tid, data, func, arg))

#define thread_cancel(tid) \
	pthread_cancel(tid)	

#define mutex_destroy(mutex) \
	pthread_mutex_destroy(mutex) 

#define	cond_destroy(cond) \
	dbg_thread(pthread_cond_destroy(cond))

#define thread_detach() \
	dbg_thread(pthread_detach(pthread_self()))

#define mutex_lock(mutex) \
	dbg_thread(pthread_mutex_lock(mutex))

#define mutex_unlock(mutex) \
	dbg_thread(pthread_mutex_unlock(mutex))

#define cond_wait(cond, mutex) \
	dbg_thread(pthread_cond_wait(cond, mutex))

#define cond_signal(cond) \
	dbg_thread(pthread_cond_signal(cond))	

#define thread_testcancel() \
 	pthread_testcancel() /* returns void */



#endif
