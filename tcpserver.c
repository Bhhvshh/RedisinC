#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h> // read(), write(), close()
#include "redislikeinc.h"
#define BUFFER_SIZE 100
#define PORT 8080
#define SA struct sockaddr_in
#define NETWORK_INTERFACE "0.0.0.0"
#define closesocket close
#define MAX_EVENTS 64

static int set_non_blocking(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void tcpServer(RedisDB *db){
    int serverFd;
    int epollFd;
    SA myAddress;
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    int enable = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
        closesocket(serverFd);
        exit(1);
    }

    if (set_non_blocking(serverFd) < 0) {
        perror("Failed to set non-blocking on server socket");
        closesocket(serverFd);
        exit(1);
    }

    memset(&myAddress, 0, sizeof(myAddress));
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = inet_addr(NETWORK_INTERFACE);
    myAddress.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr *)&myAddress, sizeof(myAddress)) < 0) {
        perror("Failed to bind socket");
        closesocket(serverFd);
        exit(1);
    }

    if (listen(serverFd, SOMAXCONN) < 0) {
        perror("Failed at listening stage");
        closesocket(serverFd);
        exit(1);
    }

    epollFd = epoll_create1(0);
    if (epollFd < 0) {
        perror("epoll_create1 failed");
        closesocket(serverFd);
        exit(1);
    }

    event.events = EPOLLIN;
    event.data.fd = serverFd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &event) < 0) {
        perror("epoll_ctl add server socket failed");
        closesocket(epollFd);
        closesocket(serverFd);
        exit(1);
    }

    printf("Server started listening on port:%d ...\n", PORT);

    while (1) {
        int ready = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        if (ready < 0) {
            perror("epoll_wait failed");
            continue;
        }

        for (int i = 0; i < ready; i++) {
            int currentFd = events[i].data.fd;

            if (currentFd == serverFd) {
                while (1) {
                    SA clientAddress;
                    socklen_t clientLen = sizeof(clientAddress);
                    int clientFd = accept(serverFd, (struct sockaddr *)&clientAddress, &clientLen);

                    if (clientFd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        perror("accept failed");
                        break;
                    }

                    if (set_non_blocking(clientFd) < 0) {
                        perror("Failed to set non-blocking on client socket");
                        closesocket(clientFd);
                        continue;
                    }

                    event.events = EPOLLIN;
                    event.data.fd = clientFd;
                    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &event) < 0) {
                        perror("epoll_ctl add client failed");
                        closesocket(clientFd);
                        continue;
                    }

                    printf("Client connected (fd=%d)\n", clientFd);
                }
            } else {
                char buffer[BUFFER_SIZE];
                char sendBuffer[BUFFER_SIZE];
                int bytes = recv(currentFd, buffer, BUFFER_SIZE - 1, 0);

                if (bytes <= 0) {
                    printf("Client disconnected (fd=%d)\n", currentFd);
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, currentFd, NULL);
                    closesocket(currentFd);
                    continue;
                }

                buffer[bytes] = '\0';
                printf("Received from fd=%d: %s\n", currentFd, buffer);

                execute_command_for_server(db, parse_command(buffer), sendBuffer);
                send(currentFd, sendBuffer, strlen(sendBuffer), 0);
            }
        }
    }

    closesocket(epollFd);
    closesocket(serverFd);
}
