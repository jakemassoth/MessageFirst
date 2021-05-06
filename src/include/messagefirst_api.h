#ifndef MESSAGEFIRST_MESSAGEFIRST_API_H
#define MESSAGEFIRST_MESSAGEFIRST_API_H
#include "messagefirst.h"

int mf_send_msg(int socket, struct mf_msg *msg, struct mf_ctx *ctx);
int mf_poll(int socket, struct mf_ctx *ctx);
mf_error_t mf_send_msg_response(int socket, struct mf_msg *msg);
void mf_ctx_cleanup(struct mf_ctx *ctx);
void mf_wait(struct mf_ctx *ctx);

#endif //MESSAGEFIRST_MESSAGEFIRST_API_H
