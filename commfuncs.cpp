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
#include <ctime>
#include <boost/algorithm/string.hpp>
#include <stdbool.h>
#include "commfuncs.h"

namesaving_smatch::namesaving_smatch()
{
}

namesaving_smatch::namesaving_smatch(const string pattern)
{
  init(pattern);
}

namesaving_smatch::~namesaving_smatch() { }

void namesaving_smatch::init(const string pattern)
{
  string pattern_str = pattern;
  sregex capture_pattern = sregex::compile("\\?P?<(\\w+)>");
  sregex_iterator words_begin = sregex_iterator(pattern_str.begin(), pattern_str.end(), capture_pattern);
  sregex_iterator words_end = sregex_iterator();

  for (sregex_iterator i = words_begin; i != words_end; i++){
    string name = (*i)[1].str();
    m_names.push_back(name);
  }
}

vector<string>::const_iterator namesaving_smatch::names_begin() const
{
    return m_names.begin();
}

vector<string>::const_iterator namesaving_smatch::names_end() const
{
    return m_names.end();
}

vector<string> split(string str, char delim, char quoter, char escape) 
{
  vector<string> v;
  size_t i = 0, j = 0, begin = 0;
  bool quoted = false;
  while(i < str.size()) {
    if (str[i] == delim && i>0 && !quoted) {
      //printf("found delim\n");
      v.push_back(string(str, begin, i));
      begin = i+1;
    }
    if (str[i] == quoter) {
      //printf("found quoter\n");
      if (i>0 && str[i-1]!=escape)
        quoted = !quoted;
    }
    ++i;
  }
  if (begin<str.size())
    v.push_back(string(str, begin, str.size()));

  /*while(i < str.size()) {
    if(str[i] == delim || i == 0) {
      if(i + 1 < str.size() && str[i + 1] == escape) {
        j = begin + 1;
        while(j < str.size() && str[j++] != escape);
          v.push_back(string(str, begin, j - 1 - i));
        begin = j - 1;
        i = j - 1;
        continue;
      }

      j = begin + 1;
      while(j < str.size() && str[j++] != delim);
      v.push_back(string(str, begin, j - 1 - i - (i ? 1 : 0) ));
      begin = j;
    }
    ++i;
  }*/

  return v;
}

string trim_one(string str, char c)
{
  string newstr = str;
  if (newstr[0] == c)
    newstr = newstr.substr(1);
  if (newstr[newstr.size()-1] == c)
    newstr = newstr.substr(0,newstr.size()-1);
  //printf("Old Reg: %s\n",str.c_str());
  //printf("New Reg: %s\n",newstr.c_str());
  return newstr;
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

bool isDate(const string& str, string& fmt)
{
  struct tm tm;
  std::set<string> alldatefmt, alltimefmt;
  alldatefmt.insert("%Y-%m-%d");alldatefmt.insert("%Y/%m/%d");alldatefmt.insert("%d/%m/%Y");alldatefmt.insert("%d-%m-%Y");
  alltimefmt.insert("%H:%M:%S");alltimefmt.insert("%h:%M:%S");alltimefmt.insert("%H/%M/%S");alltimefmt.insert("%h/%M/%S");
  //alldatefmt = {"%Y-%m-%d", "%Y/%m/%d", "%d/%m/%Y", "%d-%m-%Y"};
  //alltimefmt = {"%H:%M:%S", "%h:%M:%S", "%H/%M/%S", "%h/%M/%S"};
  for (std::set<string>::iterator id = alldatefmt.begin(); id != alldatefmt.end(); ++id) {
    if (strptime(str.c_str(), (*id).c_str(), &tm)){
      fmt = (*id);
      return true;
    }else{
      for (std::set<string>::iterator it = alldatefmt.begin(); it != alldatefmt.end(); ++it) {
        if (strptime(str.c_str(), (*it).c_str(), &tm)){
          fmt = (*it);
          return true;
        }else if (strptime(str.c_str(), ((*id)+":"+(*it)).c_str(), &tm)){
          fmt = (*id)+":"+(*it);
          return true;
        }else if (strptime(str.c_str(), ((*it)+":"+(*id)).c_str(), &tm)){
          fmt = (*it)+":"+*id;
          return true;
        }else if (strptime(str.c_str(), ((*id)+" "+(*it)).c_str(), &tm)){
          fmt = (*id)+" "+(*it);
          return true;
        }else if (strptime(str.c_str(), ((*it)+" "+(*id)).c_str(), &tm)){
          fmt = (*it)+" "+(*id);
          return true;
        }else
          continue;
      }
    }
  }
  return false;
}

bool wildmatch(const char *candidate, const char *pattern, int p, int c, char multiwild='*', char singlewild='?', char escape='\\') {
  if (pattern[p] == '\0') {
    return candidate[c] == '\0';
  }else if (pattern[p] == escape && pattern[p+1] != '\0' && (pattern[p+1] == multiwild || pattern[p+1] == singlewild)) {
    return wildmatch(candidate, pattern, p+1, c, multiwild, singlewild, escape);
  } else if ((pattern[p] == multiwild && p == 0) || (pattern[p] == multiwild && p > 0 && pattern[p] != escape)) {
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
  return wildmatch(str1.c_str(), str2.c_str(), 0, 0);
}

bool reglike(string str, string regstr)
{
  sregex regexp = sregex::compile(regstr);
  namesaving_smatch matches(regstr);
  return regex_search(str, matches, regexp);
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

static string decodeJunction(int junction){
  switch (junction){
  case AND:
    return "AND";
  case OR:
    return "OR";
  default:
    return "UNKNOWN";
  }
}

static string decodeComparator(int comparator){
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
  default:
    return "UNKNOWN";
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

// compare data according to data type
// @return int str1 < str2: -1; str1 == str2:0; str1 > str2: 1
//             error -101~-110 -101:invalid data according to data type; -102: data type not supported
static int anyDataCompare(string str1, string str2, int type){
  if (str1.length() == 0 && str2.length() == 0)
    return 0;
  else if (str1.length() == 0)
    return -1;
  else if (str2.length() == 0)
    return 1;
  if (type == LONG){
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
  }else if (type == INTEGER){
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
  }else if (type == DOUBLE){
    if (isDouble(str1) && isDouble(str2)){
      double d1 = atod(str1.c_str());
      double d2 = atod(str2.c_str());
      if (d1 < d2)
        return -1;
      else if (d1 == d2)
        return 0;
      else
        return 1;
    }else{
      return -101;
    }
  }else if (type == DATE || type == TIMESTAMP){
    string fmt1, fmt2;
    if (isDate(str1,fmt1) && isDate(str2,fmt2)){
      struct tm tm1, tm2;
      if (strptime(str1.c_str(), fmt1.c_str(), &tm1) && strptime(str2.c_str(), fmt2.c_str(), &tm2)){
        time_t t1 = mktime(tm1);
        time_t t2 = mktime(tm2);
        double diffs = difftime(t1, t2);
        if (diffs < 0)
          return -1;
        else if (diffs > 0)
          return 1;
        else return 0;
      }
    }else{
      return -101;
    }
  }else if (type == BOOLEAN){
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
    }catch (Exception e){
      e.printStackTrace();
      return -101;
    }
  }else if (type == STRING){
    return str1.compare(str2);
  }else {
    return -102;
  }
}

// compare data according to data type
// @return int 0: false; 1: true  
//             error -101~-110 -101:invalid data according to data type; -102: data type not supported
static int anyDataCompare(string str1, int comparator, string str2, int type){
  if (type == LONG){
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
  else if (type == INTEGER){
    if (IsInt(str1) && IsInt(str2)){
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
  }else if (type == DOUBLE){
    if (isDouble(str1) && isDouble(str2)){
      double d1 = atod(str1.c_str());
      double d2 = atod(str2.c_str());
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
  }else if (type == DATE || type == TIMESTAMP){
    string fmt1, fmt2;
    if (isDate(str1,fmt1) && isDate(str2,fmt2)){
      struct tm tm1, tm2;
      if (strptime(str1.c_str(), fmt1.c_str(), &tm1) && strptime(str2.c_str(), fmt2.c_str(), &tm2)){
        time_t t1 = mktime(tm1);
        time_t t2 = mktime(tm2);
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
      }
    }else{
      return -101;
    }
  }else if (type == BOOLEAN){
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
  }else if (type == STRING){
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
    default:
      return -101;
    }
  }else {
      return -102;
  }
  return -102;
}
