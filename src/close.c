#include <ara/ara.h>
#include "async.h"

static ARAvoid
close_work_noop(ara_t *self) {
  (ARAvoid) (self);
}

static ARAvoid
on_ara_close_async_done(uv_handle_t *handle) {
  (ARAvoid) handle;
}

static ARAvoid
on_ara_noop_async_done(uv_handle_t *handle) {
  (ARAvoid) handle;
}

static ARAvoid
on_ara_close_work_done(ara_t *self) {
  if (0 == self) { return; }
  if (self->error.code < ARA_ENONE) { return; }
  self->status = ARA_STATUS_CLOSED;
  for (int i = 0; i < self->callbacks.close.length; ++i) {
    if (0 != self->callbacks.close.entries[i]) {
      self->callbacks.close.entries[i](self);
      self->callbacks.close.entries[i] = 0;
    }
  }
  self->callbacks.close.length = 0;
  uv_close((uv_handle_t *) &self->async.close, on_ara_close_async_done);
}

ARAvoid
onasyncclose(uv_async_t* handle) {
  ara_t *self = (ara_t *) handle->data;
  if (0 == self) { return; }
  if (0 == (self->bitfield.work & ARA_WORK_CLOSE)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  self->close(self, on_ara_close_work_done);
}

ARAboolean
ara_close(ara_t *self, ara_close_cb *cb) {
  if (0 == self) { return ARA_FALSE; }
  if (0 == self->loop) { return ara_throw(self, ARA_ENOUVLOOP); }
  if (0 == (self->bitfield.work & ARA_WORK_CLOSE)) {
    return ara_throw(self, ARA_ENOCALLBACK);
  }

  if (0 == cb) {
    cb = close_work_noop;
  }

  switch (self->status) {
    case ARA_STATUS_OPENING:
      uv_close((uv_handle_t *) &self->async.open, on_ara_noop_async_done);
    case ARA_STATUS_OPENED:
      goto opened;

    case ARA_STATUS_ENDING:
    case ARA_STATUS_ENDED:
      uv_close((uv_handle_t *) &self->async.end, on_ara_noop_async_done);
      goto opened;

    case ARA_STATUS_CLOSING: goto closing;
    case ARA_STATUS_CLOSED: goto closed;
    default: return ara_throw(self, ARA_EBADSTATE);
  }

opened:
  if (uv_async_send(&self->async.close) < 0) {
    return ara_throw(self, ARA_EUVASYNCSEND);
  }

closing:
  self->status = ARA_STATUS_CLOSING;
  if (close_work_noop != cb) {
    self->callbacks.close.entries[self->callbacks.close.length++] = cb;
  }

closed:
  return ARA_TRUE;
}
