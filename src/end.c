#include <ara/ara.h>
#include "async.h"
#include "work.h"

static ARAvoid
end_work_noop(ara_t *self) {
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

  if (ARA_STATUS_OPENED != self->status) {
    ara_throw(self, ARA_EBADSTATE);
    goto end;
  }

  ara_open_cb *cb = (ara_open_cb *) req->data.data;

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

  if (0 == (self->bitfield.work & ARA_WORK_END)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  switch (self->status) {
    case ARA_STATUS_OPENED:
      return self->end(self, req, &on_done);

    default:
      ara_throw(self, ARA_EBADSTATE);
      return on_done(self, req);
  }
}

ARAboolean
ara_end(ara_t *self, ara_end_cb *cb) {
  ara_async_data_t data = {0};
  ara_async_req_t *req = 0;

  if (0 == cb) {
    cb = end_work_noop;
  }

  if (self) {
    switch (self->status) {
      case ARA_STATUS_OPENED:
        break;

      default:
        WORK_THROW(self, ARA_EBADSTATE);
    }
  }

  if (ARA_FALSE == ara_async_data_init(&data)) {
    WORK_THROW(self, ARA_ENOCALLBACK);
  }

  data.data = cb;

  WORK(self, ARA_WORK_END, req, data, on_async_begin, on_async_end);
}
