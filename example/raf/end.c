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
on_ara_end(ara_t *ara, ara_async_res_t *res) {
  D(end, "on_ara_end()");
  ON_ARA_WORK_DONE(ara, res, RandomAccessFileEndCallback);
}

static ARAvoid
ara_work_end(ara_t *ara, ara_async_req_t *req, ara_open_work_done *done) {
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  RandomAccessFile *raf = rafreq->raf;

  rafreq->fs.data = req;
  rafreq->done = done;

  panic(rafreq->raf->fd > -1, uv_strerror(rafreq->raf->fd));

  D(end, "ara_work_end(fd=%d, filename=%s)", rafreq->raf->fd, rafreq->raf->filename);

  ARAuint atime = raf->atime;
  ARAuint mtime = raf->mtime;

  if (0 == atime && 0 == mtime) {
    done(ara, req);
  } else if (atime && mtime) {
  } else {
  }
}

ARAboolean
raf_end(RandomAccessFile *self,
        RandomAccessFileEndOptions *opts,
        RandomAccessFileEndCallback *callback) {
  static ara_async_data_t data = {0};
  RandomAccessFileRequest *req = 0;

  panic(0 != self, "Out of memory.");
  panic((req = make(RandomAccessFileRequest)), "Out of memory.");

  req->callback = callback;
  req->data = opts;
  req->raf = self;

  data.data = req;

  panic(ara_set(&self->ara, ARA_WORK_END, (ara_worker_cb *) ara_work_end),
        ara_error(self->ara.error.code));

  panic(ara_end(&self->ara, &data, on_ara_end),
        ara_error(self->ara.error.code));

  D(end, "end(fd=%d, filename=%s)", self->fd, self->filename);
  return ARA_TRUE;
}
