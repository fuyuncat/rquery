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
  printf("\nProgram Name: RQuery AKA RQ Version %s\n", VERSION);
  printf("Contact Email: fuyuncat@gmail.com\n");
  printf("Search string block/file/folder using regular/delimiter/wildcard/ expression parttern, and filter/group calculate/sort the matched result. One command can do what grep/xgrep/sort/uniq/awk/wc/sed/cut/tr can do and more. \n\n");
  printf("Usage: rquery [OPTION]... [FILE|FOLDER|VARIABLE]...  -q[uery] \"p[arse] [w|d]/<regular expression>/[<quoters>/][r] | m[eanwhile] <expr> .. [where filter] | s[elect] <expr>... | [se]t field datatype,... | f[ilter] <filters> | group <expr>,... | [s]o[rt] <expr>,... | l[imit] n[,topN] | u[nique] \" \"file, folder or string to be queried\"\n\n");
  printf("\t-q|--query <query string> -- The query string indicates how to query the content, valid query commands include parse,select,set,filter,group,sort,limit. One or more commands can be provide in the query, multiple commands are separated by |. They can be in any order. \n");
  printf("\t\tparse|p /<searching expression string>/ -- Choose one of three mode to match the content.\n\t\t\t// quotes a regular expression pattern string to parse the content; \n\t\t\t w/<WildCardExpr>/[quoters/] quotes wildcard expression to parse the content, wildcard '*' stands for a field, e.g. w/*abc*,*/. substrings between two * are the spliters, spliter between quoters will be skipped;\n\t\t\td/<Delmiter>/[quoters/][r] quotes delmiter to parse the content, Delmiter splits fields, delmiter between quoters will be skipped, r at the end of pattern means the delmiter is repeatable, e.g. d/ /\"\"/\n");
  printf("\t\tset|t <field datatype [date format],...> -- Set the date type of the fields.\n");
  printf("\t\tfilter|f <filter conditions> -- Provide filter conditions to filt the content.\n");
  printf("\t\textrafilter|e <extra filter conditions> [ trim <selections ...>] -- Provide filter conditions to filt the resultset. @N refer to Nth selection of the result set. the trim clause specifys the selections after trimmed the result set.\n");
  printf("\t\tmeanwhile|m <actions when searching data> -- Provide actions to be done while doing searching. The result set can be used for two or more files JOIN or IN query.\n");
  printf("\t\tselect|s <field or expression [as alias],...> -- Provide a field name/variables/expressions to be selected. If no filed name captured, @N or @fieldN can be used for field N.\n");
  printf("\t\tgroup|g <field or expression,...> -- Provide a field name/variables/expressions to be grouped.\n");
  printf("\t\tsort|o <field or expression [asc|desc],...> -- Provide a field name/variables/expressions to be sorted.\n");
  printf("\t\tlimt|l <n | bottomN,topN> -- Provide output limit range.\n");
  printf("\t\tunique|u -- Make the returned result unique.\n");
  printf("\t\ttree|h k:expr1[,expr2...];p:expr1[,expr2...] -- Provide keys and parent keys to construct tree stucture. tree cannot work with group/sort/unique. variable @level stands for the level of the node in the tree; @nodeid for an unique sequence id of the node of the tree; @root(expr) for the expression root node; @path(expr[,connector]) for an path (of the expression) from root to current node, connector is used for connecting nodes, default is '/'. \n");
  printf("\t\t>|>> -- Set output files, if not set, output to standard terminal (screen). > will overwrite existing files, >> will append to existing file.\n");
  printf("\t-r|--readmode <buffer|line> -- Provide file read mode, default is buffer.\n");
  printf("\t-s|--skip <N> -- How many bytes or lines (depends on the filemode) to be skipped.\n");
  printf("\t-b|--buffsize <N> -- The read buffer size when read mode is buffer.\n");
  printf("\t-m|--msglevel <fatal|error|warning|info> -- Logging messages output level, default is fatal.\n");
  printf("\t-n|--nameline yes/no : Specify the first matched line should be used for filed names (useful for csv files). Default is no.\n");
  printf("\t-l|--logfile <filename> -- Provide log file, if none(default) provided, the logs will be print in screen.\n");
  printf("\t-p|--progress <on|off> -- Wheather show the processing progress or not(default).\n");
  printf("\t-c|--recursive <yes|no> -- Wheather recursively read subfolder of a folder (default NO).\n");
  printf("\t-o|--outputformat <text|json> -- Provide output format, default is text.\n");
  printf("\t-d|--detecttyperows <N> : How many matched rows will be used for detecting data types, default is 1.\n");
  printf("\t-i|--delimiter <string> : Specify the delimiter of the fields, TAB will be adapted if this option is not provided\n");
  printf("\t-v|--variable \"name1:value1[:expression1[:g]][;name2:value2[:expression2[:g]]]..\" -- Pass variable to rquery, variable name can be any valid word except the reserved words, RAW,FILE,ROW,LINE,FILELINE,FILEID. Using @name to refer to the variable. variable can be a dynamic variable if an expression passed, e.g. v1:1:@v1+1, @v1 has an initial value 0, it will be plused one for each matched row. 'g' flag of a dynamic variable indecate it is a global variable when processing multiple files.\n");
  printf("More information can be found at https://github.com/fuyuncat/rquery .\n");
}

short int checkReadMode(string sContent)
{
  short int readMode = PARAMETER;
  if (findFirstCharacter(sContent, {'*','?'}, 0, "\"\"", '\\',{})!=string::npos)
    readMode = WILDCARDFILES;
  else {
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
  }
  
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
  trace(DEBUG,"Processing file: %s \n", filename.c_str());
  long int thisTime,lastTime = curtime();

  rq.setFileName(filename);
  ifstream ifile(filename.c_str());
  if (!ifile){
    trace(ERROR, "Failed to open file '%s'.\n", filename.c_str());
    return;
  }
  // size_t filesize = ifile.tellg();
  size_t filesize = getFileSize(filename);
  // trace(DEBUG2, "File size '%d'.\n", filesize);
  total = 0;

  switch (fileMode){
    case READLINE:{
      string strline;
      int readLines = 0;
      rq.setReadmode(READLINE);
      while (std::getline(ifile, strline)){
        total += (strline.length()+1);
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
          printf("\r'%s': %d lines(%.2f%%) read in %f seconds.", filename.c_str(), readLines, round(((double)total)/((double)filesize)*10000.0)/100.0, (double)(thisTime-lastTime)/1000);
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
      rq.setReadmode(READBUFF);

      memset( cachebuffer, '\0', sizeof(char)*cache_length );
      while(!ifile.eof()) {
        if (rq.searchStopped())
          break;
        ifile.read(cachebuffer, cache_length-1);
        //ifile.seekg(pos, ios::beg);
        if (ifile.eof())
          rq.setEof(true);
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
          printf("\r'%s': %ld bytes(%.2f%%) read in %f seconds.", filename.c_str(), total, round(((double)total)/((double)filesize)*10000.0)/100.0, (double)(thisTime-lastTime)/1000);
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

void processFolder(string foldername, QuerierC & rq, size_t& total, short int fileMode=READBUFF, int iSkip=0)
{
  trace(DEBUG,"Processing folder: %s \n", foldername.c_str());
  vector<string> filelist = listFilesInFolder(foldername);
  size_t this_total = 0, this_filesize = 0;
  for (int i=0; i<filelist.size(); i++){
    string filename = trim_right(foldername,'/')+"/"+filelist[i];
    short int filetype = checkReadMode(filename);
    switch(filetype){
    case SINGLEFILE:
      processFile(filename, rq, this_filesize, fileMode, iSkip);
      break;
    case FOLDER:
      if (gv.g_recursiveread)
        processFolder(filename, rq, this_filesize, fileMode, iSkip);
      break;
    default:
      trace(WARNING,"Unrecognized file type: %s (%d)\n", filename.c_str(), filetype);
    }
    this_total += this_filesize;
    this_filesize = 0;
  }
  total += this_total;
}

void processWildfiles(string wildfiles, QuerierC & rq, size_t& total, short int fileMode=READBUFF, int iSkip=0)
{
  trace(DEBUG,"Processing folder: %s \n", wildfiles.c_str());
  size_t slashPos = findNthCharacter(wildfiles, {'/'}, 0, 1, false, "\"\"", '\\',{}) ; // find the last / to get the folder 
  string foldername = slashPos!=string::npos?wildfiles.substr(0,slashPos+1):".";
  vector<string> filelist = listFilesInFolder(foldername);
  size_t this_total = 0, this_filesize = 0;
  for (int i=0; i<filelist.size(); i++){
    string filename = trim_right(foldername,'/')+"/"+filelist[i];
    if (like(filename,wildfiles)){
      short int filetype = checkReadMode(filename);
      switch(filetype){
      case SINGLEFILE:
        processFile(filename, rq, this_filesize, fileMode, iSkip);
        break;
      case FOLDER:
        if (gv.g_recursiveread)
          processFolder(filename, rq, this_filesize, fileMode, iSkip);
        break;
      default:
        trace(WARNING,"Unrecognized file type: %s (%d)\n", filename.c_str(), filetype);
      }
      this_total += this_filesize;
      this_filesize = 0;
    }
  }
  total += this_total;
}

void printResult(QuerierC & rq, size_t total)
{
  long int thisTime,lastTime = curtime();
  if (rq.toGroupOrSort()){
    rq.group();
    rq.analytic();
    thisTime = curtime();
    trace(DEBUG2, "Grouping: %u\n", thisTime-lastTime);
    lastTime = thisTime;
    rq.unique();// unique must be done before sort
    thisTime = curtime();
    trace(DEBUG2, "Unique: %u\n", thisTime-lastTime);
    lastTime = thisTime;
    rq.sort();
    thisTime = curtime();
    trace(DEBUG2, "Sorting: %u\n", thisTime-lastTime);
    rq.tree();
    thisTime = curtime();
    trace(DEBUG2, "Treeing: %u\n", thisTime-lastTime);
    rq.setOutputFormat(gv.g_ouputformat);
    rq.outputAndClean();
    thisTime = curtime();
    trace(DEBUG2, "Printing: %u\n", thisTime-lastTime);
    lastTime = thisTime;
  }
  rq.outputExtraInfo(total, gv.g_printheader);
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

  //string patternStr = "/[^\n]*/"; // if no PARSE passed, search each lines
  //string patternStr = "w/*/"; // if no PARSE passed, search each lines
  // string patternStr = ""; // an empty pattern mean each line matches.
  string patternStr = "d/ /\"\"/r"; // a repeatable space as delimiter.
  if (query.find("parse") != query.end())
    patternStr = query["parse"];
  else if (query.find("p") != query.end())
    patternStr = query["p"];

  //string rex = trim_one(patternStr, '/');
  //trace(DEBUG2,"Searching pattern: %s \n", rex.c_str());
  rq.setregexp(patternStr);

  if (query.find("filter") != query.end()){
    //trace(DEBUG,"Assigning filter: %s \n", query["filter"].c_str());
    FilterC* filter = new FilterC(query["filter"]);
    rq.assignFilter(filter);
  }else if (query.find("f") != query.end()){
    //trace(DEBUG,"Assigning filter: %s \n", query["filter"].c_str());
    FilterC* filter = new FilterC(query["f"]);
    rq.assignFilter(filter);
  }
  if (query.find("extrafilter") != query.end()){
    //trace(DEBUG,"Assigning filter: %s \n", query["filter"].c_str());
    rq.assignExtraFilter(query["extrafilter"]);
  }else if (query.find("e") != query.end()){
    //trace(DEBUG,"Assigning filter: %s \n", query["filter"].c_str());
    rq.assignExtraFilter(query["e"]);
  }
  if (query.find("set") != query.end()){
    //trace(DEBUG,"Setting fields data type: %s \n", query["set"].c_str());
    rq.setFieldTypeFromStr(query["set"]);
  }else if (query.find("t") != query.end()){
    //trace(DEBUG,"Setting fields data type: %s \n", query["set"].c_str());
    rq.setFieldTypeFromStr(query["t"]);
  }
  // assign GROUP before assigning SELECTION and SORT. expressions in SELECTION and SORT should present in GROUP
  if (query.find("group") != query.end()){
    //trace(DEBUG,"Setting group : %s \n", query["group"].c_str());
    rq.assignGroupStr(query["group"]);
  }else if (query.find("g") != query.end()){
    //trace(DEBUG,"Setting group : %s \n", query["group"].c_str());
    rq.assignGroupStr(query["g"]);
  }
  if (query.find("meanwhile") != query.end()){
    //trace(DEBUG,"Setting group : %s \n", query["group"].c_str());
    rq.assignMeanwhileString(query["meanwhile"]);
  }else if (query.find("m") != query.end()){
    //trace(DEBUG,"Setting group : %s \n", query["group"].c_str());
    rq.assignMeanwhileString(query["m"]);
  }
  if (query.find("select") != query.end()){
    //trace(DEBUG,"Assigning selections: %s \n", query["select"].c_str());
    rq.assignSelString(query["select"]);
  }else if (query.find("s") != query.end()){
    //trace(DEBUG,"Assigning selections: %s \n", query["select"].c_str());
    rq.assignSelString(query["s"]);
  }else
    rq.assignSelString("@raw");
  if (query.find("sort") != query.end()){
    //trace(DEBUG,"Assigning sorting keys: %s \n", query["sort"].c_str());
    rq.assignSortStr(query["sort"]);
  }else if (query.find("o") != query.end()){
    //trace(DEBUG,"Assigning sorting keys: %s \n", query["sort"].c_str());
    rq.assignSortStr(query["o"]);
  }
  if (query.find("unique") != query.end()){
    rq.setUniqueResult(true);
  }else if (query.find("u") != query.end()){
    rq.setUniqueResult(true);
  }
  if (query.find("tree") != query.end()){
    //trace(DEBUG,"Assigning sorting keys: %s \n", query["sort"].c_str());
    rq.assignTreeStr(query["tree"]);
  }else if (query.find("h") != query.end()){
    //trace(DEBUG,"Assigning sorting keys: %s \n", query["sort"].c_str());
    rq.assignTreeStr(query["h"]);
  }
  if (query.find("limit") != query.end()){
    //trace(DEBUG,"Assigning limit numbers: %s \n", query["limit"].c_str());
    rq.assignLimitStr(query["limit"]);
  }else if (query.find("l") != query.end()){
    //trace(DEBUG,"Assigning limit numbers: %s \n", query["limit"].c_str());
    rq.assignLimitStr(query["l"]);
  }
  if (query.find(">>") != query.end()){
    rq.setOutputFiles(query[">>"], APPEND);
  }else if (query.find(">") != query.end()){
    rq.setOutputFiles(query[">"], OVERWRITE);
  }
}

void processPrompt(QuerierC & rq, size_t & total)
{
  //trace(DEBUG1,"Processing content from input or pipe \n");
  long int thisTime,lastTime = curtime();
  const size_t cache_length = gv.g_inputbuffer;
  //char cachebuffer[cache_length];
  rq.setOutputFormat(gv.g_ouputformat);
  char* cachebuffer = (char*)malloc(cache_length*sizeof(char));
  size_t reads = 0;
  rq.setReadmode(READBUFF);
  while(std::cin) {
    if (rq.searchStopped())
      break;
    memset( cachebuffer, '\0', sizeof(char)*cache_length );
    std::cin.read(cachebuffer, cache_length);
    if (!std::cin)
      rq.setEof(true);
    rq.appendrawstr(string(cachebuffer));
    //trace(DEBUG,"(2)Processing: %s \n", cachebuffer);
    rq.searchAll();
    if (gv.g_printheader && gv.g_ouputformat==TEXT)
      rq.printFieldNames();
    if (!rq.toGroupOrSort())
      rq.outputAndClean();
    total += std::cin.gcount();
  }
  free(cachebuffer);
  thisTime = curtime();
  trace(DEBUG2, "Reading and searching: %u\n", thisTime-lastTime);
  printResult(rq, total);
}

void runQuery(vector<string> vContent, QuerierC & rq, short int fileMode=READBUFF, int iSkip=0)
{
  size_t total = 0;
  if (vContent.size() == 0)
    processPrompt(rq, total);
  for (int i=0;i<vContent.size();i++){
    short int readMode = checkReadMode(vContent[i]);
    trace(DEBUG,"(0)Processing(mode:%d): %s \n", readMode, vContent[i].c_str());
    switch (readMode){
      case PARAMETER:{
        //trace(DEBUG1,"Processing content from parameter \n");
        rq.setReadmode(READBUFF);
        rq.setEof(true);
        rq.setrawstr(vContent[i]);
        //rq.searchNext();
        //trace(DEBUG,"(1)Processing: %s \n", vContent[i].c_str());
        rq.searchAll();
        rq.group();
        rq.analytic();
        rq.unique();
        rq.sort();
        rq.tree();
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
        processFile(vContent[i], rq, total, fileMode, iSkip);
        break;
      }
      case FOLDER:{
        //trace(DEBUG1,"Processing content from folder \n");
        rq.setOutputFormat(gv.g_ouputformat);
        processFolder(vContent[i], rq, total, fileMode, iSkip);
        break;
      }
      case WILDCARDFILES:{
        //trace(DEBUG1,"Processing content from folder \n");
        rq.setOutputFormat(gv.g_ouputformat);
        processWildfiles(vContent[i], rq, total, fileMode, iSkip);
        break;
      }
      case PROMPT:{
        processPrompt(rq, total);
        break;
      }
      default:{
        usage();
        trace(FATAL,"Please provide content to be queried!\n");
        return;
      }
    }
  }
  printResult(rq, total);
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
  
  //string time("1988/08/18 08:18:58"), sFm("%Y/%m/%d %H:%M:%S");
  //convertzone(time,sFm,"AEST","BST");
  
  gv.setVars(16384, FATAL, false);
  gv.g_consolemode = false;
  short int fileMode = READBUFF;
  int iSkip = 0;
  string sQuery = "";
  vector<string> vContent;
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
      gv.g_consolemode = true;
    }else if (lower_copy(string(argv[i])).compare("-l")==0 || lower_copy(string(argv[i])).compare("--logfile")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      ofstream* logfile = new ofstream(string(argv[i+1]));
      if (!(*logfile))
        SafeDelete(logfile);
      else{
        if (gv.g_logfile){
          if ((*gv.g_logfile))
            gv.g_logfile->close();
          SafeDelete(gv.g_logfile);
        }
        gv.g_logfile = logfile;
      }
      i++;
    }else if (lower_copy(string(argv[i])).compare("-f")==0 || lower_copy(string(argv[i])).compare("--fieldheader")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
        //trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        //exitProgram(1);
        gv.g_printheader = true;
      }else{
        gv.g_printheader = (lower_copy(string(argv[i+1])).compare("off")!=0);
        i++;
      }
    }else if (lower_copy(string(argv[i])).compare("-p")==0 || lower_copy(string(argv[i])).compare("--progress")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
        //trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        //exitProgram(1);
        gv.g_showprogress = true;
      }else{
        gv.g_showprogress = (lower_copy(string(argv[i+1])).compare("on")==0);
        i++;
      }
    }else if (lower_copy(string(argv[i])).compare("-c")==0 || lower_copy(string(argv[i])).compare("--recursive")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
        //trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        //exitProgram(1);
        gv.g_recursiveread = true;
      }else{
        gv.g_recursiveread = (lower_copy(string(argv[i+1])).compare("yes")==0||lower_copy(string(argv[i+1])).compare("y")==0);
        i++;
      }
    }else if (lower_copy(string(argv[i])).compare("-r")==0 || lower_copy(string(argv[i])).compare("--readmode")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      fileMode = (lower_copy(string(argv[i+1])).compare("line")!=0?READBUFF:READLINE);
      i++;
    }else if (lower_copy(string(argv[i])).compare("-o")==0 || lower_copy(string(argv[i])).compare("--outputformat")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      gv.g_ouputformat = (lower_copy(string(argv[i+1])).compare("json")!=0?TEXT:JSON);
      i++;
    }else if (lower_copy(string(argv[i])).compare("-i")==0 || lower_copy(string(argv[i])).compare("--delimiter")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      rq.setFieldDelim(string(argv[i+1]));
      i++;
    }else if (lower_copy(string(argv[i])).compare("-v")==0 || lower_copy(string(argv[i])).compare("--variable")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      rq.setUserVars(trim_copy(string(argv[i+1])));
      i++;
    }else if (lower_copy(string(argv[i])).compare("-s")==0 || lower_copy(string(argv[i])).compare("--skip")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
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
      if (i>=argc-1 || argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      if (!isInt(argv[i+1])){
        trace(FATAL,"%s is not a correct buffer size number.\n", argv[i]);
        exitProgram(1);
      }
      gv.g_inputbuffer = atoi((argv[i+1]));
      i++;
    }else if (lower_copy(string(argv[i])).compare("-d")==0 || lower_copy(string(argv[i])).compare("--detecttyperows")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      if (!isInt(argv[i+1])){
        trace(FATAL,"%s is not a correct row number.\n", argv[i]);
        exitProgram(1);
      }
      rq.setDetectTypeMaxRowNum(atoi((argv[i+1])));
      i++;
    }else if (lower_copy(string(argv[i])).compare("-n")==0 || lower_copy(string(argv[i])).compare("--nameline")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
        //trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        //exitProgram(1);
        rq.setNameline(true);
      }else{
        if (lower_copy(string(argv[i+1])).compare("yes")==0||lower_copy(string(argv[i+1])).compare("y")==0)
          rq.setNameline(true);
        i++;
      }
    }else if (lower_copy(string(argv[i])).compare("-m")==0 || lower_copy(string(argv[i])).compare("--msglevel")==0){
      if (i>=argc-1 || argv[i+1][0] == '-'){
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
      if (i>=argc-1 || argv[i+1][0] == '-'){
        trace(FATAL,"You need to provide a value for the parameter %s.\n", argv[i]);
        exitProgram(1);
      }
      sQuery = string(argv[i+1]);
      //trace(DEBUG2,"Query string: %s.\n", sQuery.c_str());
      i++;
    }else{
      trace(DEBUG1,"Content: %s.\n", argv[i]);
      vContent.push_back(string(argv[i]));
    }
  }

  if (gv.g_consolemode){
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
        cout << "parse /<regular expression string>/ -- Choose one of three mode to match the content. \n\t// quotes a regular expression pattern string to parse the content; \n \tw/<WildCardExpr>/ quotes wildcard expression to parse the content, wildcard '*' stands for a field, e.g. w/*abc*,*/. substrings between two * are the spliters, spliter between quoters will be skipped\n\td/<Delmiter>/[quoters/][r] quotes delmiter to parse the content, Delmiter splits fields, delmiter between quoters will be skipped, r at the end of pattern means the delmiter is repeatable, e.g. d/ /\"\"/\n";
        cout << "set <field datatype [date format],...> -- Set the date type of the fields.\n";
        cout << "detecttyperows <N> : Set how many matched rows will be used for detecting data types, default is 1.\n";
        cout << "filter <filter conditions> -- Provide filter conditions to filt the content.\n";
        cout << "extrafilter <extra filter conditions> [trim <selections ...>] -- Provide filter conditions to filt the resultset. @N refer to Nth selection of the result set. the trim clause specifys the selections after trimmed the result set.\n";
        cout << "meanwhile <actions when searching data> -- Provide actions to be done while doing searching. The result set can be used for two or more files JOIN or IN query.\n";
        cout << "select <field or expression [as alias],...> -- Provide a field name/variables/expressions to be selected. If no filed name captured, @N or @fieldN can be used for field N.\n";
        cout << "group <field or expression,...> -- Provide a field name/variables/expressions to be grouped.\n";
        cout << "sort <field or expression [asc|desc],...> -- Provide a field name/variables/expressions to be sorted.\n";
        cout << "limt <n | bottomN,topN> -- Provide output limit range.\n";
        cout << "unique -- Make the returned resutl unique.\n";
        cout << "tree k:expr1[,expr2...];p:expr1[,expr2...] -- Provide keys and parent keys to construct tree stucture. tree cannot work with group/sort/unique. variable @level stands for the level of the node in the tree; @nodeid for an unique sequence id of the node of the tree; @root(expr) for the expression root node; @path(expr[,connector]) for an path (of the expression) from root to current node, connector is used for connecting nodes, default is '/'.\n";
        cout << "output|append -- Set output files, if not set, output to standard terminal (screen). 'output' will overwrite existing files, 'append' will append to existing file.\n";
        cout << "clear -- Clear all query inputs.\n";
        cout << "filemode <buffer|line> -- Provide file read mode, default is buffer.\n";
        cout << "skip <N> -- How many bytes or lines (depends on the filemode) to be skipped.\n";
        cout << "buffsize <N> -- The read buffer size when read mode is buffer.\n";
        cout << "run [query string] -- Run the query (either preprocessed or provide as a parameter).\n";
        cout << "msglevel <fatal|error|warning|info> -- Logging messages output level, default is fatal.\n";
        cout << "nameline N -- Specify which matched line should be used for filed names (useful for csv files). Default is 0, means None.\n";
        cout << "logfile <filename> -- Provide log file, if none(default) provided, the logs will be print in screen.\n";
        cout << "progress <on|off> -- Wheather show the processing progress or not(default).\n";
        cout << "recursive <yes|no> -- Wheather recursively read subfolder of a folder (default NO).\n";
        cout << "format <text|json> -- Provide output format, default is text.\n";
        cout << "delimiter <string> -- Specify the delimiter of the fields, TAB will be adapted if this option is not provided.\n";
        cout << "var \"name1:value1[:expression1[:g]][;name2:value2[:expression2[:g]]]..\" -- Pass variable to rquery, variable name can be any valid word except the reserved words, RAW,FILE,ROW,LINE. Using @name to refer to the variable. variable can be a dynamic variable if an expression passed, e.g. v1:1:@v1+1, @v1 has an initial value 0, it will be plused one for each matched row. 'g' flag of a dynamic variable indecate it is a global variable when processing multiple files.\n\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("load ")==0){
        string strParam = trim_copy(lineInput).substr(string("load ").length());
        short int readMode = checkReadMode(strParam);
        if (readMode != SINGLEFILE && readMode != FOLDER && readMode != WILDCARDFILES){
          cout << "Error: Cannot find the file or folder.\n";
        }else{
          vContent.push_back(strParam);
          cout << (readMode==SINGLEFILE?"File":"Folder");
          cout << " is loaded.\n";
        }
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("msglevel ")==0){
        string strParam = trim_copy(lineInput).substr(string("msglevel ").length());
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
        string strParam = trim_copy(lineInput).substr(string("logfile ").length());
        ofstream* logfile = new ofstream(strParam);
        if (!(*logfile))
          SafeDelete(logfile);
        else{
          if (gv.g_logfile){
            if ((*gv.g_logfile))
              gv.g_logfile->close();
            SafeDelete(gv.g_logfile);
          }
          gv.g_logfile = logfile;
        }
        cout << "Logfile has been set to ";
        cout << strParam.c_str();
        cout << "\nrquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("progress ")==0){
        string strParam = trim_copy(lineInput).substr(string("progress ").length());
        gv.g_showprogress = (lower_copy(strParam).compare("on")==0);
        cout << "Process progress has been turned ";
        cout << strParam.c_str();
        cout << "\nrquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("format ")==0){
        string strParam = trim_copy(lineInput).substr(string("format ").length());
        gv.g_ouputformat = (strParam.compare("json")!=0?TEXT:JSON);
        cout << "Output format has been set to ";
        cout << strParam.c_str();
        cout << "\nrquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("filemode ")==0){
        string strParam = trim_copy(lineInput).substr(string("filemode ").length());
        if (lower_copy(strParam).compare("line")!=0)
          fileMode=READBUFF;
        cout << "File read mode is set to ";
        cout << (fileMode==READBUFF?"buffer.\n":"line.\n");
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("skip ")==0){
        string strParam = trim_copy(lineInput).substr(string("skip ").length());
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
        string strParam = trim_copy(lineInput).substr(string("buffsize ").length());
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
        string strParam = trim_copy(lineInput).substr(string("parse ").length());
        rq.setregexp(strParam);
        cout << "Regular expression string is provided.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("set ")==0){
        string strParam = trim_copy(lineInput).substr(string("set ").length());
        rq.setFieldTypeFromStr(strParam);
        cout << "Fileds data type has been set up.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("var ")==0){
        string strParam = trim_one(trim_copy(lineInput).substr(string("set ").length()),'"');
        rq.setUserVars(strParam);
        cout << "Variables have been set up.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("filter ")==0){
        string strParam = trim_copy(lineInput).substr(string("filter ").length());
        //if (filter) // assignFilter will clear the existing filter
        //  SafeDelete(filter);
        cout << "Filter condition is provided.\n";
        filter = new FilterC(strParam);
        rq.assignFilter(filter);
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("extrafilter ")==0){
        string strParam = trim_copy(lineInput).substr(string("extrafilter ").length());
        //if (filter) // assignFilter will clear the existing filter
        //  SafeDelete(filter);
        cout << "Extra filter condition is provided.\n";
        rq.assignExtraFilter(strParam);
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("meanwhile ")==0){
        string strParam = trim_copy(lineInput).substr(string("meanwhile ").length());
        rq.assignMeanwhileString(strParam);
        cout << "Meanwhile actions are provided.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("group ")==0){
        string strParam = trim_copy(lineInput).substr(string("group ").length());
        rq.assignGroupStr(strParam);
        cout << "Group expressions are provided.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("select ")==0){
        string strParam = trim_copy(lineInput).substr(string("select ").length());
        rq.assignSelString(strParam);
        cout << "Selection is provided.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("sort ")==0){
        string strParam = trim_copy(lineInput).substr(string("sort ").length());
        rq.assignSortStr(strParam);
        cout << "Sorting keys are provided.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("delimiter ")==0){
        string strParam = trim_left(lineInput).substr(string("delimiter ").length());
        rq.setFieldDelim(strParam);
        cout << "Fields delimiter is set.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).compare("unique")==0){
        rq.setUniqueResult(true);
        cout << "Set returned result unique.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("tree ")==0){
        string strParam = trim_copy(lineInput).substr(string("tree ").length());
        rq.assignTreeStr(strParam);
        cout << "Tree keys are provided.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).compare("output")==0){
        string strParam = trim_copy(lineInput).substr(string("output ").length());
        rq.setOutputFiles(strParam, OVERWRITE);
        cout << "Set output files.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).compare("append")==0){
        string strParam = trim_copy(lineInput).substr(string("append ").length());
        rq.setOutputFiles(strParam, APPEND);
        cout << "Set append files.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).compare("clear")==0){
        rq.clear();
        cout << "All query inputs have been cleared.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).compare("recursive ")==0){
        string strParam = trim_copy(lineInput).substr(string("recursive ").length());
        gv.g_recursiveread = (lower_copy(strParam).compare("yes")==0||(lower_copy(strParam).compare("y")==0));
        cout << "Set recursively read folder.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("limit ")==0){
        string strParam = trim_copy(lineInput).substr(string("limit ").length());
        rq.assignLimitStr(strParam);
        cout << "Output limit has been set up.\n";
        cout << "rquery >";
      }else if (lower_copy(trim_copy(lineInput)).find("detecttyperows ")==0){
        string strParam = trim_copy(lineInput).substr(string("detecttyperows ").length());
        if (!isInt(strParam)){
          cout << "Error: Please provide a valid number.\n";
        }else{
          rq.setDetectTypeMaxRowNum(atoi(strParam.c_str()));
          cout << "Row number of detecting data type has been set up.\n";
          cout << "rquery >";
        }
      }else if (lower_copy(trim_copy(lineInput)).find("nameline ")==0){
        string strParam = trim_copy(lineInput).substr(string("nameline ").length());
        if (lower_copy(strParam).compare("yes")==0||lower_copy(strParam).compare("y")==0){
          rq.setNameline(true);
          cout << "Row number of detecting data type has been set up.\n";
          cout << "rquery >";
        }
      }else if (lower_copy(trim_copy(lineInput)).compare("run")==0 || lower_copy(trim_copy(lineInput)).find("run ")==0){
        if (trim_copy(lineInput).length() == 3){
          runQuery(vContent, rq, fileMode, iSkip);
        }else{
          string strParam = trim_copy(lineInput).substr(string("run ").length());
          processQuery(strParam, rq);
          runQuery(vContent, rq, fileMode, iSkip);
        }
        cout << "rquery >";
      }else{
        cout << "Error: unrecognized command. Input (H)elp or (H)elp command to get more details.\n";
        cout << "rquery >";
      }
    }
  }else{
    //if (!sQuery.empty())
    processQuery(sQuery, rq);
    runQuery(vContent, rq, fileMode, iSkip);
  }
  exitProgram(0);
}
