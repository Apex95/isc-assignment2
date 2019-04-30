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

#define CMD_BUFFER_SIZE 1000


 
int main() 
{ 
    int sockfd, rc, max_sd, desc_ready; 
    struct sockaddr_in servaddr; 
  
    char buffer[BUFFER_SIZE];
    char cmd_buffer[CMD_BUFFER_SIZE];

    struct timeval timer;
    timer.tv_sec = 30;
    timer.tv_usec = 0;

    fd_set working_set, master_set;

    char my_key[] = "my_secret_key";

    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created...\n"); 
    
    memset(&servaddr, 0, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(PORT); 
  
    // connect the client socket to server socket 
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) { 
        printf("Connection to the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("Connected to the server.\n"); 

    

    memset(buffer, 0, sizeof(buffer));

    // recv nonce
    rc = recv(sockfd, buffer, sizeof(char), 0);
    printf("Receiving challenge...\n");

    char nonce = buffer[0];

    // xor with known key
    for (int i = 0; i < strlen(my_key); i++)
        buffer[i] = my_key[i] ^ nonce;

    printf("Replying...\n");
    rc = send(sockfd, buffer, sizeof(my_key), 0);

    FD_ZERO(&master_set);
    FD_SET(sockfd, &master_set);
    FD_SET(STDIN_FILENO, &master_set);
    
    max_sd = sockfd;

    printf("Done! Type your commands below:\n\n");
    while (1)
    {
        memcpy(&working_set, &master_set, sizeof(master_set));

        rc = select(max_sd + 1, &working_set, NULL, NULL, &timer);

        if (rc < 0)
        {
            printf("select() failed...\n");
            exit(0);
        }


        desc_ready = rc;

        for (int i = 0; i <= max_sd && desc_ready > 0; i++)
        {
            if (FD_ISSET(i, &working_set))
            {
                desc_ready = -1;

                if (i == STDIN_FILENO)
                {
                    fgets(buffer, BUFFER_SIZE-1, stdin);
                    buffer[strlen(buffer)-1] = '\0';

                    printf("----------------\n");
                    printf(">> Executing [%s]\n", buffer);
                    rc = send(sockfd, buffer, strlen(buffer) + 1, 0);

                    printf("----------------\n");
                }
                else
                {
                    memset(cmd_buffer, 0, sizeof(cmd_buffer));      
                   
                    
                    rc = recv(i, cmd_buffer, sizeof(cmd_buffer), 0);

                    int offset = 0;
                    
                    for (int j = 0; j < rc; j++)
                    {
                        if (cmd_buffer[j + offset] == '\n' && cmd_buffer[j + offset + 1] == '\0')
                           offset++;


                        if (j + offset < rc)
                        {
                            cmd_buffer[j] = cmd_buffer[j+offset];
                            
                            if (cmd_buffer[j] == '\0')
                                cmd_buffer[j] = '\n';
                        }
                        else
                            cmd_buffer[j] = '\0';
                    }      

                    printf("%s", cmd_buffer);
                }
            }
        }

    }

    close(sockfd); 
} 

