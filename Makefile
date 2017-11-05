CFLAGS=-W -Wall
OBJECTS=p1fxns.o linkedList.o
PROGRAMS= thv4

all:$(PROGRAMS)

thv4: thv4.o $(OBJECTS)
	gcc $(CFLAGS) -o thv4 $^

p1fxns.o: p1fxns.c p1fxns.h 
thv4.o: thv4.c p1fxns.h linkedList.h
linkedList.o: linkedList.c linkedList.h
