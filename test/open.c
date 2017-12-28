#include <describe/describe.h>
#include <assert.h>

#include <ara/ara.h>
#include <uv.h>

static struct {
  int work;
  int open;
} called = {0};

static void
ara_work_open(ara_t *ara, ara_work_done *done) {
  describe("ara_work_open(ara_t *ara, ara_work_done *done);") {
    it("should expose ara with an 'ARA_STATUS_OPENING' status set.") {
      assert(ARA_STATUS_OPENING == ara->status);
    }
  }
  (void) ++called.work;
  done(ara);
}

static void
onopen(ara_t *ara) {
  describe("on_open(ara_t *ara);") {
    it("should expose ara with an 'ARA_STATUS_OPENED' status set.") {
      assert(ARA_STATUS_OPENED == ara->status);
    }
  }
  (void) ++called.open;
  uv_stop(ara->loop);
}

int
main(void) {
  ara_t ara = {0};
  uv_loop_t *loop = uv_default_loop();

  called.work = 0;
  called.open = 0;

  describe("ARAboolean ara_open(ara_t *self, ara_open_work_cb *cb);") {
    it("should return 'ARA_FALSE' on 'NULL' 'ara_t' pointer.") {
      assert(ARA_FALSE == ara_open(0, 0));
    }

    it("should return 'ARA_FALSE' when not initialized") {
      assert(ARA_FALSE == ara_open(&ara, 0));
    }

    assert(ARA_TRUE == ara_init(&ara));

    it("should return 'ARA_FALSE' when 'ARA_WORK_OPEN' bit is not set.") {
      assert(ARA_FALSE == ara_open(&ara, 0));
    }

    it("should return 'ARA_TRUE' when 'ara_open_work_cb' set.") {
      assert(ARA_TRUE == ara_set(&ara, ARA_WORK_OPEN, (ara_work_cb) ara_work_open));
      assert(ARA_TRUE == ara_open(&ara, onopen));
    }
  }

  assert(0 == called.work);
  assert(0 == called.open);

  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);

  assert(called.work);
  assert(called.open);

  return assert_failures();
}
