///////////////////////////////////////////////////////////////////////////////////////////////
//
//        File: rquery.cpp
// Description: Main File. query string/file using regular expression
//       Usage: rquery "parse <Named Captures regular expression> | set <col type,..> | filter <filters> | group <col1,..> | select <col1,..> | sort <col1 [asc|desc],..> | limit <N | FromN,ToN> " "file or string to be queried"
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
//     examples:
// ./rquery "parse /\\\"(?P<origip>.*)\\\" (?P<host>\S+) (?P<value>(?i)abc|cba|\/)/|select origip" "\"185.7.214.104\" 10.50.26.20 aBc"
// ./rquery "parse /\\\"(?P<origip>.*)\\\" (?P<host>\S+) (?P<value>(?i)abc|cba|\/)/|select origip|filter host like '10.50.26.*' and value='aBc'" "\"185.7.214.104\" 10.50.26.20 aBc"
// ./rquery "parse /\\\"(?P<origip>.*)\\\" (?P<host>\S+) (?P<value>(?i)abc|cba|\/)/|select origip|filter host like '10.50.26.*' and value='aB'+'c'" "\"185.7.214.104\" 10.50.26.20 aBc"
// ./rquery "parse /\\\"(?P<origip>.*)\\\" (?P<host>\S+) (?P<value>(?i)abc|cba|\/) (?P<hits>\S+)/|set hits double|select origip+'/'+host,now()|filter host like '10.50.'+26+'.*' and value='a'+(upper('b')+'c') and @field4=round(880.3)+(18-10) and @row<=1 and value in ('abc','aBc')" "\"185.7.214.104\" 10.50.26.20 aBc 888"
// ./rquery "parse /\\\"(?P<origip>.*)\\\" (?P<host>\S+) (?P<value>(?i)abc|cba|\/)/|select origip|filter host like '10.50.26.*' and (value='aBc' or value='CbA')" "\"185.7.214.104\" 10.50.26.20 aBc"
// ./rquery "parse /\\\"(?P<origip>[^\n]*)\\\" (?P<host>\S+) (?P<value>(?i)abc|cba|\/)/|select origip" "\"185.7.214.104\" 10.50.26.20 cBA asa
//\"185.7.214.104\" 10.50.26.20 AbC "
// echo "\"185.7.214.104\" 10.50.26.20 cBA asa
// ./rquery "parse /\\\"(?P<origip>[^\n]*)\\\" (?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter origip like '101.189.81.25*'" < result.lst
//\"185.7.214.104\" 10.50.26.20 AbC "|./rquery "parse /\\\"(?P<origip>[^\n]*)\\\" (?P<host>\S+) (?P<value>(?i)abc|cba|\/)/|select origip"
// ./rquery "parse /\\\"(?P<origip>[^\n]*)\\\" (?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter origip like '185.7.214.104*'" "\"185.7.214.104\" 10.50.26.20 - - [23/Jun/2022:06:47:41 +0100] \"GET /actuator/gateway/routes HTTP/1.1\" 302 239 \"http://108.128.20.93:80/actuator/gateway/routes\" \"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36\""
// ./rquery "parse /\\\"(?P<origip>[^\n]*)\\\" (?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter origip like '101.189.81.25*'" < /var/log/httpd/cpa_access_log.29062022
///////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <sys/time.h>
#include <sys/stat.h>
#include "commfuncs.h"
//#include "regexc.h"
#include "parser.h"
#include "querierc.h"
//#include <boost/regex.hpp>

//#define BOOST_REGEX_MATCH_EXTRA

//size_t g_inputbuffer;

GlobalVars gv;

void usage()
{
  printf("Program Name: RQuery AKA RQ\nContact Email: fuyuncat@gmail.com\nUsage: rquery \"parse <regular expression> | select | set | filter <filters> | group | sort \" \"file or string to be queried\"\nquery string/file using regular expression\n");
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
    usage();
    return 1;
  }
  
  gv.setVars(16384*2, ERROR, true);
  ParserC ps;
  bool bGroup = false;
  short int readMode = PROMPT;
  string sContent = "";
  QuerierC rq;

  for (int i=1; i<argc; i++){
    if (argv[i][0]=='-' && i>=argc && boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("-h")!=0 && boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--help")!=0){
      usage();
      trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
      return 1;
    }
    if (boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("-q")==0 || boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--query")==0){
      map<string,string> query = ps.parseparam(argv[i+1]);
      dumpMap(query);

      int rst;
      map<string,string> matches;
      vector<string> cmatches;

      string patternStr = "[^\n]*"; // if no PARSE passed, search each lines
      if (query.find("parse") != query.end())
        patternStr = query["parse"];

      string rex = trim_one(patternStr, '/');
      rq.setregexp(rex);

      if (query.find("filter") != query.end()){
        trace(DEBUG,"Assigning filter: %s \n", query["filter"].c_str());
        FilterC* filter = new FilterC(query["filter"]);
        rq.assignFilter(filter);
      }
      if (query.find("set") != query.end()){
        trace(DEBUG,"Setting fields data type: %s \n", query["set"].c_str());
        rq.setFieldTypeFromStr(query["set"]);
      }
      // assign GROUP before assigning SELECTION and SORT. expressions in SELECTION and SORT should present in GROUP
      if (query.find("group") != query.end()){
        trace(DEBUG,"Setting group : %s \n", query["group"].c_str());
        rq.assignGroupStr(query["group"]);
        bGroup = true;
      }
      if (query.find("select") != query.end()){
        trace(DEBUG,"Assigning selections: %s \n", query["select"].c_str());
        rq.assignSelString(query["select"]);
      }
      if (query.find("sort") != query.end()){
        trace(DEBUG,"Assigning sorting keys: %s \n", query["sort"].c_str());
        rq.assignSortStr(query["sort"]);
      }
      if (query.find("limit") != query.end()){
        trace(DEBUG,"Assigning limit numbers: %s \n", query["limit"].c_str());
        rq.assignLimitStr(query["limit"]);
      }
      i++;
    }else if (boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("-h")==0 || boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--help")==0){
      if (i==argc-1){
        usage();
        return 0;
      }
      string topic = string(argv[i+1]);
      return 0;
      i++;
    }else if (boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("-m")==0 || boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--msglevel")==0){
      int iLevel=encodeTracelevel(string(argv[i+1]));
      if (iLevel!=UNKNOWN){
        gv.g_tracelevel = iLevel;
      }else{
        trace(FATAL,"Unrecognized message level %s. It should be one of INFO, WARNING, ERROR, FATAL.\n", argv[i]);
        return 1;
      }
      i++;
    }else{
      //trace(DEBUG1,"Content: %s.\n", argv[i]);
      sContent = string(argv[i]);
      readMode = PARAMETER;

      struct stat s;
      if( stat(argv[i],&s) == 0 ){
        if( s.st_mode & S_IFDIR )
          readMode = FOLDER;
        else if( s.st_mode & S_IFREG )
          readMode = FILE;
        else
          readMode = PARAMETER;
      }else
        readMode = PARAMETER;
    }
  }
    
  switch (readMode){
    case PARAMETER:{
      rq.setrawstr(sContent);
      //rq.searchNext();
      rq.searchAll();
      rq.group();
      rq.sort();
      rq.printFieldNames();
      rq.outputAndClean();
      break;
    }
    case FILE:{
      long int thisTime,lastTime = curtime();

      ifstream ifile(sContent.c_str());
      //streamsize size = ifile.tellg();
      ifile.seekg(0, ios::beg);

      const size_t cache_length = gv.g_inputbuffer;
      char cachebuffer[cache_length];
      size_t pos = ios::beg, reads = 0;

      memset( cachebuffer, '\0', sizeof(char)*cache_length );
      while (ifile.read(cachebuffer, cache_length)){
        ifile.seekg(pos, ios::beg);
        rq.appendrawstr(string(cachebuffer));
        rq.searchAll();
        rq.printFieldNames();
        if (!bGroup)
          rq.outputAndClean();
        pos += cache_length;
        memset( cachebuffer, '\0', sizeof(char)*cache_length );
      }
      thisTime = curtime();
      trace(DEBUG2, "Reading and searching: %u\n", thisTime-lastTime);
      lastTime = thisTime;
      if (bGroup){
        rq.group();
        thisTime = curtime();
        trace(DEBUG2, "Grouping: %u\n", thisTime-lastTime);
        lastTime = thisTime;
        rq.sort();
        thisTime = curtime();
        trace(DEBUG2, "Sorting: %u\n", thisTime-lastTime);
        rq.outputAndClean();
        thisTime = curtime();
        trace(DEBUG2, "Printing: %u\n", thisTime-lastTime);
        lastTime = thisTime;
      }
      lastTime = thisTime;
      trace(DEBUG1,"%d bytes read.\n", pos);
      break;
    }
    case FOLDER:{
      break;
    }
    case PROMPT:{
      long int thisTime,lastTime = curtime();
      const size_t cache_length = gv.g_inputbuffer;
      char cachebuffer[cache_length];
      size_t howmany = 0, reads = 0;
      while(std::cin) {
        memset( cachebuffer, '\0', sizeof(char)*cache_length );
        std::cin.read(cachebuffer, cache_length);
        rq.appendrawstr(string(cachebuffer));
        rq.searchAll();
        rq.printFieldNames();
        if (!bGroup)
          rq.outputAndClean();
        howmany += std::cin.gcount();
      }
      thisTime = curtime();
      trace(DEBUG2, "Reading and searching: %u\n", thisTime-lastTime);
      lastTime = thisTime;
      if (bGroup){
        rq.group();
        thisTime = curtime();
        trace(DEBUG2, "Grouping: %u\n", thisTime-lastTime);
        lastTime = thisTime;
        rq.sort();
        thisTime = curtime();
        trace(DEBUG2, "Sorting: %u\n", thisTime-lastTime);
        rq.outputAndClean();
        thisTime = curtime();
        trace(DEBUG2, "Printing: %u\n", thisTime-lastTime);
        lastTime = thisTime;
      }
      lastTime = thisTime;
      trace(DEBUG1,"%d bytes read.\n", howmany);
      break;
    }
    default:{
      usage();
      trace(FATAL,"Please provide content to be queried!\n");
      return 1;
    }
  }
  trace(DEBUG, "Found %d row(s).\n", rq.getOutputCount());
  trace(DEBUG, "Processed %d line(s).\n", rq.getLines());

  /*
  ParserC ps;
  map<string,string> query = ps.parseparam(argv[1]);
  dumpMap(query);
  //ps.dumpQueryparts();
  //for (map<string,string>::iterator it=query.begin(); it!=query.end(); ++it)
    printf("%s: %s\n", it->first.c_str(), it->second.c_str());

  int rst;
  map<string,string> matches;
  vector<string> cmatches;
  bool bGroup = false;
  //vector<string> fields = ps.parsereg(argv[2],query["parse"], rst);
  //for (int i = 0; i < fields.size(); ++i){
  //  printf("field %d: %s\n", i+1, fields[i].c_str());
  //}

  string patternStr = "[^\n]*"; // if no PARSE passed, search each lines
  if (query.find("parse") != query.end())
    patternStr = query["parse"];

  string rex = trim_one(patternStr, '/');
  QuerierC rq(rex);

  if (query.find("filter") != query.end()){
    trace(INFO,"Assigning filter: %s \n", query["filter"].c_str());
    FilterC* filter = new FilterC(query["filter"]);
    rq.assignFilter(filter);
  }
  // assign GROUP before assigning SELECTION and SORT. expressions in SELECTION and SORT should present in GROUP
  if (query.find("group") != query.end()){
    trace(INFO,"Setting group : %s \n", query["group"].c_str());
    rq.assignGroupStr(query["group"]);
    bGroup = true;
  }
  if (query.find("select") != query.end()){
    trace(INFO,"Assigning selections: %s \n", query["select"].c_str());
    rq.assignSelString(query["select"]);
  }
  if (query.find("set") != query.end()){
    trace(INFO,"Setting fields data type: %s \n", query["set"].c_str());
    rq.setFieldTypeFromStr(query["set"]);
  }
  
  if ( argc < 3 ){
    bool namePrinted = false;

    trace(INFO, "g_inputbuffer:%d\n",gv.g_inputbuffer);
    //printf("g_inputbuffer:%d\n",gv.g_inputbuffer);
    const size_t cache_length = gv.g_inputbuffer;
    char cachebuffer[cache_length];
    size_t howmany = 0, reads = 0;
    while(std::cin) {
      memset( cachebuffer, '\0', sizeof(char)*cache_length );
      std::cin.read(cachebuffer, cache_length);
      //reads = getstr(cachebuffer, cache_length);
      //if (reads < 1)
      //  break;
      rq.appendrawstr(string(cachebuffer));
      rq.searchAll();
      if (!namePrinted){
        rq.printFieldNames();
        namePrinted = true;
      }
      if (!bGroup)
        rq.outputAndClean();
      howmany += std::cin.gcount();
    }
    if (bGroup){
      rq.group();
      rq.outputAndClean();
    }

    //string lineInput;
    //int ln = 0;
    ////while (cin >> lineInput) {
    //while (getline(cin,lineInput)) {
    //  //printf("%d:%s\n",ln,lineInput.c_str());
    //  rq.setrawstr(lineInput);
    //  rq.searchAll();
    //  if (!namePrinted){
    //    rq.printFieldNames();
    //    namePrinted = true;
    //  }
    //  rq.outputAndClean();

    //  //rst = rq.boostmatch( matches );
    //  //for (map<string,string>::iterator it=matches.begin(); it!=matches.end(); ++it)
    //  //  printf("%s: %s\n", it->first.c_str(), it->second.c_str());
      
    //  //rst = rq.boostmatch( &cmatches );
    //  //for (int i=0; i<cmatches.size(); i++)
    //  //  printf("%d: %s\n", i, cmatches[i].c_str());
    //  ln++;
    //}
    //if ( ln < 1 ){
    //  printf("%s\n",usage().c_str());
    //  return 1;
    //}
  }else{
    rq.setrawstr(argv[2]);
    //rq.searchNext();
    rq.searchAll();
    rq.group();
    rq.printFieldNames();
    rq.outputAndClean();
  }
  trace(DEBUG, "Found %d row(s).\n", rq.getOutputCount());

  //boost::regex reg(rex);
  //boost::smatch match;
  //if (boost::regex_search(string(argv[2]), match, reg, boost::match_extra)){
  //  for (int i=1; i<match.size(); i++)
  //    printf("%s\t",string(match[i]).c_str());
  //  printf("%s\n",string(match["host"]).c_str());
  //}
  */
}
