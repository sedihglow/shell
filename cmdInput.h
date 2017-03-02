/******************************************************************************
 *  filename: cmdInput.h
 *
 *  Library call to handle command inputs for shell
 *
 *  written by: James Ross
 *****************************************************************************/

#ifndef CMDINPUT_H
#define CMDINPUT_H

#include "utility_sys.h"

#define BUFF_SIZE 256 // size of input buffer

#define NEW_BUFF 1    // identifies if the buffer passed into parseInput is new
#define OLD_BUFF 0    // identifies if the buffer is the same as previous.
#define FIRST_BUFF 0 // shows its the first buffer sent to parseInput

#define EXIT 0x1
#define PRINT_HIST 0x2
#define NL_FOUND 1

#define SET_BACKGROUND 1 // sets the cmd to run in background
#define NO_BACKGROUND  0 // sets the cmd to not run in background (default)

#define NL_CHECK(nextWord, wordLen, endOfInput){                               \
        wordLen = strlen(nextWord);                                            \
        if((nextWord)[wordLen-1] == '\n'){                                       \
            endOfInput = NL_FOUND;                                             \
            (nextWord)[wordLen-1] = '\0';                                        \
            --wordLen;                                                         \
        }                                                                      \
}

typedef struct cmdInfo{
    char *cmdName;
    char **args; // all characters in the command called with cmdName
    char *input;
    char *output;
    bool pipeOut;
    bool runBackground;
}cmdInfo_s;

// create a cmdInfo_s *struct
cmdInfo_s* init_cmdInfo_s(void);

// free a cmdInfo_s struct
void free_cmdInfo_s(cmdInfo_s **toFree);

// parse the next word from the input buffer, bfPl keeps track of location
// inside buff so calling function does not have to.
char* parseInput(char *inBuff, int32_t *bfPl, int32_t mode);

// aquire the argument list for a command
char** aquireArgs(char *progName, char **nextWord, char *inBuff, int32_t *bfPl, bool *endOfInput);

// fills a cmdInfo_s struct, adjusts cmdHist for background proccess as well.
// TODO: find somewhere else that is less intrusive for cmdHist alteration, its
//       just one line
cmdInfo_s* getCMD(char *clBuff, int32_t *bfPl, int32_t *exitFlag);

/*********** EOF **********/
#endif
