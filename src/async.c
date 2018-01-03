#include <ara/ara.h>
#include <stdlib.h>
#include <string.h>
#include "async.h"

static void
on_uv_async_send(uv_async_t *handle) {
  if (0 == handle || 0 == handle->data) {
    return;
  }

  ara_async_req_t *req = (ara_async_req_t *) handle->data;
  ara_t *ara = req->ara;

  if (req->callback) {
    req->callback(ara, req);
  }
}

static ARAvoid
on_uv_close(uv_handle_t *handle) {
  ara_async_req_t *req = (ara_async_req_t *) handle->data;
  ara_async_req_destroy(req);
}

ara_async_data_t *
ara_async_data_new() {
  ara_async_data_t *self = 0;
  self = (ara_async_data_t *) malloc(sizeof(ara_async_data_t));

  if (ARA_FALSE == ara_async_data_init(self)) {
    goto error;
  }

  self->alloc = self;

  return self;

error:

  if (self) {
    ara_async_data_destroy(self);
    self = 0;
  }

  return self;
}

ARAboolean
ara_async_data_init(ara_async_data_t *self) {
  if (0 == self) { return ARA_FALSE; }
  self->offset = 0;
  self->length = 0;
  self->alloc = 0;
  self->data = 0;
  return ARA_TRUE;
}

ARAvoid
ara_async_data_destroy(ara_async_data_t *self) {
  if (0 == self) { return; }
  if (self == self->alloc) {
    self->alloc = 0;
    free(self);
    self = 0;
  }
}

ara_async_req_t *
ara_async_req_new(ara_t *ara, ara_async_req_cb *onreq, ara_async_res_cb *onres) {
  ara_async_req_t *self = 0;
  self = (ara_async_req_t *) malloc(sizeof(ara_async_req_t));

  if (0 == self) {
    goto error;
  }

  if (ARA_FALSE == ara_async_req_init(self, ara, onreq)) {
    goto error;
  }

  ara_async_res_init(&self->res, ara, onres);

  self->alloc = self;
  self->buffer = &self->res.buffer;
  self->res.req = self;

  return self;

error:
  if (self) {
    ara_async_req_destroy(self);
    self = 0;
  }

  return self;
}

ARAboolean
ara_async_req_init(ara_async_req_t *self, ara_t *ara, ara_async_req_cb *cb) {
  if (0 == ara) {
    return ARA_FALSE;
  }

  if (0 == self) {
    ara_throw(ara, ARA_EACCESS);
    return ARA_FALSE;
  }

  ara_clear_error(&self->error);
  ara_async_data_init(&self->data);

  memset(self, 0, sizeof(ara_async_req_t));

  self->ara = ara;
  self->alloc = 0;
  self->callback = cb;
  self->handle.data = self;

  return ARA_TRUE;
}

ARAboolean
ara_async_res_init(ara_async_res_t *self, ara_t *ara, ara_async_res_cb *cb) {
  if (0 == ara) {
    return ARA_FALSE;
  }

  if (0 == self) {
    ara_throw(ara, ARA_EACCESS);
    return ARA_FALSE;
  }

  memset(self, 0, sizeof(ara_async_res_t));

  ara_clear_error(&self->error);
  ara_async_data_init(&self->data);

  self->ara = ara;
  self->alloc = 0;
  self->callback = cb;
  self->handle.data = self;

  return ARA_TRUE;
}

ARAboolean
ara_async_req_send(ara_async_req_t *self, ara_async_data_t *data) {
  if (0 == self) {
    goto error;
  }

  if (0 == self->ara) {
    goto error;
  }

  if (uv_async_init(self->ara->loop, &self->handle, on_uv_async_send) < 0) {
    ara_throw(self->ara, ARA_EUVASYNCINIT);
    goto error;
  }

  self->data.data = data->data;
  self->data.offset = data->offset;
  self->data.length = data->length;
  self->data.callback = data->callback;

  ara_buffer_init(self->buffer, data->length);

  if (uv_async_send(&self->handle) < 0) {
    ara_throw(self->ara, ARA_EUVASYNCSEND);
    goto error;
  }

  return ARA_TRUE;

error:

  return ARA_FALSE;
}

ARAboolean
ara_async_req_end(ara_async_req_t *self) {
  if (0 == self) {
    goto error;
  }

  if (0 != self->res.callback) {
    self->res.callback(self->ara, &self->res);
  }

  uv_close((uv_handle_t *) &self->handle, on_uv_close);

  return ARA_TRUE;

error:

  return ARA_FALSE;
}

ARAvoid
ara_async_req_destroy(ara_async_req_t *self) {
  if (0 != self && self == self->alloc) {
    ara_buffer_destroy(&self->res.buffer);
    self->alloc = 0;
    free(self);
    self = 0;
  }
}
