#include "include/messagefirst.h"

void mf_error_print(mf_error_t e) {
    fprintf(stderr, "MessageFirst Error: %s\n", mf_error_str(e));
}

const char *mf_error_str(mf_error_t e) {
    const char *MF_ERROR_STRS[] = {
            "MF_ERROR_OK",
            "MF_ERROR_RECV_LEN",
            "MF_ERROR_SEND",
            "MF_ERROR_RECV_MSG",
            "MF_ERROR_SOCKET_CLOSED",
            "MF_ERROR_EPOLL_CTL",
            "MF_ERROR_NONBLOCKING"
    };

    const char *str = NULL;

    if (e >=  MF_ERROR_COUNT) return str;

    str = MF_ERROR_STRS[e];
    return str;
}
