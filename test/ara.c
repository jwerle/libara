#include <describe/describe.h>

#include <ara/ara.h>
#include <uv.h>

static void
ara_work_open(ara_t *ara, ara_work_done *done) { }

int
main(void) {
  ara_t ara = {0};
  uv_loop_t *loop = uv_default_loop();

  describe("ARAboolean ara_init(ara_t *ara);") {
    it("should return 'ARA_FALSE' if given 'NULL' pointer.") {
      assert(ARA_FALSE == ara_init(0));
    }

    it("should initialize a 'ara_t' structure and return 'ARA_TRUE'.") {
      assert(ARA_TRUE == ara_init(&ara));
    }

    it("should set default status to 'ARA_STATUS_INIT'.") {
      assert(ARA_STATUS_INIT == ara.status);
    }

    it("should set 'bitfield.work' to '0'.") {
      assert(0 == ara.bitfield.work);
    }

    it("should reset error state.") {
      assert(ARA_ENONE == ara.error.code);
      assert(0 == ara.error.data);
    }

    it("should set loop to 'uv_default_loop()' return value.") {
      assert(uv_default_loop() == ara.loop);
    }
  }

  describe("ARAboolean ara_set_loop(ara_t *ara, uv_loop_t *loop);") {
    it("should return 'ARA_FALSE' on 'NULL' 'ara_t' pointer.") {
      assert(ARA_FALSE == ara_set_loop(0, 0));
    }

    it("should return 'ARA_FALSE' on 'NULL' 'uv_loop_t' pointer.") {
      assert(ARA_FALSE == ara_set_loop(&ara, 0));
    }

    it("should set loop to given 'uv_loop_t *' value.") {
      uv_loop_t *loop = uv_loop_new();
      assert(ARA_TRUE == ara_set_loop(&ara, loop));
      assert(loop == ara.loop);

      // reset back to default loop
      assert(ARA_TRUE == ara_set_loop(&ara, uv_default_loop()));
      assert(uv_default_loop() == ara.loop);
    }
  }

  describe("ARAboolean ara_set(ara_t *self, ara_work_t type, ara_work_cb *cb);") {
    it("should return 'ARA_FALSE' on 'NULL' 'ara_t' pointer.") {
      assert(ARA_FALSE == ara_set(0, 0, 0));
    }

    it("should return 'ARA_FALSE' on '0' 'ara_work_t' value.") {
      assert(ARA_FALSE == ara_set(&ara, 0, 0));
    }

    it("should return 'ARA_FALSE' on 'NULL' 'ara_work_cb' pointer.") {
      assert(ARA_FALSE == ara_set(&ara, ARA_WORK_OPEN, 0));
    }

    it("should return 'ARA_TRUE' when setting correct work type.") {
      assert(ARA_TRUE == ara_set(&ara, ARA_WORK_OPEN, (ara_work_cb) &ara_work_open));
    }
  }

  return assert_failures();
}
