#include <ara/ara.h>
#include <assert.h>
#include "util.h"
#include "raf.h"

ARAboolean
raf_init(RandomAccessFile *self, const ARAchar *filename) {
  RAF_DEBUG_INIT(init, "raf:init.c");
  RAF_DEBUG_INIT(open, "raf:open.c");
  RAF_DEBUG_INIT(close, "raf:close.c");
  RAF_DEBUG_INIT(end, "raf:end.c");
  RAF_DEBUG_INIT(read, "raf:read.c");
  RAF_DEBUG_INIT(write, "raf:write.c");
  RAF_DEBUG_INIT(unlink, "raf:unlink.c");

  D(init, "raf_init(): filename=%s", filename);

  panic(0 != self, "Out of memory.");
  panic(0 != filename, "Missing filename.");

  assert(memset(self, 0, sizeof(RandomAccessFile)));
  assert(ARA_TRUE == ara_init(&self->ara));

  self->filename = filename;
  self->readable = ARA_FALSE;
  self->writable = ARA_FALSE;
  self->opened = ARA_FALSE;
  self->atime = 0;
  self->mtime = 0;
  self->size = 0;
  self->fd = -1;

  return ARA_TRUE;
}
