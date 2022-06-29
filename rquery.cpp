///////////////////////////////////////////////////////////////////////////////////////////////
//
//        File: rquery.cpp
// Description: Main File. query string/file using regular expression
//       Usage: rquery "parse <Named Captures regular expression> | filter <filters> | group | select | sort " "file or string to be queried"
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
//     examples:
// ./rquery "parse /\\\"(?P<origip>.*)\\\" (?P<host>\S+) (?P<value>(?i)abc|cba|\/)/|select origip" "\"185.7.214.104\" 10.50.26.20 aBc"
// ./rquery "parse /\\\"(?P<origip>[^\n]*)\\\" (?P<host>\S+) (?P<value>(?i)abc|cba|\/)/|select origip" "\"185.7.214.104\" 10.50.26.20 cBA asa
//\"185.7.214.104\" 10.50.26.20 AbC "
// echo "\"185.7.214.104\" 10.50.26.20 cBA asa
//\"185.7.214.104\" 10.50.26.20 AbC "|./rquery "parse /\\\"(?P<origip>[^\n]*)\\\" (?P<host>\S+) (?P<value>(?i)abc|cba|\/)/|select origip"
// ./rquery "parse /\\\"(?P<origip>.*)\\\" (?P<host>\S+) (?P<value>(?i)abc|cba|\/).*/|select origip" "\"185.7.214.104\" 10.50.26.20 cBA asa"
// ./rquery "parse /\\\"(?P<origip>.*)\\\" (?P<host>\S+) (?P<value>(?i)(abc|cba|\/))/|select origip" "\"185.7.214.104\" 10.50.26.20 aBc"
// ./rquery "\"(?P<origip>.*)\" (?P<host>\S+)" "\"185.7.214.104\" 10.50.26.20"
// ./rquery "\"(?P<origip>.*)\" (?P<host>\S+) \S+ (?P<user>\S+) \[(?P<time>.+)\] \"(?P<request>.*)\" (?P<status>[0-9]+) (?P<size>\S+) \"(?P<referrer>.*)\" \"(?P<agent>.*)\"" "\"185.7.214.104\" 10.50.26.20 - - [23/Jun/2022:06:47:41 +0100] \"GET /actuator/gateway/routes HTTP/1.1\" 302 239 \"http://108.128.20.93:80/actuator/gateway/routes\" \"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36\""
// ./rquery "\"(?P<origip>.*)\" (?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>.+)\] \"(?P<request>.*)\" (?P<status>[0-9]+) (?P<size>\S+) \"(?P<referrer>.*)\" \"(?P<agent>.*)\"" "\"185.7.214.104\" 10.50.26.20 - - [23/Jun/2022:06:47:41 +0100] \"GET /actuator/gateway/routes HTTP/1.1\" 302 239 \"http://108.128.20.93:80/actuator/gateway/routes\" \"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36\""
///////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <iomanip>
#include <iostream>
#include "commfuncs.h"
//#include "regexc.h"
#include "parser.h"
#include "querierc.h"
//#include <boost/regex.hpp>

//#define BOOST_REGEX_MATCH_EXTRA

string usage()
{
  return string("Usage: rquery \"parse <regular expression> | where <filters> | group | select | sort \" \"file or string to be queried\"\nquery string/file using regular expression\n");
}

int main(int argc, char *argv[])
{
  /*for (int i = 1; i < argc; ++i){
    string raw(argv[i]);
    string date = convertdate(raw);
    if (date.length()>0)
      printf("%s\n",(raw+": "+date).c_str());
    else
      printf("%s\n",(raw+": Cannot be converted!").c_str());
  }*/
  if ( argc < 2 ){
    printf("%s\n",usage().c_str());
    return 1;
  }
  
  ParserC ps;
  map<string,string> query = ps.parseparam(argv[1]);
  /*
  for (map<string,string>::iterator it=query.begin(); it!=query.end(); ++it)
    printf("%s: %s\n", it->first.c_str(), it->second.c_str());
  */

  int rst;
  map<string,string> matches;
  vector<string> cmatches;
  //vector<string> fields = ps.parsereg(argv[2],query["parse"], rst);
  //for (int i = 0; i < fields.size(); ++i){
  //  printf("field %d: %s\n", i+1, fields[i].c_str());
  //}

  string patternStr = "";
  if (query.find("parse") != query.end())
    patternStr = query["parse"];

  string rex = trim_one(patternStr, '/');
  QuerierC rq(rex);

  if (query.find("filter") != query.end())
    rq.assignFilter(ps.buildFilter(query["filter"]));

  if ( argc < 3 ){
    bool namePrinted = false;

    const size_t cache_length = 16384;
    char cachebuffer[cache_length];
    size_t howmany = 0, reads = 0;
    while(std::cin) {
      std::cin.read(cachebuffer, cache_length);
      //reads = getstr(cachebuffer, cache_length);
      if (reads < 1)
        break;
      rq.appendrawstr(string(cachebuffer));
      rq.searchAll();
      if (!namePrinted){
        rq.printFieldNames();
        namePrinted = true;
      }
      rq.outputAndClean();
      howmany += std::cin.gcount();
    }

    /*
    string lineInput;
    int ln = 0;
    //while (cin >> lineInput) {
    while (getline(cin,lineInput)) {
      //printf("%d:%s\n",ln,lineInput.c_str());
      rq.setrawstr(lineInput);
      rq.searchAll();
      if (!namePrinted){
        rq.printFieldNames();
        namePrinted = true;
      }
      rq.outputAndClean();

      //rst = rq.boostmatch( matches );
      //for (map<string,string>::iterator it=matches.begin(); it!=matches.end(); ++it)
      //  printf("%s: %s\n", it->first.c_str(), it->second.c_str());
      
      //rst = rq.boostmatch( &cmatches );
      //for (int i=0; i<cmatches.size(); i++)
      //  printf("%d: %s\n", i, cmatches[i].c_str());
      ln++;
    }
    if ( ln < 1 ){
      printf("%s\n",usage().c_str());
      return 1;
    }*/
  }else{
    rq.setrawstr(argv[2]);
    //rq.searchNext();
    rq.searchAll();
    rq.printFieldNames();
    rq.outputAndClean();
  }

  //boost::regex reg(rex);
  //boost::smatch match;
  //if (boost::regex_search(string(argv[2]), match, reg, boost::match_extra)){
  //  for (int i=1; i<match.size(); i++)
  //    printf("%s\t",string(match[i]).c_str());
  //  printf("%s\n",string(match["host"]).c_str());
  //}
}
