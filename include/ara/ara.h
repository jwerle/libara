#ifndef ARA_H
#define ARA_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <uv.h>
#include "common.h"
#include "error.h"

typedef struct ara_async_data ara_async_data_t;
typedef struct ara_async_req ara_async_req_t;
typedef struct ara_async_res ara_async_res_t;
typedef struct ara_buffer ara_buffer_t;
typedef struct ara ara_t;

typedef enum ara_status ara_status_t;
typedef enum ara_work ara_work_t;

typedef ARAvoid (ara_async_req_cb)(ara_async_req_t *req);
typedef ARAvoid (ara_async_res_cb)(ara_async_res_t *res);

typedef ARAvoid (ara_done_cb)(ara_async_req_t *req);
typedef ARAvoid (ara_work_cb)(ara_async_req_t *req, ara_done_cb *done);
typedef ARAvoid (ara_cb)(ara_async_res_t *res);

#define ARA_MAX_WORK 64

enum ara_work {
#define WORK(which) ARA_##which

  WORK(UNKNOWN) = 0,
  WORK(OPEN),
  WORK(CLOSE),
  WORK(END),
  WORK(READ),
  WORK(WRITE),
  WORK(UNLINK),

  WORK(NONE) = ARA_MAX_ENUM

#undef WORK
};

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

struct ara_async_data {
  ARAvoid *data;
  ARAuint64 offset;
  ARAuint64 length;
  ARAvoid *alloc;
  ara_cb *callback;
};

struct ara_async_res {
  uv_async_t handle;
  ara_async_data_t data;
  ara_error_t error;
  ara_t *ara;
  ara_async_res_cb *callback;
  ara_async_req_t *req;
  ara_buffer_t buffer;
  ARAvoid *alloc;
};

struct ara_async_req {
  uv_async_t handle;
  ara_async_data_t data;
  ara_error_t error;
  ara_t *ara;
  ara_async_req_cb *callback;
  ara_async_res_t res;
  ara_buffer_t *buffer;
  ARAvoid *alloc;
};

struct ara {
  uv_loop_t *loop;

  ara_error_t error;
  ara_status_t status;

  ara_work_cb *work[ARA_MAX_WORK];

  struct {
    ARAbitfield work;
  } bitfield;
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

// noop
ARAvoid
ara_noop_done_cb(ara_async_req_t *req);

ARAvoid
ara_noop_work_cb(ara_async_req_t *req, ara_done_cb *done);

ARAvoid
ara_noop_cb(ara_async_res_t *res);

// async
ARA_EXPORT ara_async_data_t *
ara_async_data_new();

ARA_EXPORT ARAboolean
ara_async_data_init();

ARA_EXPORT ARAvoid
ara_async_data_destroy(ara_async_data_t *data);

ARA_EXPORT ara_async_req_t *
ara_async_req_new(ara_t *ara, ara_async_req_cb *onreq, ara_async_res_cb *onres);

ARA_EXPORT ara_async_res_t *
ara_async_res_new(ara_t *ara, ara_async_res_cb *cb);

ARA_EXPORT ARAboolean
ara_async_req_init(ara_async_req_t *req, ara_t *ara, ara_async_req_cb *cb);

ARA_EXPORT ARAboolean
ara_async_res_init(ara_async_res_t *res, ara_t *ara, ara_async_res_cb *cb);

ARA_EXPORT ARAvoid
ara_async_req_destroy(ara_async_req_t *req);

ARA_EXPORT ARAboolean
ara_async_req_send(ara_async_req_t *req, ara_async_data_t *data);

ARA_EXPORT ARAboolean
ara_async_req_end(ara_async_req_t * req);

// buffer
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
