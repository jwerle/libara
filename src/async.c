#include <ara/ara.h>
#include <stdlib.h>
#include "async.h"

ara_async_data_t *
ara_async_data_new() {
  ara_async_data_t *self = 0;
  self = (ara_async_data_t *) malloc(sizeof(ara_async_data_t));
  if (ARA_TRUE == ara_async_data_init(self)) {
    self->alloc = self;
    return self;
  } else if (self) {
    ara_async_data_destroy(self);
  }
  return 0;
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
