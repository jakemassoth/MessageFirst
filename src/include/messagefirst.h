#ifndef MESSAGEFIRST_MESSAGEFIRST_H
#define MESSAGEFIRST_MESSAGEFIRST_H
#define MAX_DATA_LEN 16384
#define THREADS 6
#define MAX_EVENTS 32
#include <include/thread_pool.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>


typedef enum mf_error_e {
    MF_ERROR_OK = 0,
    MF_ERROR_RECV_LEN = 1,
    MF_ERROR_SEND = 2,
    MF_ERROR_RECV_MSG = 3,
    MF_ERROR_SOCKET_CLOSED = 4,
    MF_ERROR_EPOLL_CTL = 5,
    MF_ERROR_NONBLOCKING = 6,
    MF_ERROR_NULL_CTX = 7,
    MF_ERROR_NO_THREAD_POOL = 8,
    MF_ERROR_NO_ERROR_CB = 9,
    MF_ERROR_NO_POLL_CB = 10,
    MF_ERROR_NO_TIMEOUT_CB = 11,
    MF_ERROR_NO_RECV_CB = 12,
    MF_ERROR_CLOSE_FAILED = 13,

    MF_ERROR_COUNT = 14
} mf_error_t;

struct __attribute__((packed)) mf_msg {
    char data[MAX_DATA_LEN];
    int len;
};

typedef void (*mf_error_cb_t)(int, struct mf_msg*, mf_error_t err);
typedef struct mf_msg (*mf_poll_resp_cb)(struct mf_msg);
typedef void (*mf_timeout_cb_t)(int, struct mf_msg*);

struct __attribute__((packed)) mf_ctx {
    int timeout;
    mf_error_cb_t error_cb;
    mf_timeout_cb_t timeout_cb;
    mf_poll_resp_cb poll_resp_cb;
    thread_pool_t *tp;
};

const char *mf_error_str(mf_error_t e);

void mf_error_print(mf_error_t e);

#endif //MESSAGEFIRST_MESSAGEFIRST_H
