#ifndef ARA_READ_H
#define ARA_READ_H

#include <ara/ara.h>

typedef struct ara_async_read_data ara_async_read_data_t;
struct ara_async_read_data {
  ara_async_data_t *data;
  ara_read_cb *callback;
  ara_t *ara;
};

ara_async_read_data_t *
ara_async_read_data_new(ara_t *ara, ara_async_data_t *data, ara_read_cb *cb);

ARAvoid
ara_async_read_data_destroy(ara_async_read_data_t *read_data);

#endif
