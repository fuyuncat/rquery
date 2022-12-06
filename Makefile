#all: rquery

#rquery: rquery.cpp commfuncs.cpp commfuncs.h querierc.cpp querierc.h parser.cpp parser.h filter.cpp filter.h
#	g++ -I /usr/lib/boost/include -L /usr/lib/boost/lib rquery.cpp commfuncs.cpp querierc.cpp parser.cpp filter.cpp -lboost_regex-mt -o $@

all: rquery
commfuncs.o: commfuncs.cpp commfuncs.h
	g++ -std=c++11 -lboost_regex -c commfuncs.cpp
function.o: function.cpp function.h
	g++ -std=c++11 -c function.cpp
expression.o: expression.cpp expression.h function.o
	g++ -std=c++11 -c expression.cpp
filter.o: filter.cpp filter.h expression.o
	g++ -std=c++11 -c filter.cpp
parser.o: parser.cpp parser.h filter.o
	g++ -std=c++11 -c parser.cpp
querierc.o: querierc.cpp querierc.h filter.o expression.o
	g++ -std=c++11 -c querierc.cpp
rquery.o: rquery.cpp commfuncs.h querierc.h parser.h filter.h
	g++ -std=c++11 -c rquery.cpp
rquery: rquery.o commfuncs.o querierc.o parser.o expression.o filter.o
	g++ -std=c++11 -lboost_regex -I /usr/lib/boost/include -L /usr/lib/boost/lib rquery.o commfuncs.o function.o expression.o filter.o parser.o querierc.o -o rq
install:
	install -m 755 rq /usr/local/bin/
check: all
	./rq
clean:
	rm -f *.o rq
