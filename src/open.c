#include <ara/ara.h>
#include <uv.h>
#include "op.h"

static ARAvoid
on_async_end(ara_async_res_t *res) {
  (ARAvoid) (res);
}

static ARAvoid
on_done(ara_async_req_t *req) {
  if (0 == req || 0 == req->ara) {
    return;
  }

  ara_t *self = req->ara;

  if (self->error.code < ARA_ENONE) {
    goto end;
  }

  if (ARA_STATUS_OPENING != self->status) {
    ara_throw(self, ARA_EBADSTATE);
    goto end;
  }

  ara_cb *callback = (ara_cb *) req->data.callback;

  self->status = ARA_STATUS_OPENED;

  if (callback) {
    callback(&req->res);
  }

end:
  ara_async_req_end(req);
}

static ARAvoid
on_async_begin(ara_async_req_t *req) {
  if (0 == req || 0 == req->ara) {
    return;
  }

  ara_t *self = req->ara;

  if (0 == (self->bitfield.work & ARA_OPEN)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  switch (self->status) {
    case ARA_STATUS_OPENED:
    case ARA_STATUS_OPENING:
      return on_done(req);

    case ARA_STATUS_INIT:
      self->status = ARA_STATUS_OPENING;
      ara_call(self, ARA_OPEN, req, &on_done);
      return;

    default:
      ara_throw(self, ARA_EBADSTATE);
      return on_done(req);

  }
}

ARAboolean
ara_open(ara_t *self, ara_async_data_t *data, ara_cb *callback) {
  ara_async_req_t *req = 0;
  static ara_async_data_t empty = {0};

  if (0 == data) {
    ara_async_data_init(&empty);
    data = &empty;
  }

  if (0 == callback) {
    data->callback = ara_noop_cb;
  } else {
    data->callback = callback;
  }

  OP(self, ARA_OPEN, req, data, on_async_begin, on_async_end);
}
