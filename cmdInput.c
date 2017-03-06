/******************************************************************************
 *  filename: cmdInput.c
 *
 *  implements the functionality for cmdInput.h
 *
 *  written by: James Ross
 *****************************************************************************/

#include "cmdInput.h"

#define MATCH 0


cmdInfo_s* init_cmdInfo_s(void)
{
    cmdInfo_s *newInfo = (cmdInfo_s*)malloc(sizeof(cmdInfo_s));
    if(newInfo == NULL) errExit("malloc failure, toExecute");

    newInfo -> cmdName = NULL;
    newInfo -> args    = NULL;
    newInfo -> input   = NULL;
    newInfo -> output  = NULL;
    newInfo -> pipeOut = false;
    newInfo -> runBackground = 0;
    return newInfo;
}

void free_cmdInfo_s(cmdInfo_s **toFree)
{
    int32_t i;

    if(*toFree == NULL) return;

    if((*toFree) -> cmdName != NULL){
        free((*toFree) -> cmdName);
        (*toFree) -> cmdName = NULL;
    }
    if((*toFree) -> input   != NULL){
        free((*toFree) -> input);
        (*toFree) -> input = NULL;
    }
    if((*toFree) -> output  != NULL){
        free((*toFree) -> output);
        (*toFree) -> output = NULL;
    }
    if((*toFree) -> args    != NULL){ 
        for(i=0; (*toFree) -> args[i] != NULL; ++i){
            free((*toFree) -> args[i]);
            (*toFree) -> args[i] = NULL;
        }
    }

    free((*toFree) -> args);
    (*toFree) -> args = NULL;

    free(*toFree);
    *toFree = NULL;
}// end free_cmdInfo_s

/* Parses through the buffer, keeps track on where it is in the buffer. any new
 * buffer requires mode to be set to NEW_BUFF so the buffer placement is not 
 * in the old buffer. The first buffer used does not need mode set.
 *
 * returns char* containing the next word in the buffer
 *         NULL on error, could not allocate space for the string 
 */
char* parseInput(char *inBuff, int32_t *bfPl, int32_t mode)
{
    char *retStr = NULL;         // final string to be returned
    int32_t retIO = 0;
    size_t len = 0;             // length of the resulting string
    char parseStr[BUFF_SIZE] = {'\0'}; // string result from parse

    if(NEW_BUFF == mode) *bfPl = 0;

    // fills parseStr with the characters up to the next ' ' or '\n', has '\0'
    CL_READ_PARSE(STDIN_FILENO, inBuff, *bfPl, BUFF_SIZE-2, retIO, parseStr,
               BUFF_SIZE, 
               inBuff[*bfPl] != ' ' && inBuff[*bfPl] != '\n' &&
               inBuff[*bfPl] != '<' && inBuff[*bfPl] != '>' &&
               inBuff[*bfPl] != '|' && inBuff[*bfPl] != '\t');
    
    // build final string
    len = strlen(parseStr);

    if(inBuff[*bfPl] == '\n') parseStr[len] = '\n'; // keep the \n for ref
    else if(inBuff[*bfPl] == ' ' || inBuff[*bfPl] == '\t'){
        do{
            ++(*bfPl); // skip over ' ' and '\t' for next parse
        }while(inBuff[*bfPl] == ' ' || inBuff[*bfPl] == '\t');
    }
    else if(parseStr[0] == '\0'){
        if(inBuff[*bfPl] != ' ' && inBuff[*bfPl] != '\t')
            parseStr[0] = inBuff[*bfPl];
        do{
            ++(*bfPl); // skip over ' ' and '\t' for next parse
        }while(inBuff[*bfPl] == ' ' || inBuff[*bfPl] == '\t');
    }

    len += 2; // place room for '\n' and '\0'

    retStr = malloc(sizeof(char)*len);
    if(retStr == NULL){
        errMsg("parseInput, malloc failure retStr");
        return NULL;
    }
   
    strncpy(retStr, parseStr, len);

    return retStr;
}// end parseInput


// parse through inBuff untill all arguments are aquired and returned to the
// the caller
char** aquireArgs(char *progName, char **nextWord, char *inBuff, int32_t *bfPl, bool *endOfInput){
    char **finalArgs = NULL;
    char **retArgs = NULL;
    int32_t argCount = 0;
    int32_t i = 0;
    size_t len = 0; 
   
    finalArgs = (char**)malloc(sizeof(char*)*BUFF_SIZE);
    if(finalArgs == NULL) errExit("aquireArgs, malloc failure finalArgs");
  
    // build argv for program to be called
    len = strlen(progName) + 1;
    finalArgs[i] = (char*)malloc(sizeof(char)*len);
    if(finalArgs == NULL) errExit("aquireArgs, malloc failure finalArgs[0]");

    strncpy(finalArgs[i], progName, len);
    ++i;

    // for each argument for the command
    do{
        NL_CHECK(*nextWord, len, *endOfInput);
        
        ++len; // room for null
        finalArgs[i] = (char*)malloc(sizeof(char*)*len);
        if(finalArgs[i] == NULL) errExit("aquireArgs, finalArgs malloc fail");

        strncpy(finalArgs[i], *nextWord, len);
    
        if(*nextWord != NULL){
            free(*nextWord); // clean up for next parse
            *nextWord = NULL;
        }
        *nextWord = parseInput(inBuff, bfPl, OLD_BUFF);
        if(NULL == *nextWord) errExit("aquireArgs, nextWord malloc failure");

       ++i; // go to next index
    }while(*endOfInput != NL_FOUND && **nextWord != '<' && **nextWord != '>'
            && **nextWord != '|');
    
    argCount = ++i; // leaves room for null pointer

    retArgs = (char**)malloc(sizeof(char*)*argCount);
    if(retArgs == NULL) errExit("AquireArgs, retArgs malloc failure");
    for(i = 0; i < argCount-1; ++i)
        retArgs[i] = finalArgs[i];

    retArgs[i] = NULL; // NULL stopper for args
    
    free(finalArgs); // clean up final args
    finalArgs = NULL;

    return retArgs;
}

// does not account for things ilke !n | !n etc.
cmdInfo_s* getCMD(char *clBuff, int32_t *bfPl, int32_t *shellCmd, int32_t buffState)
{
    char *nextWord = NULL;
    bool endOfInput = false;
    size_t wordLen = 0;
    cmdInfo_s *toExecute = NULL;

    // parse the word in the command
    nextWord = parseInput(clBuff, bfPl, buffState);
    if(NULL == nextWord) errExit("parseInput, nextWord malloc failure");
    
    NL_CHECK(nextWord, wordLen, endOfInput);

    // check for shell command
    if(shellCmd != NULL){
        if(strcmp(nextWord, "exit") == MATCH){
            *shellCmd = EXIT;
            free(nextWord);
            return NULL;
        }
        else if(strcmp(nextWord, "history") == MATCH){
            *shellCmd = PRINT_HIST;
            free(nextWord);
            return NULL;
        }
        else if(strcmp(nextWord, "cd") == MATCH){
            *shellCmd = CD;
            free(nextWord);
            return NULL;
        }
        else if(strcmp(nextWord, "!!") == MATCH){
            *shellCmd = BANG_BANG;
            free(nextWord);
            return NULL;
        }
        else if(strcmp(nextWord, "pwd") == MATCH){
            *shellCmd = PWD;
            free(nextWord);
            return NULL;
        }
    }
    
    // any trailing & and '\n' are removed. Set up cmd and place in history
    toExecute = init_cmdInfo_s();
    
    wordLen = strlen(nextWord) + 1;
    toExecute -> cmdName = (char*)malloc(sizeof(char)*wordLen);
    if(toExecute -> cmdName == NULL) errExit("getCMD: malloc failure, cmdName");

    strncpy(toExecute -> cmdName, nextWord, wordLen);

    while(endOfInput != NL_FOUND){
        // check following character
        if(nextWord != NULL){
            free(nextWord); // clean up for next value
            nextWord = NULL;
        }
        nextWord = parseInput(clBuff, bfPl, OLD_BUFF);

        // next values must be arguments
        if(*nextWord != '\n' && *nextWord != '>' && *nextWord != '<' && *nextWord != '|'){ 
            toExecute -> args = aquireArgs(toExecute -> cmdName, &nextWord, 
                                           clBuff, bfPl, &endOfInput);
        }
        
        switch(*nextWord){
            case '>': // change output to file given
                if(nextWord != NULL){
                    free(nextWord);
                    nextWord = NULL;
                }
                nextWord = parseInput(clBuff, bfPl, OLD_BUFF);
                NL_CHECK(nextWord, wordLen, endOfInput);
                ++wordLen;
                toExecute -> output = (char*)malloc(sizeof(char)*wordLen);
                if(toExecute -> output == NULL) 
                    errExit("exec_shell, toExecute -> output malloc failure");
                
                strncpy(toExecute -> output, nextWord, wordLen);
                break;
            case '<':  // change input to file given
                if(nextWord != NULL){
                    free(nextWord);
                    nextWord = NULL;
                }
                nextWord = parseInput(clBuff, bfPl, OLD_BUFF);
                NL_CHECK(nextWord, wordLen, endOfInput);
                ++wordLen;
                toExecute -> input = (char*)malloc(sizeof(char)*wordLen);
                if(toExecute -> input == NULL) 
                    errExit("exec_shell, toExecute -> output malloc failure");
                
                strncpy(toExecute -> input, nextWord, wordLen);
                break;
            case '|':  // set input of cmd2 from output cmd1. cmd1|cmd2
                /* set end of input, since the input for this particular
                 * command is found 
                 */
                endOfInput = true;
                toExecute -> pipeOut = true;
                break;
        }

    } // end while(eoi != NL_FOUND)
    
    if(nextWord != NULL){
        free(nextWord);
        nextWord = NULL;
    }

    return toExecute;
}//end getCMD 

