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

typedef enum ara_error_code ara_error_code_t;
typedef enum ara_status ara_status_t;
typedef enum ara_work ara_work_t;

typedef ARAvoid * ara_work_cb;

typedef void (ara_work_done)(ara_t *ara);

typedef void (ara_open_work)(ara_t *ara, ara_work_done *done);
typedef void (ara_open_work_cb)(ara_t *ara);

typedef void (ara_close_work)(ara_t *ara, ara_work_done *done);
typedef void (ara_close_work_cb)(ara_t *ara);

typedef void (ara_end_work)(ara_t *ara, ara_work_done *done);
typedef void (ara_end_work_cb)(ara_t *ara);

typedef void (ara_read_work)(ara_t *ara, ara_work_done *done);
typedef void (ara_read_work_cb)(ara_t *ara);

typedef void (ara_write_work)(ara_t *ara, ara_work_done *done);
typedef void (ara_write_work_cb)(ara_t *ara);

typedef void (ara_unlink_work)(ara_t *ara, ara_work_done *done);
typedef void (ara_unlink_work_cb)(ara_t *ara);

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
  X(ENDING),
  X(ENDED),
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
  E(NONE) = ARA_MAX_ENUM

#undef E
};

struct ara_error {
  ara_error_code_t code;
  ARAvoid *data;
};

struct ara {
  ara_error_t error;
  ara_status_t status;

  uv_loop_t *loop;

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

  struct ara_callbacks {
#define X(which) struct {                            \
  ARAuint length;                                    \
  ara_##which##_work_cb *entries[ARA_MAX_CALLBACKS]; \
} which;

    X(open)
    X(close)
    X(end)
    X(read)
    X(write)
    X(unlink)

#undef X
  } callbacks;

  struct ara_uv_async {
#define X(which) uv_async_t which;

    X(open)
    X(close)
    X(end)
    X(read)
    X(write)
    X(unlink)

#undef X
  } async;
};

ARA_EXPORT ARAboolean
ara_init(ara_t *self);

ARA_EXPORT ARAboolean
ara_set(ara_t *ara, ara_work_t type, ara_work_cb *cb);

ARA_EXPORT ARAboolean
ara_set_loop(ara_t *ara, uv_loop_t *loop);

ARA_EXPORT ARAboolean
ara_set_error(ara_t *ara, ara_error_code_t code, ARAvoid *data);

ARA_EXPORT ARAboolean
ara_clear_error(ara_t *self);

ARA_EXPORT ARAchar *
ara_error(ara_error_code_t code);

ARA_EXPORT ARAboolean
ara_throw(ara_t *ara, ara_error_code_t code);

ARA_EXPORT ARAboolean
ara_open(ara_t *ara, ara_open_work_cb *cb);

ARA_EXPORT ARAboolean
ara_close(ara_t *ara, ara_close_work_cb *cb);

ARA_EXPORT ARAboolean
ara_end(ara_t *ara, ara_end_work_cb *cb);

#if defined(__cplusplus)
}
#endif

#endif
