/******************************************************************************
 *  filename: history.h
 *
 *  Library for history managment for the shell
 *
 *  written by: James Ross
 *****************************************************************************/

#ifndef HISTORY_H
#define HISTORY_H

#include "utility_sys.h"

#define HIST_SIZE 5   // size of history buffer
#define SET_BACKGROUND 1 // sets the cmd to run in background
#define NO_BACKGROUND  0 // sets the cmd to not run in background (default)

typedef struct comLineInput{
    char *cmdInput;
    bool runBackground;
}clInput_s;

typedef struct cmdHist{
    clInput_s **history;
    char *lastBang;
    int32_t bangHistNum;
    int32_t recent; 
    int32_t oldest;
}cmdHist_s;

// returns an initialized cmdHist_s*.
cmdHist_s* init_cmdHist_s(size_t histSize);

// frees all allocated memory in a cmdHist_s struct
void free_cmdHist_s(cmdHist_s **toFree);

// adds what is in the inbuffer to the history according to FIFO
void addHistory(char* inBuff, cmdHist_s *cmdHist);

// sets inBuff to the previous command given in current input buffer history
// value and settings
// TODO: Make it so instead of replacing inBuff, it sets it as the history
//       string + anything else they added. | args w/e
int32_t callHistory(char *inBuff, cmdHist_s *cmdHist);

// print the contents of the histroy
void printHistory(cmdHist_s *cmdHist);

/********* EOF **********/
#endif
