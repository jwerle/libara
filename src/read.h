#ifndef ARA_READ_H
#define ARA_READ_H

#include <ara/ara.h>

typedef struct ara_async_read_data ara_async_read_data_t;
struct ara_async_read_data {
  ara_async_data_t *data;
  ara_t *ara;
};

#endif
