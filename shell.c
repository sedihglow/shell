/******************************************************************************
 *  filename: shell.c
 *
 *  Implements the shell calls and its helper functions
 *
 *  written by: James Ross
 *****************************************************************************/

#include "shell.h"

#define HIST_STRLEN 7

char *_currentCmd = NULL;

/********************** HEADER FUNCTIONS *************************************/
void exec_shell(char *envp[])
{
#define PROMPT_LEN 10
    cmdHist_s *cmdHist = NULL;
    cmdInfo_s *toExecute = NULL;     // temp fill of a command
    char *cdPath = NULL;             // path to take after a cd command.
    int32_t len = 0;             // length of cdPath
    pid_t pid = 0;
    ssize_t retIO = 0;               // return values from IO
    int32_t shellCmd = 0;            // gets set with EXIT and PRINT_HIST
    int32_t bfPl = 0;
    int32_t histNum = 0;             // history value we are using
    char clBuff[BUFF_SIZE];          // command line buffer

    // Initialize history. all history feilds are NULL untill allocated
    cmdHist = init_cmdHist_s((size_t)HIST_SIZE);

    // loop until exit command is sent
    while(true){ 
        shellCmd = 0; // reset the flag to exit or print history and repeat

        // display prompt for user
        retIO = write(STDOUT_FILENO, "[shell] $ ", PROMPT_LEN);
        if(FAILURE == retIO) 
            errExit("exec_shell: write() for prompt failure");

        // wait for user input
        memset(clBuff, '\0', BUFF_SIZE);
        READ_INPUT(STDIN_FILENO, clBuff, BUFF_SIZE, retIO);

        if(clBuff[0] == '\n') continue; // no input, go back and wait

        if(clBuff[0] != '!'){ // fill next history slot
            if(strncmp("history", clBuff, HIST_STRLEN) != 0)
                addHistory(clBuff, cmdHist);
        }
        else{// takes the !x input and sets clBuff to the previous cmd
            if(callHistory(clBuff, cmdHist) == FAILURE){
                continue; // next loop for new input
            }
        }

        toExecute = getCMD(clBuff, &bfPl, &shellCmd, NEW_BUFF);
        switch(shellCmd){
            case EXIT:
                free_cmdHist_s(&cmdHist);
                return; // exit the shell program
            case PRINT_HIST:
                printHistory(cmdHist);
                continue;
            case CD:
                // toExecute -> cmdName == cd, next word is path, bfPl on path
                cdPath = parseInput(clBuff, &bfPl, OLD_BUFF);
                NL_CHECK(cdPath, len, retIO); // using retIO for '\n' eoi flag
                if(chdir(cdPath) == FAILURE)
                    errMsg("Invalid path name.");
                continue;
            case BANG_BANG:
                len = strlen(clBuff);        
                memset(clBuff, '\0', len);
                if(cmdHist != NULL || cmdHist -> lastBang != NULL){
                    strcpy(clBuff, cmdHist -> lastBang);
                    callHistory(clBuff, cmdHist);
                    shellCmd = 0;
                    toExecute = getCMD(clBuff, &bfPl, &shellCmd, NEW_BUFF);
                    // TODO: make each case its own macro for call clarity
                    switch(shellCmd){// check for !! being shell cmd
                        case CD:
                            // toExecute -> cmdName == cd, next word is path, bfPl on path
                            cdPath = parseInput(clBuff, &bfPl, OLD_BUFF);
                            NL_CHECK(cdPath, len, retIO); // using retIO for '\n' eoi flag
                            if(chdir(cdPath) == FAILURE)
                                errMsg("Invalid path name.");
                            continue;
                        case PWD:
                            cdPath = (char*)malloc(sizeof(char)*BUFF_SIZE);
                            if(cdPath == NULL){
                                errMsg("cdPath malloc failure");
                                continue;
                            }
                            memset(cdPath, '\0', BUFF_SIZE);
                            if(getcwd(cdPath, BUFF_SIZE-1) != NULL){
                                len = strlen(cdPath);
                                cdPath[len] = '\n';
                                if(write(STDOUT_FILENO, cdPath, ++len) == FAILURE){
                                    errMsg("Failure to write current working directory");
                                    free(cdPath);
                                    continue;
                                }
                                free(cdPath);
                                continue;
                            }
                            else{
                                errMsg("Failed to get current working directory path");
                                    free(cdPath);
                                    continue;
                            }
                    }// end internal switch
                    break; // break out of extrnal switch
                }
                else{ // is NULL
                    errMsg("No history for previous ! command");
                    continue;
                }
            case PWD:
                cdPath = (char*)malloc(sizeof(char)*BUFF_SIZE);
                if(cdPath == NULL){
                    errMsg("cdPath malloc failure");
                    continue;
                }
                memset(cdPath, '\0', BUFF_SIZE);
                if(getcwd(cdPath, BUFF_SIZE-1) != NULL){
                    len = strlen(cdPath);
                    cdPath[len] = '\n';
                    if(write(STDOUT_FILENO, cdPath, ++len) == FAILURE){
                        errMsg("Failure to write current working directory");
                        free(cdPath);
                        continue;
                    }
                    free(cdPath);
                    continue;
                }
                else{
                    errMsg("Failed to get current working directory path");
                        free(cdPath);
                        continue;
                    }
            }// end switch

        // set runBackground if the command line input ended in &
        histNum = cmdHist -> bangHistNum;
        toExecute -> runBackground = cmdHist -> history[histNum] -> runBackground;
        _currentCmd = cmdHist -> history[histNum] -> cmdInput;
        
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
