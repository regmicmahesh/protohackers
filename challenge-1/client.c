#include <arpa/inet.h>
#include <stdbool.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_MESSAGE_SIZE 1000


int main(int argc, char **argv) {

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));


  server_addr.sin_family = AF_INET;

  // reset errno before doing anything.
  errno = 0;

  if (argc != 3) {
    fprintf(stderr, "[USAGE] : %s [IP Address] [Port]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int status = inet_pton(AF_INET, argv[1], &(server_addr.sin_addr));
  if (status <= 0) {
    fprintf(stderr, "Failed to convert IP standard network address: %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  char *endptr;
  server_addr.sin_port = htons(strtol(argv[2], &endptr, 0));

  if (*endptr != '\0') {
    fprintf(stderr, "Invalid Port: %s\n", argv[2]);
    exit(EXIT_FAILURE);
  }

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd < 0){
    fprintf(stderr, "Failed to obtain socket.");
  }


  status = connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if(status < 0){
    fprintf(stderr, "Failed to connect: %s\n", strerror(errno));
  }

  char message[MAX_MESSAGE_SIZE];

  while(true){
    printf("Enter your message:");

    char *read = fgets(message, sizeof(message), stdin);
    if(read == NULL){
        fprintf(stderr, "Goodbye.\n");
        exit(EXIT_SUCCESS);
    }

    int total_wb = 0;

    // adding 1 to include null terminator as well
    int total_length = strlen(message);

    while(total_wb < total_length) {
        int wb = write(fd, message + total_wb, total_length - total_wb);
        if(wb == -1){
            fprintf(stderr, "Failed to write to socket.\n");
        }
        total_wb += wb;
    }
  }

  close(fd);

  return EXIT_SUCCESS;
}
