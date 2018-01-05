#include <ara/ara.h>
#include <uv.h>
#include "work.h"

static ARAvoid
open_work_noop(ara_t *self, ara_async_res_t *res) {
  (ARAvoid) (self); (ARAvoid) (res);
}

static ARAvoid
on_async_end(ara_t *self, ara_async_res_t *res) {
  (ARAvoid) (self); (ARAvoid) (res);
}

static ARAvoid
on_done(ara_t *self, ara_async_req_t *req) {
  if (0 == self || 0 == req) {
    return;
  }

  if (self->error.code < ARA_ENONE) {
    goto end;
  }

  if (ARA_STATUS_OPENING != self->status) {
    ara_throw(self, ARA_EBADSTATE);
    goto end;
  }

  ara_open_cb *cb = (ara_open_cb *) req->data.callback;

  self->status = ARA_STATUS_OPENED;

  if (cb) {
    cb(self, &req->res);
  }

end:
  ara_async_req_end(req);
}

static ARAvoid
on_async_begin(ara_t *self, ara_async_req_t *req) {
  if (0 == self) { return; }

  if (0 == (self->bitfield.work & ARA_WORK_OPEN)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  switch (self->status) {
    case ARA_STATUS_OPENED:
    case ARA_STATUS_OPENING:
      return on_done(self, req);

    case ARA_STATUS_INIT:
      self->status = ARA_STATUS_OPENING;
      return self->open(self, req, &on_done);

    default:
      ara_throw(self, ARA_EBADSTATE);
      return on_done(self, req);

  }
}

ARAboolean
ara_open(ara_t *self, ara_async_data_t *data, ara_open_cb *cb) {
  ara_async_req_t *req = 0;
  static ara_async_data_t empty = {0};

  if (0 == data) {
    ara_async_data_init(&empty);
    data = &empty;
  }

  if (0 == cb) {
    cb = open_work_noop;
  }

  data->callback = cb;

  WORK(self, ARA_WORK_OPEN, req, data, on_async_begin, on_async_end);
}
