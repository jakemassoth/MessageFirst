#include <include/messagefirst_api.h>
#include <include/util.h>

int receive_size(int socket) {
    int payload_size = 0;

    ssize_t len;
    for (;;) {
        len = read(socket, &payload_size, sizeof(payload_size));
        if (len < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) continue;
        else if (len <= 0) {
            DEBUG_PRINT("recv() error: %s\n", strerror(errno));
            return len;
        } else break;
    }

    assert(len == sizeof(payload_size));

    return payload_size;
}

mf_error_t receive_variable_size(int socket, int expected, char dest[MAX_DATA_LEN]) {
    ssize_t so_far = 0;

    char p_buff[MAX_DATA_LEN];
    char buff[MAX_DATA_LEN];
    memset(buff, 0, MAX_DATA_LEN);
    memset(p_buff, 0, MAX_DATA_LEN);

    while (so_far < expected) {
        ssize_t i = read(socket, &p_buff, expected);
        if (i < 0) {
            DEBUG_PRINT("recv error: %s\n", strerror(errno));
            return MF_ERROR_RECV_MSG;
        }

        if (i == 0) return MF_ERROR_SOCKET_CLOSED;

        memcpy(&buff + so_far, &p_buff, i);

        so_far += i;
        memset(p_buff, 0, MAX_DATA_LEN);
    }

    memcpy(dest, &buff, expected);
    return MF_ERROR_OK;
}

mf_error_t recv_msg(int socket, struct mf_msg *msg_recv, struct mf_ctx *ctx) {
    mf_error_t err;

    int expected;
    if ((expected = receive_size(socket)) < 0) {
        DEBUG_PRINT("recv error: %s\n", strerror(errno));
        return MF_ERROR_RECV_LEN;
    }
    if (expected == 0) return MF_ERROR_SOCKET_CLOSED;

    char buff[MAX_DATA_LEN];
    memset(buff, 0, MAX_DATA_LEN);

    if ((err = receive_variable_size(socket, expected, buff)) != MF_ERROR_OK) return err;

    msg_recv->len = expected;
    memcpy(msg_recv->data, buff, MAX_DATA_LEN);
    DEBUG_PRINT("Message len: %d Message content: %s\n", msg_recv->len, msg_recv->data);
    return err;
}


mf_error_t send_data(int socket, struct mf_msg *msg, struct mf_ctx *ctx) {
    DEBUG_PRINT("Sending message of len %d Message content: %s\n", msg->len, msg->data);
    ssize_t len;
    if ((len = write(socket, &msg->data, msg->len)) < 0) {
        return MF_ERROR_SEND;
    }
    assert(len == msg->len);

    return MF_ERROR_OK;
}

mf_error_t ctx_send_verify(struct mf_ctx *ctx) {
    if (!ctx) return MF_ERROR_NULL_CTX;
    if (!ctx->recv_cb) return MF_ERROR_NO_RECV_CB;
    if (!ctx->timeout_cb) return MF_ERROR_NO_TIMEOUT_CB;

    return MF_ERROR_OK;
}

int set_timeout(int socket, int timeout) {
    struct timeval tv;
    memset(&tv, 0, sizeof(struct timeval));
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) {
        return errno;
    }
    return 0;
}

int mf_send_msg(int socket, struct mf_msg *msg, struct mf_ctx *ctx) {
    mf_error_t err;

    if ((err = ctx_send_verify(ctx)) != MF_ERROR_OK) {
        mf_error_print(err);
        return -1;
    }

    if (ctx->timeout > -1) {
        if (set_timeout(socket, ctx->timeout) != 0) {
            perror("setsockopt()");
        }
    }

    if ((err = send_data(socket, msg, ctx)) != MF_ERROR_OK) {
        mf_error_print(err);
        return -1;
    }

    struct mf_msg response;
    if ((err = recv_msg(socket, &response, ctx)) != MF_ERROR_OK) {
        mf_error_print(err);
        return -1;
    }

    if ((err = ctx->recv_cb(socket, &response)) != MF_ERROR_OK) {
        mf_error_print(err);
        return -1;
    }

    return 0;
}


int mf_ctx_send_init(struct mf_ctx *ctx, int timeout, mf_timeout_cb_t timeout_cb, mf_recv_cb recv_cb) {
    ctx->timeout = timeout;
    ctx->timeout_cb = timeout_cb;
    ctx->recv_cb = recv_cb;

    return 0;
}
