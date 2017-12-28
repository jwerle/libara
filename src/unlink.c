#include <ara/ara.h>
#include <uv.h>
#include "uv.h"

static ARAvoid
unlink_work_noop(ara_t *self) {
  (ARAvoid) (self);
}

static ARAvoid
on_ara_unlink_async_done(uv_handle_t *handle) {
  (ARAvoid) handle;
}

static ARAvoid
on_ara_unlink_work_done(ara_t *self) {
  if (0 == self) { return; }
  if (self->error.code < ARA_ENONE) { return; }
  for (int i = 0; i < self->callbacks.unlink.length; ++i) {
    if (0 != self->callbacks.unlink.entries[i]) {
      self->callbacks.unlink.entries[i](self);
      self->callbacks.unlink.entries[i] = 0;
    }
  }
  self->callbacks.unlink.length = 0;
  uv_close((uv_handle_t *) &self->async.unlink, on_ara_unlink_async_done);
}

ARAvoid
onuvunlink(uv_async_t* handle) {
  ara_t *self = (ara_t *) handle->data;
  if (0 == self) { return; }
  if (0 == (self->bitfield.work & ARA_WORK_UNLINK)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  self->unlink(self, on_ara_unlink_work_done);
}

ARAboolean
ara_unlink(ara_t *self, ara_unlink_cb *cb) {
  if (0 == self) { return ARA_FALSE; }
  if (0 == self->loop) { return ara_throw(self, ARA_ENOUVLOOP); }
  if (0 == (self->bitfield.work & ARA_WORK_UNLINK)) {
    return ara_throw(self, ARA_ENOCALLBACK);
  }

  if (0 == cb) {
    cb = unlink_work_noop;
  }

  switch (self->status) {
    case ARA_STATUS_OPENING:
    case ARA_STATUS_OPENED:
      goto opened;

    default: return ara_throw(self, ARA_EBADSTATE);
  }

opened:
  if (uv_async_send(&self->async.unlink) < 0) {
    return ara_throw(self, ARA_EUVASYNCSEND);
  }

  if (unlink_work_noop != cb) {
    self->callbacks.unlink.entries[self->callbacks.unlink.length++] = cb;
  }

ended:
  return ARA_TRUE;
}
