#ifndef MESSAGEFIRST_MESSAGEFIRST_H
#define MESSAGEFIRST_MESSAGEFIRST_H
#define MAX_DATA_LEN 128

struct __attribute__((packed)) mf_msg {
    char data[MAX_DATA_LEN];
    long expect_resp;
};

struct __attribute__((packed)) mf_ctx {
    long timeout;
};

#endif //MESSAGEFIRST_MESSAGEFIRST_H
