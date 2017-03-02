/******************************************************************************
 *  filename: pipeShell.h
 *
 *  Implements the pipe functionality for the shell
 *
 *  written by: James Ross
 *****************************************************************************/

#ifndef PIP_SHELL_H
#define PIP_SHELL_H

#include "utility_sys.h"
#include "cmdInput.h"
#include "handleExec.h"

void handlePipe(cmdInfo_s *toExecute, char *inBuff, int32_t *bfPl);
/****** EOF *****/
#endif
