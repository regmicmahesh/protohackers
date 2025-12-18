#ifndef _INC_H
#define _INC_H
#define CHK_EQ(x, y)                                                           \
  if ((x) == (y)) {                                                            \
    perror("error occured");                                                   \
    exit(EXIT_FAILURE);                                                        \
  }
#define CHK_ZERO(x) CHK_EQ(x, 0)
#define CHK_ERR(x)                                                             \
  if ((x) < 0) {                                                               \
    perror("error occured");                                                   \
    exit(EXIT_FAILURE);                                                        \
  }

#endif // _INC_H
