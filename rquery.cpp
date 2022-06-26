/*******************************************************************************
//
//        File: rquery.cpp
// Description: Main File. query string/file using regular expression
//       Usage: rquery "parse <Named Captures regular expression> | where <filters> | group | select | sort " "file or string to be queried"
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
//     examples:
// ./rquery "parse /\\\"(?P<origip>.*)\\\" (?P<host>\S+) (?P<value>(?i)(abc|cba|\/))/|select origip" "\"185.7.214.104\" 10.50.26.20 aBc"
// ./rquery "\"(?P<origip>.*)\" (?P<host>\S+)" "\"185.7.214.104\" 10.50.26.20"
// ./rquery "\"(?P<origip>.*)\" (?P<host>\S+) \S+ (?P<user>\S+) \[(?P<time>.+)\] \"(?P<request>.*)\" (?P<status>[0-9]+) (?P<size>\S+) \"(?P<referrer>.*)\" \"(?P<agent>.*)\"" "\"185.7.214.104\" 10.50.26.20 - - [23/Jun/2022:06:47:41 +0100] \"GET /actuator/gateway/routes HTTP/1.1\" 302 239 \"http://108.128.20.93:80/actuator/gateway/routes\" \"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36\""
// ./rquery "\"(?P<origip>.*)\" (?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>.+)\] \"(?P<request>.*)\" (?P<status>[0-9]+) (?P<size>\S+) \"(?P<referrer>.*)\" \"(?P<agent>.*)\"" "\"185.7.214.104\" 10.50.26.20 - - [23/Jun/2022:06:47:41 +0100] \"GET /actuator/gateway/routes HTTP/1.1\" 302 239 \"http://108.128.20.93:80/actuator/gateway/routes\" \"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36\""
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <iomanip>
#include <boost/algorithm/string.hpp>
#include "commfuncs.h"
//#include "regexc.h"
#include "querierc.h"

string usage()
{
  return string("Usage: rquery \"parse <regular expression> | where <filters> | group | select | sort \" \"file or string to be queried\"\nquery string/file using regular expression\n");
}

// return operation type: -1 error; 0: unused; 1: parse; 2:select; 3: filter; 4: group; 5: sort
map<string,string> parseparam(string parameterstr)
{
  map<string,string> query;
  printf("Original string: %s\n", parameterstr.c_str());
  vector<string> params = split(parameterstr,'|','/','\\');
  for (int i = 0; i < params.size(); ++i){
    string trimmedstr = boost::algorithm::trim_copy<string>(params[i]);
    size_t found = params[i].find_first_of(" ");
    printf("Parameter %d: %s. Space at %d\n", i+1, params[i].c_str(),found);
    if  (found!=string::npos){
      printf("Operation %s: %s\n", boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(params[i].substr(0,found))).c_str(), boost::algorithm::trim_copy<string>(params[i].substr(found+1)).c_str());
      query.insert( pair<string,string>(boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(params[i].substr(0,found))),boost::algorithm::trim_copy<string>(params[i].substr(found+1))) );
    }
  }
  return query;
}

map<string,string> parsequery(string raw)
{
  map<string,string> query;
  query.insert( pair<string,string>("parse",raw) );
  return query;
}

/*string convertdate(string raw)
{
  string rex( "([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})" );
  RegExC re;

  vector<string> vs;
  string newstr;

  int r = re.sub( rex + ".*", raw, "\\4:\\5 \\2/\\3/\\1", &newstr );
  if ( r ) {
    r = re.match( rex, raw, &vs );
    if ( r ) 
      return "";
    else{
      return vs[ 2 ]+"/"+vs[ 3 ]+"/"+vs[ 1 ];
    }
  }else
    return newstr;
}*/

vector<string> parsereg(string raw, string rex, int & rst )
{
  QuerierC re(rex);
  vector<string> vs;
  re.setrawstr(raw);
  rst = re.boostmatch( &vs );
  return vs;
}

map<string,string> parseregmap(string raw, string rex, int & rst )
{
  map<string,string> matches;
  string newrex = rex;
  if (newrex[0] == '/')
    newrex = newrex.substr(1);
  if (newrex[newrex.size()-1] == '/')
    newrex = newrex.substr(0,newrex.size()-1);
  printf("Old Reg: %s\n",rex.c_str());
  printf("New Reg: %s\n",newrex.c_str());

  QuerierC re(newrex);
  re.setrawstr(raw);
  rst = re.boostmatch( matches );
  return matches;
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
  
  map<string,string> query = parseparam(argv[1]);
  /*
  for (map<string,string>::iterator it=query.begin(); it!=query.end(); ++it)
    printf("%s: %s\n", it->first.c_str(), it->second.c_str());
  */

  int rst;
  //vector<string> fields = parsereg(argv[2],query["parse"], rst);
  //for (int i = 0; i < fields.size(); ++i){
  //  printf("field %d: %s\n", i+1, fields[i].c_str());
  //}
  
  map<string,string> matches = parseregmap(argv[2],query["parse"], rst);
  for (map<string,string>::iterator it=matches.begin(); it!=matches.end(); ++it)
    printf("%s: %s\n", it->first.c_str(), it->second.c_str());
  
  vector<string> cmatches = parsereg(argv[2], query["parse"], rst);
  for (int i=1; i<cmatches.size(); i++)
    printf("%d: %s\n", i, cmatches[i].c_str());
}
