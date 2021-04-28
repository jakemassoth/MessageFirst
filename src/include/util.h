#ifndef MESSAGEFIRST_UTIL_H
#define MESSAGEFIRST_UTIL_H

#define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
    __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) /* Don't do anything in release builds */
#endif

#define CB_IF_ERROR(err, fn_call, msg, ctx) \
if ((err = fn_call) != MF_ERROR_OK) {  \
    ctx->error_cb(socket, msg, err);     \
    return 1;                               \
}
#endif //MESSAGEFIRST_UTIL_H
