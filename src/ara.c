#include <ara/ara.h>
#include <string.h>
#include "async.h"

ARAboolean
ara_init(ara_t *self) {
  if (0 == self) { return ARA_FALSE; }
  memset(self, 0, sizeof(ara_t));
  self->status = ARA_STATUS_INIT;
  self->bitfield.work = 0;

  self->error.code = ARA_ENONE;
  self->error.data = 0;
  self->loop = uv_default_loop();

#define INIT(which)                                                        \
  self->which = 0;                                                         \
  self->callbacks.which.length = 0;                                        \
  self->callbacks.which.entries[0] = 0;                                    \
  if (uv_async_init(self->loop, &self->async.which, onasync##which) < 0) { \
    return ara_throw(self, ARA_EUVASYNCINIT);                              \
  } else {                                                                 \
    self->async.which.data = (ARAvoid *) self;                             \
  }                                                                        \

  INIT(open)
  INIT(close)
  INIT(end)
  INIT(read)
  INIT(write)
  INIT(unlink)

#undef INIT

  return ARA_TRUE;
}

ARAboolean
ara_set_loop(ara_t *self, uv_loop_t *loop) {
  if (0 == self) { return ARA_FALSE; }
  if (0 == loop) { return ARA_FALSE; }
  self->loop = loop;
  return ARA_TRUE;
}

ARAboolean
ara_set(ara_t *self, ara_work_t type, ara_cb *cb) {
  if (0 == self) { return ARA_FALSE; }
  if (0 == cb) { return ARA_FALSE; }

#define SET(which)                         \
  self->which = (ara_##which##_work *) cb; \
  self->callbacks.which.entries[0] = 0;    \
  self->callbacks.which.length = 0;        \

  switch (type) {
    case ARA_WORK_OPEN: SET(open); break;
    case ARA_WORK_CLOSE: SET(close); break;
    case ARA_WORK_END: SET(end); break;
    case ARA_WORK_READ: SET(read); break;
    case ARA_WORK_WRITE: SET(write); break;
    case ARA_WORK_UNLINK: SET(unlink); break;
    default: return ARA_FALSE;
  }

#undef SET

  ara_clear_error(&self->error);
  self->bitfield.work |= type;
  return ARA_TRUE;
}

ARAboolean
ara_throw(ara_t *self, ara_error_code_t code) {
  ara_set_error(&self->error, code, ara_error(code));
  return ARA_FALSE;
}
