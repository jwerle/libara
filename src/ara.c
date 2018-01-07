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

  memset(self->work, 0, ARA_MAX_WORK * sizeof(ara_work_cb *));

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
ara_set(ara_t *self, ara_work_t type, ara_work_cb *cb) {
  if (0 == self) { return ARA_FALSE; }
  if (0 == cb) { return ARA_FALSE; }

  if (type < ARA_MAX_WORK) {
    self->work[type] = cb;
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

ARAboolean
ara_call(ara_t *self, ara_work_t type, ara_async_req_t *req, ara_done_cb *done) {
  ara_work_cb *cb = 0;
  if (0 == self) { return ARA_FALSE; }
  if (0 == done) { return ARA_FALSE; }
  if (0 == type) { return ARA_FALSE; }
  if (0 == req) { return ARA_FALSE; }
  cb = self->work[type];
  if (cb) {
    cb(req, done);
    return ARA_TRUE;
  }
  return ARA_FALSE;
}

ARAchar *
ara_status_string(ara_t *self) {
  if (0 == self) { return 0; }
  return ara_strstatus(self->status);
}

ARAchar *
ara_strstatus(ara_status_t status) {
#define X(x) ARA_STATUS_##x
  switch (status) {
    case X(INIT): return "init";
    case X(OPENING): return "opening";
    case X(OPENED): return "opened";
    case X(CLOSING): return "closed";
    case X(CLOSED): return "closed";
    default: return "unknown";
  }
#undef X
}
