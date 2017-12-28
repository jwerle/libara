#include <describe/describe.h>

#include <ara/ara.h>
#include <uv.h>

static struct {
  int work;
  int open;
  int unlink;
} called = {0};

static void
onunlink(ara_t *ara) {
  (void) ++called.unlink;
  uv_stop(ara->loop);
}

static void
onopen(ara_t *ara) {
  describe("onopen(ara_t *ara);") {
    it("should expose ara with an 'ARA_STATUS_OPENED' status set.") {
      assert(ARA_STATUS_OPENED == ara->status);
    }
  }
  (void) ++called.open;

  describe("ARAboolean ara_unlink(ara_t *self, ara_unlink_cb *cb);") {
    it("should return 'ARA_TRUE' even if 'ara_unlink_cb' set.") {
      assert(ARA_TRUE == ara_unlink(ara, onunlink));
    }
  }
}

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
ara_work_unlink(ara_t *ara, ara_work_done *done) {
  (void) ++called.work;
  done(ara);
}

int
main(void) {
  ara_t ara = {0};
  uv_loop_t *loop = uv_default_loop();

  called.work = 0;
  called.unlink = 0;

  describe("ARAboolean ara_unlink(ara_t *self, ara_unlink_cb *cb);") {
    it("should return 'ARA_FALSE' on 'NULL' 'ara_t' pointer.") {
      assert(ARA_FALSE == ara_unlink(0, 0));
    }

    it("should return 'ARA_FALSE' when not initialized") {
      assert(ARA_FALSE == ara_unlink(&ara, 0));
    }

    assert(ARA_TRUE == ara_init(&ara));

    it("should return 'ARA_FALSE' when 'ARA_WORK_UNLINK' bit is not set.") {
      assert(ARA_FALSE == ara_unlink(&ara, 0));
    }

    it("should return 'ARA_FALSE' even if 'ara_unlink_cb' set.") {
      assert(ARA_TRUE == ara_set(&ara, ARA_WORK_UNLINK, (ara_cb) ara_work_unlink));
      assert(ARA_FALSE == ara_unlink(&ara, onunlink));
    }

    assert(ARA_TRUE == ara_set(&ara, ARA_WORK_OPEN, (ara_cb) ara_work_open));
    assert(ARA_TRUE == ara_open(&ara, onopen));
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
