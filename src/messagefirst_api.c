#include "include/messagefirst_api.h"
#include "include/util.h"

struct args {
    struct epoll_event event;
    int listen_sock;
    int epfd;
    struct mf_ctx *ctx;
};

int receive_size(int socket) {
    int payload_size = 0;

    ssize_t len;
    for (;;) {
        len = read(socket, &payload_size, sizeof(payload_size));
        if (len < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            continue;
        } else if (len <= 0) {
            DEBUG_PRINT("recv() error: %s\n", strerror(errno));
            return len;
        } else {
            break;
        }
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
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                memset(p_buff, 0, MAX_DATA_LEN);
                continue;
            }
            DEBUG_PRINT("recv error: %s\n", strerror(errno));
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
    DEBUG_PRINT("Sending message of len %d Message content: %s\n", msg->len, msg->data);
    ssize_t size_len;
    if ((size_len = write(socket, &msg->len, sizeof(msg->len))) < 0) {
        DEBUG_PRINT("send error: %s\n", strerror(errno));
        return MF_ERROR_SEND;
    }
    assert(size_len == sizeof(msg->len));

    ssize_t len;
    if ((len = write(socket, &msg->data, msg->len)) < 0) {
        return MF_ERROR_SEND;
    }
    assert(len == msg->len);

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
    if ((err = receive_variable_size(socket, expected, buff)) != MF_ERROR_OK) {
        return err;
    }
    msg_recv->len = expected;
    memcpy(msg_recv->data, buff, MAX_DATA_LEN);
    DEBUG_PRINT("Message len: %d Message content: %s\n", msg_recv->len, msg_recv->data);
    return err;
}


int mf_send_msg_blocking(int socket, struct mf_msg *msg, struct mf_ctx *ctx) {
    mf_error_t err;
    CB_IF_ERROR(err,send_len_and_data(socket, msg, ctx), msg, ctx)

    struct mf_msg response;
    CB_IF_ERROR(err,recv_msg(socket, &response, ctx), &response, ctx)

    CB_IF_ERROR(err,ctx->recv_cb(socket, &response), &response, ctx)

    return 0;
}

mf_error_t recv_and_response(int socket, struct mf_ctx *ctx) {
    mf_error_t err;
    struct mf_msg response;

    if ((err = recv_msg(socket, &response, ctx)) != MF_ERROR_OK) {
        return err;
    }

    if ((err = ctx->recv_cb(socket, &response)) != MF_ERROR_OK) {
        return err;
    }

    return MF_ERROR_OK;
}

mf_error_t epoll_ctl_add(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if ((epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev)) == -1) {
        return MF_ERROR_EPOLL_CTL;
    }
    return MF_ERROR_OK;
}

mf_error_t set_nonblocking(int sock_fd) {
    if ((fcntl(sock_fd, F_SETFL, fcntl(sock_fd, F_GETFL, 0) | O_NONBLOCK)) == -1) {
        DEBUG_PRINT("fctnl error: %s\n", strerror(errno));
        return MF_ERROR_NONBLOCKING;
    }
    return MF_ERROR_OK;
}

mf_error_t handle_new_connection(int listen_sock, int epfd) {
    socklen_t socklen = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    int connection_sock;
    if ((connection_sock = accept(listen_sock, (struct sockaddr * ) &addr, &socklen)) < 0) {
        DEBUG_PRINT("accept() error: %s\n", strerror(errno));
        return MF_ERROR_SEND;
    }

    mf_error_t err;
    if ((err = set_nonblocking(connection_sock)) != MF_ERROR_OK) {
        return err;
    }

    if ((err = epoll_ctl_add(epfd, connection_sock, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP)) != MF_ERROR_OK) {
        return err;
    }

    return MF_ERROR_OK;
}

mf_error_t remove_connection(int epfd, int conn_fd) {
    DEBUG_PRINT("connection closed with fd: %d\n", conn_fd);

    if ((epoll_ctl(epfd, EPOLL_CTL_DEL, conn_fd, NULL)) < 0) {
        return MF_ERROR_EPOLL_CTL;
    }
    close(conn_fd);

    return MF_ERROR_OK;
}

mf_error_t handle_data_in(int epfd, int event_fd, struct mf_ctx *ctx) {
    mf_error_t err;
    if ((err = recv_and_response(event_fd, ctx)) != MF_ERROR_OK) {
        if (err == MF_ERROR_SOCKET_CLOSED) return remove_connection(epfd, event_fd);
        else return err;
    }
    return MF_ERROR_OK;
}

void *handle_event(void *in) {
    struct args *args = (struct args*) in;
    struct epoll_event event = args->event;
    int listen_sock = args->listen_sock;
    int epfd = args->epfd;
    struct mf_ctx *ctx = args->ctx;

    if (event.data.fd == listen_sock) return (void *) handle_new_connection(listen_sock, epfd);

    else if (event.events & EPOLLIN) return (void *) handle_data_in(epfd, event.data.fd, ctx);

    else if (event.events & (EPOLLRDHUP | EPOLLHUP)) return (void *) remove_connection(epfd, event.data.fd);

    else { DEBUG_PRINT("Unhandled epoll event: %04x\n", event.events); }

    return (void *) MF_ERROR_OK;
}

int mf_poll(int listen_sock, struct mf_ctx *ctx) {
    struct epoll_event events[MAX_EVENTS];
    int ret = -1;

    int epfd = epoll_create1(0);

    mf_error_t err;
    if ((err = epoll_ctl_add(epfd, listen_sock, EPOLLIN | EPOLLOUT)) != MF_ERROR_OK) {
        mf_error_print(err);
        ret = 1;
        goto cleanup;
    }

    for (;;) {
        int n_fds;
        if ((n_fds = epoll_wait(epfd, events, MAX_EVENTS, ctx->timeout)) < 0) {
            DEBUG_PRINT("epoll_wait() error: %s\n", strerror(errno));
            ret = n_fds;
            goto cleanup;
        }

        if (n_fds > 0) {
            DEBUG_PRINT("waiting fds: %d\n", n_fds);
        }

        for (int i = 0; i < n_fds; i++) {
            struct args args;
            args.event = events[i];
            args.listen_sock = listen_sock;
            args.epfd = epfd;
            args.ctx = ctx;

            mf_error_t res = (mf_error_t) handle_event((void *) &args);
            if (res != MF_ERROR_OK) {
                ret = 1;
                ctx->error_cb(listen_sock, NULL, res);
                goto cleanup;
            }
        }


//        pthread_t thread_ids[n_fds];
//        for (int i = 0; i < n_fds; i++) {
//            struct args args;
//            args.event = events[i];
//            args.listen_sock = listen_sock;
//            args.epfd = epfd;
//            args.ctx = ctx;
//
//            pthread_create(&thread_ids[i], NULL, handle_event, (void *) &args);
//            DEBUG_PRINT("Created thread %lu\n", thread_ids[i]);
//        }
//
//        for (int i = 0; i < n_fds; i++) {
//            mf_error_t ret_val;
//
//            pthread_join(thread_ids[i], (void **) &ret_val);
//            DEBUG_PRINT("thread id %lu terminated with ret %d\n", thread_ids[i], ret_val);
//
//            if (ret_val != MF_ERROR_OK) {
//                ret = 1;
//                ctx->error_cb(listen_sock, NULL, ret_val);
//                goto cleanup;
//            }
//        }

    }

    ret = 0;
    cleanup:
    close(epfd);
    return ret;
}

int mf_send_msg(int socket, struct mf_msg *msg, struct mf_ctx *ctx) {
    return mf_send_msg_blocking(socket, msg, ctx);
}

mf_error_t mf_send_msg_response(int socket, struct mf_msg *msg) {
    mf_error_t err;
    struct mf_ctx *ctx = (struct mf_ctx*) malloc(sizeof(struct mf_ctx));
    assert(ctx);
    CB_IF_ERROR(err,send_len_and_data(socket, msg, ctx), msg, ctx)
    free(ctx);

    return err;
}
