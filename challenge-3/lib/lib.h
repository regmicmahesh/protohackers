#include <stdint.h>

#ifndef LIB_H
#define LIB_H

struct message_insert_t {
  int32_t timestamp;
  int32_t price;
};

struct message_query_t {
  int32_t mintime;
  int32_t maxtime;
};

struct __attribute__((packed)) message_t {
  uint8_t op;
  union {
    struct message_insert_t imsg;
    struct message_query_t qmsg;
  };
};

int message_init(struct message_t *message, void *payload);

typedef struct price_item_t {
  int32_t timestamp;
  int32_t price;
  struct price_item_t *next;
} price_item_t;

price_item_t *price_list_init(int32_t timestamp, int32_t price);

void price_list_free(price_item_t *l);

int32_t price_list_mean(price_item_t *l, int32_t mintime, int32_t maxtime);

price_item_t* price_list_append(price_item_t *l, int32_t timestamp, int32_t price);

#endif
