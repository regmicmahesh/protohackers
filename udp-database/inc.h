#ifndef _INC_H
#define _INC_H

#include <string.h>
#define CHK_ERR(x)                                                             \
  if ((x) < 0) {                                                               \
    perror("error occured");                                                   \
  }
#define CHK_ZERO(x)                                                            \
  if ((x) == 0) {                                                              \
    perror("error occured");                                                   \
  }

#endif

void strtrim(char *s) {
  for (size_t i = 0; i < strlen(s); i++) {
    if (s[i] == '\n') {
      s[i] = '\0';
      break;
    }
  }
}