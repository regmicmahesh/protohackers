#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wchar.h>

#include "server.h"

// address should be in the format "ip:port"
error_t simple_server_init(simple_server_t *server, char *address){


    struct sockaddr_in *server_addr = malloc(sizeof(struct sockaddr_in));

    size_t address_length = strlen(address);


    int port_idx = -1;

    for(size_t i = 0; i < address_length; i++){
        if(address[i] == ':'){
            port_idx = i+1;
            address[i] = '\0';
            break;
        }
    }


    if(port_idx == -1 || port_idx >= (int)address_length){
        return ERR_INVALID_ADDRESS;
    }

    int status = inet_pton(AF_INET, address, &(server_addr->sin_addr));
    if(status <= 0){
        return ERR_INVALID_IP_ADDRESS;
    }

    char *endptr;
    int port = strtol(&address[port_idx], &endptr, 0);
    if(*endptr != '\0'){
        return ERR_INVALID_PORT;
    }
    server_addr->sin_port = htons(port);

    server_addr->sin_family = AF_INET;
    server->server_addr = server_addr;

    return ERR_NONE;
}


error_t simple_server_listen(simple_server_t *server){
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server->server_fd < 0){
        return ERR_FAILED_OBTAIN_SOCKET;
    }

    int yes = 1;
    setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes));


    int status = bind(server->server_fd, 
        (struct sockaddr*)server->server_addr,
        sizeof(*server->server_addr)
    );
    if(status < 0){
        return ERR_FAILED_TO_BIND;
    }

    status = listen(server->server_fd, 10);
    if(status != 0){
        return ERR_FAILED_TO_LISTEN;
    }

    if(server->handler == NULL){
    return ERR_NONE;
    }

    struct sockaddr_in client_addr = {};
    socklen_t client_addr_len;

    while(true){
        int client_fd = accept(server->server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if(client_fd < 0){
            return ERR_FAILED_OBTAIN_CLIENT_SOCKET;
        }

        int pid = fork();
        if(pid == 0){
            server->handler(client_fd);
        }

    }


}

void simple_server_register_handler(simple_server_t *server,
                                    void (*handler)(uint32_t client_fd)) {

  server->handler = handler;
}