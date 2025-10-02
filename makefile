mmake: mmake.o parser.o
	gcc -Wall $^ -o mmake

mmake.o: mmake.c parser.h
	gcc -Wall $^ -c

parser.o: parser.c
	gcc -Wall $^ -c
