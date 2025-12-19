
#ifndef _KV_H
#define _KV_H
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct kv_entry {
  char *key;
  char *value;
};
struct kv_entry kv[1024 * 1024 * 10] = {};
size_t _kv_size = 0;

void kv_append(const char *key, const char *value) {
  kv[_kv_size] = (struct kv_entry){.key = strdup(key), .value = strdup(value)};
  _kv_size++;
}

struct kv_entry *kv_find(const char *key) {

  for (size_t i = 0; i < _kv_size; i++) {
    if (strcmp(kv[i].key, key) == 0) {
      return &kv[i];
    }
  }

  return NULL;
}

void kv_insert(const char *key, const char *value) {
  struct kv_entry *existing = kv_find(key);
  if (existing == NULL) {
    kv_append(key, value);
  } else {
    free(existing->key);
    free(existing->value);
    existing->key = strdup(key);
    existing->value = strdup(value);
  }
}
#endif