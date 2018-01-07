libara
======

`libara` is a library for constructing random access memory interfaces.
Operations are asynchronous and are provided data structures for
relaying memory between functions. It is built on top of libuv
(libuv >= 1.x.x). It is inspired by [abstract-random-access](
https://github.com/juliangruber/abstract-random-access).

## Abstract

TODO

## Installation

With [clib](https://github.com/clibs/clib) do:

```sh
$ clib install jwerle/libara
```

Or from source do:

```sh
$ git clone git@github.com:jwerle/libara.git
$ cd librara
$ ./configure
$ make
$ sudo make install
```

## API

TODO

## Example

```c
#include <ara/ara.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <uv.h>

static ara_buffer_t buffer;
static uv_loop_t *loop = 0;
static ara_t ara = {0};

void onopen(ara_t *ara, ara_async_req_t *req, ara_open_work_done *done) {
  static ara_async_data_t data = {0};
  data.offset = 32;
  data.length = 16;
  data.data = "abcd efgh 1234 f";
  done(ara, req);
  ara_write(ara, &data, 0);
}

void onwrite(ara_t*ara, ara_async_req_t *req, ara_open_work_done *done) {
  static ara_async_data_t data = {0};
  data.offset = req->data.offset;
  data.length = req->data.length;
  ara_buffer_write(&buffer, req->data.offset, req->data.length, req->data.data);
  done(ara, req);
  ara_read(ara, &data, 0);
}

void onread(ara_t*ara, ara_async_req_t *req, ara_open_work_done *done) {
  ara_async_data_t *data = &req->data;
  ARAchar out[data->length - 5];
  ara_buffer_read(&buffer, data->offset, data->length, out);
  out[data->length] = '\0';
  printf("%s\n", out);
  done(ara, req);
  ara_close(ara, 0, 0);
}

void onclose(ara_t*ara, ara_async_req_t *req, ara_open_work_done *done) {
  done(ara, req);
}

int
main(void) {
  loop = uv_default_loop();

  ara_buffer_init(&buffer, BUFSIZ);

  assert(ara_init(&ara));
  assert(ara_set_loop(&ara, loop));
  assert(ara_set(&ara, ARA_WORK_OPEN, (ara_worker_cb *) onopen));
  assert(ara_set(&ara, ARA_WORK_CLOSE, (ara_worker_cb *) onclose));
  assert(ara_set(&ara, ARA_WORK_READ, (ara_worker_cb *) onread));
  assert(ara_set(&ara, ARA_WORK_WRITE, (ara_worker_cb *) onwrite));
  assert(ara_open(&ara, 0, 0));

  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);

  return 0;
}
```

## License

MIT
