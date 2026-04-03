CC = cc
FLAGS = -Wall -Wextra -Werror -ggdb -std=c99

all: test

test: test.c bt.h
	$(CC) $(FLAGS) test.c -o test 
