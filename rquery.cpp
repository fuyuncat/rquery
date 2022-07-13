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
#include <sys/stat.h>
#include "commfuncs.h"
//#include "regexc.h"
#include "parser.h"
#include "querierc.h"
//#include <boost/regex.hpp>

#if defined _WIN32 || defined _WIN64
#include <windows.h>
#include <tchar.h>
#elif __unix || __unix__ || __linux__
#include <dirent.h>
#endif

//size_t g_inputbuffer;

GlobalVars gv;

void usage()
{
  printf("Program Name: RQuery AKA RQ\nContact Email: fuyuncat@gmail.com\nUsage: rquery \"parse <regular expression> | select | set | filter <filters> | group | sort \" \"file or string to be queried\"\nquery string/file using regular expression\n");
}

short int checkReadMode(string sContent)
{
  short int readMode = PARAMETER;

  struct stat s;
  if( stat(sContent.c_str(),&s) == 0 ){
    if( s.st_mode & S_IFDIR )
      readMode = FOLDER;
    else if( s.st_mode & S_IFREG )
      readMode = FILE;
    else
      readMode = PARAMETER;
  }else
    readMode = PARAMETER;
  
  return readMode;
}

vector<string> listFilesInFolder(string foldername)
{
  vector<string> filelist;
#if defined _WIN32 || defined _WIN64
  WIN32_FIND_DATA fd; 
  HANDLE hFind = ::FindFirstFile(foldername.c_str(), &fd); 
  if(hFind != INVALID_HANDLE_VALUE) { 
    do { 
      // read all (real) files in current folder
      // , delete '!' read other 2 default folder . and ..
      if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
        filelist.push_back(fd.cFileName);
      }
    }while(::FindNextFile(hFind, &fd)); 
    ::FindClose(hFind); 
  } 
#elif __unix || __unix__ || __linux__
  struct dirent *pDirent;
  DIR *pDir = opendir(foldername.c_str());
  while ((pDirent = readdir(pDir)) != NULL)
    if (strcmp(pDirent->d_name,".")!=0 && strcmp(pDirent->d_name,"..")!=0)
      filelist.push_back(string(pDirent->d_name));
  (void)closedir(pDir);
#endif 
  return filelist;
}

void processFile(string filename, QuerierC & rq, short int fileMode=READBUFF, int iSkip=0)
{
  long int thisTime,lastTime = curtime();

  rq.setFileName(filename);
  ifstream ifile(filename.c_str());
  if (!ifile){
    trace(ERROR, "Failed to open file '%s'.\n", filename.c_str());
    return;
  }
  
  switch (fileMode){
    case READLINE:{
      string strline;
      int readLines = 0;
      while (std::getline(ifile, strline)){
        if (readLines<iSkip){
          readLines++;
          continue;
        }
        if (rq.searchStopped())
          break;
        rq.appendrawstr(strline);
        rq.searchAll();
        rq.printFieldNames();
        if (!rq.toGroupOrSort())
          rq.outputAndClean();
        rq.setrawstr("");
        readLines++;
        thisTime = curtime();
        trace(DEBUG2, "\r%d lines read in %f seconds, Raw string size is %d bytes.", readLines, (double)(thisTime-lastTime)/1000, rq.getRawStrSize());
      }
      trace(DEBUG2, "\n");
      thisTime = curtime();
      trace(DEBUG2, "Reading and searching: %u\n", thisTime-lastTime);
      lastTime = thisTime;
      trace(DEBUG2,"%d lines read. (%d lines skipped)\n", readLines, iSkip);
      trace(DEBUG2, "Found %d row(s).\n", rq.getOutputCount());
      trace(DEBUG2, "Parsed %d line(s).\n", rq.getLines());
      break;
    }case READBUFF:
    default:{
      //streamsize size = ifile.tellg();
      ifile.seekg(iSkip, ios::beg);

      const size_t cache_length = gv.g_inputbuffer;
      //char cachebuffer[cache_length];
      char* cachebuffer = (char*)malloc(cache_length*sizeof(char));
      size_t howmany = 0;

      memset( cachebuffer, '\0', sizeof(char)*cache_length );
      while(!ifile.eof()) {
        if (rq.searchStopped())
          break;
        ifile.read(cachebuffer, cache_length);
        //ifile.seekg(pos, ios::beg);
        rq.appendrawstr(string(cachebuffer));
        rq.searchAll();
        rq.printFieldNames();
        if (!rq.toGroupOrSort())
          rq.outputAndClean();
        howmany += ifile.gcount();
        memset( cachebuffer, '\0', sizeof(char)*cache_length );
        thisTime = curtime();
        trace(DEBUG2, "\r%d bytes read in %f seconds, Raw string size is %d bytes.", howmany, (double)(thisTime-lastTime)/1000, rq.getRawStrSize());
      }
      free(cachebuffer);
      thisTime = curtime();
      trace(DEBUG2, "Reading and searching: %u\n", thisTime-lastTime);
      lastTime = thisTime;
      trace(DEBUG2,"%d bytes read. (%d bytes skipped)\n", howmany, iSkip);
      trace(DEBUG2, "Found %d row(s).\n", rq.getOutputCount());
      trace(DEBUG2, "Parsed %d line(s).\n", rq.getLines());
      break;
    }
  }
}

void printResult(QuerierC & rq)
{
  long int thisTime,lastTime = curtime();
  if (rq.toGroupOrSort()){
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
  rq.clear();
}

void processQuery(string sQuery, QuerierC & rq)
{
  ParserC ps;
  map<string,string> query = ps.parseparam(sQuery);
  dumpMap(query);

  int rst;
  map<string,string> matches;
  vector<string> cmatches;

  string patternStr = "[^\n]*"; // if no PARSE passed, search each lines
  if (query.find("parse") != query.end())
    patternStr = query["parse"];

  string rex = trim_one(patternStr, '/');
  //trace(DEBUG2,"Searching pattern: %s \n", rex.c_str());
  rq.setregexp(rex);

  if (query.find("filter") != query.end()){
    //trace(DEBUG,"Assigning filter: %s \n", query["filter"].c_str());
    FilterC* filter = new FilterC(query["filter"]);
    rq.assignFilter(filter);
  }
  if (query.find("set") != query.end()){
    //trace(DEBUG,"Setting fields data type: %s \n", query["set"].c_str());
    rq.setFieldTypeFromStr(query["set"]);
  }
  // assign GROUP before assigning SELECTION and SORT. expressions in SELECTION and SORT should present in GROUP
  if (query.find("group") != query.end()){
    //trace(DEBUG,"Setting group : %s \n", query["group"].c_str());
    rq.assignGroupStr(query["group"]);
  }
  if (query.find("select") != query.end()){
    //trace(DEBUG,"Assigning selections: %s \n", query["select"].c_str());
    rq.assignSelString(query["select"]);
  }
  if (query.find("sort") != query.end()){
    //trace(DEBUG,"Assigning sorting keys: %s \n", query["sort"].c_str());
    rq.assignSortStr(query["sort"]);
  }
  if (query.find("limit") != query.end()){
    //trace(DEBUG,"Assigning limit numbers: %s \n", query["limit"].c_str());
    rq.assignLimitStr(query["limit"]);
  }
}

void runQuery(string sContent, short int readMode, QuerierC & rq, short int fileMode=READBUFF, int iSkip=0)
{
  switch (readMode){
    case PARAMETER:{
      //trace(DEBUG1,"Processing content from parameter \n");
      rq.setrawstr(sContent);
      //rq.searchNext();
      rq.searchAll();
      rq.group();
      rq.sort();
      rq.printFieldNames();
      rq.outputAndClean();
      rq.clear();
      break;
    }
    case FILE:{
      //trace(DEBUG1,"Processing content from file \n");
      processFile(sContent, rq, fileMode, iSkip);
      printResult(rq);
      break;
    }
    case FOLDER:{
      //trace(DEBUG1,"Processing content from folder \n");
      vector<string> filelist = listFilesInFolder(sContent);
      for (int i=0; i<filelist.size(); i++){
        trace(DEBUG1,"Processing file: %s \n", (sContent+"/"+filelist[i]).c_str());
        processFile(sContent+"/"+filelist[i], rq, fileMode, iSkip);
      }
      printResult(rq);
      break;
    }
    case PROMPT:{
      //trace(DEBUG1,"Processing content from input or pipe \n");
      long int thisTime,lastTime = curtime();
      const size_t cache_length = gv.g_inputbuffer;
      //char cachebuffer[cache_length];
      char* cachebuffer = (char*)malloc(cache_length*sizeof(char));
      size_t howmany = 0, reads = 0;
      while(std::cin) {
        if (rq.searchStopped())
          break;
        memset( cachebuffer, '\0', sizeof(char)*cache_length );
        std::cin.read(cachebuffer, cache_length);
        rq.appendrawstr(string(cachebuffer));
        rq.searchAll();
        rq.printFieldNames();
        if (!rq.toGroupOrSort())
          rq.outputAndClean();
        howmany += std::cin.gcount();
      }
      free(cachebuffer);
      thisTime = curtime();
      trace(DEBUG2, "Reading and searching: %u\n", thisTime-lastTime);
      lastTime = thisTime;
      if (rq.toGroupOrSort()){
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
      trace(DEBUG2, "Found %d row(s).\n", rq.getOutputCount());
      trace(DEBUG2, "Processed %d line(s).\n", rq.getLines());
      rq.clear();
      trace(DEBUG2,"%d bytes read.\n", howmany);
      break;
    }
    default:{
      usage();
      trace(FATAL,"Please provide content to be queried!\n");
      return;
    }
  }
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
  
  gv.setVars(16384*2, FATAL, true);
  bool bConsoleMode = false;
  short int readMode = PROMPT, fileMode = READBUFF;
  int iSkip = 0;
  string sQuery = "", sContent = "";
  QuerierC rq;

  for (int i=1; i<argc; i++){
    if (argv[i][0]=='-' && i>=argc && boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("-h")!=0 && boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--help")!=0 && boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--console")!=0){
      usage();
      trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
      return 1;
    }
    if (boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("-h")==0 || boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--help")==0){
      if (i==argc-1){
        usage();
        return 0;
      }
      string topic = string(argv[i+1]);
      return 0;
      i++;
    }else if (boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--console")==0){
      bConsoleMode = true;
    }else if (boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("-f")==0 || boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--fieldheader")==0){
      gv.g_printheader = (boost::algorithm::to_lower_copy<string>(string(argv[i+1])).compare("off")!=0);
      i++;
    }else if (boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("-r")==0 || boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--readmode")==0){
      fileMode = (boost::algorithm::to_lower_copy<string>(string(argv[i+1])).compare("line")!=0?READBUFF:READLINE);
      i++;
    }else if (boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("-s")==0 || boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--skip")==0){
      if (!isInt(argv[i+1])){
        trace(FATAL,"%s is not a correct skip size number.\n", argv[i]);
        return 1;
      }
      iSkip = atoi((argv[i+1]));
      i++;
    }else if (boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("-b")==0 || boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--buffsize")==0){
      if (!isInt(argv[i+1])){
        trace(FATAL,"%s is not a correct buffer size number.\n", argv[i]);
        return 1;
      }
      gv.g_inputbuffer = atoi((argv[i+1]));
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
    }else if (boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("-q")==0 || boost::algorithm::to_lower_copy<string>(string(argv[i])).compare("--query")==0){
      sQuery = string(argv[i+1]);
      //trace(DEBUG2,"Query string: %s.\n", sQuery.c_str());
      i++;
    }else{
      //trace(DEBUG1,"Content: %s.\n", argv[i]);
      sContent = string(argv[i]);
      readMode = checkReadMode(sContent);
    }
  }

  if (bConsoleMode){
    cout << "Welcome to RQuery ";
    cout << VERSION;
    cout << "\n";
    cout << "Author: Wei Huang; Contact Email: fuyuncat@gmail.com\n";
    cout << "\n";
    cout << "rquery >";
    string lineInput;
    FilterC* filter = NULL;
    while (getline(cin,lineInput)) {
      if (boost::algorithm::trim_copy<string>(lineInput).empty())
        cout << "rquery >";
      else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).compare("q")==0 || boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).compare("quit")==0){
        break;
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("h ")==0 || boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("help")==0){
        cout << "load <file/folder> -- Provide a file or folder to be queried.\n";
        cout << "parse /<regular expression string>/ -- Provide a regular express string quoted by \"//\" to parse the content.\n";
        cout << "set <field datatype [date format],...> -- Set the date type of the fields.\n";
        cout << "filter <filter conditions> -- Provide filter conditions to filt the content.\n";
        cout << "select <field or expression,...> -- Provide a field name/variables/expressions to be selected.\n";
        cout << "group <field or expression,...> -- Provide a field name/variables/expressions to be grouped.\n";
        cout << "sort <field or expression [asc|desc],...> -- Provide a field name/variables/expressions to be sorted.\n";
        cout << "limt <n | bottomN,topN> -- Provide output limit range.\n";
        cout << "filemode <buffer|line> -- Provide file read mode, default is buffer.\n";
        cout << "skip <N> -- How many bytes or lines (depends on the filemode) to be skipped.\n";
        cout << "run [query string] -- Run the query (either preprocessed or provide as a parameter).\n";
        cout << "rquery >";
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("load ")==0){
        string strParam = boost::algorithm::trim_copy<string>(lineInput).substr(string("load ").size());
        readMode = checkReadMode(strParam);
        if (readMode != FILE && readMode != FOLDER){
          cout << "Error: Cannot find the file or folder.\n";
        }else{
          sContent = strParam;
          cout << (readMode==FILE?"File":"Folder");
          cout << " is loaded.\n";
        }
        cout << "rquery >";
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("filemode ")==0){
        string strParam = boost::algorithm::trim_copy<string>(lineInput).substr(string("filemode ").size());
        if (boost::algorithm::to_lower_copy<string>(strParam).compare("line")!=0)
          fileMode=READBUFF;
        cout << "File read mode is set to ";
        cout << (fileMode==READBUFF?"buffer.\n":"line.\n");
        cout << "rquery >";
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("skip ")==0){
        string strParam = boost::algorithm::trim_copy<string>(lineInput).substr(string("skip ").size());
        if (!isInt(strParam)){
          cout << "Error: Please provide a valid number.\n";
        }else{
          iSkip = atoi(strParam.c_str());
          cout << strParam.c_str();
          cout << (fileMode==READBUFF?"bytes":"lines");
          cout << " will be skipped.\n";
        }
        cout << "rquery >";
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("parse ")==0){
        string strParam = boost::algorithm::trim_copy<string>(lineInput).substr(string("parse ").size());
        rq.setregexp(trim_one(strParam, '/'));
        cout << "Regular expression string is provided.\n";
        cout << "rquery >";
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("set ")==0){
        string strParam = boost::algorithm::trim_copy<string>(lineInput).substr(string("set ").size());
        rq.setFieldTypeFromStr(strParam);
        cout << "Fileds data type has been set up.\n";
        cout << "rquery >";
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("filter ")==0){
        string strParam = boost::algorithm::trim_copy<string>(lineInput).substr(string("filter ").size());
        //if (filter) // assignFilter will clear the existing filter
        //  delete filter;
        cout << "Filter condition is provided.\n";
        filter = new FilterC(strParam);
        rq.assignFilter(filter);
        cout << "rquery >";
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("group ")==0){
        string strParam = boost::algorithm::trim_copy<string>(lineInput).substr(string("group ").size());
        rq.assignGroupStr(strParam);
        cout << "Group expressions are provided.\n";
        cout << "rquery >";
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("select ")==0){
        string strParam = boost::algorithm::trim_copy<string>(lineInput).substr(string("select ").size());
        rq.assignSelString(strParam);
        cout << "Selection is provided.\n";
        cout << "rquery >";
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("sort ")==0){
        string strParam = boost::algorithm::trim_copy<string>(lineInput).substr(string("sort ").size());
        rq.assignSortStr(strParam);
        cout << "Sorting keys are provided.\n";
        cout << "rquery >";
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("limit ")==0){
        string strParam = boost::algorithm::trim_copy<string>(lineInput).substr(string("limit ").size());
        rq.assignLimitStr(strParam);
        cout << "Output limit has been set up.\n";
        cout << "rquery >";
      }else if (boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).compare("run")==0 || boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(lineInput)).find("run ")==0){
        if (boost::algorithm::trim_copy<string>(lineInput).length() == 3){
          if (readMode == FILE || readMode == FOLDER)
            runQuery(sContent, readMode, rq, fileMode, iSkip);
        }else{
          string strParam = boost::algorithm::trim_copy<string>(lineInput).substr(string("run ").size());
          processQuery(strParam, rq);
          if (readMode == FILE || readMode == FOLDER)
            runQuery(sContent, readMode, rq, fileMode, iSkip);
        }
        cout << "rquery >";
      }else{
        cout << "Error: unrecognized command. Input (H)elp or (H)elp command to get more details.\n";
        cout << "rquery >";
      }
    }
  }else{
    if (!sQuery.empty())
      processQuery(sQuery, rq);
    runQuery(sContent, readMode, rq, fileMode, iSkip);
  }
}
