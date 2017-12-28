#ifndef ARA_UV_H
#define ARA_UV_H

#include <uv.h>

#define ONUV(which) void onuv##which(uv_async_t* handle);

  ONUV(open)
  ONUV(close)
  ONUV(end)
  ONUV(read)
  ONUV(write)
  ONUV(unlink)

#undef ONUV

#endif
