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
  if (0 == self) {
    return ARA_FALSE;
  }

  self->alloc = 0;
  self->written = 0;

  ara_clear_error(&self->error);

  if (length) {
    self->data.base = (ARAvoid *) malloc(length);
    self->data.length = length;
    memset(self->data.base, 0, length);
  } else {
    self->data.base = 0;
    self->data.length = 0;
  }

  if (0 == self->data.base) {
    return ARA_FALSE;
  }

  self->data.buffer.base = self->data.base;
  self->data.buffer.len = self->data.length;

  return ARA_TRUE;
}

ARAboolean
ara_buffer_realloc(ara_buffer_t *self, const ARAuint64 length) {
  if (0 == self) {
    return ARA_FALSE;
  }

  if (length) {
    if (self->data.base && self->data.length) {
      ARAchar *region = (ARAchar *) realloc(self->data.base, length);
      if (region) {
        memset(region + length, 0, length);
        self->data.base = region;
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

  self->data.buffer.base = self->data.base;
  self->data.buffer.len = self->data.length;
  return ARA_TRUE;
}

ARAvoid
ara_buffer_destroy(ara_buffer_t *self) {
  if (0 == self) {
    return;
  }

  if (self->data.base) {
    free(self->data.base);
    self->data.base = 0;
    self->data.buffer.base = 0;
    self->data.buffer.len = 0;
    self->data.buffer_mut = self->data.buffer;
  }

  if (self == self->alloc) {
    self->alloc = 0;
    free(self);
    self = 0;
  }
}

ARA_EXPORT ARAuint
ara_buffer_write(ara_buffer_t *self, ARAuint offset, ARAuint length, ARAvoid *data) {
  ARAchar *region = 0;

  if (0 == self) {
    return 0;
  }

  if (0 == self->data.base) {
    if (ARA_FALSE == ara_buffer_init(self, length)) {
      return 0;
    }
  }

  if (offset > self->data.length && length > 0) {
    if (ARA_FALSE == ara_buffer_realloc(self, self->data.length + offset)) {
      return 0;
    }
  }

  region = self->data.base;

  memcpy(region + offset, data, length);

  self->written += length;
  return length;
}

ARA_EXPORT ARAuint
ara_buffer_read(ara_buffer_t *self, ARAuint offset, ARAuint length, ARAvoid *out) {
  if (0 == self || 0 == out) {
    return 0;
  }

  if (offset > self->data.length && length > 0) {
    return 0;
  }

  ARAchar *region = self->data.base;

  if (0 == region) {
    return 0;
  }

  memset(out, 0, length);

  if (0 == memcpy(out, region + offset, length)) {
    return 0;
  }

  return length;
}
