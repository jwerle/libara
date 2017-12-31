#include <assert.h>
#include "raf.h"
#include <uv.h>

#define D(...) RAF_DEBUG(main, __VA_ARGS__)

RandomAccessFile *file = 0;
uv_loop_t *loop = 0;

ARAvoid
onopen(RandomAccessFile *self, RandomAccessFileRequest *req) {
  if (self->fd > -1) {
    D("onopen(%s): fd=%d", self->filename, self->fd);
  } else {
    D("onopen(%s): does not exist", self->filename);
  }
}

int
main(int argc, const char **argv) {
  const char *filename = argv[1];

  RAF_DEBUG_INIT(main, "raf:main.c");
  D("main(): argc=%d", argc);

  D("main(): uv loop=uv_default_loop()");
  loop = uv_default_loop();

  D("main(): raf(filename=%s)", filename);
  file = raf(filename);

  D("main(): open(RAF_MODE_READ_ONLY)");
  assert(ARA_TRUE == raf_open(file, RAF_MODE_READ_ONLY, onopen));

  D("main(): uv run=UV_RUN_DEFAULT");
  uv_run(loop, UV_RUN_DEFAULT);

  D("main(): uv close loop");
  uv_loop_close(loop);
  return 0;
}
