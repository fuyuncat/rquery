#all: rquery

#rquery: rquery.cpp commfuncs.cpp commfuncs.h querierc.cpp querierc.h parser.cpp parser.h filter.cpp filter.h
#	g++ -I /usr/lib/boost/include -L /usr/lib/boost/lib rquery.cpp commfuncs.cpp querierc.cpp parser.cpp filter.cpp -lboost_regex-mt -o $@

all: rquery
commfuncs.o: commfuncs.cpp commfuncs.h
	g++ -c commfuncs.cpp
filter.o: filter.cpp filter.h
	g++ -c filter.cpp
parser.o: parser.cpp parser.h filter.o
	g++ -c parser.cpp
querierc.o: querierc.cpp querierc.h filter.o
	g++ -c querierc.cpp
rquery.o: rquery.cpp commfuncs.h querierc.h parser.h filter.h
	g++ -c rquery.cpp
rquery: rquery.o commfuncs.o querierc.o parser.o filter.o
	g++ -I /usr/lib/boost/include -L /usr/lib/boost/lib rquery.o commfuncs.o filter.o parser.o querierc.o -o $@
check: all
	./rquery
clean:
	rm -f *.o rquery
