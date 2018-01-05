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

  D(open, "on_uv_fs_open(): uv_fs_fstat()");
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
on_uv_fs_write(uv_fs_t *fs) {
  D(write, "on_uv_fs_write()");

  RandomAccessFileRequest *rafreq = (RandomAccessFileRequest *) fs;
  RandomAccessFileWriteOptions *opts = (RandomAccessFileWriteOptions *) rafreq->opts;

  UV_FS_PROCESS_REQ(fs, opts->offset, uv_fs_write, on_uv_fs_write) {
    uv_fs_fstat(rafreq->raf->ara.loop, fs, rafreq->raf->fd, on_uv_fs_stat);
  }
}

static ARAvoid
on_ara_write(ara_t *ara, ara_async_res_t *res) {
  D(write, "on_ara_write()");
  ON_ARA_WORK_DONE(ara, res, RandomAccessFileWriteCallback);
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

  if (opts->buffer && buffer->data.base) {
    memcpy(buffer->data.base, opts->buffer, opts->length);
  }

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

  req->callback = callback;
  req->opts = opts;
  req->raf = self;

  data.data = req;
  data.length = opts->length;

  panic(ara_set(&self->ara, ARA_WORK_WRITE, (ara_worker_cb *) ara_work_write),
        "ara: error: '%s'", ara_strerror(self->ara.error.code));

  panic(ara_write(&self->ara, &data, on_ara_write),
        "ara: error: '%s'", ara_strerror(self->ara.error.code));

  D(write, "raf_write(): fd=%d filename=%s offset=%d length=%d",
      self->fd, self->filename, opts->offset, opts->length);

  return ARA_TRUE;
}
