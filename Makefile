CCFLAGS=-Wall -Wextra -pedantic -std=c17

all:
	gcc ntpc.c -o ntpc $(CCFLAGS)
