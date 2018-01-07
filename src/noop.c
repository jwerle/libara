#include <ara/ara.h>

ARAvoid
ara_noop_done_cb(ara_async_req_t *req) {
  (ARAvoid) (req);
}

ARAvoid
ara_noop_work_cb(ara_async_req_t *req, ara_done_cb *done) {
  (ARAvoid) (req); (ARAvoid) (done);
}

ARAvoid
ara_noop_cb(ara_async_res_t *res) {
  (ARAvoid) (res);
}
