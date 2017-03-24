CXX = g++
CXXFLAGS = -std=c++0x -g -Wall -lcurl

all: site-tester

site-tester: site-tester.cpp tsqueue.h curlUtil.h utils.h writeHtml.h
	${CXX} ${CXXFLAGS} -o $@ site-tester.cpp tsqueue.h curlUtil.h utils.h writeHtml.h

clean:
	rm -f *.out site-tester
	rm *.csv *.html
