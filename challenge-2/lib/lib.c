
#include "json.h"
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_METHOD_SIZE 1024

int extract_values(const char *data_string, char **method, double **number) {

  char *tmp_method = NULL;
  double *tmp_number = NULL;

  struct json_value_s *data = json_parse(data_string, strlen(data_string));

  if (data == NULL) {
    goto error;
  }

  //  should be object
  if (data->type != json_type_object) {
    goto error;
  }

  struct json_object_s *object = (struct json_object_s *)data->payload;
  struct json_object_element_s *el = object->start;

  while (el != NULL) {

    struct json_string_s *key = el->name;

    if (strncmp(key->string, "method", 7) == 0) {
      struct json_value_s *method_val = el->value;
      if (method_val->type != json_type_string) {
        return -1;
      }
      struct json_string_s *method_val_string =
          (struct json_string_s *)method_val->payload;

      if (strncmp(method_val_string->string, "isPrime", 8) != 0) {
        if (tmp_number != NULL)
          free(tmp_number);
        return -1;
      }
      tmp_method = (char *)malloc(MAX_METHOD_SIZE * sizeof(char));
      strncpy(tmp_method, "isPrime", MAX_METHOD_SIZE);
    }

    if (strncmp(key->string, "number", 7) == 0) {
      struct json_value_s *number_val = el->value;
      if (number_val->type != json_type_number) {
        goto error;
      }
      struct json_number_s *number_val_number =
          (struct json_number_s *)number_val->payload;

      tmp_number = (double *)malloc(sizeof(double));
      char *endptr;
      *tmp_number = strtod(number_val_number->number, &endptr);

      if (*endptr != '\0') {
        goto error;
      }
    }

    // do sth
    el = el->next;
  }

  if (tmp_method == NULL || tmp_number == NULL) {
    goto error;
  }

  *method = tmp_method;
  *number = tmp_number;
  free(data);
  return 1;

error:
  if (data != NULL)
    free(data);
  if (tmp_method != NULL)
    free(tmp_method);
  if (tmp_number != NULL)
    free(tmp_number);
  *method = NULL;
  *number = NULL;
  return -1;
}

bool is_prime(double d_n) {
  if (ceil(d_n) != floor(d_n)) {
    return false;
  }

  int n = d_n;

  if (n < 2) {
    return false;
  }

  if (n == 2) {
    return true;
  }

  if (n % 2 == 0) {
    return false;
  }

  int sqrt_n = sqrt(n);

  for (int i = 3; i <= sqrt_n; i += 2) {
    if (n % i == 0) {
      return false;
    }
  }
  return true;
}
