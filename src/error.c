#include <ara/ara.h>

ARA_EXPORT ARAboolean
ara_set_error(ara_t *self, ara_error_code_t code, ARAvoid *data) {
  if (0 == self) { return ARA_FALSE; }

#define E(which) ARA_E##which
  switch (code) {
    case E(UNKNOWN):
    case E(NOMEM):
    case E(NOCALLBACK):
    case E(NOUVLOOP):
    case E(UVASYNCINIT):
    case E(UVASYNCSEND):
    case E(BADSTATE):
      self->error.code = code;
      break;

    default: return ARA_FALSE;
  }
#undef E

  if (data) {
    self->error.data = data;
  } else {
    self->error.data = 0;
  }

  return ARA_TRUE;
}

ARAchar *
ara_error(ara_error_code_t code) {
#define E(code, message) ARA_E##code: return message;
  switch (code) {
    case E(UNKNOWN, "An unknown error has occurred.");
    case E(NOMEM, "Out of memory.");
    case E(NOCALLBACK, "Missing callback.");
    case E(NOUVLOOP, "Missing uv loop.");
    case E(UVASYNCINIT, "Failed to initialize async handle.");
    case E(UVASYNCSEND, "Failed to trigger async handle.");
    case E(BADSTATE, "Bad state.");
    case E(NONE, "");
    default: return "An error has occurred";
  }
#undef E
}

ARAboolean
ara_clear_error(ara_t *self) {
  if (0 == self) { return ARA_FALSE; }
  self->error.code = ARA_ENONE;
  self->error.data = 0;
  return ARA_TRUE;
}

ARAboolean
ara_throw(ara_t *self, ara_error_code_t code) {
  ara_set_error(self, code, ara_error(code));
  return ARA_FALSE;
}
