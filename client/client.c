#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h> 

#define BUFFER_SIZE 80 
#define PORT 1234 




  
int main() 
{ 
    int sockfd, rc; 
    struct sockaddr_in servaddr; 
  
    unsigned char buffer[BUFFER_SIZE];


    char my_key[] = "my_secret_key";

    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    memset(&servaddr, 0, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(PORT); 
  
    // connect the client socket to server socket 
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("connected to the server..\n"); 
  
    // recv nonce
    rc = recv(sockfd, buffer, sizeof(unsigned char), 0);
    
    unsigned char nonce = buffer[0];

    // xor with known key
    for (int i = 0; i < strlen(my_key); i++)
        buffer[i] = my_key[i] ^ nonce;

    rc = send(sockfd, buffer, sizeof(my_key), 0);

    // close the socket 
    close(sockfd); 
} 

