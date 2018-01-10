#ifndef ARA_WORK_H
#define ARA_WORK_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "async.h"
#include "common.h"

typedef enum ara_work ara_work_t;

typedef ARAvoid (ara_work_cb)(ara_async_req_t *req, ara_done_cb *done);

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

#if defined(__cplusplus)
}
#endif

#endif
