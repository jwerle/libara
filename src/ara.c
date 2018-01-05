#include <ara/ara.h>
#include <string.h>
#include <uv.h>

ARAboolean
ara_init(ara_t *self) {
  if (0 == self) { return ARA_FALSE; }
  memset(self, 0, sizeof(ara_t));

  self->status = ARA_STATUS_INIT;
  self->bitfield.work = 0;

  self->error.code = ARA_ENONE;
  self->error.data = 0;
  self->loop = uv_default_loop();

  self->open = 0;
  self->close = 0;
  self->end = 0;
  self->read = 0;
  self->write = 0;
  self->unlink = 0;

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
ara_set(ara_t *self, ara_work_t type, ara_worker_cb *cb) {
  if (0 == self) { return ARA_FALSE; }
  if (0 == cb) { return ARA_FALSE; }

  switch (type) {
#define SET(which)  self->which = (ara_##which##_work *) cb;

    case ARA_WORK_OPEN: SET(open); break;
    case ARA_WORK_CLOSE: SET(close); break;
    case ARA_WORK_END: SET(end); break;
    case ARA_WORK_READ: SET(read); break;
    case ARA_WORK_WRITE: SET(write); break;
    case ARA_WORK_UNLINK: SET(unlink); break;
    default: return ARA_FALSE;

#undef SET
  }

  ara_clear_error(&self->error);
  self->bitfield.work |= type;
  return ARA_TRUE;
}

ARAboolean
ara_throw(ara_t *self, ara_error_code_t code) {
  ara_set_error(&self->error, code, ara_strerror(code));
  return ARA_FALSE;
}
