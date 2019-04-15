#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "license_module.h"

#define LICENSE_LENGTH 20


const char *LOW_BUDGET_LICENSING_SERVER_URL = "https://www.random.org/strings/?num=1&len=20&digits=on&upperalpha=on&loweralpha=on&unique=on&format=plain&rnd=new";

int validate_serial_key(char *serial_key)
{
	char cmd_buffer[300];
	sprintf(cmd_buffer, "wget -qO- \"%s\"", LOW_BUDGET_LICENSING_SERVER_URL);	
	
	FILE *f = popen(cmd_buffer, "r");
	if (f == NULL)
		return 0;

	char server_generated_serial_key[21]; 
	fgets(server_generated_serial_key, LICENSE_LENGTH, f);

	printf("Debug: Key from server: %s\n", server_generated_serial_key);

	if (strcmp(serial_key, server_generated_serial_key) != 0)
	{
		generate_license(serial_key);
		return 1;
	}
	else
		return 0;
}

int hash_hash_baby(int h, int j)
{
	h = h ^ (h >> 20) ^ (h >> 12);
	return j ^ h ^ (h >> 7) ^ (h >> 4);
}

int generate_license(char *serial_key)
{
	char client_data[HOST_NAME_MAX+LOGIN_NAME_MAX];

	gethostname(client_data, HOST_NAME_MAX);
	getlogin_r(client_data + strlen(client_data), LOGIN_NAME_MAX);

	
	printf("%s", client_data);


	int client_data_len = strlen(client_data);


	for (int i = 0; i < client_data_len; i++)
	{
		client_data[i] ^= (client_data + 541 - client_data_len)[i];
		client_data[i] = hash_hash_baby(client_data[i], serial_key[i % LICENSE_LENGTH])  ^ serial_key[i % LICENSE_LENGTH];
	}

	printf("Debug: License hash: [%s]", client_data);

	FILE *f = fopen("license.dat", "wb");
	fwrite(client_data, sizeof(char), client_data_len, f);
        fclose(f);	


	check_license();

	return 0;

}


int check_license()
{
	FILE *f = fopen("license.dat", "rb");
	
	if (f == NULL)
		return 0;

	char buffer[LICENSE_LENGTH];
	int buffer_len = fread(buffer, sizeof(char), LICENSE_LENGTH, f); 

	
	char client_data[HOST_NAME_MAX+LOGIN_NAME_MAX];

        gethostname(client_data, HOST_NAME_MAX);
        getlogin_r(client_data + strlen(client_data), LOGIN_NAME_MAX);

	int client_data_len = strlen(client_data);

        for (int i = 0; i < strlen(client_data); i++)
        {
                client_data[i] ^= (client_data + 941 - client_data_len)[i];
                client_data[i] = hash_hash_baby(client_data[i], 0xB00B1E2) ^ 0xB00B1E2;
        }

        printf("Debug: Check License hash: [%s]\n", client_data);

	if (memcmp(buffer, client_data, buffer_len) == 0)
	       printf("license is valid\n");	

}
