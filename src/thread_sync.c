#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include "thread_sync.h"

static void* task_runner(void* arg)
{
	TaskRunnerData* data = arg;

	for (;;)
	{
		sem_wait(&data->ts->dispatch_sem);
		if (data->ts->destroy)
			break;

		data->task(data->task_arg, data->thread_index);
		sem_post(&data->ts->finished_sem);
	}

	return NULL;
}

bool threadsync_init(
	ThreadSync* ts,
	void (*task)(void*, int),
	void* task_arg,
	int threads
) {
	bool success = true;

	ts->thread_count = threads;
	ts->destroy = false;
	ts->finished = true;

	if (sem_init(&ts->dispatch_sem, 0, 0))
	{
		printf("ThreadSync: Failed to initialize dispatch_sem\n");
		success = false;
	}
	if (sem_init(&ts->finished_sem, 0, 0))
	{
		printf("ThreadSync: Failed to initialize finished_sem\n");
		success = false;
	}

	ts->task_threads = malloc(threads * sizeof(pthread_t));
	ts->runner_data = malloc(threads * sizeof(TaskRunnerData));

	for (int i = 0; i < threads; i++)
	{
		ts->runner_data[i] = (TaskRunnerData){
			.ts = ts,
			.task = task,
			.task_arg = task_arg,
			.thread_index = i
		};

		if (pthread_create(
			&ts->task_threads[i],
			NULL,
			&task_runner,
			&ts->runner_data[i]
		)) {
			printf("ThreadSync: Failed to create thread #%d\n", i);
			success = false;
		}
	}

	return success;
}

bool threadsync_destroy(ThreadSync* ts)
{
	bool success = true;

	ts->destroy = true;
	threadsync_dispatch(ts);

	for (int i = 0; i < ts->thread_count; i++)
		if (pthread_join(ts->task_threads[i], NULL))
		{
			printf("ThreadSync: Failed to join thread #%d\n", i);
			success = false;
		}

	free(ts->task_threads);
	free(ts->runner_data);

	if (sem_destroy(&ts->dispatch_sem))
	{
		printf("ThreadSync: Failed to destroy dispatch_sem\n");
		success = false;
	}
	if (sem_destroy(&ts->finished_sem))
	{
		printf("ThreadSync: Failed to destroy finished_sem\n");
		success = false;
	}

	return success;
}

void threadsync_dispatch(ThreadSync* ts)
{
	threadsync_wait(ts);

	ts->finished = false;
	for (int i = 0; i < ts->thread_count; i++)
		sem_post(&ts->dispatch_sem);
}

void threadsync_wait(ThreadSync* ts)
{
	if (ts->finished)
		return;

	for (int i = 0; i < ts->thread_count; i++)
		sem_wait(&ts->finished_sem);

	ts->finished = true;
}
