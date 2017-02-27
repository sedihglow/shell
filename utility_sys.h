/******************************************************************************
 * filename: utility_sys.h
 * Written by: James Ross
 *****************************************************************************/

#ifndef _UTL_SYS_H_
#define _UTL_SYS_H_

#define _POSIX_C_SOURCE 199309

#define FAILURE -1
#define SUCCESS 0

#define RW_END 0

#define P_RD 0 // value for a pipe read fd in pipefd[2]
#define P_WR 1 // value for a pip write fd in pipefd[2]

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <inttypes.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#define NDEBUG 1
#include <assert.h>

#include "./convNum/convNum.h"       /* str to int functions */
#include "./err_handle/err_handle.h" /* general error printing */

/* Allocate an input buffer with a '\0' terminatior at the end.
 *  If nothing is read, buff[0] = '\0'.      
 *  - fd    == int   , File descriptor used for allocInputBuff()
 *  - buff  == char* , Buffer to be filled with character data from fd.
 *  - nbyte == size_t, number of bytes to read 
 *  - retBytes == ssize_t, number of bytes read from file
 *                   (typically the size of buffer array) 
 */
#define READ_INPUT(fd, buff, nByte, retBytes)                                  \
{                                                                              \
    assert(buff != NULL);                                                      \
    if((retBytes = read(fd, (void*) buff, nByte)) == FAILURE)                  \
        errExit("READ_INPUT, read() failure");                                 \
} /* end READ_INPUT */

/* Copy a variable ammount of characters from a buffer based on a given 
 * position. Places a null value at end of inBUFF
 * Place resulting string in resStr based on a given conditional. 
 * NOTE: Typically used in specific steps or in a while loop. 
 * - fd     == int  , File descriptor corresponding to inBuf.
 * - inBuf  == char*, copy from buff. Must not be NULL, leave room for '\0'
 * - bfPl   == int, the current location inside inBuf.
 * - nbyte  == size_t, number of bytes to read
 * - retBytes == number of bytes returned when READ_INPUT is called
 * - resStr == char*, buffer to copy to.
 * - resLen == size_t, length of resStr for conidional overflow stop point 
 * - conditional == The conditionals desired in the copy process.
 *                Example: inBuf[i] != ' ' && inBuf[i] != '\n' 
 */
#define READ_PARSE(fd, inBuf, bfPl, nByte, retBytes, resStr, resLen, conditional)\
{                                                                              \
    int _TM_ = 0;                                                              \
    assert(resStr != NULL && inBuf != NULL);                                   \
                                                                               \
    for(_TM_ = 0; conditional && _TM_ < resLen; ++_TM_)                        \
    {                                                                          \
        resStr[_TM_] = inBuf[bfPl];                                            \
        ++bfPl;                  /* increase buff placement */                 \
        if('\0' == inBuf[(bfPl)]){   /* reached end of current buffer */       \
            READ_INPUT(fd, inBuf, nByte, retBytes);                            \
            inBuff[retBytes] = '\0';                                           \
            bfPl = 0;                                                          \
        }                                                                      \
    } /* end for */                                                            \
} // end READ_NEXT_FILE

/* TODO: Adjust this macro or make an alternate that can call a function with
         variable arguments, rather than just one argument. (i.e. free(pntr);) */
/* vectorizes a function funct, its C99 as fuck tho.
   -Type is the type of pointer used. (VA_ARGS could be void for example.). 
   -... is a variable argument list.
   -will execute every argument into the function.
   -funct only takes in one argument. */
#define APPLY_FUNCT(type, funct, ...)                                          \
{                                                                              \
    void *stopper = (int[]){0};                                                      \
    type **apply_list = (type*[]){__VA_ARGS__, stopper};                       \
    int __i_;                                                                  \
                                                                               \
    for(__i_ = 0; apply_list[__i_] != stopper; ++__i_){                        \
        (funct)(apply_list[__i_]);}                                            \
} /* end apply_funct */
    
/* apply free to every pointer given in the argument list using the
   apply_funct macro */
#define FREE_ALL(...)   APPLY_FUNCT(void, free, __VA_ARGS__)

/* Subtract two timespec structures and place them in a resulting timespec
 * struct.
 * All passed values are pointers of struct timespec.
 */
#define _NANO_1SEC 1000000000
#define TIMESPEC_SUB(toSubPtr, subByPtr, resRetPtr)                            \
{                                                                              \
    assert((toSubPtr) != NULL && (subByPtr) != NULL && (resRetPtr) != NULL);   \
                                                                               \
    (resRetPtr) -> tv_sec  = ((toSubPtr) -> tv_sec) - ((subByPtr) -> tv_sec);  \
    (resRetPtr) -> tv_nsec = ((toSubPtr) -> tv_nsec) - ((subByPtr) -> tv_nsec);\
                                                                               \
    /* If nano seconds is negetive, need to adjust for carry by adding 1       \
     * second to nano until nano is no longer negetive or seconds is zero. */  \
    while(0 > ((resRetPtr) -> tv_nsec) && 0 < ((resRetPtr) -> tv_sec)){        \
        --((resRetPtr) -> tv_sec);                                             \
        (resRetPtr) -> tv_nsec = ((resRetPtr) -> tv_nsec) + _NANO_1SEC;        \
    }                                                                          \
}//end TIMESPEC_SUB

#endif /************ EOF **************/
