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
#include <random>

#if defined unix || defined __unix || defined __unix__ || defined __APPLE__ || defined __MACH__ || defined __linux__ || defined linux || defined __linux || defined __FreeBSD__ || defined __ANDROID__
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#endif

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
  g_recursiveread = false;
}

void GlobalVars::setVars(size_t inputbuffer, short tracelevel, bool printheader)
{
  g_inputbuffer = inputbuffer;
  g_tracelevel = tracelevel;
  g_printheader = printheader;
}

void GroupProp::init()
{
  funcID = UNKNOWN;
  count = 0;
  sum = 0;
  //uniquec = NULL;
  varray = NULL;
  inited = false;
}

GroupProp::GroupProp()
{
  init();
}

GroupProp::GroupProp(const GroupProp& other)
{
  if (this != &other){
    inited = other.inited;
    funcID = other.funcID;
    sum = other.sum;
    max = other.max;
    min = other.min;
    count = other.count;
    varray = other.varray;
    //uniquec = other.uniquec; // To improve performance, reuse the point of the source object, this can avoid the set copy
    //if (other.uniquec){
    //  uniquec = new std::set <string>;
    //  uniquec->insert(other.uniquec->begin(),other.uniquec->end());
    //}else
    //  uniquec = NULL;
  }
}

GroupProp::~GroupProp()
{
  // To improve performance, we dont delete the pointer in the destructor, so the address can be reused by the assigned or Copy constructed target
  //SafeDelete(uniquec);
}

GroupProp& GroupProp::operator=(const GroupProp& other)
{
  if (this != &other){
    //SafeDelete(uniquec);
    SafeDelete(varray);
    inited = other.inited;
    funcID = other.funcID;
    sum = other.sum;
    max = other.max;
    min = other.min;
    count = other.count;
    varray = other.varray;
    //uniquec = other.uniquec; // To improve performance, reuse the point of the source object, this can avoid the set copy
    //if (other.uniquec){
    //  uniquec = new std::set <string>;
    //  uniquec->insert(other.uniquec->begin(),other.uniquec->end());
    //}else
    //  uniquec = NULL;
  }
  return *this;
}

void GroupProp::clear()
{
  //SafeDelete(uniquec);
  SafeDelete(varray);
  init();
}

TimeZone::TimeZone()
{
  init();
}

TimeZone::~TimeZone()
{
  
}

unordered_map< string, int > TimeZone::m_nameoffmap = {};

void TimeZone::init()
{
  if (m_nameoffmap.size() == 0){
    trace(DEBUG,"Initializing time zone list\n");
    string time("1988/08/18 08:18:58"), sFm("%Y/%m/%d %H:%M:%S");
    struct tm tm; 
    int iOff=0,iOffSet = 0;
    strToDate(time, tm, iOffSet, sFm);
    time_t t1 = mktime(&tm);
    for (int i=0; i<=24; i++){
      int iCurOff = iOff+(int)(i/2)*100;
      if (i%2==1)
        iCurOff+=30;
      tm = zonetime(t1, iCurOff);
      tm = zonetime(t1, -1*iCurOff);
      string sOff=intToStr(iCurOff);
      if (sOff.length()==3)
        sOff = "0"+sOff;
      string curtime = time+" +"+sOff;
      char * c = strptime(curtime.c_str(), sFm.c_str(), &tm);
      cleanuptm(tm);
      if (c && c == curtime.c_str()+curtime.length())
        m_nameoffmap.insert(pair<string,int>(tm.tm_zone?string(tm.tm_zone):"", tm.tm_gmtoff/36));
      //if (strToDate(curtime, tm, iCurOff, sFm))
      //  m_nameoffmap.insert(pair<string,int>(tm.tm_zone?string(tm.tm_zone):"", tm.tm_gmtoff/36));
      curtime = time+" -"+sOff;
      if (strToDate(curtime, tm, iCurOff, sFm))
        m_nameoffmap.insert(pair<string,int>(tm.tm_zone?string(tm.tm_zone):"", tm.tm_gmtoff/36));
    }
  }
}

void TimeZone::dump()
{
  for (unordered_map< string, int >::iterator it=m_nameoffmap.begin();it!=m_nameoffmap.end();it++)
    printf("%s:%d\n",it->first.c_str(),it->second);
}

#if defined unix || defined __unix || defined __unix__ || defined __APPLE__ || defined __MACH__ || defined __linux__ || defined linux || defined __linux || defined __FreeBSD__ || defined __ANDROID__
string exec(const string & cmd) {
  array<char, 128> buffer;
  string result;
  unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe) {
    //throw runtime_error("popen() failed!");
    trace(ERROR, "Failed to run system command '%s'!\n", cmd.c_str());
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return trim_right(result,'\n');
}
#endif

// manually free the memory of GroupProp, as it's not freed in the destructor to improve performance.
void clearGroupPropMap(unordered_map< string,GroupProp > & aggProps)
{
  for (unordered_map< string,GroupProp >::iterator it=aggProps.begin(); it!=aggProps.end(); ++it)
    it->second.clear();
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
string readQuotedStr(const string & str, size_t& pos, const string & targetquoters, const string & quoters, const char & escape, std::set<char> nestedQuoters)
{
  string trimmedStr="";
  if (targetquoters.length()<2)
    return trimmedStr;
  size_t quoteStart = -1, i = pos, quoteDeep=0;
  bool quoted = false;
  vector<int> q;
  while(i < str.length()) {
    if (quoteDeep > 0){ // checking right quoter only when the string is quoted.
      if (str[i]==escape && i<str.length()-1 && (str[i+1] == targetquoters[0] || str[i+1] == targetquoters[1])){ // skip escape
        i++;
        trimmedStr.push_back(str[i]);
      }
      if (quoteDeep>1 || (str[i]!=targetquoters[1] || q.size()>0))
        trimmedStr.push_back(str[i]);
      if (str[i] == targetquoters[1] && q.size()==0){
        if (i>0 && str[i-1]!=escape){
          quoteDeep--;
          if (quoteDeep == 0){
            pos = i+1;
            //trace(DEBUG2, "Read trimmed string \"%s\" from \"%s\"\n", trimmedStr.c_str(), str.c_str());
            return trimmedStr;
            //return str.substr(quoteStart,pos-quoteStart);
          }
        }
      }else{
        if (q.size()>0 && str[i] == quoters[q[q.size()-1]]) // checking the latest quoter
          if (i>0 && str[i-1]!=escape){
            q.pop_back();
            //trace(DEBUG, "Pop out quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
            ++i;
            continue;
          }
        if (q.size()==0 || nestedQuoters.find(quoters[q[q.size()-1]])!=nestedQuoters.end()) // if not quoted or the latest quoter is a nested quoter, then search quoters
          for (int k=0; k<(int)(quoters.length()/2); k++){
            if (str[i] == quoters[k*2])
              if (k*2+1<quoters.length() && (i==0 || (i>0 && str[i-1]!=escape))){
                //trace(DEBUG, "Found quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
                q.push_back(k*2+1); // quoted start, need to pair the right quoter
                break;
              }
          }
      }
    }
    if (str[i] == targetquoters[0] && q.size()==0)
      if (i==0 || (i>0 && str[i-1]!=escape)){
        quoteDeep++;
        if (quoteDeep == 1)
          quoteStart = i;
      }
    i++;
  }
  pos = -1; // not find
  return "";
}

// find the Nth position of the any character in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. Nested quoters like "()" can quote other quoters, while any other quoters in unnested quoters like ''{}// should be ignored.
size_t findNthCharacter(const string & str, const std::set<char> & lookfor, const size_t & pos, const int & seq, const bool & forward, const string & quoters, const char & escape, const std::set<char> & nestedQuoters)
{
  //trace(DEBUG, "findFirstCharacter '%s', quoters: '%s' !\n",str.c_str(),quoters.c_str());
  int iFound=0;
  size_t i = pos;
  vector<int> q;
  while(i>=0 && i < str.length()) {
    if (lookfor.find(str[i]) != lookfor.end() && i>0 && q.size()==0){
      iFound++;
      if (iFound>=seq)
        return i;
    }else{
      if (q.size()>0 && str[i] == quoters[q[q.size()-1]]) // checking the latest quoter
        if (i>0 && str[i-1]!=escape){
          q.pop_back();
          //trace(DEBUG, "Pop out quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
          forward?++i:--i;
          continue;
        }
      if (q.size()==0 || nestedQuoters.find(quoters[q[q.size()-1]])!=nestedQuoters.end()) // if not quoted or the latest quoter is a nested quoter, then search quoters
        for (int k=0; k<(int)(quoters.length()/2); k++){
          if (str[i] == quoters[k*2])
            if (k*2+1<quoters.length() && (i==0 || (i>0 && str[i-1]!=escape))){
              //trace(DEBUG, "Found quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
              q.push_back(k*2+1); // quoted start, need to pair the right quoter
              break;
            }
        }
    }
    forward?++i:--i;
  }
  if (q.size() > 0)
    trace(ERROR, "(1)Quoters in '%s' are not paired!\n",str.c_str());
  return -1;
}

// find the first position of the any character in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. Nested quoters like "()" can quote other quoters, while any other quoters in unnested quoters like ''{}// should be ignored.
size_t findFirstCharacter(const string & str, const std::set<char> & lookfor, const size_t & pos, const string & quoters, const char & escape, const std::set<char> & nestedQuoters)
{
  return findNthCharacter(str, lookfor, pos, 1, true, quoters, escape, nestedQuoters);
  //trace(DEBUG, "findFirstCharacter '%s', quoters: '%s' !\n",str.c_str(),quoters.c_str());
  size_t i = pos;
  vector<int> q;
  while(i < str.length()) {
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
      for (int k=0; k<(int)(quoters.length()/2); k++){
        if (str[i] == quoters[k*2])
          if (k*2+1<quoters.length() && (i==0 || (i>0 && str[i-1]!=escape))){
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

// find the first position of a substring in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. Nested quoters like "()" can quote other quoters, while any other quoters in unnested quoters like ''{}// should be ignored.
size_t findNthSub(const string & str, const string & lookfor, const size_t & pos, const int & seq, const bool & forward, const string & quoters,  const char & escape, const std::set<char> & nestedQuoters, bool casesensitive)
{
  trace(DEBUG, "findNthSub '%s'\n",str.c_str());
  trace(DEBUG, "looking for(%d): '%s', start from %d, quoters '%s' !\n",seq,lookfor.c_str(),pos,quoters.c_str());
  int iFound=0;
  size_t i = pos;
  vector<int> q;
  while(i != string::npos && i < str.length()) {
    size_t l=i,r=forward?0:lookfor.length()-1;
    while(q.size()==0 && l != string::npos && r != string::npos && l<str.length() && r<lookfor.length() && ((casesensitive?str[l]:upper_char(str[l])) == (casesensitive?lookfor[r]:upper_char(lookfor[r])))){
      forward?++l:--l;
      forward?r++:r--;
    }
    if ( forward?r==lookfor.length():r==string::npos ) {
      iFound++;
      if (iFound>=seq)
        return i;
    }else{
      if (q.size()>0 && str[i] == quoters[q[q.size()-1]]) // checking the latest quoter
        if (i>0 && str[i-1]!=escape){
          q.pop_back();
          //trace(DEBUG, "Pop out quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
          forward?++i:--i;
          continue;
        }
      if (q.size()==0 || nestedQuoters.find(quoters[q[q.size()-1]])!=nestedQuoters.end()) // if not quoted or the latest quoter is a nested quoter, then search quoters
        for (int k=0; k<(int)(quoters.size()/2); k++){
          if (str[i] == quoters[k*2])
            if (k*2+1<quoters.length() && (i==0 || (i>0 && str[i-1]!=escape))){
              //trace(DEBUG, "Found quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
              q.push_back(k*2+1); // quoted start, need to pair the right quoter
              break;
            }
        }
    }
    forward?++i:--i;
  }
  if (q.size() > 0)
    trace(ERROR, "(1)Quoters in '%s' are not paired!\n",str.c_str());
  return -1;
}

// find the first position of a substring in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. Nested quoters like "()" can quote other quoters, while any other quoters in unnested quoters like ''{}// should be ignored.
size_t findFirstSub(const string & str, const string & lookfor, const size_t & pos, const string & quoters,  const char & escape, const std::set<char> & nestedQuoters, bool casesensitive)
{
  return findNthSub(str, lookfor, pos, 1, true, quoters, escape, nestedQuoters, casesensitive);
  trace(DEBUG, "findFirstSub '%s'\n",str.c_str());
  trace(DEBUG, "looking for: '%s', start from %d, quoters '%s' !\n",lookfor.c_str(),pos,quoters.c_str());
  size_t i = pos;
  vector<int> q;
  while(i < str.length()) {
    size_t l=i,r=0;
    while(q.size()==0 && l<str.length() && r<lookfor.length() && ((casesensitive?str[l]:upper_char(str[l])) == (casesensitive?lookfor[r]:upper_char(lookfor[r])))){
      l++;
      r++;
    }
    if (r==lookfor.length()) {
      trace(DEBUG, "found at %d !\n",i);
      return i;
    }
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
          if (k*2+1<quoters.length() && (i==0 || (i>0 && str[i-1]!=escape))){
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

string readLine(string str, size_t & pos)
{
  string sReturn="";
  size_t origpos = pos;
  while(pos<str.length() && str[pos]!='\n'){
    sReturn.push_back(str[pos]);
    pos++;
  }
  if (pos<str.length() && str[pos]=='\n')
    pos++;
  else{
    pos = origpos;
    sReturn = "";
  }
  return sReturn;
}

string readWordTillStop(const string & str, size_t & pos, char stopper, char escape)
{
  string sub="";
  while(pos<str.length() && str[pos]!=stopper){
    if (str[pos] == escape && pos<str.length()-1 && str[pos+1] == stopper)
      pos++;
    sub.push_back(str[pos]);
    pos++;
  }
  return sub;
}

vector<string> matchWildcard(const string & str, const string & wildStr, string quoters, char escape, std::set<char> nestedQuoters)
{
  vector<string> matches;
  size_t iBPos=0, iWPos=0;
  bool bToMatch;
  while (iWPos<wildStr.length() && iBPos<str.length()){
    bToMatch = false;
    if (wildStr[iWPos]=='*'){
      bToMatch = true;
      while(wildStr[iWPos]=='*')
        iWPos++;
    }
    string sub = readWordTillStop(wildStr,iWPos,'*','\\');
    if (bToMatch){
      if (!sub.empty()){
        size_t iEPos = findFirstSub(str,sub,iBPos,quoters,escape,nestedQuoters,true);
        if (iEPos != string::npos)
          matches.push_back(str.substr(iBPos,iEPos-iBPos));
        iBPos=iEPos+sub.length();
      }else{
        matches.push_back(str.substr(iBPos));
        iBPos = str.length();
      }
    }
  }
  return matches;
}

void replaceunquotedstr(string & str, const string & sReplace, const string & sNew, string quoters, char escape, std::set<char> nestedQuoters)
{
  size_t i = 0;
  string sReturn="";
  vector<int> q;
  while(i < str.length()) {
    bool bReplaced = false;
    if (str.substr(i,sReplace.length()).compare(sReplace) == 0 && i>=0 && q.size()==0) {
      sReturn.append(sNew);
      bReplaced = true;
      i+=(sReplace.length()-1);
    }else{
      if (str[i]==escape && i<str.length()-1 && (quoters.find(str[i+1]) >= 0 || nestedQuoters.find(str[i+1]) != nestedQuoters.end())){ // skip escape
        sReturn.push_back(str[i]);
        i++;
      }
    }
    if (q.size()>0 && str[i] == quoters[q[q.size()-1]]) // checking the latest quoter
      if (i>0 && str[i-1]!=escape){
        //trace(DEBUG, "Pop out quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
        q.pop_back();
        sReturn.push_back(str[i]);
        ++i;
        continue;
      }
    if (q.size()==0 || nestedQuoters.find(quoters[q[q.size()-1]])!=nestedQuoters.end()) // if not quoted or the latest quoter is a nested quoter, then search quoters
      for (int k=0; k<(int)(quoters.length()/2); k++){
        if (str[i] == quoters[k*2])
          if (k*2+1<quoters.length() && (i==0 || (i>0 && str[i-1]!=escape))){
            //trace(DEBUG, "Found quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
            q.push_back(k*2+1); // quoted start, need to pair the right quoter
            break;
          }
      }
    if (!bReplaced)
      sReturn.push_back(str[i]);
    ++i;
  }
  if (q.size() > 0)
    trace(ERROR, "(2)Quoters in '%s' are not paired!\n",str.c_str());
  str = sReturn;
}

vector<string> split(const string & str, std::set<char> delims, string quoters, char escape, std::set<char> nestedQuoters, bool repeatable, bool skipemptyelement)
{
  trace(DEBUG, "Splitting string:'%s', quoters: '%s'\n",str.c_str(), quoters.c_str());
  vector<string> v;
  size_t i = 0, j = 0, begin = 0;
  vector<int> q;
  while(i < str.length()) {
    if (delims.find(str[i])!=delims.end() && (i==0 || (i>0 && str[i-1]!=escape)) && q.size()==0) {
      trace(DEBUG, "(1)found delim '%s', split string:'%s' (%d to %d)\n",str.substr(i,1).c_str(),str.substr(begin, i-begin).c_str(), begin, i);
      if (!skipemptyelement || i>begin)
        v.push_back(str.substr(begin, i-begin));
      while(repeatable && i<str.length()-1 && delims.find(str[i+1])!=delims.end()) // skip repeated delim
        i++;
      begin = i+1;
    }else{
      if (str[i]==escape && i<str.length()-1 && (quoters.find(str[i+1]) >= 0 || nestedQuoters.find(str[i+1]) != nestedQuoters.end())) // skip escape
        i++;
    }
    if (q.size()>0 && str[i] == quoters[q[q.size()-1]]) // checking the latest quoter
      if (i>0 && str[i-1]!=escape){
        //trace(DEBUG, "Pop out quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
        q.pop_back();
        ++i;
        continue;
      }
    if (q.size()==0 || nestedQuoters.find(quoters[q[q.size()-1]])!=nestedQuoters.end()) // if not quoted or the latest quoter is a nested quoter, then search quoters
      for (int k=0; k<(int)(quoters.length()/2); k++){
        if (str[i] == quoters[k*2])
          if (k*2+1<quoters.length() && (i==0 || (i>0 && str[i-1]!=escape))){
            //trace(DEBUG, "Found quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
            q.push_back(k*2+1); // quoted start, need to pair the right quoter
            break;
          }
      }
    ++i;
  }
  trace(DEBUG, "(1)Split the end of the string:%d/%d\n",begin, str.length());
  if (!skipemptyelement || str.length()>begin)
    v.push_back(str.length()>begin?str.substr(begin, str.length()-begin):"");
  //if (begin<str.length() && (!skipemptyelement || str.length()>begin))
  //  v.push_back(str.substr(begin, str.length()-begin));

  if (q.size() > 0)
    trace(ERROR, "(2)Quoters in '%s' are not paired!\n",str.c_str());

  return v;
}

// split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting. Nested quoters like "()" can quote other quoters, while any other quoters in unnested quoters like ''{}// should be ignored.
vector<string> split(const string & str, char delim, string quoters, char escape, std::set<char> nestedQuoters, bool repeatable, bool skipemptyelement) 
{
  vector<string> v;
  size_t i = 0, j = 0, begin = 0;
  vector<int> q;
  while(i < str.length()) {
    if (str[i] == delim && (i==0 || (i>0 && str[i-1]!=escape)) && q.size()==0) {
    //if (str[i] == delim && q.size()==0) {
      //trace(DEBUG, "(1)found delim, split string:%s (%d to %d)\n",str.substr(begin, i-begin).c_str(), begin, i);
      if (!skipemptyelement || i>begin)
        v.push_back(str.substr(begin, i-begin));
      while(repeatable && i<str.length()-1 && str[i+1] == delim) // skip repeated delim
        i++;
      begin = i+1;
    }else{
      if (str[i]==escape && i<str.length()-1 && (quoters.find(str[i+1]) >= 0 || nestedQuoters.find(str[i+1]) != nestedQuoters.end())) // skip escape
        i++;
    }
    if (q.size()>0 && str[i] == quoters[q[q.size()-1]]) // checking the latest quoter
      if (i>0 && str[i-1]!=escape){
        //trace(DEBUG, "Pop out quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
        q.pop_back();
        ++i;
        continue;
      }
    if (q.size()==0 || nestedQuoters.find(quoters[q[q.size()-1]])!=nestedQuoters.end()) // if not quoted or the latest quoter is a nested quoter, then search quoters
      for (int k=0; k<(int)(quoters.length()/2); k++){
        if (str[i] == quoters[k*2])
          if (k*2+1<quoters.length() && (i==0 || (i>0 && str[i-1]!=escape))){
            //trace(DEBUG, "Found quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
            q.push_back(k*2+1); // quoted start, need to pair the right quoter
            break;
          }
      }
    ++i;
  }
  trace(DEBUG, "(2)Split the end of the string:%d/%d\n",begin, str.length());
  if (!skipemptyelement || str.length()>begin)
    v.push_back(str.length()>begin?str.substr(begin, str.length()-begin):"");
  //if (begin<str.length() && (!skipemptyelement || str.length()>begin))
  //  v.push_back(str.substr(begin, str.length()-begin));

  if (q.size() > 0)
    trace(ERROR, "(2)Quoters in '%s' are not paired!\n",str.c_str());

  return v;
}

// split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting. Nested quoters like "()" can quote other quoters, while any other quoters in unnested quoters like ''{}// should be ignored.
vector<string> split(const string & str, string delim, string quoters, char escape, std::set<char> nestedQuoters, bool repeatable, bool skipemptyelement) 
{
  if (delim.length()==1)
    return split(str,delim[0],quoters,escape,nestedQuoters,repeatable,skipemptyelement);
  vector<string> v;
  string upDelim = upper_copy(delim);
  size_t i = 0, j = 0, begin = 0;
  vector<int> q;
  while(i < str.length()) {
    if (str.length()>=i+upDelim.length() && upper_copy(str.substr(i,upDelim.length())).compare(upDelim)==0 && i>=0 && q.size()==0) {
      //trace(DEBUG, "(2)found delim, split string:%s (%d to %d)\n",str.substr(begin, i-begin).c_str(), begin, i);
      if (!skipemptyelement || i>begin)
        v.push_back(str.substr(begin, i-begin));
      while(repeatable && str.length()>=i+upDelim.length()*2 && upper_copy(str.substr(i+upDelim.length(),upDelim.length())).compare(upDelim)==0) // skip repeated delim
        i+=upDelim.length();
      begin = i+upDelim.length();
      i+=(upDelim.length()-1);
    }else{
      if (str[i]==escape && i<str.length()-1 && (quoters.find(str[i+1]) >= 0 || nestedQuoters.find(str[i+1]) != nestedQuoters.end())) // skip escape
        i++;
    }
    if (q.size()>0 && str[i] == quoters[q[q.size()-1]]) // checking the latest quoter
      if (i>0 && str[i-1]!=escape){
        //trace(DEBUG, "Pop out quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
        q.pop_back();
        ++i;
        continue;
      }
    if (q.size()==0 || nestedQuoters.find(quoters[q[q.size()-1]])!=nestedQuoters.end()) // if not quoted or the latest quoter is a nested quoter, then search quoters
      for (int k=0; k<(int)(quoters.length()/2); k++){
        if (str[i] == quoters[k*2])
          if (k*2+1<quoters.length() && (i==0 || (i>0 && str[i-1]!=escape))){
            //trace(DEBUG, "Found quoter <%s>(%d) !\n",str.substr(i,1).c_str(),i);
            q.push_back(k*2+1); // quoted start, need to pair the right quoter
            break;
          }
      }
    ++i;
  }
  trace(DEBUG, "(2)Split the end of the string:%d/%d\n",begin, str.length());
  if (!skipemptyelement || str.length()>begin)
    v.push_back(str.length()>begin?str.substr(begin, str.length()-begin):"");
  //if (begin<str.length() && (!skipemptyelement || str.length()>begin))
  //  v.push_back(str.substr(begin, str.length()-begin));

  if (q.size() > 0)
    trace(ERROR, "(2)Quoters in '%s' are not paired!\n",str.c_str());

  return v;
}

string trim_pair(const string & str, const string & pair)
{
  if(str.length() > 1 && pair.length() == 2 && str[0] == pair[0] && str[str.length()-1] == pair[1])
    return str.substr(1,str.length()-2);
  else
    return str;
}

string trim(const string & str, char c, bool repeat)
{
  //string newstr = trim_one(str, c);
  //while (newstr.length() != str.length())
  //  newstr = trim_one(str, c);
  //return newstr;
  return trim_left(trim_right(str,c,repeat),c,repeat);
}

string trim_right(const string & str, char c, bool repeat)
{
  size_t i = str.length()-1;
  while ((str[i] == c || (c==' ' && str[i] == '\t')) && (i == str.length()-1 || repeat))
    i--;
  return str.substr(0,i+1);
}

string trim_left(const string & str, char c, bool repeat)
{
  size_t i = 0;
  while ((str[i] == c || (c==' ' && str[i] == '\t')) && (i == 0 || repeat))
    i++;
  return str.substr(i);
}

string trim_one(const string & str, char c)
{
  string newstr = str;
  if (newstr[0] == c)
    newstr = newstr.substr(1);
  if (newstr[newstr.length()-1] == c)
    newstr = newstr.substr(0,newstr.length()-1);
  if (c == ' ')
    newstr = trim_one(str, '\t');
  return newstr;
}

// count occurences of substr in str
int countstr(const string & str, const string & substr)
{
  int pos=0, count=0;
  //trace(DEBUG, "counting '%s','%s' ...",str.c_str(),substr.c_str());
  pos = str.find(substr, pos);
  while (pos != string::npos){
    count++;
    pos = str.find(substr, pos+substr.length());
  }
  return count;
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

void replacestr(string & sRaw, const vector<string> & vReplace, const vector<string> & vNew)
{
  if (vReplace.size() != vNew.size()){
    trace(ERROR, "Replace string array size %d doesnot match new string array size %d! \n", vReplace.size(), vNew.size());
    return;
  }
  //for (int i=0; i<vReplace.size(); i++)
  //  replacestr(sRaw,vReplace[i],vNew[i]);
  string newStr="";
  size_t pos=0;
  while (pos<sRaw.length()){
    bool bMatched=false;
    for (int i=0; i<vReplace.size(); i++){
      if (!vReplace[i].empty() && sRaw.length()-pos>=vReplace[i].length() && sRaw.substr(pos,vReplace[i].length()).compare(vReplace[i])==0){
        newStr.append(vNew[i]);
        pos+=vReplace[i].length()-1;
        bMatched=true;
        break;
      }
    }
    if (!bMatched)
      newStr.append(sRaw.substr(pos,1));
    pos++;
  }
  sRaw = newStr;
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

char upper_char(const char & c)
{
  if (c>='a' && c<='z')
    return char(int(c)-32);
  else
    return c;
}

char lower_char(const char & c)
{
  if (c>='A' && c<='Z')
    return char(int(c)+32);
  else
    return c;
}

string camelstr(const string & str)
{
  string sCamel = "";
  bool preIsLetter = false;
  for (int i=0; i<str.length(); i++){
    if ((str[i]>='a' && str[i]<='z') || (str[i]>='A' && str[i]<='Z')){
      if (!preIsLetter){
        sCamel.push_back(upper_char(str[i]));
      }else{
        sCamel.push_back(lower_char(str[i]));
      }
      preIsLetter = true;
    }else{
      sCamel.push_back(str[i]);
      preIsLetter = false;
    }
  }
  return sCamel;
}

string snakestr(const string & str)
{
  string sSnake = "";
  for (int i=0; i<str.length(); i++){
    if ((str[i]>='a' && str[i]<='z') || (str[i]>='A' && str[i]<='Z')){
      if (i==0){
        sSnake.push_back(upper_char(str[i]));
      }else{
        sSnake.push_back(lower_char(str[i]));
      }
    }else
      sSnake.push_back(str[i]);
  }
  return sSnake;
}

string revertstr(const string & str)
{
  string reverse = "";
  for (int i=str.length()-1;i>=0;i--)
    reverse.push_back(str[i]);
  return reverse;
}

char from_hex(const char & ch)
{
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

char to_hex(const char & code)
{
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

string urlencode(const string & sUrl)
{
  string ecurl;
  for(int i=0; i<sUrl.length(); i++){
    if (isalnum(sUrl[i]) || sUrl[i] == '-' || sUrl[i] == '_' || sUrl[i] == '.' || sUrl[i] == '~') 
      ecurl.push_back(sUrl[i]);
    else if (sUrl[i] == ' ') 
      ecurl.push_back('+');
    else{ 
      ecurl.push_back('%');
      ecurl.push_back(to_hex(sUrl[i] >> 4));
      ecurl.push_back(to_hex(sUrl[i] & 15));
    }
  }
  return ecurl;
}

string urldecode(const string & sEncoded)
{
  string url;
  for(int i=0; i<sEncoded.length(); i++){
    if (sEncoded[i] == '%') {
      if (sEncoded[i+1] && sEncoded[i+2]) {
        url.push_back(from_hex(sEncoded[i+1]) << 4 | from_hex(sEncoded[i+2]));
        i += 2;
      }
    } else if (sEncoded[i] == '+') { 
      url.push_back(' ');
    } else {
      url.push_back(sEncoded[i]);
    }
  }
  return url;
}

static const string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static inline bool is_base64(unsigned char c) 
{
  return (isalnum(c) || (c == '+') || (c == '/'));
}

string base64encode(const string & str)
{
  string ret;
  int i = 0;
  int j = 0;
  unsigned int p = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];
  unsigned int in_len = str.length();

  while (in_len--) {
    char_array_3[i++] = str[p];
    p++;
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;
}

string base64decode(const string & sEncoded)
{
  int in_len = sEncoded.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  string ret;

  while (in_len-- && ( sEncoded[in_] != '=') && is_base64(sEncoded[in_])) {
    char_array_4[i++] = sEncoded[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

typedef union uwb {
	unsigned w;
	unsigned char b[4];
} MD5union;

typedef unsigned DigestArray[4];

static unsigned func0(unsigned abcd[]){
	return (abcd[1] & abcd[2]) | (~abcd[1] & abcd[3]);
}

static unsigned func1(unsigned abcd[]){
	return (abcd[3] & abcd[1]) | (~abcd[3] & abcd[2]);
}

static unsigned func2(unsigned abcd[]){
	return  abcd[1] ^ abcd[2] ^ abcd[3];
}

static unsigned func3(unsigned abcd[]){
	return abcd[2] ^ (abcd[1] | ~abcd[3]);
}

typedef unsigned(*DgstFctn)(unsigned a[]);

static unsigned *calctable(unsigned *k)
{
	double s, pwr;
	int i;

	pwr = pow(2.0, 32);
	for (i = 0; i<64; i++) {
		s = fabs(sin(1.0 + i));
		k[i] = (unsigned)(s * pwr);
	}
	return k;
}

static unsigned rol(unsigned r, short N)
{
	unsigned  mask1 = (1 << N) - 1;
	return ((r >> (32 - N)) & mask1) | ((r << N) & ~mask1);
}

static unsigned* MD5Hash(string msg)
{
	int mlen = msg.length();
	static DigestArray h0 = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476 };
	static DgstFctn ff[] = { &func0, &func1, &func2, &func3 };
	static short M[] = { 1, 5, 3, 7 };
	static short O[] = { 0, 1, 5, 0 };
	static short rot0[] = { 7, 12, 17, 22 };
	static short rot1[] = { 5, 9, 14, 20 };
	static short rot2[] = { 4, 11, 16, 23 };
	static short rot3[] = { 6, 10, 15, 21 };
	static short *rots[] = { rot0, rot1, rot2, rot3 };
	static unsigned kspace[64];
	static unsigned *k;

	static DigestArray h;
	DigestArray abcd;
	DgstFctn fctn;
	short m, o, g;
	unsigned f;
	short *rotn;
	union {
		unsigned w[16];
		char     b[64];
	}mm;
	int os = 0;
	int grp, grps, q, p;
	unsigned char *msg2;

	if (k == NULL) k = calctable(kspace);

	for (q = 0; q<4; q++) h[q] = h0[q];

	{
		grps = 1 + (mlen + 8) / 64;
		msg2 = (unsigned char*)malloc(64 * grps);
		memcpy(msg2, msg.c_str(), mlen);
		msg2[mlen] = (unsigned char)0x80;
		q = mlen + 1;
		while (q < 64 * grps){ msg2[q] = 0; q++; }
		{
			MD5union u;
			u.w = 8 * mlen;
			q -= 8;
			memcpy(msg2 + q, &u.w, 4);
		}
	}

	for (grp = 0; grp<grps; grp++)
	{
		memcpy(mm.b, msg2 + os, 64);
		for (q = 0; q<4; q++) abcd[q] = h[q];
		for (p = 0; p<4; p++) {
			fctn = ff[p];
			rotn = rots[p];
			m = M[p]; o = O[p];
			for (q = 0; q<16; q++) {
				g = (m*q + o) % 16;
				f = abcd[1] + rol(abcd[0] + fctn(abcd) + k[q + 16 * p] + mm.w[g], rotn[q % 4]);

				abcd[0] = abcd[3];
				abcd[3] = abcd[2];
				abcd[2] = abcd[1];
				abcd[1] = f;
			}
		}
		for (p = 0; p<4; p++)
			h[p] += abcd[p];
		os += 64;
	}

	return h;
}

string md5(const string & str)
{
	string md5str;
	int j, k;
	unsigned *d = MD5Hash(str);
	MD5union u;
	for (j = 0; j<4; j++){
		u.w = d[j];
		char s[9];
		sprintf(s, "%02x%02x%02x%02x", u.b[0], u.b[1], u.b[2], u.b[3]);
		md5str += s;
	}

	return md5str;
}

string hashstr(const string & str)
{
  return longuintToStr((long unsigned int)hash<string>{}(str));
}

int random(int min, int max)
{
  //srand( (unsigned)time(NULL) );
  //return rand()%max+min;
  random_device dev;
  mt19937 rng(dev());
  uniform_int_distribution<mt19937::result_type> distribute(min,max);
  return distribute(rng);
}

// Generate a random string. len: string length (default 8); flags (default uld) includes: u:upper alphabet;l:lower alphabet;d:digit;m:minus;n:unlderline;s:space;x:special(`~!@#$%^&*+/\|;:'"?/);b:Brackets([](){}<>); A lower flag stands for optional choice, a upper flag stands for compulsory choice
string randstr(int len, const string & flags)
{
  string genstr = "";
  if (flags.empty()){
    trace(ERROR,"Flag of randstr cannot be empty!\n");
    return genstr;
  }
  std::set<char> validflags = {'U','u','L','l','D','d','M','m','U','u','S','s','X','x','B','b'};
  string upper = "ABCDEFGHIJKLMOPQRSTUVWXYZ";
  string lower = "abcdefghijklmopqrstuvwxyz";
  string digit = "0123456789";
  string minus = "-";
  string unlderline = "-";
  string space = " ";
  string special = "`~!@#$%^&*+/\\|;:'\"?/";
  string bracket = "[](){}<>";
  
  // generate compulsory characters first
  for (int i=0;i<flags.length();i++){
    if (validflags.find(flags[i])==validflags.end()){
      trace(ERROR,"'%s' is a invlid flag of randstr!\n",flags.substr(i,1).c_str());
      return genstr;
    }
    switch(flags[i]){
    case 'U':
      genstr.push_back(upper[random(0,upper.length()-1)]);
      if (genstr.length()>=len)
        return genstr;
      break;
    case 'L':
      genstr.push_back(lower[random(0,lower.length()-1)]);
      if (genstr.length()>=len)
        return genstr;
      break;
    case 'D':
      genstr.push_back(digit[random(0,digit.length()-1)]);
      if (genstr.length()>=len)
        return genstr;
      break;
    case 'M':
      genstr.push_back('-');
      if (genstr.length()>=len)
        return genstr;
      break;
    case 'N':
      genstr.push_back('_');
      if (genstr.length()>=len)
        return genstr;
      break;
    case 'S':
      genstr.push_back(' ');
      if (genstr.length()>=len)
        return genstr;
      break;
    case 'X':
      genstr.push_back(special[random(0,special.length()-1)]);
      if (genstr.length()>=len)
        return genstr;
      break;
    case 'B':
      genstr.push_back(bracket[random(0,bracket.length()-1)]);
      if (genstr.length()>=len)
        return genstr;
      break;
    }
  }
  char flag;
  for (int i=genstr.length()-1;i<len;i++){
    flag = flags[random(0,flags.length()-1)];
    switch(flag){
    case 'U':
    case 'u':
      genstr.push_back(upper[random(0,upper.length()-1)]);
      break;
    case 'L':
    case 'l':
      genstr.push_back(lower[random(0,lower.length()-1)]);
      break;
    case 'D':
    case 'd':
      genstr.push_back(digit[random(0,digit.length()-1)]);
      break;
    case 'M':
    case 'm':
      genstr.push_back('-');
      break;
    case 'N':
    case 'n':
      genstr.push_back('_');
      break;
    case 'S':
    case 's':
      genstr.push_back(' ');
      break;
    case 'X':
    case 'x':
      genstr.push_back(special[random(0,special.length()-1)]);
      break;
    case 'B':
    case 'b':
      genstr.push_back(bracket[random(0,bracket.length()-1)]);
      break;
    }
  }
}

// concatenate an array to string
string concatArray(const vector<string> & array, const string & delim)
{
  string sResult = "";
  for (int i=0; i<array.size(); i++){
    sResult+=array[i];
    if (i<array.size()-1)
      sResult+=delim;
  }
  return sResult;
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

string longlongToStr(const long long val)
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

string longuintToStr(const long unsigned int val)
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

string dectohex(const int & val)
{
  string hex;
  int raw, d;
  if (val == 0)
    return "0";
  else if (val<0)
    raw = val*-1;
  else
    raw = val;
  while(raw>0){
    d = raw%16;
    raw=(raw-d)/16;
    if (d<10) // 0-9 ascii + 48
      hex.push_back(char(d+48));
    else // A-F ascii + 55
      hex.push_back(char(d+55));
  }
  if (val<0)
    hex.push_back('-');
  hex = revertstr(hex);
  return hex;
}

int hextodec(const string& str)
{
  int val = 0;
  for (int i=str.length()-1; i>=0; i--){
    if (str[i]>='0' && str[i]<='9')
      val = val+(int(str[i])-48)*int(pow(16,(str.length()-1)-i));
    else if (str[i]>='a' && str[i]<='f')
      val = val+(int(str[i])-87)*int(pow(16,(str.length()-1)-i));
    else if (str[i]>='A' && str[i]<='F')
      val = val+(int(str[i])-55)*int(pow(16,(str.length()-1)-i));
    else{
      trace(ERROR, "'%s' is not a valid hex number!\n", str.c_str());
      return 0;
    }
  }
  return val;
}

string dectobin(const int & val)
{
  string bin;
  int raw, d;
  if (val == 0)
    return "0";
  else if (val<0)
    raw = val*-1;
  else
    raw = val;
  while(raw>0){
    d = raw%2;
    raw=(raw-d)/2;
    bin.push_back(char(d+48));
  }
  if (val<0)
    bin.push_back('-');
  bin = revertstr(bin);
  return bin;
}

int bintodec(const string& str)
{
  int val = 0;
  for (int i=str.length()-1; i>=0; i--){
    if (str[i]>='0' && str[i]<='1')
      val = val+(int(str[i])-48)*int(pow(2,(str.length()-1)-i));
    else{
      trace(ERROR, "'%s' is not a valid binary number!\n", str.c_str());
      return 0;
    }
  }
  return val;
}

void cleanuptm(struct tm & tm)
{
  if (abs(tm.tm_gmtoff) > 43200)
    tm.tm_gmtoff = 0;
  if (abs(tm.tm_sec > 60))
    tm.tm_sec = 0;
  if (abs(tm.tm_min > 60))
    tm.tm_min = 0;
  if (abs(tm.tm_hour > 60))
    tm.tm_hour = 0;
  if (abs(tm.tm_mday > 32))
    tm.tm_mday = 1;
  if (abs(tm.tm_mon > 12))
    tm.tm_mday = 0;
}

#if defined __MINGW32__ || defined __CYGWIN32__ || defined __CYGWIN64__ || defined __CYGWIN__
extern int putenv(char*);
#endif

// get the local time in the specific timezone
struct tm zonetime(time_t t1, string zone)
{
  char const* tmp = getenv( "TZ" );
  trace(DEBUG2, "(zonetime) begin TZ: %s, offset: %d \n", tmp, timezone);
  struct tm tm;
  string sNewTimeZone = zone;
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
  if (tmp)
    putenv(const_cast<char *>(string("TZ="+string(tmp)).c_str()));
  else
    unsetenv("TZ");
  system("set"); 
#elif defined _WIN32 || defined _WIN64
  if (tmp)
    putenv(("TZ="+string(tmp)).c_str());
  else
    unsetenv("TZ");
  tzset();
#else
  if (tmp)
    setenv("TZ", tmp, 1);
  else
    unsetenv("TZ");
  tzset();
#endif
  trace(DEBUG2, "(zonetime) end TZ: %s, offset: %d\n", getenv( "TZ" ), timezone);
  return tm;
}

// get the local time in the specific timezone
struct tm zonetime(time_t t1, int iOffSet)
{
  struct tm tm;
  if (iOffSet>=-1200 && iOffSet<=1200){
    string sNewTimeZone = "GMT"+intToStr(iOffSet/100*-1);
    tm = zonetime(t1, sNewTimeZone);
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
    int off = iOffSet;
    //if (off==0 && fmt.find("%z") == string::npos) // no timezone provided, use local timezone.
    //  off = localOffset();
    struct tm tm = zonetime(t1, off);
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
  while (((sRaw[iTZ]!='+'&&sRaw[iTZ]!='-') || (iTZ>0&&sRaw[iTZ-1]!=' ')) && iTZ<sRaw.length()){
    iTZ++;
    if (iTZ>0 && sRaw[iTZ-1]==' ' && (sRaw[iTZ]=='+'||sRaw[iTZ]=='-')){
      int opos = iTZ+1;
      while (opos<sRaw.length()&&sRaw[opos]>='0'&&sRaw[opos]<='9')
        opos++;
      sTimeZone = sRaw.substr(iTZ,opos-iTZ);
      if (isInt(sTimeZone)){
        iOffSet = atoi(sTimeZone.c_str());
        replacestr(sRaw,sTimeZone,"");
        sRaw = trim_copy(sRaw);
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

int localOffset()
{
  time_t t = time(NULL);
  struct tm lt = {0};

  localtime_r(&t, &lt);
  return lt.tm_gmtoff/36;
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
  //bool localTime = false;
  //short int iOffOp = PLUS;
  sRaw = stripTimeZone(str, iOffSet, sTimeZone);
  size_t zpos = sFm.find("%z");
  if (sFm.length()>5 && zpos != string::npos){
    // trace(DEBUG, "Trying timezone '%s' : '%s'\n", fmt.c_str(), sRaw.c_str());
    replacestr(sFm,"%z","");
    sFm = trim_copy(sFm);
  }else{
    if (!sTimeZone.empty()) { // no %z in the format, there should no timezone info in the datetime string
      //trace(DEBUG2, "'%s' (format: '%s') should not contain timezone '%s' \n", str.c_str(), sFm.c_str(), sTimeZone.c_str());
      return false;
    }
    //iOffSet = localOffset();
    //localTime = true;
  }
  // bare in mind: strptime will ignore %z. means we need to treat its returned time as GMT time
  char * c = strptime(sRaw.c_str(), sFm.c_str(), &tm);
  // if the date lack of some time info, e.g. 2022-11-18 dont have time info, the time info like tm_sec in tm will be a random number. need to use cleanuptm to do cleanup.
  cleanuptm(tm);
  if (c && c == sRaw.c_str()+sRaw.length()){
  //if (c && string(c).empty()){
    //trace(DEBUG2, "(1)Converting '%s' => %d %d %d %d %d %d %d offset %d format '%s' \n",sRaw.c_str(),tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_isdst, iOffSet, sFm.c_str());
    tm.tm_isdst = 0;
    //string sDate = dateToStr(tm, 0, sFm); // as tm got from strptime ignored offset, need to set offset to 0
    //trace(DEBUG2, "(2)Converting '%s' (%s) get '%s' \n",sRaw.c_str(), sFm.c_str(), sDate.c_str());
    //if (sDate.compare(sRaw) != 0)
    //  return false;
    if (iOffSet != 0){
      addhours(tm, iOffSet/100*-1);
      //tm.tm_hour = tm.tm_hour+(iOffSet/100*-1);
    }
    //if (localTime)
    //  iOffSet = 0;
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
        if (str.length()<=10){
          if (strToDate(str, tm, iOffSet, (*it))){
            fmt = (*it);
            return true;
          }
        }else{ 
          for (std::set<string>::iterator ij = alljunction.begin(); ij != alljunction.end(); ++ij) {
            for (std::set<string>::iterator iz = alltzfmt.begin(); iz != alltzfmt.end(); ++iz) {
              //trace(DEBUG2, "Checking '%s' date format: '%s' and '%s'...\n", str.c_str(), string((*id)+(*ij)+(*it)+(*iz)).c_str(), string((*it)+(*ij)+(*id)+(*iz)).c_str());
              if (strToDate(str, tm, iOffSet, string((*id)+(*ij)+(*it)+(*iz)))){
                fmt = string((*id)+(*ij)+(*it)+(*iz));
                //trace(DEBUG2, "(1)'%s' Got date format: %s\n", str.c_str(), fmt.c_str());
                return true;
              }else if (strToDate(str, tm, iOffSet, string((*it)+(*ij)+(*id)+(*iz)))){
                fmt = string((*it)+(*ij)+(*id)+(*iz));
                //trace(DEBUG2, "(2)'%s' Got date format: %s\n", str.c_str(), fmt.c_str());
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

double timediff(struct tm & tm1, struct tm & tm2)
{
  time_t t1 = mktime(&tm1);
  time_t t2 = mktime(&tm2);
  return difftime(t1, t2);
}

void addhours(struct tm & tm, int hours)
{
  tm.tm_hour = tm.tm_hour+hours;
  while (tm.tm_hour<0){
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

  while (tm.tm_hour>24){
    tm.tm_hour -= 24;
    tm.tm_mday += 1;
    switch(tm.tm_mon){
      case 0:
      case 2:
      case 4:
      case 6:
      case 7:
      case 9:
        if (tm.tm_mday>31){
          tm.tm_mday=1;
          tm.tm_mon+=1;
        }
        break;
      case 11:
        if (tm.tm_mday>31){
          tm.tm_mday=1;
          tm.tm_mon=1;
          tm.tm_year+=1;
        }
        break;
      case 3:
      case 5:
      case 8:
      case 10:
        if (tm.tm_mday>30){
          tm.tm_mday=1;
          tm.tm_mon+=1;
        }
        break;
      case 1:
        if ((tm.tm_year+1900)%400==0 || ((tm.tm_year+1900)%4==0&&(tm.tm_year+1900)%100!=0)){
          if (tm.tm_mday>29){
            tm.tm_mday=1;
            tm.tm_mon+=1;
          }
        }else{
          if (tm.tm_mday>28){
            tm.tm_mday=1;
            tm.tm_mon+=1;
          }
        }
        break;
    }
  }
}

void addseconds(struct tm & tm, int seconds)
{
  tm.tm_sec = tm.tm_sec+seconds;
  while (tm.tm_sec>=60){
    tm.tm_sec -= 60;
    tm.tm_min += 1;
    if (tm.tm_min >= 60){
      tm.tm_min -= 60;
      addhours(tm, 1);
    }
  }
  while (tm.tm_sec<0){
    tm.tm_sec += 60;
    tm.tm_min -= 1;
    if (tm.tm_min < 0){
      tm.tm_min += 60;
      addhours(tm, -1);
    }
  }
}

void addtime(struct tm & tm, int diff, char unit)
{
  switch (unit){
  case 'y':
  case 'Y':
    tm.tm_year += diff;
    break;
  case 'n':
  case 'N':
    tm.tm_mon += diff;
    while (tm.tm_mon<=0){
      tm.tm_mon += 12;
      tm.tm_year -= 1;
    }
    while (tm.tm_mon>12){
      tm.tm_mon -= 12;
      tm.tm_year += 1;
    }
    break;
  case 'd':
  case 'D':
    addhours(tm, diff*24);
    break;
  case 'h':
  case 'H':
    addhours(tm, diff);
    break;
  case 'm':
  case 'M':
    addseconds(tm, diff*60);
    break;
  case 's':
  case 'S':
  default:
    addseconds(tm, diff);
  }
}

string truncdate(const string & datesrc, const string & fmt, const int & seconds)
{
  struct tm tm;
  string sResult, sFmt = fmt;
  int iOffSet;
  if (sFmt.empty() && !isDate(datesrc, iOffSet, sFmt))
    trace(ERROR, "(truncdate) '%s' is a invalid date format or '%s' is not a correct date!\n", sFmt.c_str(), datesrc.c_str());
  else if (strToDate(datesrc, tm, iOffSet, sFmt)){
    time_t t1 = mktime(&tm) - timezone; // adjust timezone
    time_t t2 = (time_t)(trunc((long double)t1/seconds))*seconds - timezone; // adjust timezone for gmtime
    trace(DEBUG2, "(truncdate)(a)Truncating '%s' (%d) %d %d %d %d %d %d; t1: %d seconds: %d t2: %d timezone: %d \n",datesrc.c_str(), iOffSet,tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, (long)t1, seconds, (long) t2, timezone);
    //tm = *(localtime(&t2));
    tm = *(gmtime(&t2));
    //tm = zonetime(t2, 0); // as tm returned from strToDate is GMT time
    sResult = dateToStr(tm, iOffSet, sFmt);
    trace(DEBUG2, "(truncdate)(b)Truncating '%s' (%d) => '%s' (%d %d %d %d %d %d) timezone: %d \n",datesrc.c_str(),iOffSet, sResult.c_str(), tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, timezone);
    //trace(DEBUG, "Truncating seconds %d from '%s'(%u) get '%s'(%u), format:%s\n", seconds, datesrc.c_str(), (long)t1, sResult.c_str(), (long)t2, dts.extrainfo.c_str());
  }else{
    trace(ERROR, "(truncdate)'%s' is not a correct date!\n", datesrc.c_str());
  }
  return sResult;
}

string truncdateu(const string & datesrc, const string & fmt, const char & unit)
{
  struct tm tm;
  string sResult, sFmt = fmt;
  int iOffSet;
  if (sFmt.empty() && !isDate(datesrc, iOffSet, sFmt))
    trace(ERROR, "(truncdateu) '%s' is a invalid date format or '%s' is not a correct date!\n", sFmt.c_str(), datesrc.c_str());
  else if (strToDate(datesrc, tm, iOffSet, sFmt)){
    switch(unit){
      case 'b':
      case 'B':
        tm.tm_mon = 0;
        tm.tm_mday = 1;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        // adjust timezone
        addhours(tm, -1*(tm.tm_hour+(iOffSet/100)));
        //if (iOffSet<0)
        //  addtime(tm, 1, 'd');
        break;
      case 'd':
      case 'D':
      default:
        tm.tm_mday = 1;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        // adjust timezone
        addhours(tm, -1*(tm.tm_hour+(iOffSet/100)));
        //if (iOffSet<0)
        //  addtime(tm, 1, 'd');
        break;
      case 'h':
      case 'H':
        tm.tm_min = 0;
        tm.tm_sec = 0;
        // adjust timezone
        addhours(tm, -1*(tm.tm_hour+(iOffSet/100)));
        break;
      case 'm':
      case 'M':
        tm.tm_min = 0;
        tm.tm_sec = 0;
        break;
      case 's':
      case 'S':
        tm.tm_sec = 0;
        break;
    }
    time_t t1 = mktime(&tm) - timezone - timezone; // adjust timezone, one for mktime converting, one for the gmtime
    trace(DEBUG2, "(truncdateu)(a)Truncating '%s' (%d) %d %d %d %d %d %d; t1: %d timezone: %d \n",datesrc.c_str(), iOffSet,tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, (long)t1, timezone);
    tm = *(gmtime(&t1));
    sResult = dateToStr(tm, iOffSet, sFmt);
    trace(DEBUG2, "(truncdateu)(b)Truncating '%s' (%d) => '%s' (%d %d %d %d %d %d) timezone: %d \n",datesrc.c_str(),iOffSet, sResult.c_str(), tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, timezone);
    //trace(DEBUG, "Truncating seconds %d from '%s'(%u) get '%s'(%u), format:%s\n", seconds, datesrc.c_str(), (long)t1, sResult.c_str(), (long)t2, dts.extrainfo.c_str());
  }else{
    trace(ERROR, "(truncdateu) '%s' is not a correct date!\n", datesrc.c_str());
  }
  return sResult;
}

string monthfirstmonday(const string & datesrc, const string & fmt)
{
  struct tm tm;
  string sResult, sFmt = fmt;
  int iOffSet;
  if (sFmt.empty() && !isDate(datesrc, iOffSet, sFmt))
    trace(ERROR, "(monthfirstmonday) '%s' is a invalid date format or '%s' is not a correct date!\n", sFmt.c_str(), datesrc.c_str());
  else if (strToDate(datesrc, tm, iOffSet, sFmt)){
    trace(DEBUG2, "(monthfirstmonday)(a)get monthfirstmonday '%s' (%d) %d %d %d %d %d %d; timezone: %d \n",datesrc.c_str(), iOffSet,tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, timezone);
    tm.tm_mday = tm.tm_mday%7;
    if (tm.tm_mday == 0)
      tm.tm_mday = 7;
    int stdwday = tm.tm_wday==0?7:tm.tm_wday;
    if (stdwday!=1)
      tm.tm_mday = tm.tm_mday>=stdwday?tm.tm_mday-(stdwday-1):tm.tm_mday+(8-stdwday);
    time_t t1 = mktime(&tm) - timezone - timezone; // adjust timezone, one for mktime converting, one for the gmtime
    tm = *(gmtime(&t1));
    sResult = dateToStr(tm, iOffSet, sFmt);
    trace(DEBUG2, "(monthfirstmonday)(b)get monthfirstmonday '%s' (%d) => '%s' (%d %d %d %d %d %d) timezone: %d \n",datesrc.c_str(),iOffSet, sResult.c_str(), tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, timezone);
  }else{
    trace(ERROR, "(monthfirstmonday) '%s' is not a correct date!\n", datesrc.c_str());
  }
  return sResult;
}

int yearweek(const string & datesrc, const string & fmt)
{
  struct tm tm;
  string sResult, sFmt = fmt;
  int iOffSet;
  if (sFmt.empty() && !isDate(datesrc, iOffSet, sFmt))
    trace(ERROR, "(yearweek) '%s' is a invalid date format or '%s' is not a correct date!\n", sFmt.c_str(), datesrc.c_str());
  else if (strToDate(datesrc, tm, iOffSet, sFmt)){
    // get the day in the first week
    int sday = tm.tm_yday%7;
    return (tm.tm_yday-sday)/7+(sday>=tm.tm_wday?1:0);
  }else{
    trace(ERROR, "(yearweek) '%s' is not a correct date!\n", datesrc.c_str());
    return -1;
  }
}

int yearday(const string & datesrc, const string & fmt)
{
  struct tm tm;
  string sResult, sFmt = fmt;
  int iOffSet;
  if (sFmt.empty() && !isDate(datesrc, iOffSet, sFmt))
    trace(ERROR, "(yearday) '%s' is a invalid date format or '%s' is not a correct date!\n", sFmt.c_str(), datesrc.c_str());
  else if (strToDate(datesrc, tm, iOffSet, sFmt)){
    return tm.tm_yday;
  }else{
    trace(ERROR, "(yearday) '%s' is not a correct date!\n", datesrc.c_str());
    return -1;
  }
}

string rqlocaltime(const string & datesrc, const string & fmt)
{
  struct tm tm;
  string sResult, sFmt = fmt;
  int iOffSet;
  if (sFmt.empty() && !isDate(datesrc, iOffSet, sFmt))
    trace(ERROR, "(rqlocaltime) '%s' is a invalid date format or '%s' is not a correct date!\n", sFmt.c_str(), datesrc.c_str());
  else if (strToDate(datesrc, tm, iOffSet, sFmt)){
    trace(DEBUG2, "(rqlocaltime)(a)get localtime '%s' (%d) %d %d %d %d %d %d; timezone: %d \n",datesrc.c_str(), iOffSet,tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, timezone);
    struct tm curtime = now();
    //tm.tm_isdst = curtime.tm_isdst;
    //tm.tm_gmtoff = iOffSet*36;
    //tm.tm_zone = curtime.tm_zone;
    time_t t1 = mktime(&tm) - timezone - timezone; // adjust timezone, one for mktime converting, one for the gmtime
    tm = *(gmtime(&t1));
    replacestr(sFmt," %z","");
    replacestr(sFmt," %Z","");
    // the "timezone" doesnot count the Daylight Saving Time. need to use curtime.tm_gmtoff to adjust
    sResult = dateToStr(tm, curtime.tm_gmtoff/36, sFmt);
    trace(DEBUG2, "(rqlocaltime)(b)get localtime '%s' (%d) => '%s' (%d %d %d %d %d %d) timezone: %d \n",datesrc.c_str(),iOffSet, sResult.c_str(), tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, timezone);
  }else{
    trace(ERROR, "(rqlocaltime) '%s' is not a correct date!\n", datesrc.c_str());
  }
  return sResult;
}

string rqgmtime(const string & datesrc, const string & fmt, const double & gmtoff)
{
  struct tm tm;
  string sResult, sFmt = fmt;
  int iOffSet;
  if (sFmt.empty() && !isDate(datesrc, iOffSet, sFmt))
    trace(ERROR, "(rqgmtime) '%s' is a invalid date format or '%s' is not a correct date!\n", sFmt.c_str(), datesrc.c_str());
  else if (strToDate(datesrc, tm, iOffSet, sFmt)){
    trace(DEBUG2, "(rqgmtime)(a)get localtime '%s' (%d) %d %d %d %d %d %d; timezone: %d \n",datesrc.c_str(), iOffSet,tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, timezone);
    struct tm curtime = now();
    int goffset = lrint(gmtoff*100);
    time_t t1 = mktime(&tm)-(curtime.tm_gmtoff+timezone); // adjust timezone, the "timezone" doesnot count the Daylight Saving Time. need to use curtime.tm_gmtoff to adjust
    tm = *(localtime(&t1));
    sFmt+=" %z";
    sResult = dateToStr(tm, goffset, sFmt);
    //sResult += (gmtoff>=0?" +":" ")+intToStr(gmtoff*100);
    trace(DEBUG2, "(rqgmtime)(b)get localtime '%s' (%d) => '%s' (%d %d %d %d %d %d) timezone: %d \n",datesrc.c_str(),iOffSet, sResult.c_str(), tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, timezone);
  }else{
    trace(ERROR, "(rqgmtime) '%s' is not a correct date!\n", datesrc.c_str());
  }
  return sResult;
}

struct tm convertzone(const struct tm & tm, const string & fromzone, const string & tozone)
{
  struct tm tm1 = tm;
  time_t t1 = mktime(&tm1);
  tm1 = zonetime(t1, fromzone);
  int off1=tm1.tm_gmtoff;
  tm1 = zonetime(t1, tozone);
  int off2 = tm1.tm_gmtoff;
  tm1.tm_gmtoff = off1+off2;
  return tm1;
}

string convertzone(const string & sdate, const string & sFmt, const string & fromzone, const string & tozone)
{
  struct tm tm;
  int iOffset=0;
  string ddate;
  if (strToDate(sdate, tm, iOffset, sFmt))
    ddate = dateToStr(convertzone(tm, fromzone, tozone),0,sFmt);
  return ddate;
}

//vector<string> split(const string& str, const string& delim)
//{
//  vector<string> v;
//  int ppos=0, cpos=0;
//  //trace(DEBUG, "counting '%s','%s' ...",str.c_str(),substr.c_str());
//  cpos = str.find(delim, ppos);
//  while (cpos != string::npos){
//    v.push_back(str.substring(ppos,cpos-ppos));
//    ppos = cpos+delim.length();
//    cpos = str.find(delim, ppos);
//  }
//  return v;
//}

vector<int> genlist(const int& start, const int& end, const int& step)
{
  vector<int> v;
  for (int i=start; i<=end; i+=step)
    v.push_back(i);
  return v;
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
  if (trimmedStr.length()>1 && ((trimmedStr[0]=='\'' && trimmedStr[trimmedStr.length()-1]=='\'' && matchQuoters(trimmedStr, 0, "''")==0) || (trimmedStr[0]=='/' && trimmedStr[trimmedStr.length()-1]=='/' && matchQuoters(trimmedStr, 0, "//")==0))){
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
  else if(sUpper.compare("COMPARENUM")==0)
    return COMPARENUM;
  else if(sUpper.compare("COMPAREDATE")==0)
    return COMPAREDATE;
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
  else if(sUpper.compare("GETWORD")==0)
    return GETWORD;
  else if(sUpper.compare("GETPART")==0)
    return GETPART;
  else if(sUpper.compare("GETPARTS")==0)
    return GETPARTS;
  else if(sUpper.compare("COUNTSTR")==0)
    return COUNTSTR;
  else if(sUpper.compare("FIELDNAME")==0)
    return FIELDNAME;
  else if(sUpper.compare("CONCAT")==0)
    return CONCAT;
  else if(sUpper.compare("CONCATCOL")==0)
    return CONCATCOL;
  else if(sUpper.compare("CALCOL")==0)
    return CALCOL;
  else if(sUpper.compare("APPENDFILE")==0)
    return APPENDFILE;
  else if(sUpper.compare("ZONECONVERT")==0)
    return ZONECONVERT;
  else if(sUpper.compare("RANDOM")==0)
    return RANDOM;
  else if(sUpper.compare("RANDSTR")==0)
    return RANDSTR;
  else if(sUpper.compare("REVERTSTR")==0)
    return REVERTSTR;
  else if(sUpper.compare("FINDNTH")==0)
    return FINDNTH;
  else if(sUpper.compare("CAMELSTR")==0)
    return CAMELSTR;
  else if(sUpper.compare("SNAKESTR")==0)
    return SNAKESTR;
  else if(sUpper.compare("TRIMLEFT")==0)
    return TRIMLEFT;
  else if(sUpper.compare("TRIMRIGHT")==0)
    return TRIMRIGHT;
  else if(sUpper.compare("TRIM")==0)
    return TRIM;
  else if(sUpper.compare("ISNULL")==0)
    return ISNULL;
  else if(sUpper.compare("DATATYPE")==0)
    return DATATYPE;
  else if(sUpper.compare("SWITCH")==0)
    return SWITCH;
  else if(sUpper.compare("WHEN")==0)
    return WHEN;
  else if(sUpper.compare("PAD")==0)
    return PAD;
  else if(sUpper.compare("ASCII")==0)
    return ASCII;
  else if(sUpper.compare("CHAR")==0)
    return CHAR;
  else if(sUpper.compare("MOD")==0)
    return MOD;
  else if(sUpper.compare("ABS")==0)
    return ABS;
  else if(sUpper.compare("TOINT")==0)
    return TOINT;
  else if(sUpper.compare("TOLONG")==0)
    return TOLONG;
  else if(sUpper.compare("TOFLOAT")==0)
    return TOFLOAT;
  else if(sUpper.compare("TOSTR")==0)
    return TOSTR;
  else if(sUpper.compare("TODATE")==0)
    return TODATE;
  else if(sUpper.compare("DECTOHEX")==0)
    return DECTOHEX;
  else if(sUpper.compare("HEXTODEC")==0)
    return HEXTODEC;
  else if(sUpper.compare("DECTOBIN")==0)
    return DECTOBIN;
  else if(sUpper.compare("BINTODEC")==0)
    return BINTODEC;
  else if(sUpper.compare("ADDTIME")==0)
    return ADDTIME;
  else if(sUpper.compare("JSONFORMAT")==0)
    return JSONFORMAT;
  else if(sUpper.compare("JSONQUERY")==0)
    return JSONQUERY;
  else if(sUpper.compare("XMLEXTRACT")==0)
    return XMLEXTRACT;
  else if(sUpper.compare("GREATEST")==0)
    return GREATEST;
  else if(sUpper.compare("LEAST")==0)
    return LEAST;
  else if(sUpper.compare("SUMALL")==0)
    return SUMALL;
  else if(sUpper.compare("EVAL")==0)
    return EVAL;
  else if(sUpper.compare("RCOUNT")==0)
    return RCOUNT;
  else if(sUpper.compare("RMEMBER")==0)
    return RMEMBER;
  else if(sUpper.compare("RMEMBERID")==0)
    return RMEMBERID;
  else if(sUpper.compare("EXEC")==0)
    return EXEC;
  else if(sUpper.compare("ROUND")==0)
    return ROUND;
  else if(sUpper.compare("LOG")==0)
    return LOG;
  else if(sUpper.compare("DATEFORMAT")==0)
    return DATEFORMAT;
  else if(sUpper.compare("TRUNCDATE")==0)
    return TRUNCDATE;
  else if(sUpper.compare("TRUNCDATEU")==0)
    return TRUNCDATEU;
  else if(sUpper.compare("YEARDAY")==0)
    return YEARDAY;
  else if(sUpper.compare("LOCALTIME")==0)
    return LOCALTIME;
  else if(sUpper.compare("GMTIME")==0)
    return GMTIME;
  else if(sUpper.compare("NOW")==0)
    return NOW;
  else if(sUpper.compare("DETECTDT")==0)
    return DETECTDT;
  else if(sUpper.compare("ISLONG")==0)
    return ISLONG;
  else if(sUpper.compare("ISDOUBLE")==0)
    return ISDOUBLE;
  else if(sUpper.compare("ISDATE")==0)
    return ISDATE;
  else if(sUpper.compare("ISSTRING")==0)
    return ISSTRING;
  else if(sUpper.compare("COUNTPART")==0)
    return COUNTPART;
  else if(sUpper.compare("REGCOUNT")==0)
    return REGCOUNT;
  else if(sUpper.compare("REGGET")==0)
    return REGGET;
  else if(sUpper.compare("ISLEAP")==0)
    return ISLEAP;
  else if(sUpper.compare("WEEKDAY")==0)
    return WEEKDAY;
  else if(sUpper.compare("MONTHFIRSTDAY")==0)
    return MONTHFIRSTDAY;
  else if(sUpper.compare("MONTHFIRSTMONDAY")==0)
    return MONTHFIRSTMONDAY;
  else if(sUpper.compare("YEARWEEK")==0)
    return YEARWEEK;
  else if(sUpper.compare("DATETOLONG")==0)
    return DATETOLONG;
  else if(sUpper.compare("LONGTODATE")==0)
    return LONGTODATE;
  else if(sUpper.compare("URLENCODE")==0)
    return URLENCODE;
  else if(sUpper.compare("URLDECODE")==0)
    return URLDECODE;
  else if(sUpper.compare("BASE64ENCODE")==0)
    return BASE64ENCODE;
  else if(sUpper.compare("BASE64DECODE")==0)
    return BASE64DECODE;
  else if(sUpper.compare("MD5")==0)
    return MD5;
  else if(sUpper.compare("HASH")==0)
    return HASH;
  else if(sUpper.compare("ISIP")==0)
    return ISIP;
  else if(sUpper.compare("ISIPV6")==0)
    return ISIPV6;
  else if(sUpper.compare("ISMAC")==0)
    return ISMAC;
  else if(sUpper.compare("MYIPS")==0)
    return MYIPS;
  else if(sUpper.compare("HOSTNAME")==0)
    return HOSTNAME;
  else if(sUpper.compare("RMEMBERS")==0)
    return RMEMBERS;
  else if(sUpper.compare("ISFILE")==0)
    return ISFILE;
  else if(sUpper.compare("ISFOLDER")==0)
    return ISFOLDER;
  else if(sUpper.compare("FILEEXIST")==0)
    return FILEEXIST;
  else if(sUpper.compare("ACOS")==0)
    return ACOS;
  else if(sUpper.compare("ACOSH")==0)
    return ACOSH;
  else if(sUpper.compare("ASIN")==0)
    return ASIN;
  else if(sUpper.compare("ASINH")==0)
    return ASINH;
  else if(sUpper.compare("ATAN")==0)
    return ATAN;
  else if(sUpper.compare("ATAN2")==0)
    return ATAN2;
  else if(sUpper.compare("ATANH")==0)
    return ATANH;
  else if(sUpper.compare("CBRT")==0)
    return CBRT;
  else if(sUpper.compare("COPYSIGN")==0)
    return COPYSIGN;
  else if(sUpper.compare("COS")==0)
    return COS;
  else if(sUpper.compare("COSH")==0)
    return COSH;
  else if(sUpper.compare("ERF")==0)
    return ERF;
  else if(sUpper.compare("EXP")==0)
    return EXP;
  else if(sUpper.compare("EXP2")==0)
    return EXP2;
  else if(sUpper.compare("FMA")==0)
    return FMA;
  else if(sUpper.compare("FMOD")==0)
    return FMOD;
  else if(sUpper.compare("FPCLASSIFY")==0)
    return FPCLASSIFY;
  else if(sUpper.compare("HYPOT")==0)
    return HYPOT;
  else if(sUpper.compare("ILOGB")==0)
    return ILOGB;
  else if(sUpper.compare("ISFINITE")==0)
    return ISFINITE;
  else if(sUpper.compare("ISINF")==0)
    return ISINF;
  else if(sUpper.compare("ISNORMAL")==0)
    return ISNORMAL;
  else if(sUpper.compare("LGAMMA")==0)
    return LGAMMA;
  else if(sUpper.compare("LOG10")==0)
    return LOG10;
  else if(sUpper.compare("LOG2")==0)
    return LOG2;
  else if(sUpper.compare("POW")==0)
    return POW;
  else if(sUpper.compare("REMAINDER")==0)
    return REMAINDER;
  else if(sUpper.compare("SCALBLN")==0)
    return SCALBLN;
  else if(sUpper.compare("SCALBN")==0)
    return SCALBN;
  else if(sUpper.compare("SIN")==0)
    return SIN;
  else if(sUpper.compare("SINH")==0)
    return SINH;
  else if(sUpper.compare("SQRT")==0)
    return SQRT;
  else if(sUpper.compare("TAN")==0)
    return TAN;
  else if(sUpper.compare("TANH")==0)
    return TANH;
  else if(sUpper.compare("TGAMMA")==0)
    return TGAMMA;
  else if(sUpper.compare("PI")==0)
    return PI;
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
  else if(sUpper.compare("GROUPLIST")==0)
    return GROUPLIST;
  else if(sUpper.compare("RANK")==0)
    return RANK;
  else if(sUpper.compare("DENSERANK")==0)
    return DENSERANK;
  else if(sUpper.compare("NEARBY")==0)
    return NEARBY;
  else if(sUpper.compare("PREVIOUS")==0)
    return PREVIOUS;
  else if(sUpper.compare("NEXT")==0)
    return NEXT;
  else if(sUpper.compare("SEQNUM")==0)
    return SEQNUM;
  else if(sUpper.compare("SUMA")==0)
    return SUMA;
  else if(sUpper.compare("COUNTA")==0)
    return COUNTA;
  else if(sUpper.compare("UNIQUECOUNTA")==0)
    return UNIQUECOUNTA;
  else if(sUpper.compare("MAXA")==0)
    return MAXA;
  else if(sUpper.compare("MINA")==0)
    return MINA;
  else if(sUpper.compare("AVERAGEA")==0)
    return AVERAGEA;
  else if(sUpper.compare("FOREACH")==0)
    return FOREACH;
  else if(sUpper.compare("ANYCOL")==0)
    return ANYCOL;
  else if(sUpper.compare("ALLCOL")==0)
    return ALLCOL;
  else if(sUpper.compare("COLTOROW")==0)
    return COLTOROW;
  else if(sUpper.compare("ROOT")==0)
    return ROOT;
  else if(sUpper.compare("PATH")==0)
    return PATH;
  else if(sUpper.compare("PARENT")==0)
    return PARENT;
  else if (g_userMacroFuncNames.find(sUpper)!=g_userMacroFuncNames.end())
    return USERMACROFUNC;
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
    return 3;
  case DIVIDE:
    return 3;
  case POWER:
    return 5;
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
          //time_t t1 = mktime(&tm1);
          //time_t t2 = mktime(&tm2);
          //double diffs = difftime(t1, t2); 
          double diffs = timediff(tm1, tm2);
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
    }
    case ANY:
    case STRING:{
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
          //time_t t1 = mktime(&tm1);
          //time_t t2 = mktime(&tm2);
          //double diffs = difftime(t1, t2);
          double diffs = timediff(tm1, tm2);
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
    }
    case ANY:
    case STRING:{
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
    //result = trim_right(str1,'\'') + trim_left(str2,'\'');
    result = str1 + str2;
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
  double dNum1=atof(str1.c_str()), dNum2=atof(str2.c_str());
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
      //long rslt;
      //bool gotResult = evalLong(str1, operate, str2, rslt);
      //result = longToStr(rslt);
      double rslt;
      bool gotResult = evalDouble(str1, operate, str2, rslt);
      result = (long)rslt==rslt?longToStr((long)rslt):doubleToStr(rslt);
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
int matchQuoters(const string & listStr, const size_t & offset, const string & quoters){
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
vector < vector <string> > getAllTokens(string str, string token)
{
  vector < vector <string> > findings;
  sregex regexp = sregex::compile(token);
  smatch matches;
  string::const_iterator searchStart( str.begin() );
  try{
    while ( regex_search( searchStart, str.end(), matches, regexp ) ){
      vector<string> vmatches;
      for (int i=0; i<matches.size(); i++)
        vmatches.push_back(matches[i]);
      findings.push_back(vmatches);  
      searchStart = matches.suffix().first;
    }
  }catch (exception& e) {
    trace(ERROR, "Regular search exception: %s\n", e.what());
    return findings;
  }
  return findings;
}

bool appendFile(const string & sContent, const string & sFile)
{
  ofstream ofs;
  ofs.open(sFile, ofstream::app);
  if (ofs.is_open()){
    char buf[8192];
    int bsize = 0;
    //memset( buf, '\0', sizeof(char)*1024 );
    bsize = sprintf(buf, "%s", sContent.c_str());
    ofs.write(buf, bsize);
    ofs.close();
    return true;
  }else
    return false;
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
