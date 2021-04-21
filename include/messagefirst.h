#ifndef MESSAGEFIRST_MESSAGEFIRST_H
#define MESSAGEFIRST_MESSAGEFIRST_H
#define MAX_DATA_LEN 128
#include <sys/socket.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

struct __attribute__((packed)) mf_msg {
    char data[MAX_DATA_LEN];
    int len;
};

struct __attribute__((packed)) mf_ctx {
    long timeout;
};

typedef enum mf_error_e {
    MF_ERROR_OK = 0,
    MF_ERROR_RECV_LEN = 1,
    MF_ERROR_SEND = 2,
    MF_ERROR_RECV_MSG = 3,

    MF_ERROR_COUNT = 4
} mf_error_t;

const char *mf_error_str(mf_error_t e);

void mf_error_print(mf_error_t e);


#endif //MESSAGEFIRST_MESSAGEFIRST_H
