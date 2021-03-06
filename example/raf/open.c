#include <ara/ara.h>
#include <uv.h>

#include "util.h"
#include "raf.h"

static ARAvoid
on_uv_fs_done(uv_fs_t *fs) {
  D(open, "on_uv_fs_done()");

  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  ara_done_cb *done = (ara_done_cb *) rafreq->done;
  RandomAccessFile *raf = rafreq->raf;

  panic(fs->result > -1, "on_uv_fs_done(): uv: error: '%s'", uv_strerror(fs->result));

  D(open, "on_uv_fs_done(): uv_fs_req_cleanup()");
  uv_fs_req_cleanup(fs);

  D(open, "on_uv_fs_done(): done()");
  done(req);
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
onopen(ara_async_res_t *res) {
  D(open, "onopen()");
  ON_ARA_WORK_DONE(res, RandomAccessFileOpenCallback);
}

static ARAvoid
on_uv_fs_open(uv_fs_t *fs) {
  D(open, "on_uv_fs_open()");

  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  RandomAccessFile *raf = rafreq->raf;
  ARAboolean flags = 0;

  D(open, "on_uv_fs_open(): result=%d", fs->result);
  raf->fd = fs->result;

  if (raf->fd < 0) {
    D(open, "on_uv_fs_open(): on_uv_fs_done()");
    return on_uv_fs_done(fs);
  }

  D(open, "on_uv_fs_done(): flags=%d", rafreq->flags);

  if (0 != (rafreq->flags & RAF_OPEN_READ_WRITE)) {
    D(open, "on_uv_fs_open(): flags=RAF_OPEN_READ_WRITE");
    raf->readable = ARA_TRUE;
    raf->writable = ARA_TRUE;
    flags = rafreq->flags;
  } else if (0 != (rafreq->flags & RAF_OPEN_READ_ONLY)) {
    D(open, "on_uv_fs_open(): flags=RAF_OPEN_READ_ONLY");
    raf->readable = ARA_TRUE;
    raf->writable = ARA_FALSE;
    flags = rafreq->flags;
  } else if (0 != (rafreq->flags & RAF_OPEN_WRITE_ONLY)) {
    D(open, "on_uv_fs_open(): flags=RAF_OPEN_READ_ONLY");
    raf->readable = ARA_FALSE;
    raf->writable = ARA_TRUE;
    flags = rafreq->flags;
  } else {
    panic(RAF_OPEN_NONE == rafreq->flags, "Unhandled flags '%d'", rafreq->flags);
  }

  D(open, "on_uv_fs_open(): uv_fs_req_cleanup()");
  uv_fs_req_cleanup(fs);

  D(open, "on_uv_fs_open(): uv_fs_fstat()");
  uv_fs_fstat(req->ara->loop, fs, raf->fd, on_uv_fs_stat);
}

static ARAvoid
ara_work_open(ara_async_req_t *req, ara_done_cb *done) {
  D(open, "ara_work_open()");

  RandomAccessFileRequest *rafreq = 0;
  int flags = 0;

  rafreq = (RandomAccessFileRequest *) req->data.data;

  if (0 != (rafreq->flags & RAF_OPEN_READ_WRITE)) {
    flags = UV_FS_O_RDWR | UV_FS_O_CREAT;
  } else if (0 != (rafreq->flags & RAF_OPEN_READ_ONLY)) {
    flags = UV_FS_O_RDONLY;
  } else if (0 != (rafreq->flags & RAF_OPEN_WRITE_ONLY)) {
    flags = UV_FS_O_WRONLY | UV_FS_O_CREAT;
  }

  D(open, "ara_work_open(): flags=%d", flags);

  rafreq->fs.data = req;
  rafreq->done = done;

  D(open, "ara_work_open(): uv_fs_open(): file=%s", rafreq->raf->filename);
  uv_fs_open(req->ara->loop,
             (uv_fs_t *) rafreq,
             rafreq->raf->filename,
             flags, 0666,
             on_uv_fs_open);
}

ARAboolean
raf_open(RandomAccessFile *self,
         const RandomAccessFileFlags flags,
         RandomAccessFileOpenCallback *callback) {
  D(open, "raf_open()");

  static ara_async_data_t data = {0};
  RandomAccessFileRequest *req = 0;

  panic(self, "Out of memory.");
  panic((req = make(RandomAccessFileRequest)), "Out of memory.");

  D(open, "raf_open(): flags=%d", flags);

  req->callback = callback;
  req->flags = flags;
  req->raf = self;

  data.data = req;

  panic(ara_set(&self->ara, ARA_OPEN, ara_work_open),
        "ara: error: '%s'",
        ara_strerror(self->ara.error.code));

  panic(ara_open(&self->ara, &data, onopen),
        "ara: error: '%s'",
        ara_strerror(self->ara.error.code));

  return ARA_TRUE;
}
