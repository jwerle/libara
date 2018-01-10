#ifndef ARA_H
#define ARA_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <uv.h>

#include "async.h"
#include "buffer.h"
#include "common.h"
#include "error.h"
#include "noop.h"
#include "work.h"

typedef struct ara ara_t;

typedef enum ara_status ara_status_t;

enum ara_status {
#define STATUS(which) ARA_STATUS_##which

  STATUS(UNKNOWN) = 0,
  STATUS(INIT),
  STATUS(OPENING),
  STATUS(OPENED),
  STATUS(CLOSING),
  STATUS(CLOSED),

  STATUS(NONE) = ARA_MAX_ENUM

#undef STATUS
};

struct ara {
  uv_loop_t *loop;

  ara_error_t error;
  ara_status_t status;

  ara_work_cb *work[ARA_MAX_WORK];

  struct { ARAbitfield work; } bitfield;
};

// core
ARA_EXPORT ARAboolean
ara_init(ara_t *self);

ARA_EXPORT ARAboolean
ara_set(ara_t *ara, ara_work_t type, ara_work_cb *cb);

ARA_EXPORT ARAboolean
ara_set_loop(ara_t *ara, uv_loop_t *loop);

ARA_EXPORT ARAboolean
ara_throw(ara_t *ara, ara_error_code_t code);

ARA_EXPORT ARAboolean
ara_call(ara_t *ara, ara_work_t type, ara_async_req_t *req, ara_done_cb *done);

ARA_EXPORT ARAchar *
ara_status_string(ara_t *ara);

ARA_EXPORT ARAchar *
ara_strstatus(ara_status_t status);

// api
ARA_EXPORT ARAboolean
ara_open(ara_t *ara, ara_async_data_t *data, ara_cb *cb);

ARA_EXPORT ARAboolean
ara_close(ara_t *ara, ara_async_data_t *data, ara_cb *cb);

ARA_EXPORT ARAboolean
ara_end(ara_t *ara, ara_async_data_t *data, ara_cb *cb);

ARA_EXPORT ARAboolean
ara_read(ara_t *ara, ara_async_data_t *data, ara_cb *cb);

ARA_EXPORT ARAboolean
ara_write(ara_t *ara, ara_async_data_t *data, ara_cb *cb);

ARA_EXPORT ARAboolean
ara_unlink(ara_t *ara, ara_async_data_t *data, ara_cb *cb);

#if defined(__cplusplus)
}
#endif

#endif
