#include <ara/ara.h>
#include "async.h"
#include "work.h"

static ARAvoid
close_work_noop(ara_t *self) {
  (ARAvoid) (self);
}

static ARAvoid
on_async_end(ara_t *self, ara_async_res_t *res) {
  if (res && res->req) {
    ara_async_req_destroy(res->req);
  }
}

static ARAvoid
on_done(ara_t *self, ara_async_req_t *req) {
  if (0 == self || 0 == req) {
    return;
  }

  if (self->error.code < ARA_ENONE) {
    goto end;
  }

  if (ARA_STATUS_CLOSING != self->status) {
    ara_throw(self, ARA_EBADSTATE);
    goto end;
  }

  ara_close_cb *cb = (ara_close_cb *) req->data.data;

  self->status = ARA_STATUS_CLOSED;

  if (cb) {
    cb(self);
  }

end:
  ara_async_req_end(req);
}

static ARAvoid
on_async_begin(ara_t *self, ara_async_req_t *req) {
  if (0 == self) {
    return;
  }

  if (0 == (self->bitfield.work & ARA_WORK_CLOSE)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  switch (self->status) {
    case ARA_STATUS_OPENING:
    case ARA_STATUS_OPENED:
      self->status = ARA_STATUS_CLOSING;
      return self->close(self, req, &on_done);

    case ARA_STATUS_CLOSING:
    case ARA_STATUS_CLOSED:
      return on_done(self, req);

    default:
      ara_throw(self, ARA_EBADSTATE);
      return on_done(self, req);
  }
}

ARAboolean
ara_close(ara_t *self, ara_close_cb *cb) {
  ara_async_data_t data = {0};
  ara_async_req_t *req = 0;

  if (0 == cb) {
    cb = close_work_noop;
  }

  if (self) {
    switch (self->status) {
      case ARA_STATUS_OPENING:
      case ARA_STATUS_OPENED:
      case ARA_STATUS_CLOSING:
      case ARA_STATUS_CLOSED:
        break;

      default:
        WORK_THROW(self, ARA_EBADSTATE);
    }
  }

  if (ARA_FALSE == ara_async_data_init(&data)) {
    WORK_THROW(self, ARA_ENOCALLBACK);
  }

  data.data = cb;

  WORK(self, ARA_WORK_CLOSE, req, data, on_async_begin, on_async_end);
}
