CXX = g++
CXXFLAGS = -std=c++0x -g -Wall -lcurl -lxml2

all: proj4

proj4: proj4.cpp tsqueue.h curlUtil.h utils.h writeHtml.h
	${CXX} ${CXXFLAGS} -o $@ proj4.cpp tsqueue.h curlUtil.h utils.h writeHtml.h

clean:
	rm -f *.out proj4
	rm *.csv *.html
