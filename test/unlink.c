#include <describe/describe.h>

#include <ara/ara.h>
#include <uv.h>

static struct {
  int work;
  int open;
  int unlink;
} called = {0};

static void
onunlink(ara_async_res_t *res) {
  (void) ++called.unlink;
}

static void
onopen(ara_async_res_t *res) {
  (void) ++called.open;
  describe("onopen(ara_t *ara);") {
    it("should expose ara with an 'ARA_STATUS_OPENED' status set.") {
      assert(ARA_STATUS_OPENED == res->ara->status);
    }
  }

  describe("ARAboolean ara_unlink(ara_t *self, ara_async_data_t *data, ara_cb *cb);") {
    it("should return 'ARA_TRUE' even if 'ara_cb' set.") {
      assert(ARA_TRUE == ara_unlink(res->ara, 0, onunlink));
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
ara_work_unlink(ara_async_req_t *req, ara_done_cb *done) {
  (void) ++called.work;
  done(req);
}

int
main(void) {
  ara_t ara = {0};
  uv_loop_t *loop = uv_default_loop();

  called.work = 0;
  called.unlink = 0;

  describe("ARAboolean ara_unlink(ara_t *self, ara_async_data_t *data, ara_cb *cb);") {
    it("should return 'ARA_FALSE' on 'NULL' 'ara_t' pointer.") {
      assert(ARA_FALSE == ara_unlink(0, 0, 0));
    }

    it("should return 'ARA_FALSE' when not initialized") {
      assert(ARA_FALSE == ara_unlink(&ara, 0, 0));
    }

    assert(ARA_TRUE == ara_init(&ara));

    it("should return 'ARA_FALSE' when 'ARA_UNLINK' bit is not set.") {
      assert(ARA_FALSE == ara_unlink(&ara, 0, 0));
    }

    it("should return 'ARA_FALSE' even if 'ara_cb' set.") {
      assert(ARA_TRUE == ara_set(&ara, ARA_UNLINK, ara_work_unlink));
      assert(ARA_FALSE == ara_unlink(&ara, 0, onunlink));
    }

    assert(ARA_TRUE == ara_set(&ara, ARA_OPEN, ara_work_open));
    assert(ARA_TRUE == ara_open(&ara, 0, onopen));
  }

  assert(0 == called.work);
  assert(0 == called.open);
  assert(0 == called.unlink);

  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);

  assert(2 == called.work);
  assert(called.open);
  assert(called.unlink);

  return assert_failures();
}
