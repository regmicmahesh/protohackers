#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CHAT_ROOM_CAPACITY 1000

typedef struct {
  int client_fd;
  char *username;
} user_t;

void user_init(user_t *user, int client_fd, char *username) {
  user->client_fd = client_fd;
  user->username = malloc(strlen(username) + 1);
  strncpy(user->username, username, strlen(username) + 1);
}

void user_free(user_t *user) {
  free(user->username);
  free(user);
}

typedef struct {
  user_t **users;
  size_t user_count;
  size_t room_capacity;
} chat_room_t;

void chat_room_init(chat_room_t *cr) {
  cr->user_count = 0;
  cr->room_capacity = INITIAL_CHAT_ROOM_CAPACITY;
  cr->users = malloc(sizeof(user_t) * cr->room_capacity);
}

// chat room free doesn't clear the memory used for the user.:w
void chat_room_free(chat_room_t *cr) {
  free(cr->users);
  free(cr);
}

void chat_room_user_connect(chat_room_t *cr, user_t *user) {
  if (cr->user_count >= cr->room_capacity) {
    cr->room_capacity *= 2;
    cr->users = realloc(cr->users, sizeof(user_t) * cr->room_capacity);
  }
  cr->users[cr->user_count] = user;
  cr->user_count++;
}

void chat_room_user_disconnect(chat_room_t *cr, user_t *user) {
  size_t user_idx = -1;
  for (size_t i = 0; i < cr->user_count; i++) {
    if (cr->users[i] == user) {
      user_idx = i;
      break;
    }
  }

  for (size_t i = user_idx; i < cr->user_count; i++) {
    cr->users[i] = cr->users[i + 1];
  }

  cr->user_count--;
}