
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<assert.h>
#include<pthread.h>

#include "log/log.h"
#include "debug/debug.h"
#include "async_runner.h"

struct thread {
	int num;
	pthread_t tid; /* thread id */
	int (*func)(void *arg);
	void (*res_func)(void *arg, bool success);
	void *data;
	bool busy;
	bool destroy;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

struct async_runner {
	int num;
	struct thread **threads;
	pthread_mutex_t mutex;
};


/* this is a single thread waiting for work(functions to call) */
static void *worker(void *data)
{
	struct thread *threadp = data;

	//PTHREAD_TEST(pthread_detach(pthread_self())); 
	for(;;){
		PTHREAD_TEST(pthread_mutex_lock(&threadp->mutex));
		while(!threadp->busy && !threadp->destroy){
			/* wait for work */
			PTHREAD_TEST(pthread_cond_wait(&threadp->cond, &threadp->mutex));
		}
		PTHREAD_TEST(pthread_mutex_unlock(&threadp->mutex));

        if (threadp->destroy) {
            log1(LINFO, "Destroying thread %d\n", threadp->num);
            return NULL;
        }

		/* call users function */
		log1(LINFO, "Server thread %d woken up\n", threadp->num);
		bool res = threadp->func(threadp->data);

		/* return result */
        if(threadp->res_func)
            threadp->res_func(threadp->data, res);

		pthread_testcancel();

		PTHREAD_TEST(pthread_mutex_lock(&threadp->mutex));
		threadp->busy = false;
		threadp->func = NULL;
		threadp->res_func = NULL;
		PTHREAD_TEST(pthread_mutex_unlock(&threadp->mutex));
	}
}

struct async_runner *async_runner_init(int num)
{
	int i;
	struct async_runner *h;
	struct thread *threadp;

	NULL_TEST(h = malloc(sizeof *h));
	h->num = num;
	PTHREAD_TEST(pthread_mutex_init(&h->mutex, NULL));

	NULL_TEST(h->threads = malloc(sizeof *h->threads * num));

	for(i=0; i < num; i++){
		NULL_TEST(h->threads[i] = calloc(1, sizeof **h->threads));
		threadp = h->threads[i]; 
		threadp->num = i;
        threadp->destroy = false;
		threadp->busy = false;
		PTHREAD_TEST(pthread_create(&threadp->tid, NULL, worker, threadp));
		PTHREAD_TEST(pthread_mutex_init(&threadp->mutex, NULL));
		PTHREAD_TEST(pthread_cond_init(&threadp->cond, NULL));
	}
	return h; 
}


int async_runner_free(struct async_runner *h)
{
	int i;
	struct thread *threadp;

	ASSERT(h);

    PTHREAD_TEST(pthread_mutex_lock(&h->mutex));

	for(i=0; i < h->num; i++){
		threadp = h->threads[i];
		log1(LINFO, "Cancelling server thread: %d", 
			(int)threadp->tid);
	    //PTHREAD_TEST(pthread_cancel(threadp->tid));	
	    PTHREAD_TEST(pthread_mutex_lock(&threadp->mutex));
        threadp->destroy = true;
        PTHREAD_TEST(pthread_cond_signal(&threadp->cond));
	    PTHREAD_TEST(pthread_mutex_unlock(&threadp->mutex));
    }

	for(i=0; i < h->num; i++){
		threadp = h->threads[i];
		PTHREAD_TEST(pthread_join(threadp->tid, NULL));
		PTHREAD_TEST(pthread_mutex_destroy(&threadp->mutex));
		PTHREAD_TEST(pthread_cond_destroy(&threadp->cond));
		free(threadp);
		threadp = NULL;
	}
	free(h->threads);
	h->threads = NULL;
    PTHREAD_TEST(pthread_mutex_unlock(&h->mutex));
	PTHREAD_TEST(pthread_mutex_destroy(&h->mutex));

	free(h);
	h = NULL;

	return 0;
}

int async_runner_exec(struct async_runner *h, void *(*func)(void *arg),
    void *data, void (*res_func)(void *arg, bool succ))
{
	int i;
	int ret = 1;
	struct thread *threadp;

	ASSERT(h);

	PTHREAD_TEST(pthread_mutex_lock(&h->mutex));
	for(i=0; i < h->num; i++){
		threadp = h->threads[i];
		PTHREAD_TEST(pthread_mutex_lock(&threadp->mutex));
		if(!threadp->busy){
			threadp->busy = true;
			threadp->data = data;
                    threadp->func = func;
                    threadp->res_func = res_func;
			log1(LINFO, "Adding work for thread: %d\n", 
				threadp->num);
			PTHREAD_TEST(pthread_cond_signal(&threadp->cond));
			PTHREAD_TEST(pthread_mutex_unlock(&threadp->mutex));
			ret = 0;
			break;
		}
		PTHREAD_TEST(pthread_mutex_unlock(&threadp->mutex));
	}
	PTHREAD_TEST(pthread_mutex_unlock(&h->mutex));
	return ret;
}


