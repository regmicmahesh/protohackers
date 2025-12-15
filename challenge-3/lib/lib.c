#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"



int message_init(struct message_t *message, void *payload) {

  struct message_t tmp_message = *(struct message_t *)payload;

  switch (tmp_message.op) {
  case 'I':
    tmp_message.imsg.timestamp = ntohl(tmp_message.imsg.timestamp);
    tmp_message.imsg.price = ntohl(tmp_message.imsg.price);
    break;
  case 'Q':
    tmp_message.qmsg.maxtime = ntohl(tmp_message.qmsg.maxtime);
    tmp_message.qmsg.mintime = ntohl(tmp_message.qmsg.mintime);
    break;
  default:
    return -1;
  }
  memcpy(message, &tmp_message, sizeof(*message));
  return 0;
}

price_item_t *price_list_init(int32_t timestamp, int32_t price) {
  price_item_t *item = malloc(sizeof(price_item_t));
  *item = (price_item_t){
      .price = price,
      .timestamp = timestamp,
  };
  return item;
}

void price_list_free(price_item_t *l) {
  while (l != NULL) {
    price_item_t *next = l->next;
    free(l);
    l = next;
  }
}

int32_t price_list_mean(price_item_t *l, int32_t mintime, int32_t maxtime) {
  int c = 0;

  int64_t sum = 0;

  while (l != NULL) {
    if (l->timestamp >= mintime && l->timestamp <= maxtime) {
      c++;
      sum += l->price;
    }
    l = l->next;
  }

  if (c == 0) {
    return 0;
  }

  return sum / c;
}

price_item_t* price_list_append(price_item_t *l, int32_t timestamp, int32_t price) {
  price_item_t *new_item = price_list_init(timestamp, price);
  new_item->next = l;
  return new_item;
}

