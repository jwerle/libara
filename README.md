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

static ara_buffer_t ram = {0};
static uv_loop_t *loop = 0;
static ara_t ara = {0};

void onopen(ara_async_req_t *req, ara_done_cb *done) {
  static ara_async_data_t data = {0};
  printf("onopen()\n");

  data.offset = 32;
  data.length = 16;
  data.data = "abcd efgh 1234 f";

  done(req);
  ara_write(req->ara, &data, 0);
}

void onwrite(ara_async_req_t *req, ara_done_cb *done) {
  static ara_async_data_t data = {0};
  printf("onwrite()\n");

  data.offset = req->data.offset;
  data.length = req->data.length;

  ara_buffer_write(&ram, req->data.offset, req->data.length, req->data.data);
  done(req);
  ara_read(req->ara, &data, 0);
}

void onread(ara_async_req_t *req, ara_done_cb *done) {
  ara_async_data_t *data = &req->data;
  ARAchar out[data->length + 1];

  printf("onread(): offset=%d length=%d\n", data->offset, data->length);
  ara_buffer_read(&ram, data->offset, data->length, out);
  printf("%s\n", out);

  done(req);
  ara_close(req->ara, 0, 0);
}

void onclose(ara_async_req_t *req, ara_done_cb *done) {
  printf("onclose()\n");
  done(req);
  ara_buffer_destroy(&ram);
}

int
main(void) {
  loop = uv_default_loop();

  ara_buffer_init(&ram, BUFSIZ);

  assert(ara_init(&ara));
  assert(ara_set_loop(&ara, loop));
  assert(ara_set(&ara, ARA_OPEN, onopen));
  assert(ara_set(&ara, ARA_CLOSE, onclose));
  assert(ara_set(&ara, ARA_READ, onread));
  assert(ara_set(&ara, ARA_WRITE, onwrite));
  assert(ara_open(&ara, 0, 0));

  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);

  return 0;
}
```

## License

MIT
