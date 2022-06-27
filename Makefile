all: rquery

rquery: rquery.cpp commfuncs.cpp commfuncs.h querierc.cpp querierc.h parser.cpp parser.h filter.cpp filter.h
	g++ -I /usr/lib/boost/include -L /usr/lib/boost/lib rquery.cpp commfuncs.cpp querierc.cpp parser.cpp filter.cpp -lboost_regex-mt -o $@

