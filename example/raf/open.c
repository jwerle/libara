#include <ara/ara.h>
#include <uv.h>

#include "util.h"
#include "raf.h"

static ARAvoid
on_uv_fs_done(uv_fs_t *fs) {
  D(open, "on_uv_fs_done()");

  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  ara_open_work_done *done = (ara_open_work_done *) rafreq->done;
  RandomAccessFile *raf = rafreq->raf;

  panic(fs->result > -1, "on_uv_fs_done(): uv: error: '%s'", uv_strerror(fs->result));

  D(open, "on_uv_fs_done(): uv_fs_req_cleanup()");
  uv_fs_req_cleanup(fs);

  D(open, "on_uv_fs_done(): done()");
  done(req->ara, req);
}

static ARAvoid
on_uv_fs_stat(uv_fs_t *fs) {
  D(open, "on_uv_fs_stat()");

  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  RandomAccessFile *raf = rafreq->raf;

  raf->size = 0;
  raf->atime = 0;
  raf->mtime = 0;

  if (fs->result > -1) {
    raf->size = fs->statbuf.st_size;
    raf->atime = fs->statbuf.st_atim.tv_sec;
    raf->mtime = fs->statbuf.st_mtim.tv_sec;
    D(open, "on_uv_fs_stat(): fd=%d size=%d atime=%d mtime=%d",
      raf->fd, raf->size, raf->atime, raf->mtime);
  }

  D(open, "on_uv_fs_stat(): on_uv_fs_done()");
  on_uv_fs_done(fs);
}

static ARAvoid
on_ara_open(ara_t *ara, ara_async_res_t *res) {
  D(open, "on_ara_open()");
  ON_ARA_WORK_DONE(ara, res, RandomAccessFileOpenCallback);
}

static ARAvoid
on_uv_fs_open(uv_fs_t *fs) {
  D(open, "on_uv_fs_open()");

  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  RandomAccessFile *raf = rafreq->raf;

  D(open, "on_uv_fs_open(): result=%d", fs->result);
  raf->fd = fs->result;

  if (raf->fd < 0) {
    D(open, "on_uv_fs_open(): on_uv_fs_done()");
    return on_uv_fs_done(fs);
  }

  D(open, "on_uv_fs_done(): mode=%d", rafreq->mode);

  switch (rafreq->mode) {
    case RAF_MODE_READ_WRITE:
      D(open, "on_uv_fs_open(): mode=RAF_MODE_READ_WRITE");
      raf->readable = ARA_TRUE;

    case RAF_MODE_WRITE_ONLY:
      raf->writable = ARA_TRUE;
      break;

    case RAF_MODE_READ_ONLY:
      D(open, "on_uv_fs_open(): mode=RAF_MODE_READ_ONLY");
      raf->writable = ARA_FALSE;
      raf->readable = ARA_TRUE;
      break;

    default:
      panic(RAF_MODE_NONE == rafreq->mode, "Unhandled mode '%d'", rafreq->mode);
  }

  D(open, "on_uv_fs_open(): uv_fs_req_cleanup()");
  uv_fs_req_cleanup(fs);

  D(open, "on_uv_fs_open(): uv_fs_fstat()");
  uv_fs_fstat(req->ara->loop, fs, raf->fd, on_uv_fs_stat);
}

static ARAvoid
ara_work_open(ara_t *ara, ara_async_req_t *req, ara_open_work_done *done) {
  D(open, "ara_work_open()");

  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  int flags = 0;

  switch (rafreq->mode) {
    case RAF_MODE_READ_WRITE: flags = UV_FS_O_RDWR; break;
    case RAF_MODE_WRITE_ONLY: flags = UV_FS_O_WRONLY; break;
    case RAF_MODE_READ_ONLY: flags = UV_FS_O_RDONLY; break;
    case RAF_MODE_NONE: flags = 0; break;
  }

  D(open, "ara_work_open(): mode=%d flags=%d", rafreq->mode, flags);

  rafreq->fs.data = req;
  rafreq->done = done;

  D(open, "ara_work_open(): uv_fs_open(): file=%s", rafreq->raf->filename);
  uv_fs_open(ara->loop,
             (uv_fs_t *) rafreq,
             rafreq->raf->filename,
             flags, 0, on_uv_fs_open);
}

ARAboolean
raf_open(RandomAccessFile *self,
         const RandomAccessFileMode mode,
         RandomAccessFileOpenCallback *callback) {
  D(open, "raf_open()");

  static ara_async_data_t data = {0};
  RandomAccessFileRequest *req = 0;

  panic(self, "Out of memory.");
  panic((req = make(RandomAccessFileRequest)), "Out of memory.");

  D(open, "raf_open(): mode=%d", mode);

  req->callback = callback;
  req->mode = mode;
  req->raf = self;

  data.data = req;

  panic(ara_set(&self->ara, ARA_WORK_OPEN, (ara_worker_cb *) ara_work_open),
        "ara: error: '%s'",
        ara_error(self->ara.error.code));

  panic(ara_open(&self->ara, &data, on_ara_open),
        "ara: error: '%s'",
        ara_error(self->ara.error.code));

  return ARA_TRUE;
}
