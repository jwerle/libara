#include <describe/describe.h>

#include <ara/ara.h>
#include <uv.h>

static struct {
  int work;
  int open;
  int close;
} called = {0};

static void
onclose(ara_t *ara) {
  describe("onclose(ara_t *ara);") {
    it("should expose ara with an 'ARA_STATUS_CLOSED' status set.") {
      assert(ARA_STATUS_CLOSED == ara->status);
    }
  }
  (void) ++called.close;
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

  describe("ARAboolean ara_close(ara_t *self, ara_close_cb *cb);") {
    it("should return 'ARA_TRUE' even if 'ara_close_cb' set.") {
      assert(ARA_TRUE == ara_close(ara, onclose));
    }
  }
}

static void
onread(ara_t *ara, ARAuint length, ARAvoid *buffer) {
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
ara_work_close(ara_t *ara, ara_work_done *done) {
  describe("ara_work_close(ara_t *ara, ara_work_done *done);") {
    it("should expose ara with an 'ARA_STATUS_CLOSING' status set.") {
      assert(ARA_STATUS_CLOSING == ara->status);
    }
  }

  (void) ++called.work;
  done(ara);
}

static void
ara_work_read(ara_t *ara,
              const ARAuint offset,
              const ARAuint length,
              ara_buffer_t *buffer,
              ara_read_work_done *done)
{
  (void) ++called.work;
  done(ara, buffer);
}

int
main(void) {
  ara_t ara = {0};
  uv_loop_t *loop = uv_default_loop();

  describe("ARAboolean ara_read(ara_t *self, ARAuint offset, ARAuint length, ara_read_cb *cb);") {
  }

  assert(0 == called.work);
  assert(0 == called.open);
  assert(0 == called.close);

  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);

  //assert(2 == called.work);
  //assert(called.open);
  //assert(called.close);

  return assert_failures();
}
