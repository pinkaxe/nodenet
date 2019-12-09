
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<assert.h>
#include<pthread.h>

#include "dlog.h"
#include "debug.h"
#include "thread_pool.h"

struct thread {
	int num;
	pthread_t tid; /* thread id */
	bool busy;
	void *(*func)(void *arg);
	void *data;
	pthread_mutex_t mutex;	
	pthread_cond_t cond;	
};

struct thread_pool {
	int num;
	struct thread **threads;
	pthread_mutex_t mutex;
};


static void *worker(void *data);

static void *worker(void *data)
{
	struct thread *threadp;
	
	threadp = data;
	
	PTHREAD_TEST(pthread_detach(pthread_self())); 
	for(;;){
		PTHREAD_TEST(pthread_mutex_lock(&threadp->mutex));
		while(!threadp->busy){
			/* wait for work */
			PTHREAD_TEST(pthread_cond_wait(&threadp->cond, &threadp->mutex));
		}
		PTHREAD_TEST(pthread_mutex_unlock(&threadp->mutex));
			
		/* do work */
		DLOG1(LOG_INFO, "Server thread %d woken up\n", threadp->num);
		threadp->func(threadp->data);

		pthread_testcancel();
			
		PTHREAD_TEST(pthread_mutex_lock(&threadp->mutex));
		threadp->busy = false;
		PTHREAD_TEST(pthread_mutex_unlock(&threadp->mutex));
	}
}

struct thread_pool *thread_pool_init(int num, void *(*func)(void *arg))
{	
	int i;
	struct thread_pool *poolh;
	struct thread *threadp;

	NULL_TEST(poolh = malloc(sizeof *poolh));
	poolh->num = num;
	PTHREAD_TEST(pthread_mutex_init(&poolh->mutex, NULL));

	NULL_TEST(poolh->threads = malloc(sizeof *poolh->threads * num));

	for(i=0; i < num; i++){
		NULL_TEST(poolh->threads[i] = calloc(1, sizeof **poolh->threads));
		threadp = poolh->threads[i]; 
		threadp->num = i;
		PTHREAD_TEST(pthread_create(&threadp->tid, NULL, worker, threadp));
		PTHREAD_TEST(pthread_mutex_init(&threadp->mutex, NULL));
		PTHREAD_TEST(pthread_cond_init(&threadp->cond, NULL));
		threadp->busy = false;
		threadp->func = func;
	}
	return poolh; 
}


int thread_pool_free(struct thread_pool *poolh)
{
	int i;
	struct thread *threadp;

	ASSERT(poolh);

	for(i=0; i < poolh->num; i++){
		threadp = poolh->threads[i]; 
		DLOG1(LOG_INFO, "Cancelling server thread: %d", 
			(int)threadp->tid);
        	PTHREAD_TEST(pthread_cancel(threadp->tid));	
		PTHREAD_TEST(pthread_mutex_destroy(&threadp->mutex));
		PTHREAD_TEST(pthread_cond_destroy(&threadp->cond));
		free(threadp);
		threadp = NULL;
	}
	PTHREAD_TEST(pthread_mutex_destroy(&poolh->mutex));
	free(poolh->threads);
	poolh->threads = NULL;
	free(poolh);
	poolh = NULL;

	return 0;
}

int thread_pool_add_work(struct thread_pool *poolh, void *data,
	void (*res_func)(void *arg, bool succ))
{
	int i;
	int ret = 1;
	struct thread *threadp;

	ASSERT(poolh);

	PTHREAD_TEST(pthread_mutex_lock(&poolh->mutex));
	for(i=0; i < poolh->num; i++){
		threadp = poolh->threads[i];
		PTHREAD_TEST(pthread_mutex_lock(&threadp->mutex));
		if(!threadp->busy){			
			if(res_func) res_func(data, true);			
			threadp->busy = true;
			threadp->data = data;
			DLOG1(LOG_INFO, "Adding work for thread: %d\n", 
				threadp->num);
			PTHREAD_TEST(pthread_cond_signal(&threadp->cond));
			PTHREAD_TEST(pthread_mutex_unlock(&threadp->mutex));
			ret = 0;
			break;						
		}
		PTHREAD_TEST(pthread_mutex_unlock(&threadp->mutex));
	}	
	PTHREAD_TEST(pthread_mutex_unlock(&poolh->mutex));
	if(i == poolh->num){
		if(res_func) res_func(data, false);			
	}
	return ret;
}

