#include <ara/ara.h>
#include <stdlib.h>

#include "async.h"
#include "write.h"
#include "work.h"

static ARAvoid
write_work_noop(ara_t *self, ara_async_res_t *res) {
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

  if (ARA_STATUS_OPENED != self->status) {
    ara_throw(self, ARA_EBADSTATE);
    goto end;
  }

  ara_open_cb *cb = (ara_open_cb *) req->data.callback;

  if (cb) {
    cb(self, &req->res);
  }

end:
  ara_async_req_end(req);
}

static ARAvoid
on_async_begin(ara_t *self, ara_async_req_t *req) {
  if (0 == self) {
    return;
  }

  if (0 == (self->bitfield.work & ARA_WORK_WRITE)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  switch (self->status) {
    case ARA_STATUS_OPENED:
      return self->write(self, req, &on_done);

    default:
      ara_throw(self, ARA_EBADSTATE);
      return on_done(self, req);
  }
}

ARAboolean
ara_write(ara_t *self, ara_async_data_t *data, ara_write_cb *cb) {
  ara_async_req_t *req = 0;
  ara_async_data_t empty = {0};

  if (0 == data) {
    ara_async_data_init(&empty);
    data = &empty;
  }

  if (0 == cb) {
    cb = write_work_noop;
  }

  if (self) {
    switch (self->status) {
      case ARA_STATUS_OPENED: break;
      default: WORK_THROW(self, ARA_EBADSTATE);
    }
  }

  data->callback = cb;

  WORK(self, ARA_WORK_WRITE, req, data, on_async_begin, on_async_end);
}

ara_async_write_data_t *
ara_async_write_data_new(ara_t *ara, ara_async_data_t *data, ara_write_cb *cb) {
  ara_async_write_data_t *self= 0;
  self = (ara_async_write_data_t *) malloc(sizeof(ara_async_write_data_t));
  self->ara = ara;
  self->data = data;
  return self;
}

ARAvoid
ara_async_write_data_destroy(ara_async_write_data_t *self) {
  if (self) {
    if (self->data) {
      ara_async_data_destroy(self->data);
    }
    self->data = 0;
    self->ara = 0;
    free(self);
    self = 0;
  }
}
