/******************************************************************************
 * filename: main.c
 *
 * Shell program.
 *
 * written by: James Ross
 *****************************************************************************/
#include "shell.h"

#define CMD_ONLY 1

int main(int argc, char *argv[], char *envp[])
{
    if(argc != CMD_ONLY) errMsg("$cmd takes no commands, additional input is"
                                "ignored. $cmd");

    exec_shell(); // run the shell

    exit(EXIT_SUCCESS);
}// end main
/********** EOF ***********/
