#include "module_binding.h"
#include "commands_module.h"
#include <stdlib.h>

int run_command(Command command)
{
	return system(command.command);
}

