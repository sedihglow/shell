/******************************************************************************
 *  filename: pipeShell.h
 *
 *  Implements the pipe functionality for the shell
 *
 *  written by: James Ross
 *****************************************************************************/

#include "pipeShell.h"

static void executePipe(cmdInfo_s *cmd1, cmdInfo_s *cmd2, char *inBuff, int32_t *bfPl){
    cmdInfo_s *cmd3 = NULL;
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
            else{
                cmd3 = getCMD(inBuff, bfPl, (int32_t*)NULL, OLD_BUFF);
                cmd3 -> runBackground = cmd1 -> runBackground;
                executePipe(cmd2, cmd3, inBuff, bfPl);
                free_cmdInfo_s(&cmd1);
                free_cmdInfo_s(&cmd2); 
            }
    }
}// end executePipe

void handlePipe(cmdInfo_s *toExecute, char *inBuff, int32_t *bfPl)
{
    cmdInfo_s *nextCmd = NULL;
    pid_t chPid = 0;

    if(toExecute -> pipeOut == false)
        noerr_exit("handlePipe: pipeOut flag not set, no pipe command found");
    
        // get next command that toExecute will get piped into
        nextCmd = getCMD(inBuff, bfPl, (int32_t*)NULL, OLD_BUFF);
        nextCmd -> runBackground = toExecute -> runBackground;
        switch(chPid = fork()){
            case -1: err_exit("handlePipe: fork() failure"); break;
            case  0: //child
                executePipe(toExecute, nextCmd, inBuff, bfPl);
                break;
            default: //parent
                waitpid(chPid, NULL, 0);
        }

    free_cmdInfo_s(&nextCmd); 
    _exit(EXIT_SUCCESS);

}// end handlePipe
