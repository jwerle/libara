#include <ara/ara.h>
#include <stdlib.h>

#include "async.h"
#include "read.h"

static ARAvoid
read_work_noop(ara_t *self, ara_buffer_t *buffer) {
  (ARAvoid) (self); (ARAvoid) (buffer);
}

static ARAvoid
on_ara_read_work_done(ara_t *self, ara_buffer_t *buffer) {
  ara_buffer_destroy(buffer);
}

static ara_async_read_data_t *
ara_async_read_data_new(ara_t *ara, ara_async_data_t *data) {
  ara_async_read_data_t *read_data = 0;
  read_data = (ara_async_read_data_t *) malloc(sizeof(ara_async_read_data_t));
  read_data->ara = ara;
  read_data->data = data;
  return read_data;
}

static ARAvoid
ara_async_read_data_destroy(ara_async_read_data_t *read_data) {
  if (read_data) {
    if (read_data->data) {
      ara_async_data_destroy(read_data->data);
    }
    read_data->data = 0;
    read_data->ara = 0;
    free(read_data);
    read_data = 0;
  }
}

ARAvoid
onasyncread(uv_async_t* handle) {
  ara_async_read_data_t *read_data = (ara_async_read_data_t *) handle->data;
  ara_async_data_t *data = read_data->data;
  ara_t *self = read_data->ara;

  const ARAuint64 offset = data->offset;
  const ARAuint64 length = data->length;

  ara_buffer_t buffer = {0};

  if (0 == self) { return; }
  if (0 == (self->bitfield.work & ARA_WORK_READ)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  if (ARA_TRUE == ara_buffer_init(&buffer, length)) {
    self->read(self, offset, length, &buffer, on_ara_read_work_done);
  }
}

ARAboolean
ara_read(ara_t *self, const ARAuint64 offset, const ARAuint64 length, ara_read_cb *cb) {
  ara_async_data_t *data = 0;

  if (0 == self) { return ARA_FALSE; }
  if (0 == self->loop) { return ara_throw(self, ARA_ENOUVLOOP); }
  if (0 == (self->bitfield.work & ARA_WORK_READ)) {
    return ara_throw(self, ARA_ENOCALLBACK);
  }

  if (0 == cb) {
    cb = read_work_noop;
  }

  switch (self->status) {
    case ARA_STATUS_OPENED: goto opened;
    default: return ara_throw(self, ARA_EBADSTATE);
  }

opened:
  data = ara_async_data_new();
  data->offset = offset;
  data->length = length;
  self->async.read.data = ara_async_read_data_new(self, data);
  if (uv_async_send(&self->async.read) < 0) {
    return ara_throw(self, ARA_EUVASYNCSEND);
  }
  return ARA_TRUE;
}
