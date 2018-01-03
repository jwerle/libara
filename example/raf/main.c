#include <clamp/clamp.h>
#include <flag/flag.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <uv.h>

#define D(...) RAF_DEBUG(main, __VA_ARGS__)

#include "util.h"
#include "raf.h"

#define PROGRAM "raf"
#define MAX_READERS 128

static RandomAccessFile *file = 0;
static ARAboolean error = ARA_FALSE;
static uv_loop_t *loop = 0;

static flagset_t flagsetref = {0};
static flagset_t *flagset = (flagset_t *) &flagsetref;

static struct {
  const char *filename;
  ARAuint offset;
  ARAuint length;
} opts = {
  .filename = 0,
  .offset = 0,
  .length = BUFSIZ,
};

static struct {
  ARAuint read;
} worker_index = {
  .read = 0,
};

static ara_buffer_t buffer = {0};

static inline RandomAccessFileReadOptions *
readopts_read_next() {
  static RandomAccessFileReadOptions readopts_stack[MAX_READERS] = {0};
  return &readopts_stack[ worker_index.read++ % MAX_READERS ];
}

ARAvoid
onclose(RandomAccessFile *self, RandomAccessFileRequest *req) {
  D("onclose()");
  if (self->fd > -1) {
    D("onclose(): filename=%s fd=%d", self->filename, self->fd);
  } else {
    error = ARA_TRUE;
  }
}

ARAvoid
onread(RandomAccessFile *self, RandomAccessFileRequest *req) {
  D("onread()");
  RandomAccessFileReadOptions *opts = req->opts;

  if (self->fd > -1) {
    if (req->buffer) {
      D("onread(): fd=%d filename=%s size=%d offset=%d length=%d written=%d",
          self->fd, self->filename, self->size, opts->offset, opts->length, buffer.written);

      ara_buffer_write(&buffer, opts->offset, opts->length, req->buffer->data.base);

      printf("size=%d written=%d B=%d\n",
          self->size, buffer.written, self->size == buffer.written);

      if (self->size == buffer.written) {
        printf("BUF = %s\n", req->buffer->data.base);
        ARAchar out[self->size];
        ara_buffer_read(&buffer, 0, self->size, out);
        //printf("out=%s\n", out);
      }
    }
  } else {
    error = ARA_TRUE;
  }
}

ARAvoid
onopen(RandomAccessFile *self, RandomAccessFileRequest *req) {
  D("onopen()");
  ARAuint max = MAX_READERS;

  if (self->fd > -1) {
    D("onopen(): fd=%d filename=%s size=%d",
        self->fd, self->filename, self->size);

    if (MAX_READERS >= self->size){
      max = self->size;
    }

    D("onopen(): readers=%d", max);
    ara_buffer_init(&buffer, self->size);

    for (int i = 0; i < max; ++i) {
      RandomAccessFileReadOptions *readopts = readopts_read_next();

      readopts->length = npot(ceil((double) self->size / max));
      readopts->offset = i * readopts->length;

      D("onopen(): raf_read(): offset=%d length=%d", readopts->offset, readopts->length);
      assert(ARA_TRUE == raf_read(self, readopts, onread));
    }
  } else {
    error = ARA_TRUE;
    D("onopen(): error: '%s' does not exist", self->filename);
  }
}

static void
print_usage() {
  flagset->usage = "[options]";
  flagset_write_usage(flagset, stdout, PROGRAM);
}

static void
parse_arguments(int argc, const char **argv) {
  D("parse_arguments(): argc=%d ", argc);

  bool help = false;
  short did_error = 0;

  opts.filename = "";

  D("parse_arguments(): flagset_string(): --file");
  flagset_string(flagset, &opts.filename, "file", "File path");

  D("parse_arguments(): flagset_int(): --offset");
  flagset_int(flagset, &opts.offset, "offset", "Range buffer offset");

  D("parse_arguments(): flagset_int(): --length");
  flagset_int(flagset, &opts.length, "length", "Range buffer length");

  D("parse_arguments(): flagset_bool(): --help");
  flagset_bool(flagset, &help, "help", "Output this message");

  D("parse_arguments(): flagset_parse(): argc=%d", argc-1);
  flag_error err = flagset_parse(flagset, argc-1, argv+1);

  if (help) {
    D("parse_arguments(): print_usage(): --help");
    print_usage();
    exit(0);
  }

  switch (err) {
    case FLAG_ERROR_PARSING:
      error("Invalid value for option `--%s'.", flagset->error.flag->name);
      did_error = 1;
      break;

    case FLAG_ERROR_ARG_MISSING:
      error("Missing value for option `--%s'.", flagset->error.flag->name);
      did_error = 1;
      break;

    case FLAG_ERROR_UNDEFINED_FLAG:
      error("Unknown option `%s'.", flagset->error.arg);
      did_error = 1;
      break;

    case FLAG_OK: break;
  }

  if (did_error) {
    D("parse_arguments(): print_usage(): error=true");
    print_usage();
    exit(1);
  }
}

int
main(int argc, const char **argv) {
  RAF_DEBUG_INIT(main, "raf:main.c");

  D("main(): parse_arguments(): argc=%d", argc);
  parse_arguments(argc, argv);

  if (0 == opts.filename || 0 == strlen(opts.filename)) {
    error("Missing filename. Set with `--file <file>'.");
    print_usage();
    return 1;
  }

  D("main(): uv: loop=uv_default_loop()");
  assert(loop = uv_default_loop());

  D("main(): raf(): filename=%s", opts.filename);
  assert(file = raf(opts.filename));

  D("main(): raf_open(): mode=RAF_MODE_READ_ONLY)");
  assert(ARA_TRUE == raf_open(file, RAF_MODE_READ_ONLY, onopen));

  D("main(): uv_run() mode=UV_RUN_DEFAULT");
  uv_run(loop, UV_RUN_DEFAULT);

  D("main(): uv_loop_close()");
  uv_loop_close(loop);

  return ARA_TRUE == error ? 1 : 0;
}
