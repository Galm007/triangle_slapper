#ifndef __THREADSYNC_H__
#define __THREADSYNC_H__

#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct ThreadSync ThreadSync;

typedef struct {
	ThreadSync* ts;
	void (*task)(void*, int);
	void* task_arg;
	int thread_index;
} TaskRunnerData;

struct ThreadSync {
	sem_t dispatch_sem, finished_sem;
	pthread_t* task_threads;
	TaskRunnerData* runner_data;
	int thread_count;
	bool destroy, finished;
};

bool threadsync_init(
	ThreadSync* ts,
	void (*task)(void*, int),
	void* task_arg,
	int threads
);
bool threadsync_destroy(ThreadSync* ts);
void threadsync_dispatch(ThreadSync* ts);
void threadsync_wait(ThreadSync* ts);

#endif
