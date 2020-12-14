CCFLAGS=-std=c++11 -Wall -O3
CPP = g++

mancala: mancala.o
	$(CPP) $(CCFLAGS) -o mancala mancala.o

.cpp.o:
	$(CPP) $(CCFLAGS) -c $<
