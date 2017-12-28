#include <ara/ara.h>
#include <uv.h>
#include "uv.h"

static ARAvoid
open_work_noop(ara_t *self) {
  (ARAvoid) (self);
}

static ARAvoid
on_ara_open_async_done(uv_handle_t *handle) {
  (ARAvoid) handle;
}

static ARAvoid
on_ara_open_work_done(ara_t *self) {
  if (0 == self) { return; }
  if (self->error.code < ARA_ENONE) { return; }
  if (ARA_STATUS_OPENING != self->status) { return; }

  self->status = ARA_STATUS_OPENED;

  for (int i = 0; i < self->callbacks.open.length; ++i) {
    if (0 != self->callbacks.open.entries[i]) {
      self->callbacks.open.entries[i](self);
      self->callbacks.open.entries[i] = 0;
    }
  }

  self->callbacks.open.length = 0;
  uv_close((uv_handle_t *) &self->async.open, on_ara_open_async_done);
}

ARAvoid
onuvopen(uv_async_t* handle) {
  ara_t *self = (ara_t *) handle->data;
  if (0 == self) { return; }
  if (0 == (self->bitfield.work & ARA_WORK_OPEN)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  self->open(self, on_ara_open_work_done);
}

ARAboolean
ara_open(ara_t *self, ara_open_work_cb *cb) {
  if (0 == self) { return ARA_FALSE; }
  if (0 == self->loop) { return ara_throw(self, ARA_ENOUVLOOP); }
  if (0 == (self->bitfield.work & ARA_WORK_OPEN)) {
    return ara_throw(self, ARA_ENOCALLBACK);
  }

  if (0 == cb) {
    cb = open_work_noop;
  }

  switch (self->status) {
    case ARA_STATUS_INIT: goto init;
    case ARA_STATUS_OPENING: goto opening;
    case ARA_STATUS_OPENED: goto opened;
    default: return ara_throw(self, ARA_EBADSTATE);
  }

init:
  if (ARA_STATUS_INIT == self->status) {
    self->status = ARA_STATUS_OPENING;
    if (uv_async_send(&self->async.open) < 0) {
      return ara_throw(self, ARA_EUVASYNCSEND);
    }
  }

opening:
  if (open_work_noop != cb) {
    self->callbacks.open.entries[self->callbacks.open.length++] = cb;
  }

opened:
  return ARA_TRUE;
}
