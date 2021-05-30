#include "include/messagefirst_api.h"
#include "include/util.h"

struct event_args {
    int epfd;
    int event_fd;
    struct mf_ctx *ctx;
};

mf_error_t send_len_and_data(int socket, struct mf_msg *msg) {
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

mf_error_t recv_msg_poll(int socket, struct mf_msg *msg_recv) {
    ssize_t so_far = 0;

    char temp_buff[MAX_DATA_LEN];
    char buff[MAX_DATA_LEN];
    memset(buff, 0, MAX_DATA_LEN);
    memset(temp_buff, 0, MAX_DATA_LEN);

    for (;;) {
        ssize_t i = read(socket, &temp_buff, MAX_DATA_LEN);
        if (i < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            DEBUG_PRINT("recv error: %s\n", strerror(errno));
            return MF_ERROR_RECV_MSG;
        }

        if (i == 0) return MF_ERROR_SOCKET_CLOSED;

        memcpy(&buff + so_far, &temp_buff, i);

        so_far += i;
        memset(temp_buff, 0, MAX_DATA_LEN);
    }
    if (so_far == 0) {
        return MF_ERROR_NONBLOCKING;
    }

    memcpy(msg_recv->data, buff, so_far);
    msg_recv->len = so_far;
    DEBUG_PRINT("Message content: %s, len %d\n", msg_recv->data, msg_recv->len);
    return MF_ERROR_OK;
}

mf_error_t recv_and_response(int socket, struct mf_ctx *ctx) {
    mf_error_t err;
    struct mf_msg req;

    if ((err = recv_msg_poll(socket, &req)) != MF_ERROR_OK) {
        return err;
    }

    struct mf_msg resp = ctx->poll_resp_cb(req);

    if ((err = send_len_and_data(socket, &resp)) != MF_ERROR_OK) {
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

    if (close(conn_fd) < 0) {
        return MF_ERROR_CLOSE_FAILED;
    }

    return MF_ERROR_OK;
}

void *handle_data_in(void *args) {
    struct event_args *arg = (struct event_args*) args;
    mf_error_t err;
    if ((err = recv_and_response(arg->event_fd, arg->ctx)) != MF_ERROR_OK) {
        if (err == MF_ERROR_SOCKET_CLOSED) return (void *) remove_connection(arg->epfd, arg->event_fd);
        else {
            arg->ctx->error_cb(arg->event_fd, NULL, err);
            return (void *) err;
        }
    }
    return (void *) MF_ERROR_OK;
}

mf_error_t handle_event(int listen_sock, int epfd, struct epoll_event event, struct mf_ctx *ctx) {
    if (event.data.fd == listen_sock) return handle_new_connection(listen_sock, epfd);

    else if (event.events & EPOLLIN) {
        struct event_args args = {
                .event_fd = event.data.fd,
                .ctx = ctx,
                .epfd = epfd
        };
        if (thread_pool_submit_work(handle_data_in, &args, sizeof(struct event_args), ctx->tp) != 0)
            return MF_ERROR_RECV_MSG;

        return MF_ERROR_OK;
    }

    else if (event.events & (EPOLLRDHUP | EPOLLHUP)) return remove_connection(epfd, event.data.fd);

    else { DEBUG_PRINT("Unhandled epoll event: %04x\n", event.events); }

    return MF_ERROR_OK;
}

mf_error_t ctx_poll_verify(struct mf_ctx *ctx) {
    if (!ctx) return MF_ERROR_NULL_CTX;
    if (!ctx->tp) return MF_ERROR_NO_THREAD_POOL;
    if (!ctx->error_cb) return MF_ERROR_NO_ERROR_CB;
    if (!ctx->poll_resp_cb) return MF_ERROR_NO_POLL_CB;
    if (!ctx->timeout_cb) return MF_ERROR_NO_TIMEOUT_CB;

    return MF_ERROR_OK;
}

int mf_poll(int listen_sock, struct mf_ctx *ctx) {
    struct epoll_event events[MAX_EVENTS];
    int ret;
    mf_error_t err;

    if ((err = ctx_poll_verify(ctx)) != MF_ERROR_OK) {
        mf_error_print(err);
        return -1;
    }

    int epfd = epoll_create1(0);

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

        if (n_fds > 0) DEBUG_PRINT("waiting fds: %d\n", n_fds);

        for (int i = 0; i < n_fds; i++) {
            mf_error_t res = handle_event(listen_sock, epfd, events[i], ctx);
            if (res != MF_ERROR_OK) {
                ret = 1;
                ctx->error_cb(listen_sock, NULL, res);
                goto cleanup;
            }
        }
//        thread_pool_wait(ctx->tp);
    }

    cleanup:
    close(epfd);
    return ret;
}

int mf_ctx_poll_init(struct mf_ctx *ctx,  int timeout, mf_error_cb_t error_cb, mf_timeout_cb_t timeout_cb,
        mf_poll_resp_cb poll_resp_cb, int num_threads) {
    ctx->tp = thread_pool_init(num_threads);
    if (!ctx->tp) {
        return -1;
    }
    ctx->timeout = timeout;
    ctx->error_cb = error_cb;
    ctx->timeout_cb = timeout_cb;
    ctx->poll_resp_cb = poll_resp_cb;

    return 0;
}

void mf_ctx_cleanup(struct mf_ctx *ctx) {
    if (ctx->tp) thread_pool_destroy(ctx->tp);
}
