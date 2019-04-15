#ifndef COMMANDS_MODULE_H
#define COMMANDS_MODULE_H
typedef struct Command {
        char command[50];
} Command;

int run_command(Command command);
#endif
