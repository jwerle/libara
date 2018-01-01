#include <ara/ara.h>
#include <uv.h>

#include "util.h"
#include "raf.h"
#include "uv.h"

static ARAvoid
on_uv_fs_done(uv_fs_t *fs) {
  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  ara_open_work_done *done = (ara_open_work_done *) rafreq->done;
  RandomAccessFile *raf = rafreq->raf;

  panic(fs->result > -1, uv_strerror(fs->result));
  uv_fs_req_cleanup(fs);
  done(req->ara, req);
}

static ARAvoid
on_uv_fs_stat(uv_fs_t *fs) {
  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  RandomAccessFile *raf = rafreq->raf;

  raf->fd = fs->result;
  raf->size = 0;
  raf->atime = 0;
  raf->mtime = 0;

  if (raf->fd > -1) {
    raf->size = fs->statbuf.st_size;
    raf->atime = fs->statbuf.st_atim.tv_sec;
    raf->mtime = fs->statbuf.st_mtim.tv_sec;
  }

  on_uv_fs_done(fs);
}

static ARAvoid
on_ara_open(ara_t *ara, ara_async_res_t *res) {
  ON_ARA_WORK_DONE(ara, res, RandomAccessFileOpenCallback);
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

ARAboolean
raf_open(RandomAccessFile *self,
         const RandomAccessFileMode mode,
         RandomAccessFileOpenCallback *callback) {
  static ara_async_data_t data = {0};
  RandomAccessFileRequest *req = 0;

  panic(self, "Out of memory.");
  panic((req = make(RandomAccessFileRequest)), "Out of memory.");

  req->callback = callback;
  req->raf = self;

  data.data = req;

  panic(ara_set(&self->ara, ARA_WORK_OPEN, (ara_worker_cb *) ara_work_open),
        ara_error(self->ara.error.code));

  panic(ara_open(&self->ara, &data, on_ara_open),
        ara_error(self->ara.error.code));

  return ARA_TRUE;
}
