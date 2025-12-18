#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include "server.h"

#define MAX_MESSAGE_SIZE (1024 * 1024)

#define CURRENT_LINE "@ %s (%d)\n", __FILE__, __LINE__

// void handle_client(uint32_t client_fd) {

//   fprintf(stderr, "Client[%d] has connected.\n", client_fd);
//   goto disconnect;

//   char message_buf[MAX_MESSAGE_SIZE];
//   memset(message_buf, 0, MAX_MESSAGE_SIZE);

//   char ch;
//   int i = 0;
//   fprintf(stderr, "Client[%d] Request: ", client_fd);
//   while (read(client_fd, &ch, 1)) {
//     fputc(ch, stderr);
//     message_buf[i] = ch;
//     i++;

//     if (ch == EOF) {
//       goto disconnect;
//     }
//   }

// disconnect:
//   fprintf(stderr, "Client[%d] is disconnected.", client_fd);
//   shutdown(client_fd, SHUT_RDWR);
//   close(client_fd);
//   return;
// }

// int main(int argc, char **argv) {

//   errno = 0;

//   if (argc != 2) {
//     fprintf(stderr, "[USAGE] : %s [Address]\n", argv[0]);
//     exit(EXIT_FAILURE);
//   }

//   char *address = argv[1];
//   simple_server_t server = {};
//   simple_server_init(&server, address);
// }



char spinner_characters[] = {'-', '/', '|', '\\'};

int main(){

  int i = 0;
  while(1){

    printf(" \r%c Loading... [%d]", spinner_characters[i % 4], i / 100);

    if(i / 100 == 100) {
      break;
    }
    i++;
    usleep(720);
    
  }
}