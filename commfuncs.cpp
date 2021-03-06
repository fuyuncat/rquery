/*******************************************************************************
//
//        File: commfuncs.cpp
// Description: common functions
//       Usage: commfuncs.cpp
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/
#include <stdio.h>
//#include <unistd.h>
//#include <termios.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/time.h>
#include <math.h> 
//#include <cstdlib>
//#include <chrono>
//#include <sstream>
//#include <stdexcept.h>
//#include <iostream>
//#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include "commfuncs.h"
#include <stdlib.h>
#if defined __MINGW32__ || defined __CYGWIN32__ || defined __CYGWIN64__ || defined __CYGWIN__
#include <time.h>
#endif

using namespace boost::xpressive;

GlobalVars::GlobalVars()
{
  initVars();
}

GlobalVars::~GlobalVars()
{
  
}

void GlobalVars::initVars()
{
  g_inputbuffer = 16384;
  g_tracelevel = FATAL;
  g_printheader = false;
  g_showprogress = false;
  g_ouputformat = TEXT;
  g_logfile = NULL;
  
  g_consolemode = false;
}

void GlobalVars::setVars(size_t inputbuffer, short tracelevel, bool printheader)
{
  g_inputbuffer = inputbuffer;
  g_tracelevel = tracelevel;
  g_printheader = printheader;
}

short int encodeTracelevel(string str)
{
  string sUpper = upper_copy(str);
  if (sUpper.compare("FATAL") == 0)
    return FATAL;
  else if (sUpper.compare("ERROR") == 0)
    return ERROR;
  else if (sUpper.compare("WARNING") == 0)
    return WARNING;
  else if (sUpper.compare("INFO") == 0)
    return INFO;
  else if (sUpper.compare("DEBUG") == 0)
    return DEBUG;
  else if (sUpper.compare("DEBUG1") == 0)
    return DEBUG1;
  else if (sUpper.compare("DEBUG2") == 0)
    return DEBUG2;
  else
    return UNKNOWN;
}

string decodeTracelevel(int level)
{
  switch (level){
  case FATAL:
    return "FATAL";
  case ERROR:
    return "ERROR";
  case WARNING:
    return "WARNING";
  case INFO:
    return "INFO";
  case DEBUG:
    return "DEBUG";
  case DEBUG1:
    return "DEBUG1";
  case DEBUG2:
    return "DEBUG2";
  case DUMP:
    return "";
  default:
    return "UNKNOWN";
  }
}

void exitProgram(short int code)
{
  if (gv.g_logfile){
    gv.g_logfile->close();
    delete gv.g_logfile;
  }
  exit(code);
}

void trace(short level, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  if (gv.g_tracelevel>=level){
    if (gv.g_logfile){
      char buf[1024];
      int bsize = 0;
      //memset( buf, '\0', sizeof(char)*1024 );
      bsize = sprintf(buf, "%s", string(decodeTracelevel(level)+(level==DUMP?"":":")).c_str());
      gv.g_logfile->write(buf, bsize);
      //memset( buf, '\0', sizeof(char)*1024 );
      bsize = vsprintf(buf, fmt, args);
      gv.g_logfile->write(buf, bsize);
    }else{
      printf("%s",string(decodeTracelevel(level)+(level==DUMP?"":":")).c_str());
      vprintf(fmt, args);
    }
    if (level == FATAL && !gv.g_consolemode)
      exitProgram(EXIT_FAILURE);
  }
  va_end(args);
}

// return most outer quoted string. pos is start pos and return the position of next char of the end of the quoted string.  
string readQuotedStr(string str, int& pos, string quoters, char escape)
{
  string trimmedStr="";
  if (quoters.size()<2)
    return trimmedStr;
  int quoteStart = -1, i = pos, quoteDeep=0;
  bool quoted = false;
  while(i < str.size()) {
    if (quoteDeep > 0){ // checking right quoter only when the string is quoted.
      if (str[i]==escape && i<str.length()-1 && (str[i+1] == quoters[0] || str[i+1] == quoters[1])){ // skip escape
        i++;
        trimmedStr.push_back(str[i]);
      }
      if (quoteDeep>1 || str[i]!=quoters[1])
        trimmedStr.push_back(str[i]);
      if (str[i] == quoters[1]){
        if (i>0 && str[i-1]!=escape){
          quoteDeep--;
          if (quoteDeep == 0){
            pos = i+1;
            //trace(DEBUG2, "Read trimmed string \"%s\" from \"%s\"\n", trimmedStr.c_str(), str.c_str());
            return trimmedStr;
            //return str.substr(quoteStart,pos-quoteStart);
          }
        }
      }
    }
    if (str[i] == quoters[0])
      if (i==0 || (i>0 && str[i-1]!=escape)){
        quoteDeep++;
        if (quoteDeep == 1)
          quoteStart = i;
      }
    i++;
  }
  return "";
}

// find the first position of the any character in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. Nested quoters like "()" can quote other quoters, while any other quoters in unnested quoters like ''{}// should be ignored.
int findFirstCharacter(string str, std::set<char> lookfor, int pos, string quoters,  char escape, std::set<char> nestedQuoters)
{
  //trace(DEBUG, "findFirstCharacter '%s', quoters: '%s' !\n",str.c_str(),quoters.c_str());
  size_t i = 0, j = 0;
  vector<int> q;
  while(i < str.size()) {
    if (lookfor.find(str[i]) != lookfor.end() && i>0 && q.size()==0) 
      return i;
    if (q.size()>0 && str[i] == quoters[q[q.size()-1]]) // checking the latest quoter
      if (i>0 && str[i-1]!=escape){
        q.pop_back();
        //trace(DEBUG, "Pop out quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
        ++i;
        continue;
      }
    if (q.size()==0 || nestedQuoters.find(quoters[q[q.size()-1]])!=nestedQuoters.end()) // if not quoted or the latest quoter is a nested quoter, then search quoters
      for (int k=0; k<(int)(quoters.size()/2); k++){
        if (str[i] == quoters[k*2])
          if (k*2+1<quoters.size() && (i==0 || (i>0 && str[i-1]!=escape))){
            //trace(DEBUG, "Found quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
            q.push_back(k*2+1); // quoted start, need to pair the right quoter
            break;
          }
      }
    ++i;
  }
  if (q.size() > 0)
    trace(ERROR, "(1)Quoters in '%s' are not paired!\n",str.c_str());
  return -1;
}

// split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting. Nested quoters like "()" can quote other quoters, while any other quoters in unnested quoters like ''{}// should be ignored.
vector<string> split(const string & str, char delim, string quoters, char escape, std::set<char> nestedQuoters) 
{
  vector<string> v;
  string element="";
  size_t i = 0, j = 0, begin = 0;
  vector<int> q;
  while(i < str.size()) {
    if (str[i] == delim && i>0 && q.size()==0) {
      trace(DEBUG, "found delim, split string:%s (%d to %d)\n",str.substr(begin, i-begin).c_str(), begin, i);
      //v.push_back(element);
      element = "";
      v.push_back(str.substr(begin, i-begin));
      begin = i+1;
    }else{
      if (str[i]==escape && i<str.length()-1 && (quoters.find(str[i+1]) >= 0 || nestedQuoters.find(str[i+1]) != nestedQuoters.end())) // skip escape
        i++;
      element.push_back(str[i]);
    }
    if (q.size()>0 && str[i] == quoters[q[q.size()-1]]) // checking the latest quoter
      if (i>0 && str[i-1]!=escape){
        //trace(DEBUG, "Pop out quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
        q.pop_back();
        ++i;
        continue;
      }
    if (q.size()==0 || nestedQuoters.find(quoters[q[q.size()-1]])!=nestedQuoters.end()) // if not quoted or the latest quoter is a nested quoter, then search quoters
      for (int k=0; k<(int)(quoters.size()/2); k++){
        if (str[i] == quoters[k*2])
          if (k*2+1<quoters.size() && (i==0 || (i>0 && str[i-1]!=escape))){
            //trace(DEBUG, "Found quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
            q.push_back(k*2+1); // quoted start, need to pair the right quoter
            break;
          }
      }
    ++i;
  }
  if (begin<str.size())
    v.push_back(str.substr(begin, str.size()-begin));

  if (q.size() > 0)
    trace(ERROR, "(2)Quoters in '%s' are not paired!\n",str.c_str());

  return v;
}

// split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting. Nested quoters like "()" can quote other quoters, while any other quoters in unnested quoters like ''{}// should be ignored.
vector<string> split(const string & str, string delim, string quoters, char escape, std::set<char> nestedQuoters) 
{
  if (delim.size()==1)
    return split(str,delim[0],quoters,escape,nestedQuoters);
  vector<string> v;
  string element;
  string upDelim = upper_copy(delim);
  size_t i = 0, j = 0, begin = 0;
  vector<int> q;
  while(i < str.size()) {
    if (str.size()>=i+upDelim.size() && upper_copy(str.substr(i,upDelim.size())).compare(upDelim)==0 && i>0 && q.size()==0) {
      trace(DEBUG, "found delim, split string:%s (%d to %d)\n",str.substr(begin, i-begin).c_str(), begin, i);
      //v.push_back(element);
      element = "";
      v.push_back(str.substr(begin, i-begin));
      begin = i+upDelim.size();
    }else{
      if (str[i]==escape && i<str.length()-1 && (quoters.find(str[i+1]) >= 0 || nestedQuoters.find(str[i+1]) != nestedQuoters.end())) // skip escape
        i++;
      element.push_back(str[i]);
    }
    if (q.size()>0 && str[i] == quoters[q[q.size()-1]]) // checking the latest quoter
      if (i>0 && str[i-1]!=escape){
        //trace(DEBUG, "Pop out quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
        q.pop_back();
        ++i;
        continue;
      }
    if (q.size()==0 || nestedQuoters.find(quoters[q[q.size()-1]])!=nestedQuoters.end()) // if not quoted or the latest quoter is a nested quoter, then search quoters
      for (int k=0; k<(int)(quoters.size()/2); k++){
        if (str[i] == quoters[k*2])
          if (k*2+1<quoters.size() && (i==0 || (i>0 && str[i-1]!=escape))){
            //trace(DEBUG, "Found quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
            q.push_back(k*2+1); // quoted start, need to pair the right quoter
            break;
          }
      }
    ++i;
  }
  if (begin<str.size())
    v.push_back(str.substr(begin, str.size()-begin));

  if (q.size() > 0)
    trace(ERROR, "(2)Quoters in '%s' are not paired!\n",str.c_str());

  return v;
}

string trim_pair(const string & str, const string & pair)
{
  if(str.size() > 1 && pair.size() == 2 && str[0] == pair[0] && str[str.size()-1] == pair[1])
    return str.substr(1,str.size()-2);
  else
    return str;
}

string trim(const string & str, char c)
{
  string newstr = trim_one(str, c);
  while (newstr.length() != str.length())
    newstr = trim_one(str, c);
  return newstr;
}

string trim_right(const string & str, char c)
{
  string newstr = str;
  if (newstr[newstr.size()-1] == c)
    newstr = newstr.substr(0,newstr.size()-1);
  return newstr;
}

string trim_left(const string & str, char c)
{
  string newstr = str;
  if (newstr[0] == c)
    newstr = newstr.substr(1);
  return newstr;
}

string trim_one(const string & str, char c)
{
  string newstr = str;
  if (newstr[0] == c)
    newstr = newstr.substr(1);
  if (newstr[newstr.size()-1] == c)
    newstr = newstr.substr(0,newstr.size()-1);
  if (c == ' ')
    newstr = trim_one(str, '\t');
  return newstr;
}

void replacestr(string & sRaw, const string & sReplace, const string & sNew)
{
  int pos=0;
  //trace(DEBUG, "replacing '%s','%s','%s' ...",sRaw.c_str(),sReplace.c_str(),sNew.c_str());
  pos = sRaw.find(sReplace, pos);
  while (pos != string::npos){
    sRaw.replace(pos, sReplace.length(), sNew);
    pos = sRaw.find(sReplace, pos+sNew.length());
  }
  //trace(DEBUG, "=> '%s'\n",sRaw.c_str());
}

void regreplacestr(string & sRaw, const string & sPattern, const string & sNew)
{
  try{
    //trace(DEBUG, "replacing '%s','%s','%s' ...",sRaw.c_str(),sPattern.c_str(),sNew.c_str());
    sregex regexp = sregex::compile(sPattern);
    sRaw = regex_replace(sRaw,regexp,sNew.c_str());
    //trace(DEBUG, "=> '%s'\n",sRaw.c_str());
  }catch (exception& e) {
    trace(ERROR, "Regular replace exception: %s\n", e.what());
  }
}

void regmatchstr(const string & sRaw, const string & sPattern, string & sExpr)
{
  string sNew = "";
  smatch matches;
  sregex regexp = sregex::compile(sPattern);
  try{
    //trace(DEBUG2, "'%s' matching '%s'\n", sRaw.c_str(), sPattern.c_str());
    if (regex_match(sRaw, matches, regexp)) {
      //trace(DEBUG2, "Regular matched: %s\n", string(matches[0]).c_str());
      int iStart=0;
      for (int i=0; i<sExpr.length(); i++){
        if (sExpr[i]=='{' && (i==0 || sExpr[i-1]!='\\')){
          iStart=i+1;
          while(sExpr[i]!='}' && i<sExpr.length()){
            i++;
          }
          if (sExpr[i]=='}' && sExpr[i-1]!='\\' && i<sExpr.length()){
            string sNum = sExpr.substr(iStart,i-iStart);
            if (isInt(sNum)){
              int iNum = atoi(sNum.c_str());
              if (iNum<matches.size())
                sNew.append(matches[iNum]);
            }
          }
        }else{
          if (sExpr[i]=='\\' && i<sExpr.length()-1 && (sExpr[i+1]=='{' || sExpr[i+1]=='}')) // skip escape
            i++;
          sNew.push_back(sExpr[i]);
        }
      }
    }
  }catch (exception& e) {
    trace(ERROR, "Regular match exception: %s\n", e.what());
    return;
  }
  sExpr = sNew;
}

string trim_copy(const string & str)
{
  return boost::algorithm::trim_copy<string>(str);
}

string upper_copy(const string & str)
{
  return boost::to_upper_copy<string>(str);
}

string lower_copy(const string & str)
{
  return boost::to_lower_copy<string>(str);
}

bool isNumber(const string& str)
{
    for (int i=0; i<str.length(); i++) {
        if (std::isdigit(str[i]) == 0) return false;
    }
    return true;
}

bool isInt(const string& str)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 

  try{
    boost::lexical_cast<int>(str);
  }catch (bad_lexical_cast &){
    return false;
  }

  return true;
}

bool isLong(const string& str)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 

  try{
    boost::lexical_cast<long>(str);
  }catch (bad_lexical_cast &){
    return false;
  }

  return true;
}

bool isFloat(const string& str)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 

  try{
    boost::lexical_cast<float>(str);
  }catch (bad_lexical_cast &){
    return false;
  }

  return true;
}

bool isDouble(const string& str)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 

  try{
    boost::lexical_cast<double>(str);
  }catch (bad_lexical_cast &){
    return false;
  }

  return true;
}

string intToStr(const int val)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 
  string str;

  try{
    str = boost::lexical_cast<string>(val);
  }catch (bad_lexical_cast &){
    return "";
  }

  return str;
}

string longToStr(const long val)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 
  string str;

  try{
    str = boost::lexical_cast<string>(val);
  }catch (bad_lexical_cast &){
    return "";
  }

  return str;
}

string floatToStr(const float val)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 
  string str;

  try{
    str = boost::lexical_cast<string>(val);
  }catch (bad_lexical_cast &){
    return "";
  }

  return str;
}

string doubleToStr(const double val)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 
  string str;

  try{
    str = boost::lexical_cast<string>(val);
  }catch (bad_lexical_cast &){
    return "";
  }

  return str;
}

#if defined __MINGW32__ || defined __CYGWIN32__ || defined __CYGWIN64__ || defined __CYGWIN__
extern int putenv(char*);
#endif

// get the local time in the specific timezone
struct tm zonetime(time_t t1, int iOffSet)
{
  char const* tmp = getenv( "TZ" );
  struct tm tm;
  if (iOffSet>=-1200 && iOffSet<=1200){
    string sOldTimeZone = tmp?string(tmp):"";
    string sNewTimeZone = "GMT"+intToStr(iOffSet/100*-1);
#if defined __MINGW32__ || defined __CYGWIN32__ || defined __CYGWIN64__ || defined __CYGWIN__
    putenv(const_cast<char *>(string("TZ="+sNewTimeZone).c_str()));
    system("set"); 
#elif defined _WIN32 || defined _WIN64
    putenv(("TZ="+sNewTimeZone).c_str());
    tzset();
#else
    setenv("TZ", sNewTimeZone.c_str(), 1);
    tzset();
#endif
    //trace(DEBUG2, "Setting timezone to '%s'(%d)\n", sNewTimeZone.c_str(), iOffSet);
    tm = *(localtime(&t1));
#if defined __MINGW32__ || defined __CYGWIN32__ || defined __CYGWIN64__ || defined __CYGWIN__
    putenv(const_cast<char *>(string("TZ="+sOldTimeZone).c_str()));
    system("set"); 
#elif defined _WIN32 || defined _WIN64
    putenv(("TZ="+sOldTimeZone).c_str());
    tzset();
#else
    setenv("TZ", sOldTimeZone.c_str(), 1);
    tzset();
#endif
  }else
    tm = *(localtime(&t1));
  return tm;
}

string dateToStr(struct tm val, int iOffSet, string fmt)
{
  try{
    char buffer [256];
    // trace(DEBUG2, "(2)Converting %d %d %d %d %d %d %d, format '%s' \n",val.tm_year+1900, val.tm_mon, val.tm_mday, val.tm_hour, val.tm_min, val.tm_sec, val.tm_isdst, fmt.c_str());
    time_t t1 = mktime(&val);
    struct tm tm = zonetime(t1, iOffSet);
    if (strftime(buffer,256,fmt.c_str(),&tm)){
      return string(buffer);
      //trace(DEBUG2, "(3)Converting %d %d %d %d %d %d %d to '%s' \n",tm->tm_year+1900, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_isdst, string(buffer).c_str());
    }else {
      trace(ERROR, "Unrecognized date format '%s'.\n", fmt.c_str());
      return "";
    }
  }catch (exception& e) {
    //trace(ERROR, "Regular search exception: %s\n", e.what());
    return "";
  }
}

// strip timezone from the datetime string. 
// function returns the stripped datetime string; iOffSet returns the digital timezone offset; sTimeZone return timezone
string stripTimeZone(string str, int & iOffSet, string & sTimeZone)
{
  string sRaw = str;
  iOffSet = 0;
  sTimeZone = "";
  int iTZ = 0;
  while ((sRaw[iTZ]!='+'&&sRaw[iTZ]!='-') && iTZ<sRaw.length()){
    iTZ++;
    if (sRaw[iTZ]=='+'||sRaw[iTZ]=='-'){
      sTimeZone = sRaw.substr(iTZ);
      if (isInt(sTimeZone)){
        iOffSet = atoi(sTimeZone.c_str());
        sRaw = trim_copy(sRaw.substr(0,iTZ));
      }else{
        sTimeZone = "";
        // trace(ERROR, "Trying It is not digit number : %s\n", sRaw.c_str());
      }
    }
  }
  return sRaw;
}

// get expected data format minimum length
// %Y 4;%y 2;%m 2;%b(abb month) 3;%d 2;%H 2;%M 2;%S 2;
// %a(abb week) 3; %A(Full week) 6+; %B(Full month) 3+; %c (Thu Aug 23 14:55:02 2001) 24; %C (year%100) 2; %D(MM/DD/YY) 8; %e (day of month) 1+; %F(YYYY-MM-DD) 10;%g	(week based year) 2; %G (week based year) 4; %h	(abb month) 3; %I(12hours) 2; %j(day of year) 3; %n	(new line) 1;%p	(AM/PM) 2; %r(12 hour clock 02:55:02 pm) 11; %R(%H:%M) 5; %t(tab) 1; %T	(HH:MM:SS) 8; %u (week day in start 1) 1; %U (week number of year start 0) 2; %V (week number of year start 1) 2; %w (week day in start 0) 1; %W (week number of year, monday is first day) 2; %x(08/23/01) 8; %X(14:55:02) 8; %z(timezone +100) 4+;%Z(timezone UTC) 3+;%% (%sign) 1;
int dateFormatLen(string fmt)
{
  int len = 0, i=0;
  while(i<fmt.length()){
    if (fmt[i]=='%' && i<fmt.length()-1){
      switch(fmt[i+1]){
        case 'e':
        case 'n':
        case 't':
        case 'u':
        case 'w':
        case '%':
          len+=1;
          break;
        case 'y':
        case 'm':
        case 'd':
        case 'H':
        case 'M':
        case 'S':
        case 'C':
        case 'g':
        case 'I':
        case 'p':
        case 'U':
        case 'V':
        case 'W':
          len+=2;
          break;
        case 'b':
        case 'a':
        case 'B':
        case 'h':
        case 'j':
        case 'Z':
          len+=3;
          break;
        case 'Y':
        case 'G':
        case 'z':
          len+=4;
          break;
        case 'R':
          len+=5;
          break;
        case 'A':
          len+=6;
          break;
        case 'D':
        case 'T':
        case 'x':
        case 'X':
          len+=8;
          break;
        case 'F':
          len+=10;
          break;
        case 'r':
          len+=11;
          break;
        case 'c':
          len+=24;
          break;
      }
      i++;
    }else
      len++;
    i++;
  }
  return len;
}

// note: tm returns GMT time, iOffSet is the timezone
// strptime doesnot consider the whole datetime string. For example, strptime("20/Jul/2022:01:00:00", "%Y/%b/%d", tm) will return true.
// The return value of the strptime is a pointer to the first character not processed in this function call. In case the whole input string is consumed, the return value points to the null byte at the end of the string.
bool strToDate(string str, struct tm & tm, int & iOffSet, string fmt)
{
  // accept %z at then of the time string only
  //trace(DEBUG2, "Trying date format: '%s' (expected len:%d) for '%s'\n", fmt.c_str(), dateFormatLen(fmt), str.c_str());
  if (fmt.empty() || str.length() < dateFormatLen(fmt)){
    //trace(DEBUG2, "'%s' len %d doesnot match expected (Format: '%s') \n", str.c_str(), str.length(), fmt.c_str(), dateFormatLen(fmt) );
    return false;
  }
  string sRaw, sTimeZone, sFm = fmt;
  iOffSet = 0;
  //short int iOffOp = PLUS;
  sRaw = stripTimeZone(str, iOffSet, sTimeZone);
  if (sFm.length()>5 && sFm[sFm.length()-2]=='%' && sFm[sFm.length()-1]=='z'){
    // trace(DEBUG, "Trying timezone '%s' : '%s'\n", fmt.c_str(), sRaw.c_str());
    sFm = trim_copy(sFm.substr(0,sFm.size()-2));
  }else{
    if (!sTimeZone.empty()) { // no %z in the format, there should no timezone info in the datetime string
      //trace(DEBUG2, "'%s' (format: '%s') should not contain timezone '%s' \n", str.c_str(), sFm.c_str(), sTimeZone.c_str());
      return false;
    }
  }
  // bare in mind: strptime will ignore %z. means we need to treat its returned time as GMT time
  char * c = strptime(sRaw.c_str(), sFm.c_str(), &tm);
  if (c && c == sRaw.c_str()+sRaw.size()){
  //if (c && string(c).empty()){
    //trace(DEBUG2, "(1)Converting '%s' => %d %d %d %d %d %d %d offset %d format '%s' \n",sRaw.c_str(),tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_isdst, iOffSet, sFm.c_str());
    tm.tm_isdst = 0;
    //string sDate = dateToStr(tm, 0, sFm); // as tm got from strptime ignored offset, need to set offset to 0
    //trace(DEBUG2, "(2)Converting '%s' (%s) get '%s' \n",sRaw.c_str(), sFm.c_str(), sDate.c_str());
    //if (sDate.compare(sRaw) != 0)
    //  return false;
    if (iOffSet != 0){
      tm.tm_hour = tm.tm_hour+(iOffSet/100*-1);
      if (tm.tm_hour<0){
        tm.tm_hour += 24;
        tm.tm_mday -= 1;
        if (tm.tm_mday<=0){
          switch(tm.tm_mon){
            case 0:
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
              tm.tm_mday+=31;
              break;
            case 4:
            case 6:
            case 9:
            case 11:
              tm.tm_mday+=30;
              break;
            case 2:
              if ((tm.tm_year+1900)%400==0 || ((tm.tm_year+1900)%4==0&&(tm.tm_year+1900)%100!=0))
                tm.tm_mday+=29;
              else
                tm.tm_mday+=28;
              break;
          }
          tm.tm_mon -= 1;
          if(tm.tm_mon<0)
            tm.tm_year-=1;
        }
      }
    }
    //time_t t1;
    //t1 = mktime(&tm);
    //tm = zonetime(t1, iOffSet*-1); // we cannot get any local time as strptime ignored the timezone, return GMT time only.
    //tm.tm_isdst = 0;
    //trace(DEBUG2, "(3)Converting '%s'(format '%s') to local time => %d %d %d %d %d %d %d \n",str.c_str(), fmt.c_str(),tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_isdst);
    //trace(DEBUG, "Trying final get format %s : %s\n", str.c_str(), fmt.c_str());
    return true;
  }else{
    //trace(ERROR, "Trying strptime('%s','%s',tm) failed!\n", sRaw.c_str(), sFm.c_str());
    return false;
  }
}

struct tm now()
{
  //time_t timer;
  //time_t now = time(&timer);  // get current time; same as: timer = time(NULL) 
  time_t now = time(NULL);
  return *(localtime(&now));

  //using namespace std::chrono;
  //duration<int,std::ratio<60*60*24> > one_day (1);
  //system_clock::time_point curtime = system_clock::now();
  //system_clock::time_point tomorrow = today + one_day;
  //time_t tt;
  //tt = system_clock::to_time_t ( curtime );
  //std::cout << "today is: " << ctime(&tt);
  //tt = system_clock::to_time_t ( tomorrow );
  //std::cout << "tomorrow will be: " << ctime(&tt);
}

long int curtime()
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

bool isDate(const string& str, int & iOffSet, string& fmt)
{
  struct tm tm;
  if (strToDate(str, tm, iOffSet, fmt))
    return true;
  std::set<string> alldatefmt, alltimefmt, alljunction, alltzfmt;
  alldatefmt.insert("%Y-%m-%d");alldatefmt.insert("%Y/%m/%d");alldatefmt.insert("%d/%m/%Y");alldatefmt.insert("%m/%d/%Y");alldatefmt.insert("%m-%d-%Y");alldatefmt.insert("%d-%m-%Y");alldatefmt.insert("%d/%b/%Y");alldatefmt.insert("%b/%d/%Y");alldatefmt.insert("%Y-%b-%d");alldatefmt.insert("%Y/%b/%d");
  alltimefmt.insert("%H:%M:%S");alltimefmt.insert("%H/%M/%S");
  alljunction.insert(":");alljunction.insert("/");alljunction.insert(" ");
  alltzfmt.insert(" %z");alltzfmt.insert(" %Z");alltzfmt.insert("%z");alltzfmt.insert("%Z");alltzfmt.insert("");
  for (std::set<string>::iterator id = alldatefmt.begin(); id != alldatefmt.end(); ++id) {
    if (str.length()<=12 && strToDate(str, tm, iOffSet, (*id))){
      fmt = (*id);
      return true;
    }else{
      for (std::set<string>::iterator it = alltimefmt.begin(); it != alltimefmt.end(); ++it) {
        if (str.length()<=10 && strToDate(str, tm, iOffSet, (*it))){
          fmt = (*it);
          return true;
        }else{ 
          for (std::set<string>::iterator ij = alljunction.begin(); ij != alljunction.end(); ++ij) {
            for (std::set<string>::iterator iz = alltzfmt.begin(); iz != alltzfmt.end(); ++iz) {
              if (strToDate(str, tm, iOffSet, string((*id)+(*ij)+(*it)+(*iz)))){
                fmt = string((*id)+(*ij)+(*it)+(*iz));
                // trace(DEBUG2, "(1)'%s' Got date format: %s\n", str.c_str(), fmt.c_str());
                return true;
              }else if (strToDate(str, tm, iOffSet, string((*it)+(*ij)+(*id)+(*iz)))){
                fmt = string((*it)+(*ij)+(*id)+(*iz));
                // trace(DEBUG2, "(2)'%s' Got date format: %s\n", str.c_str(), fmt.c_str());
                return true;
              }else
                continue;
            }
          }
        }
      }
    }
  }
  return false;
}

// get the compatible data type from two data types
DataTypeStruct getCompatibleDataType(const DataTypeStruct & ldatatype, const DataTypeStruct & rdatatype) 
{
  DataTypeStruct dts;
  dts.extrainfo = "";
  if (ldatatype.datatype == ANY || rdatatype.datatype == ANY)
    dts.datatype = ldatatype.datatype == ANY?rdatatype.datatype:ldatatype.datatype;
  else if (ldatatype.datatype == STRING || rdatatype.datatype == STRING)
    //if (ldatatype.datatype == DATE || rdatatype.datatype == DATE || ldatatype.datatype == TIMESTAMP || rdatatype.datatype == TIMESTAMP){ // incompatible types
    //  trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(STRING).c_str(), decodeDatatype(ldatatype.datatype==STRING?rdatatype.datatype:ldatatype.datatype).c_str());
    //  dts.datatype = UNKNOWN;
    //}else{
      dts.datatype = STRING;
    //}
  else if (ldatatype.datatype == DOUBLE || rdatatype.datatype == DOUBLE)
    if (ldatatype.datatype == DATE || rdatatype.datatype == DATE || ldatatype.datatype == TIMESTAMP || rdatatype.datatype == TIMESTAMP || ldatatype.datatype == STRING || rdatatype.datatype == STRING){ // incompatible types
      trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(DOUBLE).c_str(), decodeDatatype(ldatatype.datatype==DOUBLE?rdatatype.datatype:ldatatype.datatype).c_str());
      dts.datatype = UNKNOWN;
    }else{
      dts.datatype = DOUBLE;
    }
  else if (ldatatype.datatype == LONG || rdatatype.datatype == LONG)
    if (ldatatype.datatype == DATE || rdatatype.datatype == DATE || ldatatype.datatype == TIMESTAMP || rdatatype.datatype == TIMESTAMP || ldatatype.datatype == STRING || rdatatype.datatype == STRING || ldatatype.datatype == DOUBLE || rdatatype.datatype == DOUBLE){ // incompatible types
      trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(LONG).c_str(), decodeDatatype(ldatatype.datatype==LONG?rdatatype.datatype:ldatatype.datatype).c_str());
      dts.datatype = UNKNOWN;
    }else{
      dts.datatype = LONG;
    }
  else if (ldatatype.datatype == INTEGER || rdatatype.datatype == INTEGER)
    if (ldatatype.datatype == DATE || rdatatype.datatype == DATE || ldatatype.datatype == TIMESTAMP || rdatatype.datatype == TIMESTAMP || ldatatype.datatype == STRING || rdatatype.datatype == STRING || ldatatype.datatype == DOUBLE || rdatatype.datatype == DOUBLE || ldatatype.datatype == LONG || rdatatype.datatype == LONG){ // incompatible types
      trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(INTEGER).c_str(), decodeDatatype(ldatatype.datatype==INTEGER?rdatatype.datatype:ldatatype.datatype).c_str());
      dts.datatype = UNKNOWN;
    }else{
      dts.datatype = INTEGER;
    }
  else if (ldatatype.datatype == BOOLEAN || rdatatype.datatype == BOOLEAN)
    if (ldatatype.datatype == DATE || rdatatype.datatype == DATE || ldatatype.datatype == TIMESTAMP || rdatatype.datatype == TIMESTAMP || ldatatype.datatype == STRING || rdatatype.datatype == STRING || ldatatype.datatype == DOUBLE || rdatatype.datatype == DOUBLE || ldatatype.datatype == LONG || rdatatype.datatype == LONG || ldatatype.datatype == INTEGER || rdatatype.datatype == INTEGER){ // incompatible types
      trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(BOOLEAN).c_str(), decodeDatatype(ldatatype.datatype==BOOLEAN?rdatatype.datatype:ldatatype.datatype).c_str());
      dts.datatype = UNKNOWN;
    }else{
      dts.datatype = BOOLEAN;
    }
  else if (ldatatype.datatype == DATE || rdatatype.datatype == DATE || ldatatype.datatype == TIMESTAMP || rdatatype.datatype == TIMESTAMP)
    if (ldatatype.datatype == STRING || rdatatype.datatype == STRING || ldatatype.datatype == DOUBLE || rdatatype.datatype == DOUBLE || ldatatype.datatype == LONG || rdatatype.datatype == LONG || ldatatype.datatype == INTEGER || rdatatype.datatype == INTEGER || ldatatype.datatype == BOOLEAN || rdatatype.datatype == BOOLEAN){ // incompatible types
      trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(DATE).c_str(), decodeDatatype((ldatatype.datatype==DATE||ldatatype.datatype==TIMESTAMP)?rdatatype.datatype:ldatatype.datatype).c_str());
      dts.datatype = UNKNOWN;
    }else{
      dts.datatype = DATE;
      if (ldatatype.extrainfo.compare(rdatatype.extrainfo) == 0)
        dts.extrainfo = ldatatype.extrainfo;
    }
  else{
    dts.datatype = UNKNOWN;
  }
  return dts;
}

// detect the data type of an expression string
// STRING: quoted by '', or regular expression: //
// DATE/TIMESTAMP: quoted by {}
// INTEGER/LONG: all digits
// Double: digits + .
int detectDataType(string str, string & extrainfo)
{
  string trimmedStr = trim_copy(str);
  short int datatype = UNKNOWN;
  extrainfo = "";
  int iOffSet;
  if (trimmedStr.size()>1 && ((trimmedStr[0]=='\'' && trimmedStr[trimmedStr.size()-1]=='\'' && matchQuoters(trimmedStr, 0, "''")==0) || (trimmedStr[0]=='/' && trimmedStr[trimmedStr.size()-1]=='/' && matchQuoters(trimmedStr, 0, "//")==0))){
    datatype = STRING;
    //trace(DEBUG2, "'%s' has quoters\n", trimmedStr.c_str());
  //else if (matchQuoters(trimmedStr, 0, "{}"))
  }else if (isDate(trimmedStr, iOffSet, extrainfo))
    datatype = DATE;
  else if (isLong(trimmedStr))
    datatype = LONG;
  else if (isDouble(trimmedStr))
    datatype = DOUBLE;
  else if (isInt(trimmedStr))
    datatype = INTEGER;
  else
    datatype = UNKNOWN;
  trace(DEBUG, "Detected from '%s', data type '%s', extrainfo '%s'\n", str.c_str(), decodeDatatype(datatype).c_str(), extrainfo.c_str());
  return datatype;
}

bool wildmatch(const char *candidate, const char *pattern, int p, int c, char multiwild='*', char singlewild='?', char escape='\\') {
  //trace(DEBUG2, "Like matching '%s'(%d) => '%s'(%d)\n", string(pattern+p).c_str(),p, string(candidate+c).c_str(),c);
  if (pattern[p] == '\0') {
    return candidate[c] == '\0';
  }else if (pattern[p] == escape && pattern[p+1] != '\0' && (pattern[p+1] == multiwild || pattern[p+1] == singlewild)) {
    return wildmatch(candidate, pattern, p+1, c, multiwild, singlewild, escape);
  } else if ((pattern[p] == multiwild && p == 0) || (pattern[p] == multiwild && p > 0 && pattern[p-1] != escape)) {
    for (; candidate[c] != '\0'; c++) {
      if (wildmatch(candidate, pattern, p+1, c, multiwild, singlewild, escape))
        return true;
    }
    return wildmatch(candidate, pattern, p+1, c, multiwild, singlewild, escape);
  } else if ((pattern[p] != '?' || (p > 0 && pattern[p-1] == escape)) && pattern[p] != candidate[c]) {
    return false;
  }  else {
    return wildmatch(candidate, pattern, p+1, c+1, multiwild, singlewild, escape);
  }
}

bool like(string str1, string str2)
{
  bool matched = wildmatch(str1.c_str(), str2.c_str(), 0, 0);
  //trace(DEBUG2, "'%s' matching '%s': %d\n",str1.c_str(), str2.c_str(), matched);
  return matched;
}

bool reglike(string str, string regstr)
{
  sregex regexp = sregex::compile(regstr);
  smatch matches;
  //trace(DEBUG, "Matching: '%s' => %s\n", str.c_str(), regstr.c_str());
  try{
    return regex_search(str, matches, regexp);
  }catch (exception& e) {
    trace(ERROR, "Regular search exception: %s\n", e.what());
    return false;
  }
}

bool in(string str1, string str2)
{
  //return upper_copy(str2).find(upper_copy(str1))!=string::npos;
  return str2.find(str1)!=string::npos;
}

/*
char getch() {
  char buf = 0;
  struct termios old = {0};
  if (tcgetattr(0, &old) < 0)
    perror("tcsetattr()");
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0)
    perror("tcsetattr ICANON");
  if (read(0, &buf, 1) < 0)
    perror ("read()");
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0)
    perror ("tcsetattr ~ICANON");
  return (buf);
}

size_t getstr(char * buffer, const size_t len)
{
  char cachebuffer[len];
  size_t reads = 0;

  struct termios old = {0};
  if (tcgetattr(0, &old) < 0)
    perror("tcsetattr()");
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0){
    perror("tcsetattr ICANON");
    return -1;
  }
  //reads = read(0, &cachebuffer, len);
  std::cin.read(cachebuffer, len);
  reads = std::cin.gcount();
  if (reads < 0){
    perror ("read()");
    return -1;
  }
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0){
    perror ("tcsetattr ~ICANON");
    return -1;
  }
  memcpy(cachebuffer, buffer, reads);
  return (reads);
}
*/

string decodeJunction(int junction){
  switch (junction){
  case AND:
    return "AND";
  case OR:
    return "OR";
  default:
    return "UNKNOWN";
  }
}

string decodeComparator(int comparator){
  switch (comparator){
  case EQ:
    return "=";
  case LT:
    return ">";
  case ST:
    return "<";
  case NEQ:
    return "!=";
  case LE:
    return ">=";
  case SE:
    return "<=";
  case LIKE:
    return "LIKE";
  case REGLIKE:
    return "REGLIKE";
  case NOLIKE:
    return "NOLIKE";
  case NOREGLIKE:
    return "NOREGLIKE";
  case IN:
    return "IN";
  case NOIN:
    return "NOIN";
  default:
    return "UNKNOWN";
  }
}

string decodeDatatype(int datatype){
  switch (datatype){
  case STRING:
    return "STRING";
  case LONG:
    return "LONG";
  case INTEGER:
    return "INTEGER";
  case DOUBLE:
    return "DOUBLE";
  case DATE:
    return "DATE";
  case TIMESTAMP:
    return "TIMESTAMP";
  case BOOLEAN:
    return "BOOLEAN";
  default:
    return "UNKNOWN";
  }
}

string decodeExptype(int exptype)
{
  switch (exptype){
  case CONST:
    return "CONST";
  case COLUMN:
    return "COLUMN";
  case VARIABLE:
    return "VARIABLE";
  case FUNCTION:
    return "FUNCTION";
  default:
    return "UNKNOWN";
  }
}

string decodeOperator(int op)
{
  switch (op){
  case PLUS:
    return "+";
  case SUBTRACT:
    return "-";
  case TIMES:
    return "*";
  case DIVIDE:
    return "/";
  case POWER:
    return "^";
  default:
    return "UNKNOWN";
  }
}

short int encodeComparator(string str)
{
  //printf("encode comparator: %s\n",str.c_str());
  string sUpper = upper_copy(str);
  if (sUpper.compare("=") == 0)
    return EQ;
  else if (sUpper.compare(">") == 0)
    return LT;
  else if (sUpper.compare("<") == 0)
    return ST;
  else if (sUpper.compare("!=") == 0)
    return NEQ;
  else if (sUpper.compare(">=") == 0)
    return LE;
  else if (sUpper.compare("<=") == 0)
    return SE;
  else if (sUpper.compare("LIKE") == 0)
    return LIKE;
  else if (sUpper.compare("REGLIKE") == 0)
    return REGLIKE;
  else if (sUpper.compare("NOLIKE") == 0)
    return NOLIKE;
  else if (sUpper.compare("NOREGLIKE") == 0)
    return NOREGLIKE;
  else if (sUpper.compare("IN") == 0)
    return IN;
  else if (sUpper.compare("NOIN") == 0)
    return NOIN;
  else
    return UNKNOWN;
}

short int encodeDatatype(string str)
{
  string sUpper = upper_copy(str);
  if (sUpper.compare("STRING") == 0)
    return STRING;
  else if (sUpper.compare("LONG") == 0)
    return LONG;
  else if (sUpper.compare("INTEGER") == 0)
    return INTEGER;
  else if (sUpper.compare("DOUBLE") == 0)
    return DOUBLE;
  else if (sUpper.compare("DATE") == 0)
    return DATE;
  else if (sUpper.compare("TIMESTAMP") == 0)
    return TIMESTAMP;
  else if (sUpper.compare("BOOLEAN") == 0)
    return BOOLEAN;
  else
    return UNKNOWN;
}

short int encodeJunction(string str)
{
  string sUpper = upper_copy(str);
  if (sUpper.compare("AND") == 0)
    return AND;
  else if (sUpper.compare("OR") == 0)
    return OR;
  else
    return UNKNOWN;
}

short int encodeOperator(string str)
{
  if (str.compare("+") == 0)
    return PLUS;
  else if (str.compare("-") == 0)
    return SUBTRACT;
  else if (str.compare("*") == 0)
    return TIMES;
  else if (str.compare("/") == 0)
    return DIVIDE;
  else if (str.compare("^") == 0)
    return POWER;
  else
    return UNKNOWN;
}

short int encodeFunction(string str)
{
  string sUpper = upper_copy(str);
  if(sUpper.compare("UPPER")==0)
    return UPPER;
  else if(sUpper.compare("LOWER")==0)
    return LOWER;
  else if(sUpper.compare("SUBSTR")==0)
    return SUBSTR;
  else if(sUpper.compare("FLOOR")==0)
    return FLOOR;
  else if(sUpper.compare("CEIL")==0)
    return CEIL;
  else if(sUpper.compare("TIMEDIFF")==0)
    return TIMEDIFF;
  else if(sUpper.compare("INSTR")==0)
    return INSTR;
  else if(sUpper.compare("STRLEN")==0)
    return STRLEN;
  else if(sUpper.compare("COMPARESTR")==0)
    return COMPARESTR;
  else if(sUpper.compare("NOCASECOMPARESTR")==0)
    return NOCASECOMPARESTR;
  else if(sUpper.compare("REPLACE")==0)
    return REPLACE;
  else if(sUpper.compare("REGREPLACE")==0)
    return REGREPLACE;
  else if(sUpper.compare("REGMATCH")==0)
    return REGMATCH;
  else if(sUpper.compare("COUNTWORD")==0)
    return COUNTWORD;
  else if(sUpper.compare("ZONECONVERT")==0)
    return ZONECONVERT;
  else if(sUpper.compare("ISNULL")==0)
    return ISNULL;
  else if(sUpper.compare("SWITCH")==0)
    return SWITCH;
  else if(sUpper.compare("PAD")==0)
    return PAD;
  else if(sUpper.compare("GREATEST")==0)
    return GREATEST;
  else if(sUpper.compare("LEAST")==0)
    return LEAST;
  else if(sUpper.compare("ROUND")==0)
    return ROUND;
  else if(sUpper.compare("LOG")==0)
    return LOG;
  else if(sUpper.compare("DATEFORMAT")==0)
    return DATEFORMAT;
  else if(sUpper.compare("TRUNCDATE")==0)
    return TRUNCDATE;
  else if(sUpper.compare("NOW")==0)
    return NOW;
  else if(sUpper.compare("SUM")==0)
    return SUM;
  else if(sUpper.compare("COUNT")==0)
    return COUNT;
  else if(sUpper.compare("UNIQUECOUNT")==0)
    return UNIQUECOUNT;
  else if(sUpper.compare("MAX")==0)
    return MAX;
  else if(sUpper.compare("MIN")==0)
    return MIN;
  else if(sUpper.compare("AVERAGE")==0)
    return AVERAGE;
  else
    return UNKNOWN;
}

short int operatorPriority(int iOperator)
{
  switch (iOperator){
  case PLUS:
    return 1;
  case SUBTRACT:
    return 1;
  case TIMES:
    return 2;
  case DIVIDE:
    return 2;
  case POWER:
    return 3;
  default:
    return 0;
  }
}

int findStrArrayId(const vector<string> array, const string member)
{
  for (int i=0; i<array.size(); i++){
    //if (array[i].compare(member) == 0)
    if (boost::iequals(array[i], member))
      return i;
  }
  return -1;
}

// comoare two vector. return 0 means equal; positive means array1>array2; negative means array1<array2
int compareVector(vector<string> array1, vector<string> array2)
{
  if (array1.size() == array2.size()){
    int c;
    for (int i=0;i<array1.size();i++){
      c = array1[i].compare(array2[i]);
      if (c!=0)
        return c;
    }
    return 0;
  }else
    return array1.size()-array2.size();
}

// compare data according to data type
// @return int str1 < str2: -1; str1 == str2:0; str1 > str2: 1
//             error -101~-110 -101:invalid data according to data type; -102: data type not supported
int anyDataCompare(string str1, string str2, DataTypeStruct dts){
  if (str1.length() == 0 && str2.length() == 0)
    return 0;
  else if (str1.length() == 0)
    return -1;
  else if (str2.length() == 0)
    return 1;
  switch(dts.datatype){
    case LONG:{
      if (isLong(str1) && isLong(str2)){
        long d1 = atol(str1.c_str());
        long d2 = atol(str2.c_str());
        if (d1 < d2)
          return -1;
        else if (d1 == d2)
          return 0;
        else
          return 1;
      }else{
        return -101;
      }
    }case INTEGER:{
      if (isInt(str1) && isInt(str2)){
        int d1 = atoi(str1.c_str());
        int d2 = atoi(str2.c_str());
        if (d1 < d2)
          return -1;
        else if (d1 == d2)
          return 0;
        else
          return 1;
      }else{
        return -101;
      }
    }case DOUBLE:{
      if (isDouble(str1) && isDouble(str2)){
        double d1 = atof(str1.c_str());
        double d2 = atof(str2.c_str());
        if (d1 < d2)
          return -1;
        else if (d1 == d2)
          return 0;
        else
          return 1;
      }else{
        return -101;
      }
    }case DATE:
    case TIMESTAMP:{
      string fmt1, fmt2;
      string newstr1=trim_pair(str1,"{}"),newstr2=trim_pair(str2,"{}");
      bool bIsDate1 = true, bIsDate2 = true;
      int offSet1,offSet2;
      //if (dts.extrainfo.empty()){
      //  bIsDate1 = isDate(newstr1,offSet1,fmt1);
      //  bIsDate2 = isDate(newstr2,offSet2,fmt2);
      //}else{
        fmt1 = dts.extrainfo;
        fmt2 = dts.extrainfo;
      //}
      //if (bIsDate1 && bIsDate2){
        struct tm tm1, tm2;
        if (strToDate(newstr1, tm1, offSet1, fmt1) && strToDate(newstr2, tm2, offSet2, fmt2)){
          time_t t1 = mktime(&tm1);
          time_t t2 = mktime(&tm2);
          double diffs = difftime(t1, t2);
          if (diffs < 0)
            return -1;
          else if (diffs > 0)
            return 1;
          else return 0;
        }else{
          return -101;
        }
      //}else{
      //  return -101;
      //}
    }case BOOLEAN:{
      if (isInt(str1) && isInt(str2)){
        // convert boolean to int to compare. false => 0; true => 1
        int d1 = atoi(str1.c_str());
        int d2 = atoi(str2.c_str());
        if (d1 < d2)
          return -1;
        else if (d1 == d2)
          return 0;
        else
          return 1;
      }else{
        return -101;
      }
    }case STRING:{
      //string newstr1=trim_one(str1,'\''),newstr2=trim_one(str2,'\'');
      //newstr1=trim_one(newstr1,'/');newstr2=trim_one(newstr2,'/');
      return str1.compare(str2);
    }default: {
      return -102;
    }
  }
}

// compare data according to data type
// @return int 0: false; 1: true  
//             error -101~-110 -101:invalid data according to data type; -102: data type not supported
int anyDataCompare(string str1, int comparator, string str2, DataTypeStruct dts){
  switch (dts.datatype){
    case LONG:{
      if (isLong(str1) && isLong(str2)){
        long d1 = atol(str1.c_str());
        long d2 = atol(str2.c_str());
        switch (comparator){
        case EQ:
          return d1 == d2?1:0;
        case LT:
          return d1 > d2?1:0;
        case ST:
          return d1 < d2?1:0;
        case NEQ:
          return d1 != d2?1:0;
        case LE:
          return d1 >= d2?1:0;
        case SE:
          return d1 <= d2?1:0;
        default:
          return -101;
        }
      }else{
        return -101;
      }
    }case INTEGER:{
      if (isInt(str1) && isInt(str2)){
        int d1 = atoi(str1.c_str());
        int d2 = atoi(str2.c_str());
        switch (comparator){
        case EQ:
          return d1 == d2?1:0;
        case LT:
          return d1 > d2?1:0;
        case ST:
          return d1 < d2?1:0;
        case NEQ:
          return d1 != d2?1:0;
        case LE:
          return d1 >= d2?1:0;
        case SE:
          return d1 <= d2?1:0;
        default:
          return -101;
        }
      }else{
        return -101;
      }
    }case DOUBLE:{
      if (isDouble(str1) && isDouble(str2)){
        double d1 = atof(str1.c_str());
        double d2 = atof(str2.c_str());
        switch (comparator){
        case EQ:
          return d1 == d2?1:0;
        case LT:
          return d1 > d2?1:0;
        case ST:
          return d1 < d2?1:0;
        case NEQ:
          return d1 != d2?1:0;
        case LE:
          return d1 >= d2?1:0;
        case SE:
          return d1 <= d2?1:0;
        default:
          return -101;
        }
      }else{
        return -101;
      }
    }case DATE:
    case TIMESTAMP:{
      string fmt1, fmt2;
      bool bIsDate1 = true, bIsDate2 = true;
      int offSet1, offSet2;
      string newstr1=trim_pair(str1,"{}"),newstr2=trim_pair(str2,"{}");
      if (dts.extrainfo.empty()){
        bIsDate1 = isDate(newstr1,offSet1,fmt1);
        bIsDate2 = isDate(newstr2,offSet2,fmt2);
      }else{
        fmt1 = dts.extrainfo;
        fmt2 = dts.extrainfo;
      }
      if (bIsDate1 && bIsDate2){
        struct tm tm1, tm2;
        if (strToDate(newstr1, tm1, offSet1, fmt1) && strToDate(newstr2, tm2, offSet2, fmt2)){
          time_t t1 = mktime(&tm1);
          time_t t2 = mktime(&tm2);
          double diffs = difftime(t1, t2);
          switch (comparator){
          case EQ:
            return diffs==0?1:0;
          case LT:
            return diffs>0?1:0;
          case ST:
            return diffs<0?1:0;
          case NEQ:
            return diffs!=0?1:0;
          case LE:
            return diffs>=0?1:0;
          case SE:
            return diffs<=0?1:0;
          default:
            return -101;
          }
        }else{
          return -101;
        }
      }else{
        return -101;
      }
    }case BOOLEAN:{
      if (isInt(str1) && isInt(str2)){
        // convert boolean to int to compare. false => 0; true => 1
        int d1 = atoi(str1.c_str());
        int d2 = atoi(str2.c_str());
        switch (comparator){
        case EQ:
          return d1 == d2?1:0;
        case LT:
          return d1 > d2?1:0;
        case ST:
          return d1 < d2?1:0;
        case NEQ:
          return d1 != d2?1:0;
        case LE:
          return d1 >= d2?1:0;
        case SE:
          return d1 <= d2?1:0;
        default:
          return -101;
        }
      }else{
        return -101;
      }
    }case STRING:{
      //string newstr1=trim_one(str1,'\''),newstr2=trim_one(str2,'\'');
      //newstr1=trim_one(newstr1,'/');newstr2=trim_one(newstr2,'/');
      switch (comparator){
      case EQ:
        return str1.compare(str2)==0?1:0;
      case LT:
        return str1.compare(str2)>0?1:0;
      case ST:
        return str1.compare(str2)<0?1:0;
      case NEQ:
        return str1.compare(str2)!=0?1:0;
      case LE:
        return str1.compare(str2)>=0?1:0;
      case SE:
        return str1.compare(str2)<=0?1:0;
      case LIKE:
        return like(str1, str2);
      case REGLIKE:
        return reglike(str1, str2);
      case NOLIKE:
        return !like(str1, str2);
      case NOREGLIKE:
        return !reglike(str1, str2);
      case IN:
        return in(str1, str2);
      case NOIN:
        return !in(str1, str2);
      default:
        return -101;
      }
    }default: {
        return -102;
    }
  }
  return -102;
}

bool evalString(string str1, int operate, string str2, string& result)
{
  switch(operate){
  case PLUS:
    result = trim_right(str1,'\'') + trim_left(str2,'\'');
    return true;
  default:
    trace(ERROR, "Operation %s is not supported for STRING data type!\n", decodeOperator(operate).c_str());
    return false;
  }
}

bool evalLong(string str1, int operate, string str2, long& result)
{
  if (!isLong(str1) || !isLong(str2)){
    trace(ERROR, "Invalid LONG data detected!\n");
    return false;
  }
  switch(operate){
  case PLUS:
    result = atol(str1.c_str()) + atol(str2.c_str());
    return true;
  case SUBTRACT:
    result = atol(str1.c_str()) - atol(str2.c_str());
    return true;
  case TIMES:
    result = atol(str1.c_str()) * atol(str2.c_str());
    return true;
  case DIVIDE:
    result = atol(str1.c_str()) / atol(str2.c_str());
    return true;
  case POWER:
    result = pow(atol(str1.c_str()), atol(str2.c_str()));
    return true;
  default:
    trace(ERROR, "Operation %s is not supported for LONG data type!\n", decodeOperator(operate).c_str());
    return false;
  }
}

bool evalInteger(string str1, int operate, string str2, int& result)
{
  if (!isInt(str1) || !isInt(str2)){
    trace(ERROR, "Invalid INTEGER data detected!\n");
    return false;
  }
  switch(operate){
  case PLUS:
    result = atoi(str1.c_str()) + atoi(str2.c_str());
    return true;
  case SUBTRACT:
    result = atoi(str1.c_str()) - atoi(str2.c_str());
    return true;
  case TIMES:
    result = atoi(str1.c_str()) * atoi(str2.c_str());
    return true;
  case DIVIDE:
    result = atoi(str1.c_str()) / atoi(str2.c_str());
    return true;
  case POWER:
    result = pow(atoi(str1.c_str()), atoi(str2.c_str()));
    return true;
  default:
    trace(ERROR, "Operation %s is not supported for INETEGER data type!\n", decodeOperator(operate).c_str());
    return false;
  }
}

bool evalDouble(string str1, int operate, string str2, double& result)
{
  if (!isDouble(str1) || !isDouble(str2)){
    trace(ERROR, "Invalid DOUBLE data detected!\n");
    return false;
  }
  switch(operate){
  case PLUS:
    result = atof(str1.c_str()) + atof(str2.c_str());
    return true;
  case SUBTRACT:
    result = atof(str1.c_str()) - atof(str2.c_str());
    return true;
  case TIMES:
    result = atof(str1.c_str()) * atof(str2.c_str());
    return true;
  case DIVIDE:
    result = atof(str1.c_str()) / atof(str2.c_str());
    return true;
  case POWER:
    result = pow(atof(str1.c_str()), atof(str2.c_str()));
    return true;
  default:
    trace(ERROR, "Operation %s is not supported for DOUBLE data type!\n", decodeOperator(operate).c_str());
    return false;
  }
}

bool evalDate(string str1, int operate, string str2, string fmt, struct tm& result)
{
  time_t t1;
  int seconds, iOffSet;
  bool bIsDate1 = false, bIsDate2 = false;
  // avoid to use isDate if nfmt is provided, to improve performance
  if (fmt.empty()){
    string nfmt = fmt;
    bIsDate1 = isDate(str1, iOffSet, nfmt);
    if (bIsDate1){
      if (isInt(str2)){
        strToDate(str1, result, iOffSet, nfmt);
        t1 = mktime(&result);
        seconds = atoi(str2.c_str());
      }else{
        trace(ERROR, "(1)DATE can only +/- an INTEGER number !\n");
        return false;
      }
    }else{
      bIsDate2 = isDate(str2, iOffSet, nfmt);
      if (bIsDate2){
        if (isInt(str1)){
          strToDate(str2, result, iOffSet, nfmt);
          t1 = mktime(&result);
          seconds = atoi(str1.c_str());
        }else{
          trace(ERROR, "(2)DATE can only +/- an INTEGER number !\n");
          return false;
        }
      }
    }
  }else{
    bIsDate1 = strToDate(str1, result, iOffSet, fmt);
    if (bIsDate1){
      if (isInt(str2)){
        t1 = mktime(&result);
        seconds = atoi(str2.c_str());
      }else{
        trace(ERROR, "(1)DATE can only +/- an INTEGER number !\n");
        return false;
      }
    }else{
      bIsDate2 = strToDate(str2, result, iOffSet, fmt);
      if (bIsDate2){
        if (isInt(str1)){
          t1 = mktime(&result);
          seconds = atoi(str1.c_str());
        }else{
          trace(ERROR, "(2)DATE can only +/- an INTEGER number !\n");
          return false;
        }
      }
    }
  }
  switch(operate){
  case PLUS:
    t1+=seconds;
    //localtime_s(&result,&t1);
    result = *(localtime(&t1));
    return true;
  case SUBTRACT:
    t1-=seconds;
    result = *(localtime(&t1));
    return true;
  default:
    trace(ERROR, "Operation %s is not supported for DATE data type!\n", decodeOperator(operate).c_str());
    return false;
  }
}

// return true if operated successfully, result returns result
bool anyDataOperate(string str1, int operate, string str2, DataTypeStruct dts, string& result)
{
  switch (dts.datatype){
    case LONG:{
      long rslt;
      bool gotResult = evalLong(str1, operate, str2, rslt);
      result = longToStr(rslt);
      return gotResult;
    }case INTEGER:{
      int rslt;
      bool gotResult = evalInteger(str1, operate, str2, rslt);
      result = intToStr(rslt);
      return gotResult;
    }case DOUBLE:{
      double rslt;
      bool gotResult = evalDouble(str1, operate, str2, rslt);
      result = doubleToStr(rslt);
      return gotResult;
    }case DATE:
    case TIMESTAMP:{
      struct tm rslt;
      bool gotResult = evalDate(str1, operate, str2, dts.extrainfo, rslt);
      result = dateToStr(rslt);
      //char buffer [256];
      //strftime (buffer,256,DATEFMT,&rslt);
      //result = string(buffer);
      return gotResult;
    }
    case STRING:
    default:{
      return evalString(str1, operate, str2, result);
    }
  }
}

// detect if string start with special words
int startsWithWords(string str, vector<string> words, int offset)
{
  string upperstr = upper_copy(str);
  for (int i=0;i<words.size();i++){
    if (upperstr.find(words[i], offset) == 0)
      return i;
  }
  return -1;
}

// detect if string start with special words
int startsWithWords(string str, vector<string> words)
{
  return startsWithWords(str,words,0);
}

// remove space
string removeSpace(string originalStr, string keepPattern)
{
  //if (keepPattern == null)
  //    keepPattern =  "(\\s+OR\\s+|\\s+AND\\s+)"; //default pattern
      // keepPattern =  "\\s+NOT\\s+|\\s+OR\\s+|\\s+AND\\s+|\\s+IN\\s+|\\s+LIKE\\s+"; //default pattern
  vector<string> keepWords;
  keepWords.push_back(" OR ");keepWords.push_back(" AND ");

  string cleanedStr = "";
  int i = 0;

  //Pattern keeper = Pattern.compile(keepPattern);
  //Matcher matcher = keeper.matcher(originalStr.substring(i).toUpperCase());
  while (i < originalStr.length()) {
    int matchedWordId = startsWithWords(upper_copy(originalStr.substr(i)), keepWords);
    if (originalStr[i] != ' ') {// ' ' to be removed
      cleanedStr = cleanedStr+originalStr[i];
      i++;
    }else if (matchedWordId >= 0) {
        cleanedStr = cleanedStr+keepWords[matchedWordId];
        i+=keepWords[matchedWordId].length();
    }else
    i++;
  }
  return cleanedStr;
}

// detect if quoters matched. 
// listStr string to be detected;
// offset, off set to begin test;
// quoters,  eg. {'(',')'}
// 0 means all matched
int matchQuoters(string listStr, int offset, string quoters){
  if (quoters.empty() || quoters.length() != 2 || offset < 0)
    return -1;
  int deep = 0;
  for (int i=offset;i<listStr.length();i++) {
    if (listStr[i] == quoters[0])
      deep++;
    else if (listStr[i] == quoters[1])
      deep--;
  }
  return deep;
}

//get the first matched regelar token from a string
string getFirstToken(string str, string token){
  sregex regexp = sregex::compile(token);
  smatch matches;
  try{
    if (regex_search(str, matches, regexp))
      return matches[0];
  }catch (exception& e) {
    trace(ERROR, "Regular search exception: %s\n", e.what());
    return "";
  }
  return "";
}

//get all matched regelar token from a string
vector <string> getAllTokens(string str, string token)
{
  vector <string> findings;
  sregex regexp = sregex::compile(token);
  smatch matches;
  string::const_iterator searchStart( str.begin() );
  try{
    while ( regex_search( searchStart, str.end(), matches, regexp ) )
    {
        findings.push_back(matches[0]);  
        searchStart = matches.suffix().first;
    }
  }catch (exception& e) {
    trace(ERROR, "Regular search exception: %s\n", e.what());
    return findings;
  }
  return findings;
}    

// check if matched regelar token
bool matchToken(string str, string token)
{
  return !getFirstToken(str, token).empty();
}

void dumpVector(vector<string> v)
{
  trace(DEBUG, "Dumping vector<string>...\n");
  for (int i=0; i<v.size(); i++)
    trace(DUMP, "%d:%s\t", i, v[i].c_str());
  trace(DUMP, "\n");
}

void dumpMap(map<string, string> m)
{
  trace(DEBUG, "Dumping map<string, string>...\n");
  for (map<string,string>::iterator it=m.begin(); it!=m.end(); ++it)
    trace(DUMP, "%s: %s\n", it->first.c_str(), it->second.c_str());
  trace(DUMP, "\n");
}
