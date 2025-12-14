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

#define MAX_MESSAGE_SIZE 1024

void handle_client(int client_fd) {

  char message_buf[MAX_MESSAGE_SIZE];

  int rc;

  while ((rc = read(client_fd, message_buf, sizeof(message_buf))) > 0) {
    fprintf(stderr, "Read %d bytes", rc);
    write(client_fd, message_buf, rc);
  }
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

  client_fd = listen(server_fd, 1);
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

    // spawn a thread to handle this client_fd

    fprintf(stderr, "Client[%d] has connected!\n", client_addr.sin_addr.s_addr);
    handle_client(client_fd);
  }
}
