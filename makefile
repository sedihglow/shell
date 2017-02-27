CC = gcc
CPPFLAGS = -c -Wall -Wextra -pthread -m64 -O0 -g -pedantic -std=c99 
LDFLAGS = -pthread -m64

shell.out:				main.o shell.o err_handle.o convNum.o
						$(CC) $(LDFLAGS) main.o shell.o err_handle.o convNum.o -o shell.out

main.o:					main.c ./utility_sys.h
						$(CC) $(CPPFLAGS) main.c

err_handle.o:			./err_handle/err_handle.c ./utility_sys.h
						$(CC) $(CPPFLAGS) ./err_handle/err_handle.c

convNum.o:				./convNum/convNum.c ./utility_sys.h
						$(CC) $(CPPFLAGS) ./convNum/convNum.c

shell.o:				shell.c ./utility_sys.h
						$(CC) $(CPPFLAGS) shell.c


.PHONY: check
check:					# check for memory leak
						$(info -- Checking For Memory Leaks --)
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
						./shell.out
