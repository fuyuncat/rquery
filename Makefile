all: rquery

rquery: rquery.cpp commfuncs.cpp commfuncs.h querierc.cpp querierc.h
	g++ -I /usr/lib/boost/include -L /usr/lib/boost/lib rquery.cpp commfuncs.cpp querierc.cpp -lboost_regex-mt -o $@

