#include <describe/describe.h>
#include <ara/ara.h>
#include <unistd.h>
#include <uv.h>

static struct {
  ARAuint work;
  ARAuint open;
  ARAuint close;
  ARAuint write;
} called = {0};

struct Range {
  ARAuint64 offset;
  ARAuint64 length;
};

static const struct Range range[] = {
  { .offset = 0, .length = 1024 },
  { .offset = 100, .length = 1024 + 100 },
};

static void
onclose(ara_async_res_t *res) {
  (void) ++called.close;
  describe("onclose(ara_async_res_t *res);") {
    it("should expose ara with an 'ARA_STATUS_CLOSED' status set.") {
      assert(ARA_STATUS_CLOSED == res->ara->status);
    }
  }
}

static void
onwrite(ara_async_res_t *res) {
  (void) ++called.write;
  describe("onwrite(ara_async_res_t *res);") {
    assert(ARA_TRUE == ara_close(res->ara, 0, onclose));
  }
}

static void
ara_work_open(ara_async_req_t *req, ara_done_cb *done) {
  (void) ++called.work;
  describe("ara_work_open(ara_async_req_t *req, ara_done_cb *done);") {
    it("should expose ara with an 'ARA_STATUS_OPENING' status set.") {
      assert(ARA_STATUS_OPENING == req->ara->status);
    }
  }
  done(req);
}

static void
ara_work_close(ara_async_req_t *req, ara_done_cb *done) {
  (void) ++called.work;
  describe("ara_work_close(ara_async_req_t *req, ara_done_cb *done);") {
    it("should expose ara with an 'ARA_STATUS_CLOSING' status set.") {
      assert(ARA_STATUS_CLOSING == req->ara->status);
    }
  }

  done(req);
}

static void
ara_work_write(ara_async_req_t *req, ara_done_cb *done) {
  (void) ++called.work;
  const ARAuint offset = req->data.offset;
  const ARAuint length = req->data.length;
  ara_buffer_t *buffer = (ara_buffer_t *) req->buffer;

  describe("ara_work_write(ara, req, done)") {
    it("should expose correct range offset.") {
      switch (called.write) {
        case 0: assert(range[0].offset == offset); break;
        case 1: assert(range[1].offset == offset); break;
      }
    }

    it("should expose correct range length.") {
      switch (called.write) {
        case 0: assert(range[0].length == length); break;
        case 1: assert(range[1].length == length); break;
      }
    }

    it("should expose a buffer pointer.") {
      assert(0 != buffer);
    }
  }

  if (0 == called.write) {
    sleep(1);
  }

  done(req);
}

static void
onopen(ara_async_res_t *res) {
  (void) ++called.open;

  describe("onopen(ara_async_res_t *res);") {
    it("should expose ara with an 'ARA_STATUS_OPENED' status set.") {
      assert(ARA_STATUS_OPENED == res->ara->status);
    }
  }

  describe("ARAboolean ara_write(ara_t *self, ara_async_data_t *data, ara_cb *cb);") {
    it("should return 'ARA_FALSE' when 'ARA_WRITE' bit is not set.") {
      assert(ARA_FALSE == ara_write(res->ara, 0, 0));
      assert(ARA_ENOCALLBACK == res->ara->error.code);
    }

    it("should return 'ARA_TRUE' when 'ara_cb' set.") {
      ara_async_data_t data = {0};
      assert(ARA_TRUE == ara_set(res->ara, ARA_WRITE, ara_work_write));

      data.offset = range[0].offset;
      data.length = range[0].length;
      assert(ARA_TRUE == ara_write(res->ara, &data, onwrite));

      data.offset = range[1].offset;
      data.length = range[1].length;
      assert(ARA_TRUE == ara_write(res->ara, &data, onwrite));
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
  called.write = 0;

  describe("ARAboolean ara_write(ara_t *self, ara_async_data_t *data, ara_cb *cb);") {
    it("should return 'ARA_FALSE' on 'NULL' 'ara_t' pointer.") {
      assert(ARA_FALSE == ara_write(0, 0, 0));
    }

    it("should return 'ARA_FALSE' when not initialized") {
      assert(ARA_FALSE == ara_write(&ara, 0, 0));
    }

    assert(ARA_TRUE == ara_init(&ara));

    it("should return 'ARA_FALSE' when status is not set to 'ARA_STATUS_OPENED'.") {
      assert(ARA_FALSE == ara_write(&ara, 0, 0));
      assert(ARA_EBADSTATE == ara.error.code);
    }

    assert(ARA_TRUE == ara_set(&ara, ARA_OPEN, ara_work_open));
    assert(ARA_TRUE == ara_set(&ara, ARA_CLOSE, ara_work_close));
    assert(ARA_TRUE == ara_open(&ara, 0, onopen));
  }

  assert(0 == called.work);
  assert(0 == called.open);
  assert(0 == called.close);
  assert(0 == called.write);

  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);

  assert(4 == called.work);
  assert(called.open);
  assert(called.close);
  assert(2 == called.write);

  return assert_failures();
}
