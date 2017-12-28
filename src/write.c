#include <ara/ara.h>
#include "async.h"

static void
write_work_noop(ara_t *ara, ara_error_t *err) {
  (void) (ara); (void) (err);
}

static void
on_ara_write_work_done(ara_t *self) {
}

void onasyncwrite(uv_async_t* handle) {
  ara_t *self = (ara_t *) handle->data;
  if (0 == self) { return; }
  if (0 == (self->bitfield.work & ARA_WORK_WRITE)) {
    ara_throw(self, ARA_ENOCALLBACK);
    return;
  }

  //self->write(self, on_ara_write_work_done);
}
