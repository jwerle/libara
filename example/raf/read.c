#include <ara/ara.h>
#include <uv.h>

#include "util.h"
#include "raf.h"

static ARAvoid
on_uv_fs_done(uv_fs_t *fs) {
  D(read, "on_uv_fs_done()");
  ara_async_req_t *req = (ara_async_req_t *) fs->data;
  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  ara_open_work_done *done = (ara_open_work_done *) rafreq->done;
  RandomAccessFile *raf = rafreq->raf;

  panic(fs->result > -1, "uv: error: '%s'", uv_strerror(fs->result));
  uv_fs_req_cleanup(fs);
  done(req->ara, req);
}

static ARAvoid
on_ara_read(ara_t *ara, ara_async_res_t *res) {
  D(read, "on_ara_read()");
  ON_ARA_WORK_DONE(ara, res, RandomAccessFileReadCallback);
}

static ARAvoid
on_uv_fs_read(uv_fs_t *fs) {
  D(read, "on_uv_fs_read()");

  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) fs;
  RandomAccessFileReadOptions *opts = (RandomAccessFileReadOptions *) rafreq->opts;

  UV_FS_PROCESS_REQ(fs, opts->offset, uv_fs_read, on_uv_fs_read) {
    on_uv_fs_done(fs);
  }
}

static ARAvoid
ara_work_read(ara_t *ara, ara_async_req_t *req, ara_open_work_done *done) {
  D(read, "ara_work_read()");

  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) req->data.data;
  RandomAccessFileReadOptions *opts = (RandomAccessFileReadOptions *) rafreq->opts;
  RandomAccessFile *raf = rafreq->raf;
  ara_buffer_t *buffer = req->buffer;

  panic(raf->fd > -1, "uv: error: '%s'", uv_strerror(raf->fd));
  panic(raf->readable, "File not readable");
  panic(buffer, "Missing write buffer");

  rafreq->fs.data = req;
  rafreq->buffer = buffer;
  rafreq->done = done;

  buffer->data.buffer.base = buffer->data.base;
  buffer->data.buffer.len = buffer->data.length;
  buffer->data.buffer_mut = buffer->data.buffer;

  D(read, "ara_work_read(): uv_fs_read(): fd=%d filename=%s offset=%d length=%d",
      raf->fd, raf->filename, opts->offset, buffer->data.buffer_mut.len);

  uv_fs_read(ara->loop,
            (uv_fs_t *) rafreq,
            raf->fd,
            &(buffer->data.buffer_mut),
            1, // number of buffers ^
            opts->offset,
            on_uv_fs_read);
}

ARAboolean
raf_read(RandomAccessFile *self,
        RandomAccessFileReadOptions *opts,
        RandomAccessFileReadCallback *callback) {
  D(read, "raf_read()");

  static ara_async_data_t data = {0};
  static RandomAccessFileReadOptions defaults = {0};
  RandomAccessFileRequest *req = 0;

  panic(0 != self, "Out of memory.");
  panic((req = make(RandomAccessFileRequest)), "Out of memory.");

  if (0 == opts) {
    memset(&opts, 0, sizeof(RandomAccessFileReadOptions));
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

  panic(ara_set(&self->ara, ARA_WORK_READ, (ara_worker_cb *) ara_work_read),
        "ara: error: '%s'", ara_strerror(self->ara.error.code));

  panic(ara_read(&self->ara, &data, on_ara_read),
        "ara: error: '%s'", ara_strerror(self->ara.error.code));

  D(read, "raf_read(): fd=%d filename=%s offset=%d length=%d",
      self->fd, self->filename, opts->offset, opts->length);

  return ARA_TRUE;
}
