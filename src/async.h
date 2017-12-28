#ifndef ARA_UV_H
#define ARA_UV_H

#include <uv.h>

#define ASYNC(which) void onasync##which(uv_async_t* handle);

  ASYNC(open)
  ASYNC(close)
  ASYNC(end)
  ASYNC(read)
  ASYNC(write)
  ASYNC(unlink)

#undef ASYNC

#endif
