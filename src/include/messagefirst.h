#ifndef MESSAGEFIRST_MESSAGEFIRST_H
#define MESSAGEFIRST_MESSAGEFIRST_H
#define MAX_DATA_LEN 128
#include <sys/socket.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum mf_error_e {
    MF_ERROR_OK = 0,
    MF_ERROR_RECV_LEN = 1,
    MF_ERROR_SEND = 2,
    MF_ERROR_RECV_MSG = 3,
    MF_ERROR_SOCKET_CLOSED = 4,

    MF_ERROR_COUNT = 5
} mf_error_t;

struct __attribute__((packed)) mf_msg {
    char data[MAX_DATA_LEN];
    int len;
};

typedef void (*mf_error_cb_t)(int, struct mf_msg*, mf_error_t err);
typedef mf_error_t (*mf_recv_cb)(int, struct mf_msg*);
typedef mf_error_t (*mf_timeout_cb_t)(int, struct mf_msg*);

struct __attribute__((packed)) mf_ctx {
    long timeout;
    mf_error_cb_t error_cb;
    mf_recv_cb recv_cb;
    mf_timeout_cb_t timeout_cb;
};

const char *mf_error_str(mf_error_t e);

void mf_error_print(mf_error_t e);


#endif //MESSAGEFIRST_MESSAGEFIRST_H
