mmake: mmake.o parser.o
	gcc -Wall mmake.o parser.o -g -o mmake

mmake.o: mmake.c parser.h
	gcc -Wall -c -g mmake.c

parser.o: parser.c
	gcc -Wall -c -g parser.c
