#ifndef _SERVER_H
#define _SERVER_H

#include <arpa/inet.h>
#include <stdint.h>

typedef enum {
  ERR_NONE,
  ERR_INVALID_PORT,
  // address is the overall format containing ip:port
  ERR_INVALID_ADDRESS,
  ERR_INVALID_IP_ADDRESS,

  ERR_FAILED_OBTAIN_SOCKET,
  ERR_FAILED_TO_BIND,
  ERR_FAILED_TO_LISTEN,

  ERR_FAILED_OBTAIN_CLIENT_SOCKET

} error_t;

typedef struct simple_server_t {
  uint32_t server_fd;
  struct sockaddr_in *server_addr;
  void (*handler)(uint32_t client_fd);
} simple_server_t;

// address should be in the format "ip:port"
error_t simple_server_init(simple_server_t *server, char *address);

error_t simple_server_listen(simple_server_t *server);

void simple_server_register_handler(simple_server_t *server,
                                    void (*handler)(uint32_t client_fd));

#endif