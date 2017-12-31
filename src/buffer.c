#include <ara/ara.h>
#include <stdlib.h>
#include <string.h>

ara_buffer_t *
ara_buffer_new(const ARAuint64 length) {
  ara_buffer_t *self = 0;
  if (length) {
    self = (ara_buffer_t *) malloc(sizeof(ara_buffer_t));
    memset(self, 0, sizeof(ara_buffer_t));
    if (ARA_FALSE == ara_buffer_init(self, length)) {
      free(self);
      self = 0;
    } else {
      self->alloc = self;
    }
  }
  return self;
}

ARAboolean
ara_buffer_init(ara_buffer_t *self, const ARAuint64 length) {
  if (0 == self) { return ARA_FALSE; }

  self->alloc = 0;
  ara_clear_error(&self->error);

  if (length) {
    self->data.base = (ARAvoid *) malloc(length);
    self->data.length = length;
  } else {
    self->data.base = 0;
    self->data.length = 0;
  }

  if (0 == self->data.base) {
    return ARA_FALSE;
  }

  return ARA_TRUE;
}

ARAboolean
ara_buffer_realloc(ara_buffer_t *self, const ARAuint64 length) {
  if (0 == self) {
    return ARA_FALSE;
  }

  if (length) {
    if (self->data.base && self->data.length) {
      ARAvoid *ptr = (ARAvoid *) realloc(self->data.base, length);
      if (ptr) {
        self->data.base = ptr;
        self->data.length = length;
      } else {
        self->data.base = 0;
        self->data.length = 0;
        return ARA_FALSE;
      }
    } else {
      self->data.base = (ARAvoid *) malloc(length);
    }
  } else if (self->data.base) {
    free(self->data.base);
    self->data.length = 0;
    self->data.base = 0;
  }
  return ARA_TRUE;
}

ARAvoid
ara_buffer_destroy(ara_buffer_t *self) {
  if (0 == self) { return; }
  if (self->data.base) {
    free(self->data.base);
    self->data.base = 0;
  }
  if (self == self->alloc) {
    self->alloc = 0;
    free(self);
    self = 0;
  }
}
