#include <describe/describe.h>

#include <ara/ara.h>
#include <uv.h>

static struct {
  int work;
  int open;
  int end;
} called = {0};

static void
onend(ara_async_res_t *res) {
  (void) ++called.end;
}

static void
onopen(ara_async_res_t *res) {
  (void) ++called.open;
  describe("onopen(ara_async_res_t *res);") {
    it("should expose ara with an 'ARA_STATUS_OPENED' status set.") {
      assert(ARA_STATUS_OPENED == res->ara->status);
    }
  }

  describe("ARAboolean ara_end(ara_t *self, ara_async_data_t *data, ara_cb *cb);") {
    it("should return 'ARA_TRUE' even if 'ara_cb' set.") {
      assert(ARA_TRUE == ara_end(res->ara, 0, onend));
    }
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
ara_work_end(ara_async_req_t *req, ara_done_cb *done) {
  (void) ++called.work;
  done(req);
}

int
main(void) {
  ara_t ara = {0};
  uv_loop_t *loop = uv_default_loop();

  describe("ARAboolean ara_end(ara_t *self, ara_async_data_t *data, ara_cb *cb);") {
    it("should return 'ARA_FALSE' on 'NULL' 'ara_t' pointer.") {
      assert(ARA_FALSE == ara_end(0, 0, 0));
    }

    it("should return 'ARA_FALSE' when not initialized") {
      assert(ARA_FALSE == ara_end(&ara, 0, 0));
    }

    assert(ARA_TRUE == ara_init(&ara));

    it("should return 'ARA_FALSE' when 'ARA_END' bit is not set.") {
      assert(ARA_FALSE == ara_end(&ara, 0, 0));
    }

    it("should return 'ARA_FALSE' even if 'ara_cb' set.") {
      assert(ARA_TRUE == ara_set(&ara, ARA_END, ara_work_end));
      assert(ARA_FALSE == ara_end(&ara, 0, onend));
    }

    assert(ARA_TRUE == ara_set(&ara, ARA_OPEN, ara_work_open));
    assert(ARA_TRUE == ara_open(&ara, 0, onopen));
  }

  assert(0 == called.work);
  assert(0 == called.open);
  assert(0 == called.end);

  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);

  assert(2 == called.work);
  assert(called.open);
  assert(called.end);

  return assert_failures();
}
