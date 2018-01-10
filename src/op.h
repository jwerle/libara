#ifndef _ARA_OP_H
#define _ARA_OP_H

#include <ara/ara.h>

#define OP_ERROR(self) goto error;

#define OP_THROW(self, code) \
  ara_throw(self, code);     \
  OP_ERROR(self);            \

#define OP(self, type, req, data, begin, end)        \
  if (0 == self) {                                   \
    OP_ERROR(self);                                  \
  }                                                  \
                                                     \
  if (0 == self->loop) {                             \
    OP_THROW(self, ARA_ENOUVLOOP);                   \
  }                                                  \
                                                     \
  if (type != (self->bitfield.work & type)) {        \
    OP_THROW(self, ARA_ENOCALLBACK);                 \
  }                                                  \
                                                     \
  req = ara_async_req_new(self, begin, end);         \
                                                     \
  if (0 == req) {                                    \
    OP_ERROR(self);                                  \
  }                                                  \
                                                     \
  if (ARA_FALSE == ara_async_req_send(req, data)) {  \
    OP_THROW(self, ARA_EUVASYNCSEND);                \
  }                                                  \
                                                     \
  return ARA_TRUE;                                   \
                                                     \
error:                                               \
  if (req) {                                         \
    ara_async_req_destroy(req);                      \
    req = 0;                                         \
  }                                                  \
                                                     \
  return ARA_FALSE;                                  \

#endif
