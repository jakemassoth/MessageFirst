#include "include/thread_pool.h"

thread_work_t *work_init(thread_func_t func, void *args, ssize_t len_args) {
    assert(func);
    assert(args);

    thread_work_t *work = (thread_work_t*) malloc(sizeof(thread_work_t));
    if (!work) return NULL;

    work->func = func;
    work->args = malloc(len_args);
    if (!work->args) return NULL;

    memcpy(work->args, args, len_args);

    work->next = NULL;

    return work;
}

void work_destroy(thread_work_t *work) {
    assert(work);
    assert(work->args);
    free(work->args);
    free(work);
}

thread_work_t *thread_pool_get_work(thread_pool_t *thread_pool) {
    thread_work_t *work;

    if (thread_pool == NULL) return NULL;

    work = thread_pool->first;
    if (work == NULL) return NULL;

    if (work->next == NULL) {
        thread_pool->first = NULL;
        thread_pool->first  = NULL;
    }
    else thread_pool->first = work->next;

    return work;
}

void *worker(void *args) {
    thread_pool_t *thread_pool = (thread_pool_t*) args;

    while (1) {
        pthread_mutex_lock(&thread_pool->queue_lock);

        while (thread_pool->first == NULL && !thread_pool->stop)
            pthread_cond_wait(&thread_pool->queue_full, &thread_pool->queue_lock);

        if (thread_pool->stop)
            break;

        thread_work_t *work = thread_pool_get_work(thread_pool);
        thread_pool->num_work++;
        pthread_mutex_unlock(&thread_pool->queue_lock);

        if (work != NULL) {
            work->func(work->args);
            work_destroy(work);
        }

        pthread_mutex_lock(&thread_pool->queue_lock);

        thread_pool->num_work--;

        if (!thread_pool->stop && thread_pool->num_work == 0 && thread_pool->first == NULL)
            pthread_cond_signal(&thread_pool->working);

        pthread_mutex_unlock(&thread_pool->queue_lock);
    }

    thread_pool->num_threads--;
    pthread_cond_signal(&thread_pool->working);
    pthread_mutex_unlock(&thread_pool->queue_lock);
    return NULL;
}


int thread_pool_submit_work(thread_func_t func, void *args, ssize_t len_args, thread_pool_t *thread_pool) {
    if (thread_pool == NULL) return -1;

    thread_work_t *work = work_init(func, args, len_args);
    if (!work) return -1;

    pthread_mutex_lock(&thread_pool->queue_lock);

    if (thread_pool->first == NULL) {
        thread_pool->first = work;
        thread_pool->last = thread_pool->first;
    } else {
        thread_pool->last->next = work;
        thread_pool->last = work;
    }

    pthread_cond_broadcast(&thread_pool->queue_full);
    pthread_mutex_unlock(&thread_pool->queue_lock);

    return 0;
}

thread_pool_t *thread_pool_init(int num_threads) {
    assert(num_threads > 0);

    thread_pool_t *thread_pool = (thread_pool_t*) malloc(sizeof(thread_pool_t));
    assert(thread_pool);

    thread_pool->num_threads = num_threads;

    pthread_mutex_init(&thread_pool->queue_lock, NULL);
    pthread_cond_init(&thread_pool->queue_full, NULL);
    pthread_cond_init(&thread_pool->working, NULL);

    thread_pool->first = NULL;
    thread_pool->last  = NULL;

    pthread_t thread;
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&thread, NULL, worker, thread_pool);
        pthread_detach(thread);
    }

    return thread_pool;
}

void thread_pool_destroy(thread_pool_t *thread_pool) {
    assert(thread_pool);

    pthread_mutex_lock(&thread_pool->queue_lock);
    thread_work_t *work = thread_pool->first;

    thread_work_t *temp;
    while (work != NULL) {
        temp = work->next;
        work_destroy(work);
        work = temp;
    }
    thread_pool->stop = true;
    pthread_cond_broadcast(&thread_pool->queue_full);
    pthread_mutex_unlock(&thread_pool->queue_lock);

    thread_pool_wait(thread_pool);

    pthread_mutex_destroy(&thread_pool->queue_lock);
    pthread_cond_destroy(&thread_pool->queue_full);
    pthread_cond_destroy(&thread_pool->working);

    free(thread_pool);
}

void thread_pool_wait(thread_pool_t *thread_pool) {
    assert(thread_pool);

    pthread_mutex_lock(&thread_pool->queue_lock);
    while (1) {
        if ((!thread_pool->stop && thread_pool->num_work != 0)
            || (thread_pool->stop && thread_pool->num_threads != 0)) {
            pthread_cond_wait(&thread_pool->working, &thread_pool->queue_lock);
        } else break;
    }
    pthread_mutex_unlock(&thread_pool->queue_lock);
}
