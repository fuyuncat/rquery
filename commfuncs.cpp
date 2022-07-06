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
#include <boost/algorithm/string.hpp>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h> 
//#include <chrono>
//#include <stdexcept.h>
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


GlobalVars::GlobalVars()
{
  initVars();
}

GlobalVars::~GlobalVars()
{
  
}

size_t GlobalVars::g_inputbuffer;
short GlobalVars::g_tracelevel;
bool GlobalVars::g_printheader;

void GlobalVars::initVars(){
  g_inputbuffer = 16384;
  g_tracelevel = FATAL;
  g_printheader = true;
}

void GlobalVars::setVars(size_t inputbuffer, short tracelevel, bool printheader){
  g_inputbuffer = inputbuffer;
  g_tracelevel = tracelevel;
  g_printheader = printheader;
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
  case DUMP:
    return "";
  default:
    return "UNKNOWN";
  }
}

void trace(short level, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  if (GlobalVars::g_tracelevel>=level){
    printf((decodeTracelevel(level)+(level==DUMP?"":":")).c_str());
    vprintf(fmt, args);
    if (level == FATAL)
      exit(EXIT_FAILURE);
  }
  va_end(args);
}

vector<string>::const_iterator namesaving_smatch::names_begin() const
{
    return m_names.begin();
}

vector<string>::const_iterator namesaving_smatch::names_end() const
{
    return m_names.end();
}

//string string_format( const string& format, Args ... args )
//{
//  int size_s = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
//  if( size_s <= 0 ){ throw sruntime_error( "Error during formatting." ); }
//  size_t size = static_cast<size_t>( size_s );
//  unique_ptr<char[]> buf( new char[ size ] );
//  snprintf( buf.get(), size, format.c_str(), args ... );
//  return string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
//}

// return most outer quoted string. pos is start pos and return the position of next char of the end of the quoted string.  
string readQuotedStr(string str, int& pos, string quoters, char escape)
{
  if (quoters.size()<2)
    return "";
  int quoteStart = -1, i = pos, quoteDeep=0;
  bool quoted = false;
  while(i < str.size()) {
    if (quoteDeep > 0){ // checking right quoter only when the string is quoted.
      if (str[i] == quoters[1])
        if (i>0 && str[i-1]!=escape){
          quoteDeep--;
          if (quoteDeep == 0){
            pos = i+1;
            return str.substr(quoteStart,pos-quoteStart);
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

// find the first position of the any character in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
int findFirstCharacter(string str, std::set<char> lookfor, int pos, string quoters,  char escape)
{
  size_t i = 0, j = 0;
  bool quoted = false;
  int quoterId = -1;
  while(i < str.size()) {
    if (lookfor.find(str[i]) != lookfor.end() && i>0 && !quoted) 
      return i;
    if (!quoted){
      for (int k=0; k<(int)(quoters.size()/2); k++){
        if (str[i] == quoters[k*2])
          if (k*2+1<quoters.size() && (i==0 || (i>0 && str[i-1]!=escape))){
            quoted = !quoted;
            quoterId=k*2;
          }
      }
    }else{
      if (str[i] == quoters[quoterId+1])
        if (i>0 && str[i-1]!=escape){
          quoted = !quoted;
          quoterId = -1;
        }
    }
    ++i;
  }
  return -1;
}

// split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
vector<string> split(string str, char delim, string quoters, char escape) 
{
  vector<string> v;
  size_t i = 0, j = 0, begin = 0;
  bool quoted = false;
  int quoterId = -1;
  while(i < str.size()) {
    if (str[i] == delim && i>0 && !quoted) {
      //trace(DEBUG, "found delim, split string:%s (%d to %d)\n",str.substr(begin, i-begin).c_str(), begin, i);
      //v.push_back(string(str, begin, i));
      v.push_back(str.substr(begin, i-begin));
      begin = i+1;
    }
    if (!quoted){
      for (int k=0; k<(int)(quoters.size()/2); k++){
        if (str[i] == quoters[k*2])
          if (k*2+1<quoters.size() && (i==0 || (i>0 && str[i-1]!=escape))){
            quoted = !quoted;
            quoterId=k*2;
          }
      }
    }else{
      if (str[i] == quoters[quoterId+1])
        if (i>0 && str[i-1]!=escape){
          quoted = !quoted;
          quoterId = -1;
        }
    }
    ++i;
  }
  if (begin<str.size())
    //v.push_back(string(str, begin, str.size()));
    v.push_back(str.substr(begin, str.size()-begin));

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

string trim_pair(string str, string pair)
{
  if(str.size() > 1 && pair.size() == 2 && str[0] == pair[0] && str[str.size()-1] == pair[1])
    return str.substr(1,str.size()-2);
  else
    return str;
}

string trim(string str, char c)
{
  string newstr = trim_one(str, c);
  while (newstr.length() != str.length())
    newstr = trim_one(str, c);
  return newstr;
}

string trim_right(string str, char c)
{
  string newstr = str;
  if (newstr[newstr.size()-1] == c)
    newstr = newstr.substr(0,newstr.size()-1);
  return newstr;
}

string trim_left(string str, char c)
{
  string newstr = str;
  if (newstr[0] == c)
    newstr = newstr.substr(1);
  return newstr;
}

string trim_one(string str, char c)
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

void replacestr(string & sRaw, string sReplace, string sNew)
{
  int pos=0;
  while (sRaw.find(sReplace, pos) != string::npos){
    sRaw.replace(pos, sReplace.length(), sNew);
    pos+=sReplace.length();
  }
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

string dateToStr(struct tm val, string fmt)
{
  char buffer [256];
  if (strftime(buffer,256,fmt.c_str(),&val))
    return string(buffer);
  else {
    trace(ERROR, "Unrecognized date format '%s'.\n", fmt.c_str());
    return "";
  }
}

bool strToDate(string str, struct tm & tm)
{
  string fmt;
  if (isDate(str, fmt) && strptime(str.c_str(), fmt.c_str(), &tm))
    return true;
  else
    return false;
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

// get the compatible data type from two data types
int getCompatibleDataType(int ldatatype, int rdatatype) 
{
  if (ldatatype == ANY || rdatatype == ANY)
    return ldatatype == ANY?rdatatype:ldatatype;
  if (ldatatype == STRING || rdatatype == STRING)
    if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP){ // incompatible types
      trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(STRING).c_str(), decodeDatatype(ldatatype==STRING?rdatatype:ldatatype).c_str());
      return UNKNOWN;
    }else{
      return STRING;
    }
  else if (ldatatype == DOUBLE || rdatatype == DOUBLE)
    if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP || ldatatype == STRING || rdatatype == STRING){ // incompatible types
      trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(DOUBLE).c_str(), decodeDatatype(ldatatype==DOUBLE?rdatatype:ldatatype).c_str());
      return UNKNOWN;
    }else{
      return DOUBLE;
    }
  else if (ldatatype == LONG || rdatatype == LONG)
    if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP || ldatatype == STRING || rdatatype == STRING || ldatatype == DOUBLE || rdatatype == DOUBLE){ // incompatible types
      trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(LONG).c_str(), decodeDatatype(ldatatype==LONG?rdatatype:ldatatype).c_str());
      return UNKNOWN;
    }else{
      return LONG;
    }
  else if (ldatatype == INTEGER || rdatatype == INTEGER)
    if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP || ldatatype == STRING || rdatatype == STRING || ldatatype == DOUBLE || rdatatype == DOUBLE || ldatatype == LONG || rdatatype == LONG){ // incompatible types
      trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(INTEGER).c_str(), decodeDatatype(ldatatype==INTEGER?rdatatype:ldatatype).c_str());
      return UNKNOWN;
    }else{
      return INTEGER;
    }
  else if (ldatatype == BOOLEAN || rdatatype == BOOLEAN)
    if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP || ldatatype == STRING || rdatatype == STRING || ldatatype == DOUBLE || rdatatype == DOUBLE || ldatatype == LONG || rdatatype == LONG || ldatatype == INTEGER || rdatatype == INTEGER){ // incompatible types
      trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(BOOLEAN).c_str(), decodeDatatype(ldatatype==BOOLEAN?rdatatype:ldatatype).c_str());
      return UNKNOWN;
    }else{
      return BOOLEAN;
    }
  else if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP)
    if (ldatatype == STRING || rdatatype == STRING || ldatatype == DOUBLE || rdatatype == DOUBLE || ldatatype == LONG || rdatatype == LONG || ldatatype == INTEGER || rdatatype == INTEGER || ldatatype == BOOLEAN || rdatatype == BOOLEAN){ // incompatible types
      trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(DATE).c_str(), decodeDatatype((ldatatype==DATE||ldatatype==TIMESTAMP)?rdatatype:ldatatype).c_str());
      return UNKNOWN;
    }else{
      return DATE;
    }
  else{
    return UNKNOWN;
  }
}

// detect the data type of an expression string
// STRING: quoted by '', or regular expression: //
// DATE/TIMESTAMP: quoted by {}
// INTEGER/LONG: all digits
// Double: digits + .
int detectDataType(string str)
{
  string trimmedStr = boost::algorithm::trim_copy<string>(str);
  if (matchQuoters(trimmedStr, 0, "''") || matchQuoters(trimmedStr, 0, "//"))
    return STRING;
  else if (matchQuoters(trimmedStr, 0, "{}"))
    return DATE;
  else if (isLong(trimmedStr))
    return LONG;
  else if (isDouble(trimmedStr))
    return DOUBLE;
  else if (isInt(trimmedStr))
    return INTEGER;
  else
    return UNKNOWN;
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
  smatch matches;
  try{
    return regex_search(str, matches, regexp);
  }catch (exception& e) {
    trace(ERROR, "Regular search exception: %s\n", e.what());
    return false;
  }
}

bool in(string str1, string str2)
{
  //return boost::to_upper_copy<string>(str2).find(boost::to_upper_copy<string>(str1))!=string::npos;
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
    return "=";
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

int encodeComparator(string str)
{
  //printf("encode comparator: %s\n",str.c_str());
  string sUpper = boost::to_upper_copy<string>(str);
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

int encodeDatatype(string str)
{
  string sUpper = boost::to_upper_copy<string>(str);
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

int encodeJunction(string str)
{
  string sUpper = boost::to_upper_copy<string>(str);
  if (sUpper.compare("AND") == 0)
    return AND;
  else if (sUpper.compare("OR") == 0)
    return OR;
  else
    return UNKNOWN;
}

int encodeOperator(string str)
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

int operatorPriority(int iOperator)
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
int anyDataCompare(string str1, string str2, int type){
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
  }else if (type == DATE || type == TIMESTAMP){
    string fmt1, fmt2;
    string newstr1=trim_one(str1,'{'),newstr2=trim_one(str2,'{');
    newstr1=trim_one(newstr1,'}');newstr2=trim_one(newstr2,'}');
    if (isDate(newstr1,fmt1) && isDate(newstr2,fmt2)){
      struct tm tm1, tm2;
      if (strptime(newstr1.c_str(), fmt1.c_str(), &tm1) && strptime(newstr2.c_str(), fmt2.c_str(), &tm2)){
        time_t t1 = mktime(&tm1);
        time_t t2 = mktime(&tm2);
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
    }else{
      return -101;
    }
  }else if (type == STRING){
    string newstr1=trim_one(str1,'\''),newstr2=trim_one(str2,'\'');
    newstr1=trim_one(newstr1,'/');newstr2=trim_one(newstr2,'/');
    return newstr1.compare(newstr2);
  }else {
    return -102;
  }
}

// compare data according to data type
// @return int 0: false; 1: true  
//             error -101~-110 -101:invalid data according to data type; -102: data type not supported
int anyDataCompare(string str1, int comparator, string str2, int type){
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
  }else if (type == INTEGER){
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
  }else if (type == DOUBLE){
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
  }else if (type == DATE || type == TIMESTAMP){
    string fmt1, fmt2;
    string newstr1=trim_one(str1,'{'),newstr2=trim_one(str2,'{');
    newstr1=trim_one(newstr1,'}');newstr2=trim_one(newstr2,'}');
    if (isDate(newstr1,fmt1) && isDate(newstr2,fmt2)){
      struct tm tm1, tm2;
      if (strptime(newstr1.c_str(), fmt1.c_str(), &tm1) && strptime(newstr2.c_str(), fmt2.c_str(), &tm2)){
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
    string newstr1=trim_one(str1,'\''),newstr2=trim_one(str2,'\'');
    newstr1=trim_one(newstr1,'/');newstr2=trim_one(newstr2,'/');
    switch (comparator){
    case EQ:
      return newstr1.compare(newstr2)==0?1:0;
    case LT:
      return newstr1.compare(newstr2)>0?1:0;
    case ST:
      return newstr1.compare(newstr2)<0?1:0;
    case NEQ:
      return newstr1.compare(newstr2)!=0?1:0;
    case LE:
      return newstr1.compare(newstr2)>=0?1:0;
    case SE:
      return newstr1.compare(newstr2)<=0?1:0;
    case LIKE:
      return like(newstr1, newstr2);
    case REGLIKE:
      return reglike(newstr1, newstr2);
    case NOLIKE:
      return !like(newstr1, newstr2);
    case NOREGLIKE:
      return !reglike(newstr1, newstr2);
    case IN:
      return in(newstr1, newstr2);
    case NOIN:
      return !in(newstr1, newstr2);
    default:
      return -101;
    }
  }else {
      return -102;
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

bool evalDate(string str1, int operate, string str2, struct tm& result)
{
  time_t t1;
  string fmt;
  int seconds;
  if (isDate(str1, fmt) && isInt(str2)){
    strptime(str1.c_str(), DATEFMT, &result);
    t1 = mktime(&result);
    seconds = atoi(str2.c_str());
  }else if (isDate(str2, fmt) && isInt(str1)){
    strptime(str2.c_str(), DATEFMT, &result);
    t1 = mktime(&result);
    seconds = atoi(str1.c_str());
  }else{
    trace(ERROR, "DATE can only +/- an INTEGER number !\n");
    return false;
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
bool anyDataOperate(string str1, int operate, string str2, int type, string& result)
{
  switch (type){
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
      bool gotResult = evalDate(str1, operate, str2, rslt);
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
  string upperstr = boost::to_upper_copy<string>(str);
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
    int matchedWordId = startsWithWords(boost::to_upper_copy<string>(originalStr.substr(i)), keepWords);
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
    trace(DEBUG, "%d:%s\t", i, v[i].c_str());
  trace(DEBUG, "\n");
}

void dumpMap(map<string, string> m)
{
  trace(DEBUG, "Dumping map<string, string>...\n");
  for (map<string,string>::iterator it=m.begin(); it!=m.end(); ++it)
    trace(DEBUG, "%s: %s\n", it->first.c_str(), it->second.c_str());
  trace(DEBUG, "\n");
}
