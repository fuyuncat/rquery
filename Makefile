all: rquery

rquery: rquery.cpp regexc.cpp regexc.h commfuncs.cpp commfuncs.h querierc.cpp querierc.h
	g++ -I /usr/lib/boost/include -L /usr/lib/boost/lib rquery.cpp regexc.cpp commfuncs.cpp querierc.cpp -lboost_regex-mt -o $@

