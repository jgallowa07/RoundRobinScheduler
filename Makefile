CFLAGS=-W -Wall
OBJECTS=p1fxns.o linkedList.o
PROGRAMS= RR

all:$(PROGRAMS)

RR: RR.o $(OBJECTS)
	gcc $(CFLAGS) -o RR $^

p1fxns.o: p1fxns.c p1fxns.h 
RR.o: RR.c p1fxns.h linkedList.h
linkedList.o: linkedList.c linkedList.h
