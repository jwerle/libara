#include <ara/ara.h>
#include <stdlib.h>
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

  if (ARA_STATUS_OPENED != self->status) {
    ara_throw(self, ARA_EBADSTATE);
    goto end;
  }

  ara_cb *callback = (ara_cb *) req->data.callback;

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

  if (0 == (self->bitfield.work & ARA_READ)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  switch (self->status) {
    case ARA_STATUS_OPENED:
      ara_call(self, ARA_READ, req, &on_done);
	  break;

    default:
      ara_throw(self, ARA_EBADSTATE);
      on_done(req);
  }
  return;
}

ARAboolean
ara_read(ara_t *self, ara_async_data_t *data, ara_cb *callback) {
  ara_async_req_t *req = 0;
  ara_async_data_t empty = {0};

  if (0 == data) {
    ara_async_data_init(&empty);
    data = &empty;
  }

  if (0 == callback) {
    data->callback = ara_noop_cb;
  } else {
    data->callback = callback;
  }

  if (self) {
    switch (self->status) {
      case ARA_STATUS_OPENED:
        OP(self, ARA_READ, req, data, on_async_begin, on_async_end);
        break;

      default: OP_THROW(self, ARA_EBADSTATE);
    }
  }

  return ARA_FALSE;
}
