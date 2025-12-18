#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "inc.h"

#define MAX_CONN_COUNT 64
#define MAX_EVENT_COUNT 128

#define MESSAGE_BUF_SIZE 1024

typedef struct {
  unsigned int fd;
  char *username;
  char buf[MESSAGE_BUF_SIZE];
} _user_t;

_user_t users[MAX_CONN_COUNT];
size_t user_count = 0;

char welcome_message[] = "Welcome to mukuchat! What shall I call you?\n";

struct epoll_event events[MAX_EVENT_COUNT];

void user_add(char *username, int client_fd) {
  users[user_count] = (_user_t){.username = username, .fd = client_fd};
  user_count++;
}

// returns NULL if user is not found, else a pointer to user is returned.
_user_t *user_find_by_fd(unsigned int fd) {
  for (size_t i = 0; i < user_count; i++) {
    if (users[i].fd == fd) {
      return &users[i];
    }
  }
  return NULL;
}

void user_del_by_fd(unsigned int fd) {
  _user_t *user = user_find_by_fd(fd);
  assert(user != NULL);

  int ui = -1;
  for (size_t i = 0; i < user_count; i++) {
    if (users[i].fd == fd) {
      ui = i;
    }
  }

  for (size_t i = ui; i < user_count; i++) {
    users[i] = users[i + 1];
  }
  user_count--;
}

void write_n(int fd, char *buf, size_t _n) {

  _user_t *u = user_find_by_fd(fd);

  if (strrchr(buf, '\n')) {
    fprintf(stderr, "W | ");
    u && u->username &&fprintf(stderr, "%d %s", u->fd, u->username);
    fputs(buf, stderr);
    write(fd, buf, strlen(buf));
  } else {
    strcat(buf, "\n");
    fprintf(stderr, "W | ");
    u && u->username &&fprintf(stderr, "%d %s", u->fd, u->username);
    fputs(buf, stderr);
    write(fd, buf, strlen(buf));
  }
}

int read_n(int fd, void *buf, size_t nbytes) {
  _user_t *u = user_find_by_fd(fd);
  int rc = read(fd, buf, nbytes);

  if (u && u->username)
    fprintf(stderr, "R | %d %s -> %s\n", u->fd, u->username, (char *)buf);
  else
    fprintf(stderr, "R | unknown -> %s\n", (char *)buf);

  return rc;
}

void broadcast_joined(unsigned int fd) {

  char presence_notification[MESSAGE_BUF_SIZE] = "* The room contains:";
  char join_notification[MESSAGE_BUF_SIZE] = {};

  _user_t *u = user_find_by_fd(fd);

  sprintf(join_notification, "* %s has entered the room\n", u->username);

  for (size_t i = 0; i < user_count; i++) {
    if (users[i].username == NULL) {
      continue;
    }
    if (users[i].fd == fd) {
      continue;
    }

    char *username_cpy = strdup(users[i].username);

    if (users[i].username) {
      strcat(presence_notification, strcat(username_cpy, " "));
      free(username_cpy);
      write_n(users[i].fd, join_notification, strlen(join_notification));
    }
  }
  presence_notification[strlen(presence_notification)] = '\n';
  write_n(fd, presence_notification, strlen(presence_notification));
}

void broadcast_message(unsigned int fd) {

  _user_t *u = user_find_by_fd(fd);

  char *msg = strdup(u->buf);

  if (strchr(u->buf, '\n') == NULL) {
    sprintf(u->buf, "[%s] %s\n", u->username, msg);
  } else {
    sprintf(u->buf, "[%s] %s", u->username, msg);
  }

  free(msg);

  for (size_t i = 0; i < user_count; i++) {
    if (users[i].fd == fd) {
      continue;
    }

    if (users[i].username == NULL) {
      continue;
    }

    if (users[i].username) {
      write_n(users[i].fd, u->buf, strlen(u->buf));
    }
  }
  memset(u->buf, 0, MESSAGE_BUF_SIZE);
}

void broadcast_left(unsigned int fd) {

  char left_notification[MESSAGE_BUF_SIZE] = {};

  _user_t *u = user_find_by_fd(fd);

  sprintf(left_notification, "* %s has left the room\n", u->username);

  for (size_t i = 0; i < user_count; i++) {
    if (users[i].fd == fd) {
      continue;
    }

    if (users[i].username != NULL) {
      write_n(users[i].fd, left_notification, strlen(left_notification));
    }
  }
  left_notification[strlen(left_notification)] = '\n';
  write_n(fd, left_notification, strlen(left_notification));
}

// 1 character, 1 uppercase, lowercase and digit

int is_valid_username(char *str) {
  size_t length = strlen(str);

  if (length < 1) {
    return 0;
  }

  for (size_t i = 0; i < length; i++) {
    int pred = isupper(str[i]) || islower(str[i]) || isdigit(str[i]);
    if (!pred) {
      return 0;
    }
  }
  return 1;
}

int main(int argc, char **argv) {

  // should be in format ./binary ip port
  assert(argc == 3);

  unsigned int ip;
  CHK_ZERO(inet_pton(AF_INET, argv[1], &ip));

  int port = atoi(argv[2]);
  CHK_ZERO(port);

  struct sockaddr_in server_addr = {
      .sin_addr = {ip},
      .sin_port = htons(port),
      .sin_family = AF_INET,
  };

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1},
             sizeof(int));
  CHK_ERR(
      bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)));

  CHK_ERR(listen(server_fd, MAX_CONN_COUNT));

  int epoll_fd = epoll_create1(0);
  CHK_ERR(epoll_fd);

  CHK_ERR(epoll_ctl(
      epoll_fd, EPOLL_CTL_ADD, server_fd,
      &(struct epoll_event){.data.fd = server_fd,
                            .events = EPOLLIN | EPOLLERR | EPOLLHUP}));

  for (;;) {
    int nfds = epoll_wait(epoll_fd, events, 1, -1);
    CHK_ERR(nfds);

    for (int i = 0; i < nfds; i++) {
      fprintf(stderr, "Event<fd=%d,data=%x>\n", events[i].data.fd,
              events[i].data.u32);
      int event_fd = events[i].data.fd;

      if (event_fd == server_fd) {
        if (events[i].events & EPOLLIN) {

          struct sockaddr_in client_addr;
          int client_fd = accept(event_fd, (struct sockaddr *)&client_addr,
                                 &(socklen_t){sizeof(client_addr)});
          CHK_ERR(client_fd);
          CHK_ERR(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd,
                            &(struct epoll_event){
                                .data.fd = client_fd,
                                .events = EPOLLIN | EPOLLERR | EPOLLHUP,
                            }));

          write_n(client_fd, welcome_message, sizeof(welcome_message) - 1);
          user_add(NULL, client_fd);
        }
        continue;
      }

      if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {

        _user_t *u = user_find_by_fd(events[i].data.fd);

        if (u->username != NULL) {
          broadcast_left(u->fd);
        }
        shutdown(u->fd, SHUT_RDWR);
        close(u->fd);
        user_del_by_fd(u->fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
        continue;
      }

      if (events[i].events & EPOLLIN) {
        _user_t *u = user_find_by_fd(events[i].data.fd);

        if (u->username == NULL) {
          int rc = read_n(events[i].data.fd, u->buf, MESSAGE_BUF_SIZE);
          CHK_ERR(rc);

          u->buf[rc - 1] = '\0';

          if (!is_valid_username(u->buf)) {
            shutdown(u->fd, SHUT_RDWR);
            close(u->fd);
            user_del_by_fd(u->fd);
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
            continue;
          }

          char *username = strdup(u->buf);
          memset(u->buf, 0 , MESSAGE_BUF_SIZE);
          u->username = username;
          broadcast_joined(u->fd);

        } else {
          char temp_buf[MESSAGE_BUF_SIZE] = {};
          int rc = read_n(u->fd, temp_buf, MESSAGE_BUF_SIZE);
          //temp_buf[rc] = '\0';

          if (rc == EOF || rc == 0) {
            if (u->username != NULL) {
              broadcast_left(u->fd);
            }
            shutdown(u->fd, SHUT_RDWR);
            close(u->fd);
            user_del_by_fd(u->fd);
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
          } else if (strlen(temp_buf) == 0) {
            continue;
          } else {


            if(strchr(temp_buf, '\n')){
              strcat(u->buf, temp_buf);
              broadcast_message(u->fd);
            } else {
              strcat(u->buf, temp_buf);
            }

          }
        }
      }
    }
  }

  return EXIT_SUCCESS;
}