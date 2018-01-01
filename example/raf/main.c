#include <assert.h>
#include <uv.h>

#define D(...) RAF_DEBUG(main, __VA_ARGS__)

#include "util.h"
#include "raf.h"

RandomAccessFile *file = 0;
uv_loop_t *loop = 0;

static ARAboolean error = ARA_FALSE;

ARAvoid
onclose(RandomAccessFile *self, RandomAccessFileRequest *req) {
  if (self->fd > -1) {
    D("onclose(%s): fd=%d", self->filename, self->fd);
  } else {
    error = ARA_TRUE;
  }
}

ARAvoid
onread(RandomAccessFile *self, RandomAccessFileRequest *req) {
  if (self->fd > -1) {
    D("onread(%s): fd=%d", self->filename, self->fd);
  } else {
    error = ARA_TRUE;
  }
}

ARAvoid
onopen(RandomAccessFile *self, RandomAccessFileRequest *req) {
  if (self->fd > -1) {
    D("onopen(%s): fd=%d", self->filename, self->fd);
    assert(ARA_TRUE == raf_close(self, onclose));
  } else {
    error = ARA_TRUE;
    D("onopen(%s): does not exist", self->filename);
  }
}

int
main(int argc, const char **argv) {
  const char *filename = argv[1];

  D("main(): argc=%d", argc);
  RAF_DEBUG_INIT(main, "raf:main.c");

  D("main(): uv loop=uv_default_loop()");
  assert(loop = uv_default_loop());

  D("main(): raf(filename=%s)", filename);
  assert(file = raf(filename));

  D("main(): open(RAF_MODE_READ_ONLY)");
  assert(ARA_TRUE == raf_open(file, RAF_MODE_READ_ONLY, onopen));

  D("main(): uv run=UV_RUN_DEFAULT");
  uv_run(loop, UV_RUN_DEFAULT);

  D("main(): uv close loop");
  uv_loop_close(loop);

  return ARA_TRUE == error ? 1 : 0;
}
