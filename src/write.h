#ifndef ARA_WRITE_H
#define ARA_WRITE_H

#include <ara/ara.h>

typedef struct ara_async_write_data ara_async_write_data_t;
struct ara_async_write_data {
  ara_async_data_t *data;
  ara_write_cb *callback;
  ara_t *ara;
};

ara_async_write_data_t *
ara_async_write_data_new(ara_t *ara, ara_async_data_t *data, ara_write_cb *cb);

ARAvoid
ara_async_write_data_destroy(ara_async_write_data_t *write_data);

#endif
