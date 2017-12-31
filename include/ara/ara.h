#ifndef ARA_H
#define ARA_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <uv.h>
#include "common.h"

#define ARA_MAX_CALLBACKS 32

typedef struct ara ara_t;
typedef struct ara_error ara_error_t;
typedef struct ara_buffer ara_buffer_t;
typedef struct ara_async_data ara_async_data_t;
typedef struct ara_async_req ara_async_req_t;
typedef struct ara_async_res ara_async_res_t;

typedef enum ara_error_code ara_error_code_t;
typedef enum ara_status ara_status_t;
typedef enum ara_work ara_work_t;

typedef ARAvoid * ara_worker_cb;

typedef ARAvoid (ara_async_req_cb)(ara_t *ara, ara_async_req_t *req);
typedef ARAvoid (ara_async_res_cb)(ara_t *ara, ara_async_res_t *res);

typedef ARAvoid (ara_work_done)(ara_t *ara, ara_async_req_t *req);
typedef ARAvoid (ara_work_cb)(ara_t *ara, ara_async_res_t *res);

typedef ARAvoid (ara_open_work_done)(ara_t *ara, ara_async_req_t *req);
typedef ARAvoid (ara_open_work)(ara_t *ara, ara_async_req_t *req, ara_work_done *done);
typedef ARAvoid (ara_open_cb)(ara_t *ara, ara_async_res_t *res);

typedef ARAvoid (ara_close_work_done)(ara_t *ara, ara_async_req_t *req);
typedef ARAvoid (ara_close_work)(ara_t *ara, ara_async_req_t *req, ara_work_done *done);
typedef ARAvoid (ara_close_cb)(ara_t *ara, ara_async_res_t *res);

typedef ARAvoid (ara_end_work_done)(ara_t *ara,  ara_async_req_t *req);
typedef ARAvoid (ara_end_work)(ara_t *ara, ara_async_req_t *req, ara_work_done *done);
typedef ARAvoid (ara_end_cb)(ara_t *ara, ara_async_res_t *res);

typedef ARAvoid (ara_read_work_done)(ara_t *ara, ara_async_req_t *req);
typedef ARAvoid (ara_read_work)(ara_t *ara, ara_async_req_t *req, ara_work_done *done);
typedef ARAvoid (ara_read_cb)(ara_t *ara, ara_async_res_t *res);

typedef ARAvoid (ara_write_work_done)(ara_t *ara, ara_async_req_t *req);
typedef ARAvoid (ara_write_work)(ara_t *ara, ara_async_req_t *req, ara_work_done *done);
typedef ARAvoid (ara_write_cb)(ara_t *ara, ara_async_res_t *res);

typedef ARAvoid (ara_unlink_work_done)(ara_t *ara, ara_async_req_t *req);
typedef ARAvoid (ara_unlink_work)(ara_t *ara, ara_async_req_t *req, ara_work_done *done);
typedef ARAvoid (ara_unlink_cb)(ara_t *ara, ara_async_res_t *res);

enum ara_work {
#define X(which) ARA_WORK_##which

  X(UNKNOWN) = 0,
  X(OPEN),
  X(CLOSE),
  X(END),
  X(READ),
  X(WRITE),
  X(UNLINK),
  X(NONE) = ARA_MAX_ENUM

#undef X
};

enum ara_status {
#define X(which) ARA_STATUS_##which

  X(UNKNOWN) = 0,
  X(INIT),
  X(OPENING),
  X(OPENED),
  X(CLOSING),
  X(CLOSED),
  X(NONE) = ARA_MAX_ENUM

#undef X
};

enum ara_error_code {
#define E(type) ARA_E##type

  E(UNKNOWN) = 0,
  E(NOMEM),
  E(NOCALLBACK),
  E(NOUVLOOP),
  E(UVASYNCINIT),
  E(UVASYNCSEND),
  E(BADSTATE),
  E(BADDATA),
  E(ACCESS),
  E(NONE) = ARA_MAX_ENUM

#undef E
};

struct ara_error {
  ara_error_code_t code;
  ARAvoid *data;
};

struct ara_buffer {
  struct ara_buffer_data {
    ARAvoid *base;
    ARAuint64 length;
    uv_buf_t buffer;
    uv_buf_t buffer_mut;
  } data;

  ara_error_t error;
  ARAvoid *alloc;
};

struct ara_async_data {
  ARAvoid *data;
  ARAuint64 offset;
  ARAuint64 length;
  ARAvoid *alloc;
  ara_work_cb *callback;
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

#define X(which) ara_##which##_work *which;

  X(open);
  X(close);
  X(end);
  X(read);
  X(write);
  X(unlink);

#undef X

  struct {
    ARAbitfield work;
  } bitfield;
};

// core
ARA_EXPORT ARAboolean
ara_init(ara_t *self);

ARA_EXPORT ARAboolean
ara_set(ara_t *ara, ara_work_t type, ara_worker_cb *cb);

ARA_EXPORT ARAboolean
ara_set_loop(ara_t *ara, uv_loop_t *loop);

ARA_EXPORT ARAboolean
ara_throw(ara_t *ara, ara_error_code_t code);

// error
ARA_EXPORT ARAboolean
ara_set_error(ara_error_t *err, ara_error_code_t code, ARAvoid *data);

ARA_EXPORT ARAboolean
ara_clear_error(ara_error_t *err);

ARA_EXPORT ARAchar *
ara_error(ara_error_code_t code);

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

// api
ARA_EXPORT ARAboolean
ara_open(ara_t *ara, ara_async_data_t *data, ara_open_cb *cb);

ARA_EXPORT ARAboolean
ara_close(ara_t *ara, ara_async_data_t *data, ara_close_cb *cb);

ARA_EXPORT ARAboolean
ara_end(ara_t *ara, ara_async_data_t *data, ara_end_cb *cb);

ARA_EXPORT ARAboolean
ara_read(ara_t *ara, ara_async_data_t *data, ara_read_cb *cb);

// @TODO ara_write

ARA_EXPORT ARAboolean
ara_unlink(ara_t *ara, ara_async_data_t *data, ara_unlink_cb *cb);

#if defined(__cplusplus)
}
#endif

#endif
