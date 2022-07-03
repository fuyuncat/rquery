#all: rquery

#rquery: rquery.cpp commfuncs.cpp commfuncs.h querierc.cpp querierc.h parser.cpp parser.h filter.cpp filter.h
#	g++ -I /usr/lib/boost/include -L /usr/lib/boost/lib rquery.cpp commfuncs.cpp querierc.cpp parser.cpp filter.cpp -lboost_regex-mt -o $@

all: rquery
commfuncs.o: commfuncs.cpp commfuncs.h
	g++ -g -c commfuncs.cpp
function.o: function.cpp function.h
	g++ -g -c function.cpp
expression.o: expression.cpp expression.h function.o
	g++ -g -c expression.cpp
filter.o: filter.cpp filter.h expression.o
	g++ -g -c filter.cpp
parser.o: parser.cpp parser.h filter.o
	g++ -g -c parser.cpp
querierc.o: querierc.cpp querierc.h filter.o
	g++ -g -c querierc.cpp
rquery.o: rquery.cpp commfuncs.h querierc.h parser.h filter.h
	g++ -g -c rquery.cpp
rquery: rquery.o commfuncs.o querierc.o parser.o expression.o filter.o
	g++ -g -I /usr/lib/boost/include -L /usr/lib/boost/lib rquery.o commfuncs.o function.o expression.o filter.o parser.o querierc.o -o $@
check: all
	./rquery
clean:
	rm -f *.o rquery
