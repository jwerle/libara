#include <describe/describe.h>

#include <ara/ara.h>
#include <uv.h>

static struct {
  ARAuint work;
  ARAuint open;
  ARAuint close;
  ARAuint read;
} called = {0};

struct Range {
  ARAuint64 offset;
  ARAuint64 length;
};

static const struct Range range[] = {
  {.offset = 0, .length = 1024},
  {.offset = 100, .length = 1024 + 100},
};

static void
ara_work_open(ara_t *ara, ara_async_req_t *req, ara_work_done *done) {
  (void) ++called.work;
  describe("ara_work_open(ara_t *ara, ara_work_done *done);") {
    it("should expose ara with an 'ARA_STATUS_OPENING' status set.") {
      assert(ARA_STATUS_OPENING == ara->status);
    }
  }
  done(ara, req);
}

static void
ara_work_close(ara_t *ara, ara_async_req_t *req, ara_work_done *done) {
  (void) ++called.work;
  describe("ara_work_close(ara_t *ara, ara_work_done *done);") {
    it("should expose ara with an 'ARA_STATUS_CLOSING' status set.") {
      assert(ARA_STATUS_CLOSING == ara->status);
    }
  }

  done(ara, req);
}

static void
ara_work_read(ara_t *ara, ara_async_req_t *req, ara_read_work_done *done) {
  (void) ++called.work;
  const ARAuint offset = req->data.offset;
  const ARAuint length = req->data.length;
  ara_buffer_t *buffer = (ara_buffer_t *) req->data.data;

  describe("ara_work_read(ara, offset, length, buffer, done)") {
    it("should expose correct range offset.") {
      switch (called.read) {
        case 0: assert(range[0].offset == offset); break;
        case 1: assert(range[1].offset == offset); break;
      }
    }

    it("should expose correct range length.") {
      switch (called.read) {
        case 0: assert(range[0].length == length); break;
        case 1: assert(range[1].length == length); break;
      }
    }

    it("should expose a buffer pointer.") {
      assert(0 != buffer);
    }
  }

  if (0 == called.read) {
    sleep(1);
  } else {
    printf("NEXT\n");
  }
  done(ara, req);
}

static void
onclose(ara_t *ara) {
  (void) ++called.close;
  describe("onclose(ara_t *ara);") {
    it("should expose ara with an 'ARA_STATUS_CLOSED' status set.") {
      assert(ARA_STATUS_CLOSED == ara->status);
    }
  }
  uv_stop(ara->loop);
}

static void
onread(ara_t *ara, ara_buffer_t *buffer) {
  (void) ++called.read;
  assert(ARA_TRUE == ara_set(ara, ARA_WORK_CLOSE, (ara_worker_cb) ara_work_close));
  assert(ARA_TRUE == ara_close(ara, onclose));
}

static void
onopen(ara_t *ara) {
  (void) ++called.open;

  describe("onopen(ara_t *ara);") {
    it("should expose ara with an 'ARA_STATUS_OPENED' status set.") {
      assert(ARA_STATUS_OPENED == ara->status);
    }
  }

  describe("ARAboolean ara_read(ara_t *self, ARAuint offset, ARAuint length, ara_read_cb *cb);") {
    it("should return 'ARA_FALSE' when 'ARA_WORK_READ' bit is not set.") {
      assert(ARA_FALSE == ara_read(ara, 0, 0, 0));
      assert(ARA_ENOCALLBACK == ara->error.code);
    }

    it("should return 'ARA_TRUE' when 'ara_read_cb' set.") {
      assert(ARA_TRUE == ara_set(ara, ARA_WORK_READ, (ara_worker_cb) ara_work_read));
      assert(ARA_TRUE == ara_read(ara, range[0].offset, range[0].length, onread));
      assert(ARA_TRUE == ara_read(ara, range[1].offset, range[1].length, onread));
    }
  }
}

int
main(void) {
  ara_t ara = {0};
  uv_loop_t *loop = uv_default_loop();

  called.work = 0;
  called.open = 0;
  called.close = 0;
  called.read = 0;

  describe("ARAboolean ara_read(ara_t *self, ARAuint offset, ARAuint length, ara_read_cb *cb);") {
    it("should return 'ARA_FALSE' on 'NULL' 'ara_t' pointer.") {
      assert(ARA_FALSE == ara_read(0, 0, 0, 0));
    }

    it("should return 'ARA_FALSE' when not initialized") {
      assert(ARA_FALSE == ara_read(&ara, 0, 0, 0));
    }

    assert(ARA_TRUE == ara_init(&ara));

    it("should return 'ARA_FALSE' when status is not set to 'ARA_STATUS_OPENED'.") {
      assert(ARA_FALSE == ara_read(&ara, 0, 0, 0));
      assert(ARA_EBADSTATE == ara.error.code);
    }

    assert(ARA_TRUE == ara_set(&ara, ARA_WORK_OPEN, (ara_worker_cb) ara_work_open));
    assert(ARA_TRUE == ara_open(&ara, onopen));
  }

  assert(0 == called.work);
  assert(0 == called.open);
  assert(0 == called.close);
  assert(0 == called.read);

  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);

  assert(4 == called.work);
  assert(called.open);
  assert(called.close);
  assert(2 == called.read);

  return assert_failures();
}
