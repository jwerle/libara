RM ?= $(shell which rm)
CWD ?= $(shell pwd)
BUILD_LIBRARY_PATH = $(CWD)/../../build/lib

## dependency source files
DEPS += $(wildcard ../../deps/*/*.c)

## source files
MAIN = main.c
TARGET = raf
#SOURCE += $(filter-out $(MAIN), $(wildcard *.c) $(DEPS))
SOURCE += $(wildcard *.c) $(DEPS)
OBJECTS = $(SOURCE:.c=.o)

LIBS = $(wildcard ../../build/lib/*.a)

## compiler flags
CFLAGS += -I ../../build/include
CFLAGS += -I ../../deps
CFLAGS += -L $(BUILD_LIBRARY_PATH)
CFLAGS += -l uv
CFLAGS += -l m

## we need to set the LD_LIBRARY_PATH environment variable
## so our executables can load the built library at runtime
export LD_LIBRARY_PATH = $(BUILD_LIBRARY_PATH)
export DYLD_LIBRARY_PATH = $(BUILD_LIBRARY_PATH)

## Brief commands
ifneq (1,$(NO_BRIEF))

BRIEFC += MKDIR
BRIEFC += STRIP
BRIEFC += MAKE
BRIEFC += RM
BRIEFC += LN
BRIEFC += AR
BRIEFC += CC

-include ../../mk/brief.mk
endif

.PHONY: default
default: $(TARGET)

$(TARGET): $(OBJECTS)
	@$(CC) $(OBJECTS) $(LIBS) $(CFLAGS) -o $@

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECTS)
