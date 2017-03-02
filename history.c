/******************************************************************************
 *  filename: history.h
 *
 *  Implements the functions for history.h 
 *
 *  written by: James Ross
 *****************************************************************************/

#include "history.h"

// returns an initialized cmdHist_s*.
cmdHist_s* init_cmdHist_s(size_t histSize)
{
    int32_t i;
    cmdHist_s *toInit = (cmdHist_s*)malloc(sizeof(cmdHist_s));

    toInit -> history = (clInput_s**)malloc(sizeof(clInput_s*)*histSize);    
    
    for(i = 0; i < HIST_SIZE; ++i)
        toInit -> history[i] = NULL;

    toInit -> recent = 0;
    toInit -> oldest = 0;
    return toInit;
}// end init_cmdHist_s

// frees all allocated memory in a cmdHist_s struct
void free_cmdHist_s(cmdHist_s **toFree)
{
    int32_t i;

    if(*toFree == NULL) return;

    for(i = 0; i < (*toFree) -> oldest; ++i){
        free((*toFree) -> history[i] -> cmdInput);
        (*toFree) -> history[i] -> cmdInput = NULL;
        free((*toFree) -> history[i]);
        (*toFree) -> history[i] = NULL;
    }
    
    if((*toFree) -> history != NULL){
        free((*toFree) -> history);
        (*toFree) -> history = NULL;
    }
    free(*toFree);
    *toFree = NULL;
}// end free_cmdHist_s

// sets inBuff to the previous command given in current input buffer history
// value and settings

// TODO: Make it so instead of replacing inBuff, it sets it as the history
//       string + anything else they added. | args w/e
int32_t callHistory(char *inBuff, cmdHist_s *cmdHist)
{
    int32_t histNum = 0;
    size_t len = 0;
    bool setBackground = NO_BACKGROUND;

    assert(cmdHist != NULL);

    len = strlen(inBuff);
    if(len > 1 && inBuff[len-2] == '&'){
        inBuff[len-2] = '\0';
        setBackground = SET_BACKGROUND;
    }
    
    if(inBuff[len-1] == '\n') inBuff[len-1] = '\0';

    histNum = conv32_t(inBuff+1, GN_BASE_10 | GN_GT_O | GN_NOEXIT_, "inBuff");
    // TODO: set errno in conversion function so i can error check the
    //       result and act accordingly.
    if(histNum == 0){
        noerr_msg("Invalid history input");
        return FAILURE;
    }
    
    //Error check the number input
    if(histNum > HIST_SIZE){
        noerr_msg("Invalid history input, value to high. history holds 5");
        return FAILURE;
    }

    // set histNum to reflect actual location of command
    --histNum;
   
    // check if history entry exists
    if(cmdHist -> history[histNum] == NULL){
        noerr_msg("Invalid history input, empty history location.");
        return FAILURE;
    }

    // set inBuff as the previous command
    strcpy(inBuff, cmdHist -> history[histNum] -> cmdInput);

    // set background value in case it changed from history
    if(setBackground == SET_BACKGROUND)
        cmdHist -> history[histNum] -> runBackground = setBackground;

    return SUCCESS;
}//end call history

// adds what is in the inbuffer to the history according to FIFO
void addHistory(char* inBuff, cmdHist_s *cmdHist)
{
    size_t len;
    int32_t i;
    int32_t oldest;

    len = strlen(inBuff) + 1;
    oldest = cmdHist -> oldest;
   
    if(oldest == HIST_SIZE){
        // free oldest command stored
        FREE_ALL(cmdHist -> history[oldest-1] -> cmdInput, 
                 cmdHist -> history[oldest-1]);
        cmdHist -> history[oldest-1] = NULL;
    }
    else{ // cmdHist -> oldest < HIST_SIZE
        ++(cmdHist -> oldest);
        ++oldest;
    }

    // shift history position
    for(i = oldest-1; i > 0; --i)
        cmdHist -> history[i] = cmdHist -> history[i-1];

    // add new addition to history
    cmdHist -> history[i] = (clInput_s*)malloc(sizeof(clInput_s));
    if(cmdHist -> history[i] == NULL) 
        errExit("add history, malloc failure.");

    cmdHist -> history[i] -> cmdInput = (char*)malloc(sizeof(char)*len);
    strncpy(cmdHist -> history[i] -> cmdInput, inBuff, len);
    
    // set len back to strlen(inBuff);
    --len;
    // check if it needs to be ran in the background
    if(len > 1 && inBuff[len-2] == '&'){
        inBuff[len-2] = '\n'; // remove &
        inBuff[len-1] = '\0';
        cmdHist -> history[i] -> runBackground = SET_BACKGROUND;
    }
    else cmdHist -> history[i] -> runBackground = NO_BACKGROUND;
}//end add history

void printHistory(cmdHist_s *cmdHist)
{
    int32_t i;

    if(cmdHist == NULL)
        return;

    for(i = 0; i < cmdHist -> oldest; ++i){
        write(STDOUT_FILENO, cmdHist -> history[i] -> cmdInput, 
              strlen(cmdHist -> history[i] -> cmdInput));
    }

}//end printHistory
/************** EOF **************/
