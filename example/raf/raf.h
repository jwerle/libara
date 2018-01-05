#ifndef RAF_H
#define RAF_H

#include <debug/debug.h>
#include <ara/ara.h>
#include <string.h>
#include <uv.h>

typedef struct RandomAccessFile RandomAccessFile;
typedef struct RandomAccessFileRequest RandomAccessFileRequest;
typedef struct RandomAccessFileDebuggers RandomAccessFileDebuggers;

typedef struct RandomAccessFileEndOptions RandomAccessFileEndOptions;
typedef struct RandomAccessFileReadOptions RandomAccessFileReadOptions;
typedef struct RandomAccessFileWriteOptions RandomAccessFileWriteOptions;

typedef enum RandomAccessFileFlags RandomAccessFileFlags;

typedef ARAvoid * RandomAccessFileCallback;

#define CALLBACK(which) typedef ARAvoid \
  (RandomAccessFile##which##Callback)(  \
    RandomAccessFile *raf,              \
    RandomAccessFileRequest *req);      \

  CALLBACK(Open)
  CALLBACK(Close)
  CALLBACK(End)
  CALLBACK(Read)
  CALLBACK(Write)

#undef CALLBACK

struct RandomAccessFileDebuggers {
#define X(which) struct { ARAboolean init; debug_t d; } which;

  X(main)
  X(init)
  X(open)
  X(close)
  X(end)
  X(read)
  X(write)
  X(unlink)

#undef X
};

enum RandomAccessFileFlags {
  RAF_NONE = 0,
  RAF_OPEN_NONE = 0,
  RAF_OPEN_READ_ONLY,
  RAF_OPEN_WRITE_ONLY,
  RAF_OPEN_READ_WRITE
};

struct RandomAccessFileRequest {
  uv_fs_t fs;
  ara_work_done *done;
  ara_buffer_t *buffer;
  RandomAccessFile *raf;
  RandomAccessFileFlags flags;
  RandomAccessFileCallback callback;
  ARAvoid *data;
  ARAvoid *opts;
};

struct RandomAccessFile {
  ara_t ara;
  const ARAchar *filename;
  ARAlong fd; // (uv_file)
  ARAuint size;
  ARAuint mtime;
  ARAuint atime;
  ARAboolean opened;
  ARAboolean writable;
  ARAboolean readable;
};

struct RandomAccessFileEndOptions {
  ARAuint mtime;
  ARAuint atime;
  ARAvoid *data;
};

struct RandomAccessFileReadOptions {
  ARAuint offset;
  ARAuint length;
};

struct RandomAccessFileWriteOptions {
  ARAuint offset;
  ARAuint length;
  ARAvoid *buffer;
};

extern RandomAccessFileDebuggers __RandomAccessFileDebuggers;

#define RAF_DEBUG(which, ...) debug(    \
  &__RandomAccessFileDebuggers.which.d, \
  __VA_ARGS__                           \
)

#define RAF_DEBUG_INIT(which, label)                         \
  if (ARA_FALSE == __RandomAccessFileDebuggers.which.init) { \
    memset(&__RandomAccessFileDebuggers.which, 0,            \
           sizeof(__RandomAccessFileDebuggers.which));       \
    debug_init(&__RandomAccessFileDebuggers.which.d, label); \
    __RandomAccessFileDebuggers.which.init = ARA_TRUE;       \
  }                                                          \

#define raf(filename) ({       \
  RandomAccessFile file = {0}; \
  raf_init(&file, filename);   \
  (&file);                     \
})                             \

ARAboolean
raf_init(RandomAccessFile *raf, const ARAchar *filename);

ARAboolean
raf_open(RandomAccessFile *raf,
         const RandomAccessFileFlags flags,
         RandomAccessFileOpenCallback *callback);

ARAboolean
raf_close(RandomAccessFile *raf,
          RandomAccessFileCloseCallback *callback);

ARAboolean
raf_end(RandomAccessFile *raf,
        RandomAccessFileEndOptions *opts,
        RandomAccessFileEndCallback *callback);

ARAboolean
raf_read(RandomAccessFile *raf,
        RandomAccessFileReadOptions *opts,
        RandomAccessFileReadCallback *callback);

ARAboolean
raf_write(RandomAccessFile *raf,
          RandomAccessFileWriteOptions *opts,
          RandomAccessFileWriteCallback *callback);

#endif
