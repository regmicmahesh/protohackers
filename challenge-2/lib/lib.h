#ifndef LIB_H
#define LIB_H
#include "json.h"
#include <stdbool.h>

int extract_values(const char *data, char **method, double **number);

bool is_prime(double d_n);

#endif
