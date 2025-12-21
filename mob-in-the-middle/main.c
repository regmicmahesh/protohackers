#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "inc.h"

#define PROTOHACKER_SERVER_IP "206.189.113.124"
#define PROTOHACKER_SERVER_PORT 16963

#define TONY_ADDRESS "7YWHMfk9JZe0LM0g1ZauHuiSxhI"

struct epoll_event epoll_events[1024] = {};

typedef struct {
  int client_fd;
  int server_fd;
  char client_buf[4096];
  char server_buf[4096];
} connection_t;

connection_t conns[1024];
int conns_count = 0;

void delete_connection(connection_t *conn) {
  int idx = conns - conn;
  sprintf(stderr, "Deleting connection at index %d out of %d items", idx,
          conns_count);
  if (idx < 0) {
    return;
  }
  for (int i = idx; i < conns_count; i++) {
    conns[i] = conns[i + 1];
  }
  conns_count--;
}

connection_t *find_connection_by_client_fd(int client_fd) {
  for (int i = 0; i < conns_count; i++) {
    if (conns[i].client_fd == client_fd) {
      return &conns[i];
    }
  }
  return NULL;
}

connection_t *find_connection_by_server_fd(int server_fd) {
  for (int i = 0; i < conns_count; i++) {
    if (conns[i].server_fd == server_fd) {
      return &conns[i];
    }
  }
  return NULL;
}

void shutdown_both(int epfd, int fd) {
  int fd1, fd2 = -1;
  connection_t *c = NULL;

  connection_t *client_side = find_connection_by_client_fd(fd);

  if (client_side) {
    c = client_side;

    fd1 = client_side->client_fd;
    fd2 = client_side->server_fd;
  }
  // clean those up

  connection_t *server_side = find_connection_by_server_fd(fd);

  if (server_side) {
    c = server_side;

    fd1 = server_side->server_fd;
    fd2 = server_side->client_fd;
  }

  if (fd1 == -1 || fd2 == -1) {
    printf("Shutting down %d only.\n", fd);
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    shutdown(fd, SHUT_RDWR);
    close(fd);
    return;
  }
  printf("Shutting down %d and %d.\n", fd1, fd2);

  char buffer[4096];
  recv(fd1, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
  recv(fd2, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);

  epoll_ctl(epfd, EPOLL_CTL_DEL, fd1, NULL);
  epoll_ctl(epfd, EPOLL_CTL_DEL, fd2, NULL);

  shutdown(fd1, SHUT_RDWR);
  shutdown(fd2, SHUT_RDWR);

  close(fd1);
  close(fd2);

  delete_connection(c);
}

int protohacker_server_conn_init() {
  struct sockaddr_in server_addr = {};
  inet_pton(AF_INET, PROTOHACKER_SERVER_IP, &server_addr.sin_addr);
  server_addr.sin_port = htons(PROTOHACKER_SERVER_PORT);
  server_addr.sin_family = AF_INET;

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  CHK_ERR(server_fd);
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1},
             sizeof(int));

  CHK_ERR(
      connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)));

  return server_fd;
}

int check_valid_address(char *xs) {
  int length = strlen(xs);

  if (length < 26 || length > 35) {
    return 0;
  };

  if (*xs != '7') {
    return 0;
  }

  for (int i = 0; i < length; i++) {
    if (!isalnum(xs[i])) {
      return 0;
    }
  }
  return 1;
}

char *fix_message(char *xs) {

  int newline_flag = 0;
  char *newBuf = malloc(strlen(xs) * 2);
  memset(newBuf, 0, strlen(xs) * 2);

  if (xs[strlen(xs) - 1] == '\n') {
    newline_flag = 1;
    xs[strlen(xs) - 1] = '\0';
  }

  char *ptr = strsep(&xs, " ");

  while (ptr) {
    if (check_valid_address(ptr)) {
      strcat(newBuf, TONY_ADDRESS);
    } else {
      strcat(newBuf, ptr);
    }
    strcat(newBuf, " ");
    ptr = strsep(&xs, " ");
  };

  // consume the extra space in the end.
  newBuf[strlen(newBuf) - 1] = '\0';

  if (newline_flag) {
    strcat(newBuf, "\n");
  }

  return newBuf;
}
int main(int argc, char **argv) {

  // should be in format ./binary ip port
  assert(argc == 3);

  unsigned int ip;
  CHK_ERR(inet_pton(AF_INET, argv[1], &ip));

  int port = atoi(argv[2]);
  CHK_ZERO(port);

  struct sockaddr_in server_addr = {
      .sin_addr = {ip},
      .sin_port = htons(port),
      .sin_family = AF_INET,
  };

  int proxyfd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(proxyfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1},
             sizeof(int));

  CHK_ERR(bind(proxyfd, (struct sockaddr *)&server_addr, sizeof(server_addr)));
  CHK_ERR(listen(proxyfd, 10));

  int epfd = epoll_create1(0);
  CHK_ERR(epfd);

  struct epoll_event ev = {.data.fd = proxyfd,
                           .events = EPOLLERR | EPOLLIN | EPOLLHUP};

  epoll_ctl(epfd, EPOLL_CTL_ADD, proxyfd, &ev);

  for (;;) {
    int nfds = epoll_wait(epfd, epoll_events, sizeof(epoll_events), -1);

    struct epoll_event *evt;
    for (int i = 0; i < nfds && (evt = &epoll_events[i]); i++) {

      if (evt->data.fd == proxyfd) {
        if (evt->events & EPOLLIN) {
          struct sockaddr_in client_addr;

          int client_fd = accept(evt->data.fd, (struct sockaddr *)&client_addr,
                                 &(socklen_t){sizeof(client_addr)});
          CHK_ERR(client_fd);
          CHK_ERR(epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd,
                            &(struct epoll_event){
                                .data.fd = client_fd,
                                .events = EPOLLIN | EPOLLERR | EPOLLHUP,
                            }));

          int protohacker_server_fd = protohacker_server_conn_init();
          CHK_ERR(epoll_ctl(epfd, EPOLL_CTL_ADD, protohacker_server_fd,
                            &(struct epoll_event){
                                .data.fd = protohacker_server_fd,
                                .events = EPOLLIN | EPOLLERR | EPOLLHUP,
                            }));

          conns[conns_count] = (connection_t){
              .client_fd = client_fd, .server_fd = protohacker_server_fd};
          conns_count++;
          continue;

        } else if (evt->events & (EPOLLHUP | EPOLLERR)) {
          assert(0);
        }
      } else {

        if (evt->events & EPOLLIN) {
          connection_t *conn;
          char buf[1024 * 1024] = {};
          if ((conn = find_connection_by_client_fd(evt->data.fd))) {
            int rc = recv(evt->data.fd, buf, sizeof(buf), 0);
            printf("Received %d from client.\n", rc);
            if (!rc) {
              shutdown_both(epfd, evt->data.fd);
              continue;
            }
            if (strchr(buf, '\n')) {
              strcat(conn->client_buf, buf);
              char *fixed_buf = fix_message(conn->client_buf);
              write(conn->server_fd, fixed_buf, strlen(fixed_buf));
              printf("Writing -> %s\n", conn->client_buf);
              free(fixed_buf);
              memset(conn->client_buf, 0, 4096);
            } else {
              strcat(conn->client_buf, buf);
            }
          } else if ((conn = find_connection_by_server_fd(evt->data.fd))) {
            int rc = recv(evt->data.fd, buf, sizeof(buf), 0);
            printf("Received %d from server.\n", rc);
            if (!rc) {
              shutdown_both(epfd, evt->data.fd);
              continue;
            }
            if (strchr(buf, '\n')) {
              strcat(conn->server_buf, buf);
              char *fixed_buf = fix_message(conn->server_buf);
              write(conn->client_fd, fixed_buf, strlen(fixed_buf));
              printf("Writing -> %s\n", fixed_buf);
              free(fixed_buf);
              memset(conn->server_buf, 0, 4096);
            } else {
              strcat(conn->server_buf, buf);
            }
          } else {
            printf("Received on broken connection.\n");
            shutdown_both(epfd, evt->data.fd);
          }
        }

        if (evt->events & (EPOLLHUP | EPOLLERR)) {
          shutdown_both(epfd, evt->data.fd);
          continue;
        }
      }
    }
  }

  return EXIT_SUCCESS;
}