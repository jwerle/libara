#include <describe/describe.h>
#include <ara/ara.h>
#include <uv.h>

static struct {
  int work;
  int open;
  int close;
} called = {0};

static ara_async_data_t data = {0};

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
onclose(ara_async_res_t *res) {
  (void) ++called.close;
  describe("onclose(ara_async_res_t *res);") {
    it("should expose ara with an 'ARA_STATUS_CLOSED' status set.") {
      assert(ARA_STATUS_CLOSED == res->ara->status);
    }
  }
}

static void
onopen(ara_async_res_t *res) {
  (void) ++called.open;
  describe("onopen(ara_async_res_t *res;") {
    it("should expose ara with an 'ARA_STATUS_OPENED' status set.") {
      assert(ARA_STATUS_OPENED == res->ara->status);
    }
  }

  describe("ARAboolean ara_close(ara_t *self, ara_close_cb *cb);") {
    it("should return 'ARA_TRUE' even if 'ara_close_cb' set.") {
      assert(ARA_TRUE == ara_close(res->ara, 0, onclose));
    }
  }
}

int
main(void) {
  ara_t ara = {0};
  uv_loop_t *loop = uv_default_loop();

  describe("ARAboolean ara_close(ara_t *self, ara_async_data_t *data, ara_close_cb *cb);") {
    it("should return 'ARA_FALSE' on 'NULL' 'ara_t' pointer.") {
      assert(ARA_FALSE == ara_close(0, 0, 0));
    }

    it("should return 'ARA_FALSE' when not initialized") {
      assert(ARA_FALSE == ara_close(&ara, 0, 0));
    }

    assert(ARA_TRUE == ara_init(&ara));

    it("should return 'ARA_FALSE' when 'ARA_CLOSE' bit is not set.") {
      assert(ARA_FALSE == ara_close(&ara, 0, 0));
    }

    it("should return 'ARA_FALSE' even if 'ara_end_cb' set.") {
      assert(ARA_TRUE == ara_set(&ara, ARA_CLOSE, &ara_work_close));
      assert(ARA_FALSE == ara_close(&ara, 0, onclose));
    }

    assert(ARA_TRUE == ara_set(&ara, ARA_OPEN, &ara_work_open));
    assert(ARA_TRUE == ara_open(&ara, 0, onopen));
  }

  assert(0 == called.work);
  assert(0 == called.open);
  assert(0 == called.close);

  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);

  assert(2 == called.work);
  assert(called.open);
  assert(called.close);

  return assert_failures();
}
