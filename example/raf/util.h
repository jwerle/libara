#ifndef RAF_UTIL_H
#define RAF_UTIL_H

#include <debug/debug.h>
#include <ara/ara.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "raf.h"

#define __SS(x) #x
#define __S(x) __SS(x)

#ifndef D
#define D(...) RAF_DEBUG(__VA_ARGS__)
#endif

/** @see https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */
#define npot(x) ({      \
  unsigned int y = 0;   \
  y = (unsigned int) x; \
  y--;                  \
  y |= y >> 1;          \
  y |= y >> 2;          \
  y |= y >> 4;          \
  y |= y >> 8;          \
  y |= y >> 16;         \
  y++;                  \
  (y);                  \
})                      \

#define make(T) (T *) memset(malloc(sizeof(T)), 0, sizeof(T))
#define delete(x) ({ if (x) { free(x); x = 0; } })

#define error(format, ...) fprintf(stderr, "error: " format "\n", ##__VA_ARGS__)
#define fatal(...) ({ error(__VA_ARGS__); exit(1); })

#define panic(expr, format, ...) ({            \
  if (0 == (!! (expr))) {                      \
    fprintf(stderr,                            \
      "[panic]: %s(%s:%d): '%s' failed.\n"     \
      "[panic]: " format ".\n",                \
      __FUNCTION__, __FILE__, __LINE__,        \
      __S(expr), ##__VA_ARGS__);               \
    exit(1);                                   \
  }                                            \
})                                             \

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

#define UV_FS_PROCESS_REQ(fs, offset, fn, cb) do {  \
  RandomAccessFileRequest *rafreq = 0;              \
  ara_async_req_t *areq = 0;                        \
  ara_buffer_t *buffer = 0;                         \
  ARAlong nread = 0;                                \
                                                    \
  areq = (ara_async_req_t *) fs->data;              \
  nread = fs->result;                               \
  rafreq = (RandomAccessFileRequest *) fs;          \
  buffer = areq->buffer;                            \
                                                    \
  if (nread > 0) {                                  \
    buffer->data.buffer_mut.len -= nread;           \
                                                    \
    if (buffer->data.buffer_mut.len > 0) {          \
      ARAsizei delta = (                            \
          buffer->data.buffer.len                   \
        - buffer->data.buffer_mut.len               \
      );                                            \
      buffer->data.buffer_mut.base += nread;        \
      uv_fs_req_cleanup(fs);                        \
      fn(rafreq->raf->ara.loop,                     \
               (uv_fs_t *) fs,                      \
               rafreq->raf->fd,                     \
               &buffer->data.buffer_mut,  1,        \
               offset + delta, cb);                 \
      return;                                       \
    }                                               \
  } else if (0 == nread) {                          \
    if (buffer->data.buffer_mut.len > 0) {          \
      fs->result = -1;                              \
    }                                               \
  }                                                 \
} while (0);                                        \

#endif
