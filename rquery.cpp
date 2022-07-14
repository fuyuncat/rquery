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
#include <iostream>
#include <sys/stat.h>
#include "commfuncs.h"
//#include "regexc.h"
#include "parser.h"
#include "querierc.h"
//#include <boost/regex.hpp>

#if defined _WIN32 || defined _WIN64
#include <windows.h>
#include <tchar.h>
//#elif __unix || __unix__ || __linux__
#else
#include <dirent.h>
#endif

//size_t g_inputbuffer;

GlobalVars gv;

void usage()
{
  printf("\nProgram Name: RQuery AKA RQ\n");
  printf("Contact Email: fuyuncat@gmail.com\n");
  printf("Usage: rquery [OPTION]... [FILE|FOLDER|VARIABLE]...  -q \"parse /<regular expression>/ | select | set | filter <filters> | group | sort | limit \" \"file or string to be queried\"\nquery string/file using regular expression\n\n");
  printf("\t-q|--query <query string> -- The query string indicates how to query the content, valid query commands include parse,select,set,filter,group,sort,limit. One or more commands can be provide in the query, multiple commands are separated by |. They can be in any order. \n");
  printf("\t\tparse /<regular expression string>/ -- Provide a regular expression pattern string quoted by \"//\" to parse the content.\n");
  printf("\t\tset <field datatype [date format],...> -- Set the date type of the fields.\n");
  printf("\t\tfilter <filter conditions> -- Provide filter conditions to filt the content.\n");
  printf("\t\tselect <field or expression,...> -- Provide a field name/variables/expressions to be selected.\n");
  printf("\t\tgroup <field or expression,...> -- Provide a field name/variables/expressions to be grouped.\n");
  printf("\t\tsort <field or expression [asc|desc],...> -- Provide a field name/variables/expressions to be sorted.\n");
  printf("\t\tlimt <n | bottomN,topN> -- Provide output limit range.\n");
  printf("\t-r|--readmode <buffer|line> -- Provide file read mode, default is buffer.\n");
  printf("\t-s|--skip <N> -- How many bytes or lines (depends on the filemode) to be skipped.\n");
  printf("\t-b|--buffsize <N> -- The read buffer size when read mode is buffer.\n");
  printf("\t-m|--msglevel <fatal|error|warning|info> -- Logging messages output level, default is fatal.\n");
  printf("\t-l|--logfile <filename> -- Provide log file, if none(default) provided, the logs will be print in screen.\n");
  printf("\t-p|--progress <on|off> -- Wheather show the processing progress or not(default).\n");
  printf("\t-o|--outputformat <text|json> -- Provide output format, default is text.\n");
  printf("More information can be found at https://github.com/fuyuncat/rquery .\n");
}

short int checkReadMode(string sContent)
{
  short int readMode = PARAMETER;

  struct stat s;
  if( stat(sContent.c_str(),&s) == 0 ){
    if( s.st_mode & S_IFDIR )
      readMode = FOLDER;
    else if( s.st_mode & S_IFREG )
      readMode = SINGLEFILE;
    else
      readMode = PARAMETER;
  }else
    readMode = PARAMETER;
  
  return readMode;
}

size_t getFileSize(string filepath)
{
  struct stat s;
  if( stat(filepath.c_str(),&s) == 0 && s.st_mode & S_IFREG )
    return s.st_size;
  
  return 0;
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
//#elif __unix || __unix__ || __linux__
#else
  struct dirent *pDirent;
  DIR *pDir = opendir(foldername.c_str());
  while ((pDirent = readdir(pDir)) != NULL)
    if (strcmp(pDirent->d_name,".")!=0 && strcmp(pDirent->d_name,"..")!=0)
      filelist.push_back(string(pDirent->d_name));
  (void)closedir(pDir);
#endif 
  return filelist;
}

void processFile(string filename, QuerierC & rq, size_t& total, short int fileMode=READBUFF, int iSkip=0)
{
  long int thisTime,lastTime = curtime();

  rq.setFileName(filename);
  ifstream ifile(filename.c_str());
  if (!ifile){
    trace(ERROR, "Failed to open file '%s'.\n", filename.c_str());
    return;
  }
  // size_t filesize = ifile.tellg();
  size_t filesize = getFileSize(filename);
  trace(DEBUG2, "File size '%d'.\n", filesize);
  total = 0;

  switch (fileMode){
    case READLINE:{
      string strline;
      int readLines = 0;
      while (std::getline(ifile, strline)){
        total += (strline.size()+1);
        if (readLines<iSkip){
          readLines++;
          continue;
        }
        if (rq.searchStopped())
          break;
        rq.appendrawstr(strline);
        rq.searchAll();
        if (gv.g_printheader && gv.g_ouputformat==TEXT)
          rq.printFieldNames();
        if (!rq.toGroupOrSort())
          rq.outputAndClean();
        rq.setrawstr("");
        readLines++;
        thisTime = curtime();
        if (gv.g_showprogress)
          printf("\r%d lines(%.2f%%) read in %f seconds.", readLines, round(((double)total)/((double)filesize)*10000.0)/100.0, (double)(thisTime-lastTime)/1000);
      }
      if (gv.g_showprogress)
        printf("\n");
      total = readLines;
      thisTime = curtime();
      trace(DEBUG2, "Reading and searching time: %u\n", thisTime-lastTime);
      lastTime = thisTime;
      break;
    }case READBUFF:
    default:{
      ifile.seekg(iSkip, ios::beg);
      total = iSkip;

      const size_t cache_length = gv.g_inputbuffer;
      //char cachebuffer[cache_length];
      char* cachebuffer = (char*)malloc(cache_length*sizeof(char));

      memset( cachebuffer, '\0', sizeof(char)*cache_length );
      while(!ifile.eof()) {
        if (rq.searchStopped())
          break;
        ifile.read(cachebuffer, cache_length);
        //ifile.seekg(pos, ios::beg);
        rq.appendrawstr(string(cachebuffer));
        rq.searchAll();
        if (gv.g_printheader && gv.g_ouputformat==TEXT)
          rq.printFieldNames();
        if (!rq.toGroupOrSort())
          rq.outputAndClean();
        total += ifile.gcount();
        memset( cachebuffer, '\0', sizeof(char)*cache_length );
        thisTime = curtime();
        if (gv.g_showprogress)
          printf("\r%d bytes(%.2f%%) read in %f seconds.", total, round(((double)total)/((double)filesize)*10000.0)/100.0, (double)(thisTime-lastTime)/1000);
      }
      if (gv.g_showprogress)
        printf("\n");
      free(cachebuffer);
      thisTime = curtime();
      trace(DEBUG2, "Reading and searching time: %u\n", thisTime-lastTime);
      lastTime = thisTime;
      break;
    }
  }
}

void printResult(QuerierC & rq, size_t total, short int fileMode)
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
    rq.setOutputFormat(gv.g_ouputformat);
    rq.outputAndClean();
    thisTime = curtime();
    trace(DEBUG2, "Printing: %u\n", thisTime-lastTime);
    lastTime = thisTime;
  }
  rq.outputExtraInfo(total, fileMode, gv.g_printheader);
  rq.clear();
}

void processQuery(string sQuery, QuerierC & rq)
{
  ParserC ps;
  map<string,string> query = ps.parseparam(sQuery);
  //dumpMap(query);

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
      rq.setOutputFormat(gv.g_ouputformat);
      if (gv.g_printheader && gv.g_ouputformat==TEXT)
        rq.printFieldNames();
      rq.outputAndClean();
      rq.clear();
      break;
    }
    case SINGLEFILE:{
      //trace(DEBUG1,"Processing content from file \n");
      rq.setOutputFormat(gv.g_ouputformat);
      size_t total = 0;
      processFile(sContent, rq, total, fileMode, iSkip);
      printResult(rq, total, fileMode);
      break;
    }
    case FOLDER:{
      //trace(DEBUG1,"Processing content from folder \n");
      rq.setOutputFormat(gv.g_ouputformat);
      vector<string> filelist = listFilesInFolder(sContent);
      size_t total = 0, singlesize = 0;
      for (int i=0; i<filelist.size(); i++){
        trace(DEBUG1,"Processing file: %s \n", (sContent+"/"+filelist[i]).c_str());
        processFile(sContent+"/"+filelist[i], rq, singlesize, fileMode, iSkip);
        total += singlesize;
        singlesize = 0;
      }
      printResult(rq, total, fileMode);
      break;
    }
    case PROMPT:{
      //trace(DEBUG1,"Processing content from input or pipe \n");
      long int thisTime,lastTime = curtime();
      const size_t cache_length = gv.g_inputbuffer;
      //char cachebuffer[cache_length];
      rq.setOutputFormat(gv.g_ouputformat);
      char* cachebuffer = (char*)malloc(cache_length*sizeof(char));
      size_t howmany = 0, reads = 0;
      while(std::cin) {
        if (rq.searchStopped())
          break;
        memset( cachebuffer, '\0', sizeof(char)*cache_length );
        std::cin.read(cachebuffer, cache_length);
        rq.appendrawstr(string(cachebuffer));
        rq.searchAll();
        if (gv.g_printheader && gv.g_ouputformat==TEXT)
          rq.printFieldNames();
        if (!rq.toGroupOrSort())
          rq.outputAndClean();
        howmany += std::cin.gcount();
      }
      free(cachebuffer);
      thisTime = curtime();
      trace(DEBUG2, "Reading and searching: %u\n", thisTime-lastTime);
      printResult(rq, howmany, READBUFF);
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
    exitProgram(1);
  }
  
  gv.setVars(16384*2, FATAL, true);
  bool bConsoleMode = false;
  short int readMode = PROMPT, fileMode = READBUFF;
  int iSkip = 0;
  string sQuery = "", sContent = "";
  QuerierC rq;

  for (int i=1; i<argc; i++){
    if (argv[i][0]=='-' && i>=argc && lower_copy(string(argv[i])).compare("-h")!=0 && lower_copy(string(argv[i])).compare("--help")!=0 && lower_copy(string(argv[i])).compare("--console")!=0){
      usage();
      trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
      exitProgram(1);
    }
    if (lower_copy(string(argv[i])).compare("-h")==0 || lower_copy(string(argv[i])).compare("--help")==0){
      if (i==argc-1){
        usage();
        exitProgram(0);
      }
      string topic = string(argv[i+1]);
      exitProgram(0);
      i++;
    }else if (lower_copy(string(argv[i])).compare("--console")==0){
      bConsoleMode = true;
    }else if (lower_copy(string(argv[i])).compare("-l")==0 || lower_copy(string(argv[i])).compare("--logfile")==0){
      if (argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      ofstream* logfile = new ofstream(string(argv[i+1]));
      if (!(*logfile))
        delete logfile;
      else{
        if (gv.g_logfile){
          if ((*gv.g_logfile))
            gv.g_logfile->close();
          delete gv.g_logfile;
        }
        gv.g_logfile = logfile;
      }
      i++;
    }else if (lower_copy(string(argv[i])).compare("-f")==0 || lower_copy(string(argv[i])).compare("--fieldheader")==0){
      if (argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      gv.g_printheader = (lower_copy(string(argv[i+1])).compare("off")!=0);
      i++;
    }else if (lower_copy(string(argv[i])).compare("-p")==0 || lower_copy(string(argv[i])).compare("--progress")==0){
      if (argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      gv.g_showprogress = (lower_copy(string(argv[i+1])).compare("on")==0);
      i++;
    }else if (lower_copy(string(argv[i])).compare("-r")==0 || lower_copy(string(argv[i])).compare("--readmode")==0){
      if (argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      fileMode = (lower_copy(string(argv[i+1])).compare("line")!=0?READBUFF:READLINE);
      i++;
    }else if (lower_copy(string(argv[i])).compare("-o")==0 || lower_copy(string(argv[i])).compare("--outputformat")==0){
      if (argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      gv.g_ouputformat = (lower_copy(string(argv[i+1])).compare("json")!=0?TEXT:JSON);
      i++;
    }else if (lower_copy(string(argv[i])).compare("-s")==0 || lower_copy(string(argv[i])).compare("--skip")==0){
      if (argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      if (!isInt(argv[i+1])){
        trace(FATAL,"%s is not a correct skip size number.\n", argv[i]);
        exitProgram(1);
      }
      iSkip = atoi((argv[i+1]));
      i++;
    }else if (lower_copy(string(argv[i])).compare("-b")==0 || lower_copy(string(argv[i])).compare("--buffsize")==0){
      if (argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      if (!isInt(argv[i+1])){
        trace(FATAL,"%s is not a correct buffer size number.\n", argv[i]);
        exitProgram(1);
      }
      gv.g_inputbuffer = atoi((argv[i+1]));
      i++;
    }else if (lower_copy(string(argv[i])).compare("-m")==0 || lower_copy(string(argv[i])).compare("--msglevel")==0){
      if (argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      int iLevel=encodeTracelevel(string(argv[i+1]));
      if (iLevel!=UNKNOWN){
        gv.g_tracelevel = iLevel;
      }else{
        trace(FATAL,"Unrecognized message level %s. It should be one of INFO, WARNING, ERROR, FATAL.\n", argv[i]);
        exitProgram(1);
      }
      i++;
    }else if (lower_copy(string(argv[i])).compare("-q")==0 || lower_copy(string(argv[i])).compare("--query")==0){
      if (argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
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
      if (trim_copy(lineInput).empty())
        cout << "rquery >";
      else if (lower_copy(trim_copy(lineInput)).compare("q")==0 || lower_copy(trim_copy(lineInput)).compare("quit")==0){
        break;
      }else if (lower_copy(trim_copy(lineInput)).find("h ")==0 || lower_copy(trim_copy(lineInput)).find("help")==0){
        cout << "load <file/folder> -- Provide a file or folder to be queried.\n";
        cout << "parse /<regular expression string>/ -- Provide a regular expression pattern string quoted by \"//\" to parse the content.\n";
        cout << "set <field datatype [date format],...> -- Set the date type of the fields.\n";
        cout << "filter <filter conditions> -- Provide filter conditions to filt the content.\n";
        cout << "select <field or expression,...> -- Provide a field name/variables/expressions to be selected.\n";
        cout << "group <field or expression,...> -- Provide a field name/variables/expressions to be grouped.\n";
        cout << "sort <field or expression [asc|desc],...> -- Provide a field name/variables/expressions to be sorted.\n";
        cout << "limt <n | bottomN,topN> -- Provide output limit range.\n";
        cout << "filemode <buffer|line> -- Provide file read mode, default is buffer.\n";
        cout << "skip <N> -- How many bytes or lines (depends on the filemode) to be skipped.\n";
        cout << "buffsize <N> -- The read buffer size when read mode is buffer.\n";
        cout << "run [query string] -- Run the query (either preprocessed or provide as a parameter).\n";
        cout << "msglevel <fatal|error|warning|info> -- Logging messages output level, default is fatal.\n";
        cout << "logfile <filename> -- Provide log file, if none(default) provided, the logs will be print in screen.\n";
        cout << "progress <on|off> -- Wheather show the processing progress or not(default).\n";
        cout << "format <text|json> -- Provide output format, default is text.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("load ")==0){
        string strParam = trim_copy(lineInput).substr(string("load ").size());
        readMode = checkReadMode(strParam);
        if (readMode != SINGLEFILE && readMode != FOLDER){
          cout << "Error: Cannot find the file or folder.\n";
        }else{
          sContent = strParam;
          cout << (readMode==SINGLEFILE?"File":"Folder");
          cout << " is loaded.\n";
        }
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("msglevel ")==0){
        string strParam = trim_copy(lineInput).substr(string("msglevel ").size());
        int iLevel=encodeTracelevel(strParam);
        if (iLevel!=UNKNOWN){
          gv.g_tracelevel = iLevel;
        }else{
          trace(FATAL,"Unrecognized message level %s. It should be one of INFO, WARNING, ERROR, FATAL.\n", strParam.c_str());
          exitProgram(1);
        }
        cout << "Message level has been set to ";
        cout << decodeTracelevel(gv.g_tracelevel).c_str();
        cout << "\nrquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("logfile ")==0){
        string strParam = trim_copy(lineInput).substr(string("logfile ").size());
        ofstream* logfile = new ofstream(strParam);
        if (!(*logfile))
          delete logfile;
        else{
          if (gv.g_logfile){
            if ((*gv.g_logfile))
              gv.g_logfile->close();
            delete gv.g_logfile;
          }
          gv.g_logfile = logfile;
        }
        cout << "Logfile has been set to ";
        cout << strParam.c_str();
        cout << "\nrquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("progress ")==0){
        string strParam = trim_copy(lineInput).substr(string("progress ").size());
        gv.g_showprogress = (lower_copy(strParam).compare("on")==0);
        cout << "Process progress has been turned ";
        cout << strParam.c_str();
        cout << "\nrquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("format ")==0){
        string strParam = trim_copy(lineInput).substr(string("format ").size());
        gv.g_ouputformat = (strParam.compare("json")!=0?TEXT:JSON);
        cout << "Output format has been set to ";
        cout << strParam.c_str();
        cout << "\nrquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("filemode ")==0){
        string strParam = trim_copy(lineInput).substr(string("filemode ").size());
        if (lower_copy(strParam).compare("line")!=0)
          fileMode=READBUFF;
        cout << "File read mode is set to ";
        cout << (fileMode==READBUFF?"buffer.\n":"line.\n");
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("skip ")==0){
        string strParam = trim_copy(lineInput).substr(string("skip ").size());
        if (!isInt(strParam)){
          cout << "Error: Please provide a valid number.\n";
        }else{
          iSkip = atoi(strParam.c_str());
          cout << strParam.c_str();
          cout << (fileMode==READBUFF?" bytes":" lines");
          cout << " will be skipped.\n";
        }
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("buffsize ")==0){
        string strParam = trim_copy(lineInput).substr(string("buffsize ").size());
        if (!isInt(strParam)){
          cout << "Error: Please provide a valid number.\n";
        }else{
          gv.g_inputbuffer = atoi(strParam.c_str());
          cout << "Buffer size has been set to ";
          cout << strParam.c_str();
          cout << " bytes.\n";
        }
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("parse ")==0){
        string strParam = trim_copy(lineInput).substr(string("parse ").size());
        rq.setregexp(trim_one(strParam, '/'));
        cout << "Regular expression string is provided.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("set ")==0){
        string strParam = trim_copy(lineInput).substr(string("set ").size());
        rq.setFieldTypeFromStr(strParam);
        cout << "Fileds data type has been set up.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("filter ")==0){
        string strParam = trim_copy(lineInput).substr(string("filter ").size());
        //if (filter) // assignFilter will clear the existing filter
        //  delete filter;
        cout << "Filter condition is provided.\n";
        filter = new FilterC(strParam);
        rq.assignFilter(filter);
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("group ")==0){
        string strParam = trim_copy(lineInput).substr(string("group ").size());
        rq.assignGroupStr(strParam);
        cout << "Group expressions are provided.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("select ")==0){
        string strParam = trim_copy(lineInput).substr(string("select ").size());
        rq.assignSelString(strParam);
        cout << "Selection is provided.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("sort ")==0){
        string strParam = trim_copy(lineInput).substr(string("sort ").size());
        rq.assignSortStr(strParam);
        cout << "Sorting keys are provided.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("limit ")==0){
        string strParam = trim_copy(lineInput).substr(string("limit ").size());
        rq.assignLimitStr(strParam);
        cout << "Output limit has been set up.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).compare("run")==0 || lower_copy(trim_copy(lineInput)).find("run ")==0){
        if (trim_copy(lineInput).length() == 3){
          if (readMode == SINGLEFILE || readMode == FOLDER)
            runQuery(sContent, readMode, rq, fileMode, iSkip);
        }else{
          string strParam = trim_copy(lineInput).substr(string("run ").size());
          processQuery(strParam, rq);
          if (readMode == SINGLEFILE || readMode == FOLDER)
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
  exitProgram(0);
}
