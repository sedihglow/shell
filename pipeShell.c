/******************************************************************************
 *  filename: pipeShell.h
 *
 *  Implements the pipe functionality for the shell
 *
 *  written by: James Ross
 *****************************************************************************/

#include "pipeShell.h"

static void executePipe(cmdInfo_s *cmd1, cmdInfo_s *cmd2, char *inBuff){
    int32_t pipeFD[2] = {0};
    pid_t chPid = 0;

    if(pipe(pipeFD) == FAILURE) err_exit("handlePipe: pipe() failure");

    switch(chPid = fork()){
        case -1: err_exit("executePipe: fork() failure, chPid"); break;
        case  0: // child
            if(close(pipeFD[P_RD]) == FAILURE) err_exit("close(): child P_WR");
            // set write pipe FD to stdin for cmd2
            if(dup2(pipeFD[P_WR], STDOUT_FILENO) == FAILURE) errMsg("dup2 failure");
            if(close(pipeFD[P_WR]) == FAILURE) err_exit("output fd close failure");
            handleExec(cmd1);
            exit(EXIT_FAILURE); // should not reach this point due to exec
            break;
        default: //parent
            if(close(pipeFD[P_WR]) == FAILURE) err_exit("close(): child P_RD");
            if(dup2(pipeFD[P_RD], STDIN_FILENO) == FAILURE) errMsg("dup2 failure");
            if(close(pipeFD[P_RD]) == FAILURE) err_exit("output fd close failure");
            if(cmd2 -> pipeOut == false){
                waitpid(chPid, NULL, 0);
                handleExec(cmd2);
            }
    }

}

void handlePipe(cmdInfo_s *toExecute, char *inBuff, int32_t *bfPl)
{
    cmdInfo_s *nextCmd = NULL;
    pid_t chPid = 0;
    int32_t endOfInput = 0;

    if(toExecute -> pipeOut == false)
        noerr_exit("handlePipe: pipeOut flag not set, no pipe command found");
  
    do{
        // get next command that toExecute will get piped into
        nextCmd = getCMD(inBuff, bfPl, (int32_t*)NULL);
        switch(chPid = fork()){
            case -1: err_exit("handlePipe: fork() failure"); break;
            case  0: //child
                executePipe(toExecute, nextCmd, inBuff);
                break;
            default: //parent
                waitpid(chPid, NULL, 0);
                free(nextCmd);
        }
    }while(nextCmd -> pipeOut == true);

    _exit(EXIT_SUCCESS);

}// end handlePipe
