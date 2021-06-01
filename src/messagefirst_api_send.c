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

mf_error_t recv_msg(int socket, struct mf_msg *msg_recv) {
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


mf_error_t send_data(int socket, struct mf_msg *msg) {
    DEBUG_PRINT("Sending message of len %d Message content: %s\n", msg->len, msg->data);
    ssize_t len;
    if ((len = write(socket, &msg->data, msg->len)) < 0) {
        return MF_ERROR_SEND;
    }
    assert(len == msg->len);

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

int mf_send_msg(int socket, struct mf_msg *msg_send, struct mf_msg *msg_recv, int timeout) {
    mf_error_t err;

    if (timeout > -1) {
        if (set_timeout(socket, timeout) != 0) {
            perror("setsockopt()");
        }
    }

#ifdef NO_DELAY
    int flag = 1;
    if (setsockopt(socket,IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int)) != 0) {
        perror("setsockopt()");
    }
#else
#endif

    if ((err = send_data(socket, msg_send)) != MF_ERROR_OK) {
        mf_error_print(err);
        return -1;
    }

    struct mf_msg response;
    if ((err = recv_msg(socket, &response)) != MF_ERROR_OK) {
        mf_error_print(err);
        return -1;
    }

    memcpy(msg_recv, &response, response.len);
    msg_recv->len = response.len;

    return 0;
}
