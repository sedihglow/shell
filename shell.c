/******************************************************************************
 *  filename: shell.c
 *
 *  Implements the shell calls and its helper functions
 *
 *  written by: James Ross
 *****************************************************************************/
#include "shell.h"

#define BUFF_SIZE 256 // size of input buffer
#define HIST_SIZE 5   // size of history buffer

#define NEW_BUFF 1    // identifies if the buffer passed into parseInput is new
#define OLD_BUFF 0    // identifies if the buffer is the same as previous.
#define FIRST_BUFF 0 // shows its the first buffer sent to parseInput

#define EXIT 0x2
#define NL_FOUND 1

#define SET_BACKGROUND 1 // sets the cmd to run in background
#define NO_BACKGROUND  0 // sets the cmd to not run in background (default)

typedef struct comLineInput{
    char *cmdInput;
    bool runBackground;
}clInput_s;

typedef struct cmdHist{
    clInput_s **history;
    int32_t recent; 
    int32_t oldest;
}cmdHist_s;

typedef struct cmdInfo{
    char* cmdName;
    char* args; // all characters in the command called with cmdName
    bool runBackground;
}cmdInfo_s;


/********************** STATIC PROTOTYPES ************************************/

/********************** STATIC FUNCTIONS *************************************/

// returns an initialized cmdHist_s*.
static cmdHist_s* init_cmdHist_s(size_t histSize)
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

static void free_cmdHist_s(cmdHist_s *toFree)
{
    int32_t i;
    for(i = 0; i < 12305 SW Broadway St , 97005 12305 SW Broadway St , 97005
}// end free_cmdHist_s

/* Parses through the buffer, keeps track on where it is in the buffer. any new
 * buffer requires mode to be set to NEW_BUFF so the buffer placement is not 
 * in the old buffer. The first buffer used does not need mode set.
 *
 * returns char* containing the next word in the buffer
 *         NULL on error, could not allocate space for the string 
 */
static char* parseInput(char *inBuff, int32_t mode)
{
    char *retStr = NULL;         // final string to be returned
    int32_t retIO = 0;
    static int32_t bfPl = 0;    // current location in buffer
    size_t len = 0;             // length of the resulting string
    char parseStr[BUFF_SIZE] = {'\0'}; // string result from parse

    if(NEW_BUFF == mode) bfPl = 0;

    // fills parseStr with the characters up to the next ' ' or '\n', has '\0'
    READ_PARSE(STDIN_FILENO, inBuff, bfPl, BUFF_SIZE-2, retIO, parseStr,
               BUFF_SIZE, inBuff[bfPl] != ' ' && inBuff[bfPl] != '\n');
    
  
    // build final string
    len = strlen(parseStr);

    if(inBuff[bfPl] == '\n') parseStr[len] = '\n'; // keep the \n for ref

    len += 2; // place room for '\n' and '\0'

    retStr = malloc(sizeof(char)*len);
    if(retStr == NULL){
        errMsg("parseInput, malloc failure retStr");
        return NULL;
    }
   
    strncpy(retStr, parseStr, len);

    return retStr;
}// end parseInput


static cmdInfo_s* processCMD(char *nextWord, bool *exitFlag, size_t len)
{
    cmdInfo_s *toExecute = NULL;

    // check for exit command
    if(strcmp(nextWord, "exit") == 0){
        *exitFlag = EXIT;
        return NULL;
    }
    
    // any trailing & and '\n' are removed. Set up cmd and place in history
    toExecute = (cmdInfo_s*)malloc(sizeof(cmdInfo_s*));
    if(toExecute == NULL) errExit("malloc failure, toExecute");
    
    len = strlen(nextWord) + 1;
    toExecute -> cmdName = (char*)malloc(sizeof(char)*len);
    if(toExecute -> cmdName == NULL) errExit("malloc failure, cmdName");

    strcpy(toExecute -> cmdName, nextWord);

    return toExecute;
}//end processCMD

// sets inBuff to the previous command given in current input buffer history
// value and settings

// TODO: Make it so instead of replacing inBuff, it sets it as the history
//       string + anything else they added. | args w/e
static int32_t callHistory(char *inBuff, cmdHist_s *cmdHist)
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

static void addHistory(char* inBuff, cmdHist_s *cmdHist){
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
}//end add history

/********************** HEADER FUNCTIONS *************************************/
void exec_shell(void)
{
#define PROMPT_LEN 10
    cmdHist_s *cmdHist = NULL;
    cmdInfo_s *toExecute = NULL;     // temp fill of a command
    char* nextWord = NULL;           // next word off the command line
    ssize_t retIO = 0;               // return values from IO
    size_t  wordLen = 0;
    bool exitFlag = 0;
    bool endOfInput = 0;             // 1 when '\n' is found
//    pid_t pid = 0;
    char clBuff[BUFF_SIZE];          // command line buffer
    
    // Initialize history. all history feilds are NULL untill allocated
    cmdHist = init_cmdHist_s((size_t)HIST_SIZE);

    // loop until exit command is sent
    while(true){ 
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
                //FREE_ALL(nextWord, toExecute -> cmdName, toExecute);
                continue; // next loop for new input
            }
        }

        do{
            // parse the word in the command
            nextWord = parseInput(clBuff, NEW_BUFF);
            if(NULL == nextWord) errExit("parseInput, nextWord mallic failure");
           
            wordLen = strlen(nextWord);
            if(nextWord[wordLen-1] == '\n'){
                endOfInput = NL_FOUND;
                nextWord[wordLen-1] = '\0';
                --wordLen;
            }

            toExecute = processCMD();
            if(exitFlag == EXIT){
                // free memory
                return;
            }
            // set runBackground if the command line input ended in &
            toExecute -> runBackground = cmdHist -> history[0] -> runBackground
            
            if(wordLen != 1){ // next values must be arguments
        //        aquireArgs(toExecute);
            }
            else{ // wordLen > 1, value must be reserved symbol
                switch(*nextWord){
                    case '>': break;
                    case '<': break;
                    case '|': break;
                }
            }
            //FREE_ALL(nextWord, toExecute -> cmdName, toExecute);
        }while(endOfInput != NL_FOUND); 

    /*
            read_command(command, parameters);
            if(fork() != 0)
                wait(&status);
            else
                execvp(command, parameters);  use any version of exec */
            
        }//end while
#undef PROMPT_LEN
}//end exec_shell
/********** EOF ***********/
