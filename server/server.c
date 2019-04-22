#include <stdio.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 80 
#define PORT 1234
#define MAX_CLIENTS 15


unsigned char get_random_nonce()
{
    return rand() % 10;
}



int main() 
{ 
    int rc, listen_sd, client_sd, max_sd, desc_ready, close_conn; 
    struct sockaddr_in servaddr;
    char buffer[BUFFER_SIZE];
    unsigned char nonces[MAX_CLIENTS + 4];

    char my_key[] = "my_secret_key";

    srand(time(NULL));

    fd_set master_set, working_set; 
 
    printf("%d\n", get_random_nonce());

    // socket create and verification 
    listen_sd = socket(AF_INET, SOCK_STREAM, 0); 
    
    if (listen_sd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    
    memset(&servaddr, 0, sizeof(servaddr));
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(listen_sd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(listen_sd, MAX_CLIENTS)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
 

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

	    for (int i = 0; i <= max_sd && desc_ready > 0; ++i)
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
                    

                    nonces[client_sd] = get_random_nonce();

                    // sending nonce
                    rc = send(client_sd, &(nonces[client_sd]), sizeof(unsigned char), 0);
                    printf("sent nonce: %d\n", nonces[client_sd]); 
                }
                else
                {
                    close_conn = 0;
                    
                    rc = recv(i, buffer, sizeof(buffer), 0);
                  
                    if (rc == 0)
                        close_conn = 1;

                    if (rc > 0)
                    {
                        printf("received: [%s]\n", buffer);
                        for (int j = 0; j < strlen(my_key); j++)
                            buffer[j] ^= nonces[i];
                        printf("decrypted: [%s]\n", buffer);
                    }

                    if (close_conn)
                    {
                        close(i);
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

