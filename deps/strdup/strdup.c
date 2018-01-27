
//
// strdup.c
//
// Copyright (c) 2014 Stephen Mathieson
// MIT licensed
//

#ifndef HAVE_STRDUP
#if !defined( _MSC_VER )
#include <stdlib.h>
#include <string.h>
#include "strdup.h"

char *
strdup(const char *str) {
  int len = strlen(str) + 1;
  char *buf = malloc(len);
  if (buf) memcpy(buf, str, len);
  return buf;
}
#endif
#endif /* HAVE_STRDUP */
