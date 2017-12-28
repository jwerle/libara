#include <ara/ara.h>

ARAboolean
ara_buffer_init(ara_buffer_t *self, ARAuint length) {
  if (0 == self) { return ARA_FALSE; }
  ara_clear_error(&self->error);
  return ARA_TRUE;
}
