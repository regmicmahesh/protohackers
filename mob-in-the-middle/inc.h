#ifndef _INC_H
#define _INC_H

#include <stdio.h>
#include <string.h>

#define CHK_ERR(x)                                                             \
  if ((x) < 0) {                                                               \
    perror("error occured");                                                   \
  }
#define CHK_ZERO(x)                                                            \
  if ((x) == 0) {                                                              \
    perror("error occured");                                                   \
  }

#define FOR_EACH(val, item, len)                                               \
  for (int i = 0; i < len && (item = val[i]); i++)

#define FOR_EACH_PTR(val, item, len)                                           \
  for (int i = 0; i < len && (item = &val[i]); i++)

#endif