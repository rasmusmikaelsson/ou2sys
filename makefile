cFlags = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition
cc = gcc

mmake: mmake.o parser.o target.o
	$(cc) $(cFlags) -o mmake mmake.o parser.o target.o

mmake.o: mmake.c parser.h target.h
	$(cc) $(cFlags) -c mmake.c

parser.o: parser.c parser.h
	$(cc) $(cFlags) -c parser.c

target.o: target.c parser.h
	$(cc) $(cFlags) -c target.c

