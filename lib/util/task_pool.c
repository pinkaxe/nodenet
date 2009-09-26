
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<assert.h>

#include "wrap/xstdlib.h"
#include "arch/thread.h"

#include "util/log.h"
#include "util/debug.h"
#include "util/task_pool.h"

enum tstate {
	TSTATE_IDLE,
	TSTATE_RUNNING,
	TSTATE_CANCELLED
};

struct task {
	int num;
	thread_t tid; /* thread id */
	mutex_t mutex;
	cond_t cond;
	bool busy;
	void *(*func)(void *arg);
	void *data;
	enum tstate state;
};

struct task_pool {
	int max;
	struct task **tasks;
	mutex_t mutex; 
	/*uint64_t bitmap[6];*/ 
}; 

static void *worker(void *task); 
static void task_create(struct task *t, int num);
static void task_free(struct task *t); 

static void *worker(void *task)
{
	struct task *t = task;

	thread_detach(thread_self());
	for(;;){
		mutex_lock(&t->mutex);
		while(!t->busy){
			if(t->state == TSTATE_CANCELLED){
				mutex_unlock(&t->mutex);
				t->busy = false;
				t->state = TSTATE_IDLE;
				break;
			}
			/* wait for work */
			cond_wait(&t->cond, &t->mutex);
		}
		mutex_unlock(&t->mutex);

		/* do work */
		log1(LINFO, "Server thread %d woken up\n", t->num);
		(void)t->func(t->data);

		thread_testcancel();

		mutex_lock(&t->mutex);
		t->busy = false;
		mutex_unlock(&t->mutex);
	}
	return NULL;
}

static void task_create(struct task *t, int num)
{
	t->num = num;
	mutex_init(&t->mutex, NULL);
	cond_init(&t->cond, NULL);
	t->busy = false;
	thread_create(&t->tid, NULL, worker, t);
	log1(LINFO, "Created thread: %d", (int)t->tid);
}

static void task_free(struct task *t)
{
	if(!t) return;
	log1(LINFO, "Cancelling thread: %d", (int)t->tid);
	mutex_lock(&t->mutex);
	t->state = TSTATE_CANCELLED;
	mutex_unlock(&t->mutex);

	cond_signal(&t->cond);	
	if(thread_cancel(t->tid) != 0){	
		log1(LINFO, "Couldn't cancel thread, but OK(%s)",
				strerror(errno));
	}
	while(mutex_destroy(&t->mutex) == EBUSY){
		usleep(1);
	}
	cond_destroy(&t->cond);
	free(t);
	t = NULL;
}


struct task_pool *task_pool_init(size_t max)
{		
	int i;
	struct task_pool *tp;
	bool e = false;

	if(!(tp = calloc(1, sizeof *tp))){
		e = true;
		goto error;
	}
	tp->max = (int )max;
	mutex_init(&tp->mutex, NULL);

	if(!(tp->tasks = calloc(max, sizeof *tp->tasks))){
		e = true;
		goto error;
	}

	for(i=0; i < tp->max; i++){ 
		if(!(tp->tasks[i] = malloc(sizeof **tp->tasks))){
			e = true;
			goto error;
		}
		task_create(tp->tasks[i], i);
	}

error:
	if(e){
		task_pool_free(tp);
		tp = NULL;
	}

	return tp; 
}


int task_pool_free(struct task_pool *tp)
{
	int i;

	if(!tp) return 0;

	for(i=0; i < tp->max; i++){
		task_free(tp->tasks[i]);
	}
	mutex_destroy(&tp->mutex);
	free(tp->tasks);
	tp->tasks = NULL;
	free(tp);
	tp = NULL;

	return 0;
}

int task_pool_add(struct task_pool *tp, void *(*func)(void *), void *data)
{
	int i;
	int ret = 1;
	struct task *t;

	ASSERT(tp);

	mutex_lock(&tp->mutex);
	for(i=0; i < tp->max; i++){
		t = tp->tasks[i];
		mutex_lock(&t->mutex);
		if(!t->busy){			
			t->busy = true;
			t->func = func;
			t->data = data;
			log1(LINFO, "Adding work for thread: %d\n", t->num);
			cond_signal(&t->cond);
			mutex_unlock(&t->mutex);
			ret = 0;
			break;						
		}
		mutex_unlock(&t->mutex);
	}	
	mutex_unlock(&tp->mutex);
	return ret;
}


