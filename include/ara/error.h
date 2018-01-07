#ifndef ARA_ERROR_H
#define ARA_ERROR_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "common.h"

typedef struct ara_error ara_error_t;
typedef enum ara_error_code ara_error_code_t;

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

ARA_EXPORT ARAboolean
ara_set_error(ara_error_t *err, ara_error_code_t code, ARAvoid *data);

ARA_EXPORT ARAboolean
ara_clear_error(ara_error_t *err);

ARA_EXPORT ARAchar *
ara_strerror(ara_error_code_t code);

#if defined(__cplusplus)
}
#endif

#endif
