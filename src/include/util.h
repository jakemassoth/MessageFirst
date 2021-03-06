#ifndef MESSAGEFIRST_UTIL_H
#define MESSAGEFIRST_UTIL_H
#include <fcntl.h>

//#define DEBUG 1
#define NO_DELAY 1

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
    __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) ;
#endif

#endif //MESSAGEFIRST_UTIL_H
