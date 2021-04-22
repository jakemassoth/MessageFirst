#include "include/messagefirst_api.h"
#include "include/util.h"

int receive_size(int socket) {
    int payload_size = 0;

    ssize_t len;
    if ((len = recv(socket, &payload_size, sizeof(payload_size), 0)) <= 0) return len;

    assert(len == sizeof(payload_size));

    return payload_size;
}

mf_error_t receive_variable_size(int socket, int expected, char dest[MAX_DATA_LEN]) {
    ssize_t so_far = 0;

    char p_buff[MAX_DATA_LEN];
    char buff[MAX_DATA_LEN];

    while (so_far < expected) {
        ssize_t i = recv(socket, &p_buff, expected, 0);
        if (i < 0) {
            return MF_ERROR_RECV_MSG;
        }
        if (i == 0) {
            return MF_ERROR_SOCKET_CLOSED;
        }
        memcpy(&buff + so_far, &p_buff, i);

        so_far += i;
        memset(p_buff, 0, MAX_DATA_LEN);
    }

    memcpy(dest, &buff, expected);
    return MF_ERROR_OK;
}

mf_error_t send_len_and_data(int socket, struct mf_msg *msg, struct mf_ctx *ctx) {
    DEBUG_PRINT("Sending message of len %d\nMessage content: %s\n", msg->len, msg->data);
    ssize_t size_len;
    if ((size_len = send(socket, &msg->len, sizeof(msg->len), 0)) < 0) {
        return MF_ERROR_SEND;
    }
    assert(size_len == sizeof(msg->len));

    ssize_t len;
    if ((len = send(socket, &msg->data, msg->len, 0)) < 0) {
        return -1;
    }
    assert(len == msg->len);

    return MF_ERROR_OK;
}

mf_error_t recv_msg(int socket, struct mf_msg *msg_recv, struct mf_ctx *ctx) {
    mf_error_t err;

    int expected;
    if ((expected = receive_size(socket)) < 0) return MF_ERROR_RECV_LEN;
    if (expected == 0) return MF_ERROR_SOCKET_CLOSED;

    char buff[MAX_DATA_LEN];
    if ((err = receive_variable_size(socket, expected, buff)) != MF_ERROR_OK) {
        return err;
    }
    msg_recv->len = expected;
    memcpy(msg_recv->data, buff, MAX_DATA_LEN);
    DEBUG_PRINT("Message len: %d\nMessage content: %s\n", msg_recv->len, msg_recv->data);
    return err;
}


int mf_send_msg(int socket, struct mf_msg *msg, struct mf_ctx *ctx) {
    mf_error_t err;
    if ((err = send_len_and_data(socket, msg, ctx)) != MF_ERROR_OK) {
        ctx->error_cb(socket, msg, err);
        return -1;
    }

    struct mf_msg response;
    if ((err = recv_msg(socket, &response, ctx)) != MF_ERROR_OK) {
        ctx->error_cb(socket, msg, err);
        return -1;
    }

    return 0;
}

int mf_poll(int socket, struct mf_ctx *ctx) {
    mf_error_t err;
    struct mf_msg response;
    if ((err = recv_msg(socket, &response, ctx)) != MF_ERROR_OK) {
        ctx->error_cb(socket, &response, err);
        return -1;
    }

    return 0;
}

int mf_send_msg_response(int socket, struct mf_msg *msg, struct mf_ctx *ctx) {
    return 0;
}
