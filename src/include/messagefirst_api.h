#ifndef MESSAGEFIRST_MESSAGEFIRST_API_H
#define MESSAGEFIRST_MESSAGEFIRST_API_H
#include "messagefirst.h"

int mf_send_msg(int socket, struct mf_msg *msg, struct mf_ctx *ctx);

int mf_poll(int socket, struct mf_ctx *ctx);

mf_error_t mf_send_msg_response(int socket, struct mf_msg *msg);

void mf_ctx_cleanup(struct mf_ctx *ctx);

int mf_ctx_poll_init(struct mf_ctx *ctx,  int timeout, mf_error_cb_t error_cb, mf_timeout_cb_t timeout_cb,
                     mf_poll_resp_cb poll_resp_cb, int num_threads);

int mf_ctx_send_init(struct mf_ctx *ctx, int timeout, mf_timeout_cb_t timeout_cb,
                     mf_recv_cb recv_cb);

#endif //MESSAGEFIRST_MESSAGEFIRST_API_H
