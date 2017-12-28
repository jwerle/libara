#include <describe/describe.h>
#include <assert.h>

#include <ara/ara.h>
#include <uv.h>

static struct {
  int work;
  int open;
  int end;
} called = {0};

static void
onend(ara_t *ara) {
  describe("onend(ara_t *ara);") {
    it("should expose ara with an 'ARA_STATUS_ENDED' status set.") {
      assert(ARA_STATUS_ENDED == ara->status);
    }
  }
  (void) ++called.end;
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

  describe("ARAboolean ara_end(ara_t *self, ara_end_work_cb *cb);") {
    it("should return 'ARA_TRUE' even if 'ara_end_work_cb' set.") {
      assert(ARA_TRUE == ara_end(ara, onend));
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
ara_work_end(ara_t *ara, ara_work_done *done) {
  describe("ara_work_end(ara_t *ara, ara_work_done *done);") {
    it("should expose ara with an 'ARA_STATUS_ENDING' status set.") {
      assert(ARA_STATUS_ENDING == ara->status);
    }
  }

  (void) ++called.work;
  done(ara);
}

int
main(void) {
  ara_t ara = {0};
  uv_loop_t *loop = uv_default_loop();

  called.work = 0;
  called.end = 0;

  describe("ARAboolean ara_end(ara_t *self, ara_end_work_cb *cb);") {
    it("should return 'ARA_FALSE' on 'NULL' 'ara_t' pointer.") {
      assert(ARA_FALSE == ara_end(0, 0));
    }

    it("should return 'ARA_FALSE' when not initialized") {
      assert(ARA_FALSE == ara_end(&ara, 0));
    }

    assert(ARA_TRUE == ara_init(&ara));

    it("should return 'ARA_FALSE' when 'ARA_WORK_END' bit is not set.") {
      assert(ARA_FALSE == ara_end(&ara, 0));
    }

    it("should return 'ARA_FALSE' even if 'ara_end_work_cb' set.") {
      assert(ARA_TRUE == ara_set(&ara, ARA_WORK_END, (ara_work_cb) ara_work_end));
      assert(ARA_FALSE == ara_end(&ara, onend));
    }

    assert(ARA_TRUE == ara_set(&ara, ARA_WORK_OPEN, (ara_work_cb) ara_work_open));
    assert(ARA_TRUE == ara_open(&ara, onopen));
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
