#include <ara/ara.h>
#include <uv.h>
#include "uv.h"

static void
read_work_noop(ara_t *self, ara_error_t *err) {
  (void) (self); (void) (err);
}

static void
on_ara_read_work_done(ara_t *self) {
}

void onuvread(uv_async_t* handle) {
  ara_t *self = (ara_t *) handle->data;
  if (0 == self) { return; }
  if (0 == (self->bitfield.work & ARA_WORK_READ)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  self->read(self, on_ara_read_work_done);
}
