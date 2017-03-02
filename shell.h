/******************************************************************************
 *  filename: shell.h
 *
 *  Library call for the shell
 *
 *  written by: James Ross
 *****************************************************************************/

#ifndef SHELL_H
#define SHELL_H

#include "utility_sys.h"
#include "history.h"
#include "cmdInput.h"
#include "pipeShell.h"

void exec_shell(char *envp[]);

/********** EOF ***********/
#endif
