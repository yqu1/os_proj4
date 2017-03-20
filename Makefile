CXX = g++
CXXFLAGS = -std=c++0x -g -Wall -lcurl 

all: proj4

proj4: proj4.cpp tsqueue.h
	${CXX} ${CXXFLAGS} -o $@ proj4.cpp tsqueue.h

clean:
	rm -f *.out proj4
	rm -r *.dSYM
