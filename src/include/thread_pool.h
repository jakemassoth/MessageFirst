#ifndef MESSAGEFIRST_THREAD_POOL_H
#define MESSAGEFIRST_THREAD_POOL_H
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

typedef void (*thread_func_t)(void*);

typedef struct thread_work {
    thread_func_t func;
    void *args;
    struct thread_work *next;
} thread_work_t;

typedef struct thread_pool {
    thread_work_t *first;
    thread_work_t *last;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_full;
    pthread_cond_t working;
    size_t num_threads;
    size_t num_work;
    bool stop;
} thread_pool_t;

int thread_pool_submit_work(thread_func_t func, void *args, ssize_t len_args, thread_pool_t *thread_pool);

thread_pool_t *thread_pool_init(int num_threads);

void thread_pool_destroy(thread_pool_t *thread_pool);

void thread_pool_wait(thread_pool_t *thread_pool);

#endif //MESSAGEFIRST_THREAD_POOL_H
