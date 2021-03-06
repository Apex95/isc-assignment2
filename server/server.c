#include <stdio.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "license.h"

#define BUFFER_SIZE 80 
#define PORT 1234
#define MAX_CLIENTS 15
#define CMD_BUFFER_SIZE 1000

#define DEMO_CMD_NAME_MAX_LEN 4
char demo_commands[][DEMO_CMD_NAME_MAX_LEN] = {"ls", "pwd", "cat"};
int is_demo_mode = 0;


unsigned char get_random_nonce()
{
    return rand() % 10;
}

int is_demo_command(char *cmd)
{
    int len = sizeof(demo_commands) / DEMO_CMD_NAME_MAX_LEN;

    for (int i = 0; i < len; i++)
        if (strstr(cmd, demo_commands[i]) == cmd)
            return 1;

    return 0;
}



int main() 
{ 
    int rc, listen_sd, client_sd, max_sd, desc_ready, close_conn; 
    struct sockaddr_in servaddr;
    char buffer[BUFFER_SIZE];
    char actual_cmd[sizeof(buffer) + 5];
    unsigned char nonces[MAX_CLIENTS + 4];
    unsigned char protocol_step[MAX_CLIENTS + 4];

    char cmd_buffer[CMD_BUFFER_SIZE];

    char my_key[] = "my_secret_key";

    fd_set master_set, working_set; 
 
    srand(time(NULL));
    memset(protocol_step, 0, sizeof(protocol_step));

    printf("Launching the administration server...\n");

    // socket create and verification 
    listen_sd = socket(AF_INET, SOCK_STREAM, 0); 
    
    if (listen_sd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created...\n"); 
    
    memset(&servaddr, 0, sizeof(servaddr));
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // Binding 
    if ((bind(listen_sd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) 
    { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded...\n"); 
  
    // Listening
    if ((listen(listen_sd, MAX_CLIENTS)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Listening...\n"); 


    // license check
    if (!check_license())
        if (!prompt_for_serial())
            is_demo_mode = 1;
 

    FD_ZERO(&master_set);
    max_sd = listen_sd;
    FD_SET(listen_sd, &master_set); 

    struct timeval timer;
    timer.tv_sec = 3;
    timer.tv_usec = 0;


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

                if (i == listen_sd)
                {
                    client_sd = accept(listen_sd, NULL, NULL);

                    if (client_sd < 0)
                        break;

                    printf("Accepting connection...\n");
                    FD_SET(client_sd, &master_set);

                    if (client_sd > max_sd)
                        max_sd = client_sd;
                    
                    if (protocol_step[client_sd] == 0)
                    {
                        nonces[client_sd] = get_random_nonce();

                        // sending nonce
                        rc = send(client_sd, &(nonces[client_sd]), sizeof(unsigned char), 0);
                        printf("sent nonce: %d\n", nonces[client_sd]);

                    } 
                }
                else
                {
                    close_conn = 0;
                    
                    rc = recv(i, buffer, sizeof(buffer), 0);
                  
                    if (rc == 0)
                        close_conn = 1;

                    if (rc > 0)
                    {
                        if (protocol_step[i] == 0)
                        {
                            printf("received: [%s]\n", buffer);

                            for (int j = 0; j < strlen(my_key); j++)
                                buffer[j] ^= nonces[i];
                            
                            printf("decrypted: [%s]\n", buffer);


                            if (strcmp(buffer, my_key) == 0)
                            {
                                printf("authenticated %d\n", i);
                                protocol_step[i]++;
                            }
                        }
                        else
                            if (protocol_step[i] == 1)
                            {
                                printf("received cmd: [%s]\n", buffer);


                                // todo: check if allowed
                                if (is_demo_mode && !is_demo_command(buffer))
                                {
                                    strcpy(cmd_buffer, "Not a demo command.\n");
                                    rc = send(i, cmd_buffer, strlen(cmd_buffer)+1, 0);
                                }
                                else
                                {
                                    memcpy(actual_cmd, buffer, sizeof(buffer));
                                    memcpy(actual_cmd + strlen(actual_cmd), " 2>&1", 6);

                                    FILE *f = popen(actual_cmd, "r");
                               
                                    memset(cmd_buffer, 0, sizeof(cmd_buffer)); 
                                
                                    while (fgets(cmd_buffer, sizeof(cmd_buffer), f) != NULL)
                                    {
                                        printf("%s", cmd_buffer);

                                        // sending the result to the client
                                        rc = send(i, cmd_buffer, strlen(cmd_buffer)+1, 0); 

                                    }
                                
                                    fclose(f);
                                }


                            }
                    }

                    if (close_conn)
                    {
                        close(i);
                        protocol_step[i] = 0;

                        FD_CLR(i, &master_set);
                        
                        if (i == max_sd)
                        {
                            while (FD_ISSET(max_sd, &master_set) == 0)
                                max_sd -= 1;
                        }
                    }
                } 
            }
	    }

    }
    
} 

