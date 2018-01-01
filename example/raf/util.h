#ifndef RAF_UTIL_H
#define RAF_UTIL_H

#include <debug/debug.h>
#include <ara/ara.h>
#include <stdlib.h>
#include <uv.h>

#include "raf.h"

#define __SS(x) #x
#define __S(x) __SS(x)

#ifndef D
#define D(...) RAF_DEBUG(__VA_ARGS__)
#endif

#define make(T) (T *) malloc(sizeof(T))
#define delete(x) if (x) { free(x); x = 0; }

#define panic(expr, message, ...) ({          \
  if (0 == (!! (expr))) {                     \
    fprintf(stderr,                           \
      "[panic] %s(%s:%d): '%s' failed. %s\n", \
      __FUNCTION__, __FILE__, __LINE__,       \
      __S(expr), message __VA_ARGS__);        \
    exit(1);                                  \
  }                                           \
})                                            \

#define ON_ARA_WORK_DONE(ara, res, Callback)                \
  RandomAccessFileRequest *rafreq = 0;                      \
  rafreq = (RandomAccessFileRequest *) res->req->data.data; \
  if (rafreq) {                                             \
    Callback *callback = rafreq->callback;                  \
                                                            \
    if (callback) {                                         \
      callback(rafreq->raf, rafreq);                        \
    }                                                       \
                                                            \
    delete(rafreq);                                         \
  }                                                         \

#endif
