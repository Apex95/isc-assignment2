#include <stdio.h>
#include <string.h>

#define LICENSE_LENGTH 20


const char* LOW_BUDGET_LICENSING_SERVER_URL = "https://www.random.org/strings/?num=1&len=20&digits=on&upperalpha=on&loweralpha=on&unique=on&format=plain&rnd=new";

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

	// impossible to guess a 20 bytes random key unless...
	// 	- creates a fake random.org server and edits resolv.conf
	// 	- directly generates a valid license file	
	if (strcmp(serial_key, server_generated_serial_key) != 0)
		return 0;
	else
		return 1;
}


int check_license()
{
	FILE *f = fopen("license", "rb");
	
	if (f == NULL)
		return 0;

	return 1;

	//char license_buffer[50];

	// TODO: buffer overflow in fisierul de licenta; fread de mai multe caractere, eventual decupat hash-ul pana la primul \x00
	//fread(license_buffer, 1, 20);
}
