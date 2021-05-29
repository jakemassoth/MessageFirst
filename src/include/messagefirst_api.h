#ifndef MESSAGEFIRST_MESSAGEFIRST_API_H
#define MESSAGEFIRST_MESSAGEFIRST_API_H
#include "messagefirst.h"

int mf_send_msg(int socket, struct mf_msg *msg_send, struct mf_msg *msg_recv, int timeout);

int mf_poll(int socket, struct mf_ctx *ctx);

void mf_ctx_cleanup(struct mf_ctx *ctx);

int mf_ctx_poll_init(struct mf_ctx *ctx,  int timeout, mf_error_cb_t error_cb, mf_timeout_cb_t timeout_cb,
                     mf_poll_resp_cb poll_resp_cb, int num_threads);

#endif //MESSAGEFIRST_MESSAGEFIRST_API_H
