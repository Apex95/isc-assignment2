#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "license.h"

#define LICENSE_LENGTH 20


const char *LICENSE_KEY = "XsRn4RElGCXnfaHwytEh";
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



    // good luck
    if (strcmp(serial_key, server_generated_serial_key) == 0)
    {
        printf("Serial key is valid. Generating a license...\n");
        generate_license(serial_key);
        return 1;
    }
    else
    {
        printf("Serial key is invalid. Continuing in limited mode...\n");
        return 0;
    }
}

int generate_license(char *serial_key)
{
    char client_data[HOST_NAME_MAX+LOGIN_NAME_MAX];

    gethostname(client_data, HOST_NAME_MAX);
    getlogin_r(client_data + strlen(client_data), LOGIN_NAME_MAX);


    int client_data_len = strlen(client_data);


    for (int i = 0; i < client_data_len; i++)
        client_data[i] ^= LICENSE_KEY[i];

    printf("Debug: License hash: [%s]", client_data);

    FILE *f = fopen("license.dat", "wb");
    fwrite(client_data, sizeof(char), client_data_len, f);
    fclose(f);

    printf("License generated.");


    return 0;

}


int check_license()
{
    FILE *f = fopen("license.dat", "rb");

    if (f == NULL)
        return 0;

    char buffer[LICENSE_LENGTH];
    int buffer_len = fread(buffer, sizeof(char), LICENSE_LENGTH, f);


    char client_data[HOST_NAME_MAX + LOGIN_NAME_MAX];

    gethostname(client_data, HOST_NAME_MAX);
    getlogin_r(client_data + strlen(client_data), LOGIN_NAME_MAX);

    int client_data_len = strlen(client_data);

    for (int i = 0; i < client_data_len; i++)
        client_data[i] ^= LICENSE_KEY[i];


    if (memcmp(buffer, client_data, buffer_len) == 0)
    {
        printf("License is valid\n");
        return 1;
    }

    printf("Invalid license\n");
    return 0;
}


int prompt_for_serial()
{
    printf("Please enter a valid serial number to activate the product:\n");
    char serial[LICENSE_LENGTH];

    scanf("%s", serial);

    return validate_serial_key(serial);
}

