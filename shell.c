/******************************************************************************
 *  filename: shell.c
 *
 *  Implements the shell calls and its helper functions
 *
 *  written by: James Ross
 *****************************************************************************/

#include "shell.h"

char *_currentCmd = NULL;

/********************** HEADER FUNCTIONS *************************************/
void exec_shell(char *envp[])
{
#define PROMPT_LEN 10
    cmdHist_s *cmdHist = NULL;
    cmdInfo_s *toExecute = NULL;     // temp fill of a command
    pid_t pid = 0;
    ssize_t retIO = 0;               // return values from IO
    int exitFlag = 0;
    int32_t bfPl = 0;
    char clBuff[BUFF_SIZE];          // command line buffer


    // Initialize history. all history feilds are NULL untill allocated
    cmdHist = init_cmdHist_s((size_t)HIST_SIZE);

    // loop until exit command is sent
    while(true){ 
        bfPl = 0;
        // display prompt for user
        retIO = write(STDOUT_FILENO, "[shell] $ ", PROMPT_LEN);
        if(FAILURE == retIO) 
            errExit("exec_shell: write() for prompt failure");

        // wait for user input
        memset(clBuff, '\0', BUFF_SIZE);
        READ_INPUT(STDIN_FILENO, clBuff, BUFF_SIZE, retIO);

        if(clBuff[0] == '\n') continue; // no input, go back and wait

        if(clBuff[0] != '!'){ // fill next history slot
            addHistory(clBuff, cmdHist);
        }
        else{// takes the !x input and sets clBuff to the previous cmd
            if(callHistory(clBuff, cmdHist) == FAILURE){
                continue; // next loop for new input
            }
        }

        toExecute = getCMD(clBuff, &bfPl, &exitFlag);
        if(exitFlag == EXIT){
            free_cmdHist_s(&cmdHist);
            return; // exit the shell program
        }
        else if(exitFlag == PRINT_HIST){
            printHistory(cmdHist);
            continue;
        }

        // set runBackground if the command line input ended in &
        toExecute -> runBackground = cmdHist -> history[0] -> runBackground;
        
        _currentCmd = cmdHist -> history[0] -> cmdInput;
        switch(pid = fork()){
            case -1: errExit("exec_shell: error forking a child"); break;

            case 0: // child
                if(toExecute -> pipeOut == false)
                    handleExec(toExecute);
                else // we reached a pipe in the program
                    handlePipe(toExecute, clBuff, &bfPl);
                break;
            default: // parent
                if(toExecute -> runBackground == NO_BACKGROUND)
                    waitpid(pid, NULL, 0);
        }
        
        free_cmdInfo_s(&toExecute);
    }//end while
#undef PROMPT_LEN
}//end exec_shell
/********** EOF ***********/
