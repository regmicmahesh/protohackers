#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "lib/lib.h"

#define PAYLOAD_SIZE 9

void handle_client(int client_fd) {

  fprintf(stderr, "Client[%d] has connected.\n", client_fd);

  char message_buf[PAYLOAD_SIZE];
  memset(message_buf, 0, PAYLOAD_SIZE);

  char ch;
  int i = 0;

  price_item_t *price_list = NULL;

  fprintf(stderr, "New client connected.\n");


  while (read(client_fd, &ch, 1)) {
    message_buf[i] = ch;
    i++;
    fputc(ch, stderr);

    if (i == 9) {
      struct message_t msg = {};
      message_init(&msg, message_buf);

      if (msg.op == 'I') {
        fprintf(stderr, "Inserting...\n");
        price_list =
            price_list_append(price_list, msg.imsg.timestamp, msg.imsg.price);
      } else if (msg.op == 'Q') {
        fprintf(stderr, "Querying...\n");
        int32_t mean =
            price_list_mean(price_list, msg.qmsg.mintime, msg.qmsg.maxtime);
        fprintf(stderr, "Mean: %d\n", mean);
        int32_t be_mean = htonl(mean);
        write(client_fd, &be_mean, sizeof(be_mean));
      } else {
        goto disconnect;
      }

      i = 0;
    }

    if (ch == EOF) {
      goto disconnect;
    }
  }

disconnect:
  fprintf(stderr, "Client[%d] is disconnected.", client_fd);
  shutdown(client_fd, SHUT_RDWR);
  close(client_fd);
  return;
}

int main(int argc, char **argv) {

  struct sockaddr_in server_addr = {};
  struct sockaddr_in client_addr = {};

  server_addr.sin_family = AF_INET;

  errno = 0;

  if (argc != 3) {
    fprintf(stderr, "[USAGE] : %s [IP Address] [Port]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int client_fd = inet_pton(AF_INET, argv[1], &(server_addr.sin_addr));
  if (client_fd <= 0) {
    fprintf(stderr, "Failed to convert IP standard network address: %s\n",
            argv[1]);
    exit(EXIT_FAILURE);
  }

  char *endptr;
  server_addr.sin_port = htons(strtol(argv[2], &endptr, 0));

  if (*endptr != '\0') {
    fprintf(stderr, "Invalid Port: %s\n", argv[2]);
    exit(EXIT_FAILURE);
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "Failed to obtain socket.");
  }

  int yes = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes,
             sizeof(yes));

  client_fd =
      bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (client_fd < 0) {
    fprintf(stderr, "Bind Failed");
    exit(EXIT_FAILURE);
  }

  socklen_t client_addr_size = sizeof(client_addr_size);

  client_fd = listen(server_fd, 10);
  fprintf(stderr, "Server is up and running\n");
  if (client_fd < 0) {
    fprintf(stderr, "Failed to listen");
    exit(EXIT_FAILURE);
  }

  while (true) {
    client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_fd < 0) {
      fprintf(stderr, "Failed to accept");
      continue;
    }

    int pid = fork();
    if (pid == 0)
      handle_client(client_fd);
  }
}

recv(int fd, void *buf, size_t n, int flags)