#include <ara/ara.h>
#include <uv.h>

#include "util.h"
#include "raf.h"

static ARAvoid
on_uv_fs_done(uv_fs_t *fs) {
  D(write, "on_uv_fs_done()");
  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  ara_open_work_done *done = (ara_open_work_done *) rafreq->done;
  RandomAccessFile *raf = rafreq->raf;

  panic(fs->result > -1, "uv: error: '%s'", uv_strerror(fs->result));
  uv_fs_req_cleanup(fs);
  done(req->ara, req);
}

static ARAvoid
on_ara_write(ara_t *ara, ara_async_res_t *res) {
  D(write, "on_ara_write()");
  ON_ARA_WORK_DONE(ara, res, RandomAccessFileWriteCallback);
}

static ARAvoid
on_uv_fs_write(uv_fs_t *fs) {
  D(write, "on_uv_fs_write()");

  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) fs;
  RandomAccessFileWriteOptions *opts = (RandomAccessFileWriteOptions *) rafreq->opts;

  UV_FS_PROCESS_REQ(fs, opts->offset, uv_fs_write, on_uv_fs_write);
}

static ARAvoid
ara_work_write(ara_t *ara, ara_async_req_t *req, ara_open_work_done *done) {
  D(write, "ara_work_write()");

  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  RandomAccessFileWriteOptions *opts = (RandomAccessFileWriteOptions *) rafreq->opts;
  RandomAccessFile *raf = rafreq->raf;
  ara_buffer_t *buffer = req->buffer;

  panic(raf->fd > -1, "uv: error: '%s'", uv_strerror(raf->fd));
  panic(raf->writable, "File not writable");
  panic(buffer, "Missing write buffer");

  rafreq->fs.data = req;
  rafreq->buffer = buffer;
  rafreq->done = done;

  buffer->data.buffer.base = buffer->data.base;
  buffer->data.buffer.len = buffer->data.length;
  buffer->data.buffer_mut = buffer->data.buffer;

  D(write, "ara_work_write(): uv_fs_write(): fd=%d filename=%s offset=%d length=%d",
      raf->fd, raf->filename, opts->offset, buffer->data.buffer_mut.len);

  uv_fs_write(ara->loop,
            (uv_fs_t *) rafreq,
            raf->fd,
            &(buffer->data.buffer_mut),
            1, // number of buffers ^
            opts->offset,
            on_uv_fs_write);
}

ARAboolean
raf_write(RandomAccessFile *self,
        RandomAccessFileWriteOptions *opts,
        RandomAccessFileWriteCallback *callback) {
  D(write, "raf_write()");

  static ara_async_data_t data = {0};
  static RandomAccessFileWriteOptions defaults = {0};
  RandomAccessFileRequest *req = 0;

  panic(0 != self, "Out of memory.");
  panic((req = make(RandomAccessFileRequest)), "Out of memory.");

  if (0 == opts) {
    memset(&opts, 0, sizeof(RandomAccessFileWriteOptions));
    opts = &defaults;
  }

  if (opts->offset >= self->size) {
    opts->offset = self->size;
    opts->length = 0;
  }

  if (opts->offset + opts->length > self->size) {
    opts->length = self->size - opts->offset;
  }

  if (opts->length > self->size) {
    opts->length = self->size;
  }

  req->callback = callback;
  req->opts = opts;
  req->raf = self;

  data.data = req;
  data.length = opts->length;

  panic(ara_set(&self->ara, ARA_WORK_WRITE, (ara_worker_cb *) ara_work_write),
        "ara: error: '%s'", ara_error(self->ara.error.code));

  panic(ara_write(&self->ara, &data, on_ara_write),
        "ara: error: '%s'", ara_error(self->ara.error.code));

  D(write, "raf_write(): fd=%d filename=%s offset=%d length=%d",
      self->fd, self->filename, opts->offset, opts->length);

  return ARA_TRUE;
}
