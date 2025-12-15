#include "lib/lib.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_MESSAGE_SIZE (1024*1024)

#define IS_PRIME_RESPONSE "{\"method\":\"isPrime\",\"prime\":true}\n"
#define IS_NOT_PRIME_RESPONSE "{\"method\":\"isPrime\",\"prime\":false}\n"

void handle_client(int client_fd) {

  fprintf(stderr, "Client[%d] has connected.\n", client_fd);

  char message_buf[MAX_MESSAGE_SIZE];
  memset(message_buf, 0, MAX_MESSAGE_SIZE);

  int rc;
  char ch;
  int i = 0;
  fprintf(stderr, "Client[%d] Request: ", client_fd);
  while (read(client_fd, &ch, 1)) {
    fputc(ch, stderr);
    message_buf[i] = ch;
    i++;

    if (ch == EOF) {
      goto disconnect;
    }

    if (ch == '\n') {
      char *method;
      double *number;
      message_buf[i] = '\0';
      int extraction_success = extract_values(message_buf, &method, &number);
      if (extraction_success < 1) {
        goto disconnect;
      }

      i = 0;
      int is_prime_number = is_prime(*number);
      if (is_prime_number) {
        fprintf(stderr, "Client[%d] Response: %s", client_fd, IS_PRIME_RESPONSE);
        write(client_fd, IS_PRIME_RESPONSE, strlen(IS_PRIME_RESPONSE));
      } else {
        fprintf(stderr, "Client[%d] Response: %s", client_fd, IS_NOT_PRIME_RESPONSE);
        write(client_fd, IS_NOT_PRIME_RESPONSE, strlen(IS_NOT_PRIME_RESPONSE));
      }
    }

    // fprintf(stderr, "Read %d bytes\n", rc);
    // fputs(message_buf, stderr);
    // char *method;
    // double *number;
    // message_buf[rc] = '\0';
    // int extraction_success = extract_values(message_buf, &method, &number);
    // if (extraction_success < 1) {
    //     fprintf(stderr, "Failed extraction...\n");
    //   goto disconnect;
    // }

    // int is_prime_number = is_prime(*number);
    // if (is_prime_number) {
    //   write(client_fd, IS_PRIME_RESPONSE, strlen(IS_PRIME_RESPONSE));
    // } else {
    //   write(client_fd, IS_NOT_PRIME_RESPONSE, strlen(IS_NOT_PRIME_RESPONSE));
    // }
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
