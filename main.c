#include<stdio.h>
#include<string.h>
#include "commands_module.h"
#include "license_module.h"

int main()
{
	Command c;
	memcpy(c.command, "ls", 3);
	//run_command(c);	

	
	if (!check_license())
		prompt_for_serial();
}
