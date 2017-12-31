#include <debug/debug.h>
#include <ara/ara.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <uv.h>

#include "raf.h"

RandomAccessFileDebuggers __RandomAccessFileDebuggers = {0};

#define SS(x) #x
#define S(x) SS(x)
#define D(...) RAF_DEBUG(__VA_ARGS__)

#define make(T) (T *) malloc(sizeof(T));

#define panic(expr, message, ...)             \
  if (0 == (!! (expr))) {                     \
    fprintf(stderr,                           \
      "[panic] %s(%s:%d): '%s' failed. %s\n", \
      __FUNCTION__, __FILE__, __LINE__,       \
      S(expr), message __VA_ARGS__);          \
    exit(1);                                  \
  }

static ARAvoid
on_uv_fs_done(uv_fs_t *fs) {
  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  ara_open_work_done *done = (ara_open_work_done *) rafreq->done;
  done(req->ara, req);
}

static ARAvoid
on_uv_fs_stat(uv_fs_t *fs) {
  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  RandomAccessFile *raf = rafreq->raf;
  raf->fd = fs->result;
  raf->size = 0;
  if (raf->fd > -1) {
    raf->size = fs->statbuf.st_size;
  }
  on_uv_fs_done(fs);
}

static ARAvoid
on_uv_fs_open(uv_fs_t *fs) {
  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  RandomAccessFile *raf = rafreq->raf;
  raf->fd = fs->result;
  if (raf->fd > -1) {
    uv_fs_req_cleanup(fs);
    uv_fs_fstat(req->ara->loop, fs, raf->fd, on_uv_fs_stat);
  } else {
    on_uv_fs_done(fs);
  }
}

static ARAvoid
ara_work_open(ara_t *ara, ara_async_req_t *req, ara_open_work_done *done) {
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  int flags = 0;

  switch (rafreq->mode) {
    case RAF_MODE_READ_WRITE: flags = UV_FS_O_RDWR; break;
    case RAF_MODE_WRITE_ONLY: flags = UV_FS_O_WRONLY; break;
    case RAF_MODE_READ_ONLY: flags = UV_FS_O_RDONLY; break;
    case RAF_MODE_NONE: flags = 0; break;
  }

  rafreq->fs.data = req;
  rafreq->done = done;
  uv_fs_open(ara->loop,
             (uv_fs_t *) rafreq,
             rafreq->raf->filename,
             flags, 0, on_uv_fs_open);
}

static ARAvoid
on_ara_open(ara_t *ara, ara_async_res_t *res) {
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) res->req->data.data;
  if (rafreq) {
    RandomAccessFileOpenCallback *callback = rafreq->callback;

    if (callback) {
      callback(rafreq->raf, rafreq);
    }

    free(rafreq);
    rafreq = 0;
  }
}

ARAboolean
raf_init(RandomAccessFile *self, const ARAchar *filename) {
  RAF_DEBUG_INIT(init, "raf:init");

  panic(0 != self, "Out of memory.");
  panic(0 != filename, "Missing filename.");

  memset(self, 0, sizeof(RandomAccessFile));

  assert(ARA_TRUE == ara_init(&self->ara));
  assert(ARA_TRUE == ara_set(&self->ara, ARA_WORK_OPEN, (ara_worker_cb *) ara_work_open));

  D(init, "init(): filename=%s", filename);
  self->filename = filename;
  return ARA_TRUE;
}

ARAboolean
raf_open(RandomAccessFile *self,
         const RandomAccessFileMode mode,
         RandomAccessFileOpenCallback *callback)
{
  static ara_async_data_t data = {0};

  panic(0 != self, "Out of memory.");

  RandomAccessFileRequest *req = make(RandomAccessFileRequest);

  panic(0 != req, "Out of memory.");

  req->raf = self;
  req->callback = callback;

  data.data = req;

  panic(ARA_TRUE == ara_open(&self->ara, &data, on_ara_open),
        ara_error(self->ara.error.code));

  return ARA_TRUE;
}
