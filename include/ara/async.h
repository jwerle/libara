#ifndef ARA_ASYNC_H
#define ARA_ASYNC_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <uv.h>

#include "buffer.h"
#include "common.h"
#include "error.h"

typedef struct ara_buffer ara_buffer_t;
typedef struct ara ara_t;

typedef struct ara_async_data ara_async_data_t;
typedef struct ara_async_req ara_async_req_t;
typedef struct ara_async_res ara_async_res_t;

typedef ARAvoid (ara_cb)(ara_async_res_t *res);
typedef ARAvoid (ara_done_cb)(ara_async_req_t *req);
typedef ARAvoid (ara_async_req_cb)(ara_async_req_t *req);
typedef ARAvoid (ara_async_res_cb)(ara_async_res_t *res);

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

ARA_EXPORT ARAboolean
ara_async_data_init();

ARA_EXPORT ara_async_req_t *
ara_async_req_new(ara_t *ara, ara_async_req_cb *onreq, ara_async_res_cb *onres);

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

#if defined(__cplusplus)
}
#endif

#endif
