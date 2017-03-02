CC = gcc
CPPFLAGS = -c -Wall -Wextra -pthread -m64 -O0 -g -pedantic -std=c99 
LDFLAGS = -pthread -m64

shell.out:				main.o shell.o err_handle.o convNum.o history.o cmdInput.o pipeShell.o handleExec.o
						$(CC) $(LDFLAGS) main.o shell.o err_handle.o cmdInput.o history.o convNum.o pipeShell.o handleExec.o -o shell.out

main.o:					main.c ./utility_sys.h
						$(CC) $(CPPFLAGS) main.c

shell.o:				shell.c ./utility_sys.h
						$(CC) $(CPPFLAGS) shell.c

err_handle.o:			./err_handle/err_handle.c ./utility_sys.h
						$(CC) $(CPPFLAGS) ./err_handle/err_handle.c

convNum.o:				./convNum/convNum.c ./utility_sys.h
						$(CC) $(CPPFLAGS) ./convNum/convNum.c

history.o:				history.c ./utility_sys.h
						$(CC) $(CPPFLAGS) history.c

cmdInput.o:				cmdInput.c ./utility_sys.h
						$(CC) $(CPPFLAGS) cmdInput.c

pipeShell.o:			pipeShell.c ./utility_sys.h
						$(CC) $(CPPFLAGS) pipeShell.c

handleExec.o:			handleExec.c ./utility_sys.h
						$(CC) $(CPPFLAGS) handleExec.c

.PHONY: check
check:					# check for memory leak
						$(info -- Checking For Memory Leaks --)
						make
						valgrind --leak-check=full ./shell.out

.PHONY: debug
debug:					# GNU debugger
						$(info -- Debugging --)
						gdb ./shell.out

.PHONY: clean
clean:					# clean the directory
						$(info -- Cleaning The Directory --)
						rm -rf *.o shell.out
.PHONY: all
all:					# run the program as follows
						$(info -- Running Program --)
						make
						./shell.out
