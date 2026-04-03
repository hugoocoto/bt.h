FLAGS = -Wall -Wextra -Werror -ggdb -std=c99

all: test

test: test.c bt.h
	gcc $(FLAGS) test.c -o test 
