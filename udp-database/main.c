#include <arpa/inet.h>
#include <assert.h>
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
#include "kv.h"

char buf[1024] = {};
struct epoll_event epoll_events[1024] = {};

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

  int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1},
             sizeof(int));

  CHK_ERR(
      bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)));

  int epfd = epoll_create1(0);
  CHK_ERR(epfd);

  struct epoll_event ev = {.data.fd = server_fd,
                           .events = EPOLLERR | EPOLLIN | EPOLLHUP};

  epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

  for (;;) {
    size_t nfd = epoll_wait(epfd, epoll_events, sizeof(epoll_events), -1);

    for (size_t i = 0; i < nfd; i++) {
      int fd = epoll_events[i].data.fd;
      int evt = epoll_events[i].events;
      fprintf(stderr, "Event<data=%d,fd=%d>\n", epoll_events[i].events, fd);

      if (evt & EPOLLIN) {
        struct sockaddr_in client_addr = {};
        memset(buf, 0, 1024);
        int rc =
            recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr,
                     &(socklen_t){sizeof(client_addr)});

        fprintf(stderr, "Buf: %s\n", buf);
        CHK_ERR(rc);

        char *ptr_equals = strchr(buf, '=');
        if (ptr_equals == NULL) {
          // this is a retrieve query
          struct kv_entry *e = kv_find(buf);

          if (strncmp(buf, "version", sizeof("version")) == 0) {
            char res[1024] = "version=mukudb1.0";
            fprintf(stderr, "Res: %s", res);
            sendto(fd, res, strlen(res), 0, (struct sockaddr *)&client_addr,
                   sizeof(client_addr));
          }

          if (e == NULL) {
            continue;
          } else {
            char res[1024] = {};
            sprintf(res, "%s=%s", e->key, e->value);
            fprintf(stderr, "Res: %s", res);
            sendto(fd, res, strlen(res), 0, (struct sockaddr *)&client_addr,
                   sizeof(client_addr));
          }

        } else {
          *ptr_equals = '\0';
          if (strncmp(buf, "version", sizeof("version")) == 0) {
            continue;
          }
          char *key = strdup(buf);
          char *value = strdup(ptr_equals + 1);
          kv_insert(key, value);
        }
      }
    }
  }
}
