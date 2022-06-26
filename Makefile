all: rquery

rquery: rquery.cpp regexc.cpp regexc.h commfuncs.h commfuncs.cpp
	g++ -I /usr/lib/boost/include -L /usr/lib/boost/lib rquery.cpp regexc.cpp commfuncs.cpp -lboost_regex-mt -o $@

