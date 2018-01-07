#include <ara/ara.h>
#include <uv.h>

#include "util.h"
#include "raf.h"

static ARAvoid
on_uv_fs_done(uv_fs_t *fs) {
  D(close, "on_uv_fs_done()");

  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  ara_done_cb *done = (ara_done_cb *) rafreq->done;
  RandomAccessFile *raf = rafreq->raf;

  panic(fs->result > -1, "uv: error: '%s'", uv_strerror(fs->result));
  uv_fs_req_cleanup(fs);
  done(req);
}

static ARAvoid
on_uv_fs_close(uv_fs_t *fs) {
  D(close, "on_uv_fs_close()");

  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  RandomAccessFile *raf = rafreq->raf;

  on_uv_fs_done(fs);
}

static ARAvoid
onclose(ara_async_res_t *res) {
  D(close, "onclose()");
  ON_ARA_WORK_DONE(res, RandomAccessFileCloseCallback);
}

static ARAvoid
ara_work_close(ara_async_req_t *req, ara_done_cb *done) {
  D(close, "ara_work_close()");
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;

  rafreq->fs.data = req;
  rafreq->done = done;

  panic(rafreq->raf->fd > -1, "uv: error: '%s'", uv_strerror(rafreq->raf->fd));

  D(close, "ara_work_close(): fd=%d filename=%s", rafreq->raf->fd, rafreq->raf->filename);

  uv_fs_close(req->ara->loop, (uv_fs_t *) &rafreq->fs, rafreq->raf->fd, on_uv_fs_close);
}

ARAboolean
raf_close(RandomAccessFile *self, RandomAccessFileCloseCallback *callback) {
  static ara_async_data_t data = {0};
  RandomAccessFileRequest *req = 0;

  panic(0 != self, "Out of memory.");
  panic((req = make(RandomAccessFileRequest)), "Out of memory.");

  req->callback = callback;
  req->raf = self;

  data.data = req;

  panic(ara_set(&self->ara, ARA_CLOSE, ara_work_close),
        "ara: error: '%s'",
        ara_strerror(self->ara.error.code));

  panic(ara_close(&self->ara, &data, onclose),
        "ara: error: '%s'",
        ara_strerror(self->ara.error.code));

  D(close, "raf_close() fd=%d filename=%s", self->fd, self->filename);
  return ARA_TRUE;
}
