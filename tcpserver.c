#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> // read(), write(), close()
#include "redislikeinc.h"
#define BUFFER_SIZE 100
#define PORT 8080 
#define SA struct sockaddr_in
#define NETWORK_INTERFACE "0.0.0.0" 
#define closesocket close


void tcpServer(RedisDB * db){


    //create socket
    int serverFd, clientFd;
    SA  myAddress, clientAddress;
    int clientLen = sizeof(clientAddress);
    char buffer[BUFFER_SIZE];
     
    serverFd = socket(AF_INET,SOCK_STREAM,0);
    if(serverFd == -1){
        perror("Failed to create socket\n");
        closesocket(serverFd);
        exit(1);
    }

    //Assign ip and port
    memset(&myAddress,0,sizeof(myAddress));
    myAddress.sin_family = AF_INET; 
    myAddress.sin_addr.s_addr = inet_addr(NETWORK_INTERFACE); 
    myAddress.sin_port = htons(PORT); 

    //bind the socket with ip and port
    if(bind(serverFd,(struct sockaddr*)&myAddress, sizeof(myAddress)) < 0){
        perror("Failed to bind socket");
        closesocket(serverFd);
        exit(1);

    }

    //Start listening on server socket
    if(listen(serverFd,3)<0){
        perror("Failed at istening stage");
        closesocket(serverFd);
        exit(1);
    }
    
    printf("Server Started listening on port:%d ...\n",PORT);
    
    //Accepting the connection request
     while (1) {
        clientFd = accept(serverFd, (struct sockaddr*)&clientAddress,&clientLen);
        if (clientFd < 0) {
            perror("accept");
            continue;
        }

        printf("Client connected\n");

       
        while (1) {
            char sendBuffer[BUFFER_SIZE];
            int bytes = recv(clientFd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes <= 0) break;

            buffer[bytes] = '\0';
            printf("Received: %s\n", buffer);

            execute_command_for_server(db,parse_command(buffer),sendBuffer);
            send(clientFd,sendBuffer,strlen(sendBuffer),0);

        }

        printf("Client disconnected\n");
        closesocket(clientFd);
    }

    closesocket(serverFd);















}