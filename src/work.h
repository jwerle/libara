#ifndef ARA_WORK_H
#define ARA_WORK_H

#define WORK_ERROR(self) goto error;

#define WORK_THROW(self, code) \
  ara_throw(self, code);       \
  WORK_ERROR(self);            \

/**
 */
#define WORK(self, type, req, data, begin, end)      \
  if (0 == self) {                                   \
    WORK_ERROR(self);                                \
  }                                                  \
                                                     \
  if (0 == self->loop) {                             \
    WORK_THROW(self, ARA_ENOUVLOOP);                 \
  }                                                  \
                                                     \
  if (0 == (self->bitfield.work & type)) {           \
    WORK_THROW(self, ARA_ENOCALLBACK);               \
  }                                                  \
                                                     \
  req = ara_async_req_new(self, begin, end);         \
                                                     \
  if (0 == req) {                                    \
    WORK_ERROR(self);                                \
  }                                                  \
                                                     \
  if (ARA_FALSE == ara_async_req_send(req, &data)) { \
    WORK_THROW(self, ARA_EUVASYNCSEND);              \
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
