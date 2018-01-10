#ifndef ARA_BUFFER_H
#define ARA_BUFFER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <uv.h>

#include "common.h"
#include "error.h"

typedef struct ara_buffer ara_buffer_t;

struct ara_buffer {
  struct ara_buffer_data {
    ARAvoid *base;
    ARAuint64 length;
    uv_buf_t buffer;
    uv_buf_t buffer_mut;
  } data;

  ARAuint written;
  ara_error_t error;
  ARAvoid *alloc;
};

ARA_EXPORT ara_buffer_t *
ara_buffer_new(const ARAuint64 length);

ARA_EXPORT ARAboolean
ara_buffer_init(ara_buffer_t *buffer, const ARAuint64 length);

ARA_EXPORT ARAvoid
ara_buffer_destroy(ara_buffer_t *buffer);

ARA_EXPORT ARAuint
ara_buffer_read(ara_buffer_t *buffer, ARAuint offset, ARAuint length, ARAvoid *out);

ARA_EXPORT ARAuint
ara_buffer_write(ara_buffer_t *buffer, ARAuint offset, ARAuint length, ARAvoid *data);

#if defined(__cplusplus)
}
#endif

#endif
