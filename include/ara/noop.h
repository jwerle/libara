#ifndef ARA_NOOP_H
#define ARA_NOOP_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "async.h"
#include "common.h"

ARAvoid
ara_noop_done_cb(ara_async_req_t *req);

ARAvoid
ara_noop_work_cb(ara_async_req_t *req, ara_done_cb *done);

ARAvoid
ara_noop_cb(ara_async_res_t *res);

#if defined(__cplusplus)
}
#endif

#endif
