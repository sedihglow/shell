/******************************************************************************
 *  filename: handleExec.c
 *
 *  Library call to handle the exec operation
 *
 *  written by: James Ross
 *****************************************************************************/

#include "handleExec.h"

extern char *_currentCmd;
static void sigChld_handler(int sig){
#define SIGCHLD_PROMPT "DONE: "
        write(STDOUT_FILENO, SIGCHLD_PROMPT, 5);
        write(STDOUT_FILENO, _currentCmd, strlen(_currentCmd));
#undef SIGCHILD_PROMPT
}

void handleExec(cmdInfo_s *toExec)
{
    char **noArgs = (char*[]){toExec -> cmdName, NULL};
    int32_t fd;
    pid_t chPid;
    struct sigaction sigact;

    if(toExec -> runBackground == NO_BACKGROUND){
        if(toExec -> input != NULL){
           if((fd = open(toExec -> input, O_WRONLY)) == FAILURE)
               err_exit("failed to open input src");
        
            if(dup2(fd, STDIN_FILENO) == FAILURE) errMsg("dup2 failure");
            if(close(fd) == FAILURE) err_exit("output fd close failure");
        }

        if(toExec -> output != NULL){
           if((fd = open(toExec -> output, O_WRONLY | O_CREAT, S_IRWXU)) == FAILURE)
               err_exit("failed to open input src");

            if(dup2(fd, STDOUT_FILENO) == FAILURE) errMsg("dup2 failure");
            if(close(fd) == FAILURE) err_exit("output fd close failure");
        }

        if(toExec -> args != NULL){
            execvp(toExec -> cmdName, toExec -> args);
            err_exit("command not found");
        }
        else{
            execvp(toExec -> cmdName, noArgs);
            err_exit("command not found");
        }
    }
    else{ // run in background
        /* set and hook signal handler */
        sigact.sa_flags = 0;
        sigemptyset(&sigact.sa_mask);        // exclude all signals from the set

        sigact.sa_handler = sigChld_handler; // set signal handler address
        if(sigaction(SIGCHLD, &sigact, NULL) == FAILURE) // Set SIGCHLD
            errExit("countDown: sigaction() failure");

        switch(chPid = fork()){
            case -1: err_exit("handleExec: child failed to fork for &"); break;
            case  0: //grandchild
                // set to no background so grandchild runs
                toExec -> runBackground = NO_BACKGROUND;
                handleExec(toExec);
                break;
            default:
                waitpid(chPid, NULL, 0); // wait for grandshild to end
                // once grandchild completes, send parent a signal
                break;
        }    
            
    }
        _exit(EXIT_SUCCESS);
}// end handleExec


