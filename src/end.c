#include <ara/ara.h>
#include <uv.h>
#include "uv.h"

static ARAvoid
end_work_noop(ara_t *self) {
  (ARAvoid) (self);
}

static ARAvoid
on_ara_end_async_done(uv_handle_t *handle) {
  (ARAvoid) handle;
}

static ARAvoid
on_ara_end_work_done(ara_t *self) {
  if (0 == self) { return; }
  if (self->error.code < ARA_ENONE) { return; }
  self->status = ARA_STATUS_ENDED;
  for (int i = 0; i < self->callbacks.end.length; ++i) {
    if (0 != self->callbacks.end.entries[i]) {
      self->callbacks.end.entries[i](self);
      self->callbacks.end.entries[i] = 0;
    }
  }
  self->callbacks.end.length = 0;
  uv_close((uv_handle_t *) &self->async.end, on_ara_end_async_done);
}

ARAvoid
onuvend(uv_async_t* handle) {
  ara_t *self = (ara_t *) handle->data;
  if (0 == self) { return; }
  if (0 == (self->bitfield.work & ARA_WORK_END)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  self->end(self, on_ara_end_work_done);
}

ARAboolean
ara_end(ara_t *self, ara_end_work_cb *cb) {
  if (0 == self) { return ARA_FALSE; }
  if (0 == self->loop) { return ara_throw(self, ARA_ENOUVLOOP); }
  if (0 == (self->bitfield.work & ARA_WORK_END)) {
    return ara_throw(self, ARA_ENOCALLBACK);
  }

  if (0 == cb) {
    cb = end_work_noop;
  }

  switch (self->status) {
    case ARA_STATUS_OPENED: goto opened;
    case ARA_STATUS_ENDING: goto ending;
    case ARA_STATUS_ENDED: goto ended;
    default: return ara_throw(self, ARA_EBADSTATE);
  }

opened:
  if (ARA_STATUS_OPENED == self->status) {
    self->status = ARA_STATUS_ENDING;
    if (uv_async_send(&self->async.end) < 0) {
      return ara_throw(self, ARA_EUVASYNCSEND);
    }
  }

ending:
  if (end_work_noop != cb) {
    self->callbacks.end.entries[self->callbacks.end.length++] = cb;
  }

ended:
  return ARA_TRUE;
}
