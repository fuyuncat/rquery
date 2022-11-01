/*******************************************************************************
//
//        File: commfuncs.h
// Description: common functions
//       Usage: commfuncs.h
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/
//#pragma once
#define __DEBUG__

#ifndef __COMMFUNCS_H
#define __COMMFUNCS_H

#include <regex.h>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <ctime>
#include <fstream>

using namespace std;

#define VERSION "v0.991"

#define UNKNOWN 0

#define BRANCH 1
#define LEAF 2

#define AND 1
#define OR 2

#define EQ 1
#define LT 2
#define ST 3
#define NEQ 4
#define LE 5
#define SE 6
#define LIKE 7
#define REGLIKE 8
#define NOLIKE 9
#define NOREGLIKE 10
#define IN 11
#define NOIN 12

#define LEFT 1
#define RIGHT 2

#define PLUS 1
#define SUBTRACT 2
#define TIMES 3
#define DIVIDE 4
#define POWER 5

#define STRING 1
#define LONG 2
#define INTEGER 3
#define DOUBLE 4
#define DATE 5
#define TIMESTAMP 6
#define BOOLEAN 7
#define ANY 99

#define FATAL 1
#define ERROR 2
#define WARNING 3
#define INFO 4
#define DUMP 5
#define DEBUG2 98
#define DEBUG1 99
#define DEBUG 100

#define DATEFMT "%Y-%m-%d %H:%M:%S"

#define CONST 1
#define COLUMN 2
#define VARIABLE 3
#define FUNCTION 4

#define ASC 1
#define DESC 2

#define PROMPT 1
#define PARAMETER 2
#define SINGLEFILE 3
#define FOLDER 4
#define WILDCARDFILES 5

#define READBUFF 1
#define READLINE 2

#define UPPER 1
#define LOWER 2
#define SUBSTR 3
#define INSTR 4
#define STRLEN 5
#define COMPARESTR 6
#define NOCASECOMPARESTR 7
#define REPLACE 8
#define FLOOR 9
#define CEIL 10
#define ROUND 11
#define LOG 12
#define TIMEDIFF 13
#define DATEFORMAT 14
#define TRUNCDATE 15
#define NOW 16
#define REGREPLACE 17
#define ISNULL 18
#define SWITCH 19
#define PAD 20
#define GREATEST 21
#define LEAST 22
#define REGMATCH 23
#define COUNTWORD 24
#define GETWORD 25
#define ZONECONVERT 26
#define RANDOM 27
#define CAMELSTR 28
#define SNAKESTR 29
#define TRIMLEFT 30
#define TRIMRIGHT 31
#define TRIM 32
#define RANDSTR 33
#define DATATYPE 34
#define REVERTSTR 35
#define FINDNTH 36
#define ASCII 37
#define CHAR 38
#define MOD 39
#define ABS 40
#define TOINT 41
#define TOLONG 42
#define TOFLOAT 43
#define TOSTR 44
#define TODATE 45
#define DECTOHEX 46
#define HEXTODEC 47
#define DECTOBIN 48
#define BINTODEC 49
#define ADDTIME 50
#define JSONFORMAT 51
#define JSONQUERY 52
#define XMLEXTRACT 53
#define SUM 101
#define COUNT 102
#define UNIQUECOUNT 103
#define MAX 104
#define MIN 105
#define AVERAGE 106
#define GROUPLIST 107
#define COMPARENUM 108
#define COMPAREDATE 109
#define COUNTSTR 110
#define FIELDNAME 111
#define CONCAT 112
#define CONCATCOL 113
#define CALCOL 114
#define APPENDFILE 115
#define WHEN 116
#define GETPART 117
#define SUMALL 118
#define EVAL 119
#define RANK 201
#define PREVIOUS 202
#define NEXT 203
#define SEQNUM 204
#define DENSERANK 205
#define NEARBY 206
#define SUMA 207
#define COUNTA 208
#define UNIQUECOUNTA 209
#define MAXA 210
#define MINA 211
#define AVERAGEA 212
#define FOREACH 501
#define COLTOROW 502
#define ANYCOL 503
#define ALLCOL 504
#define ROOT 801
#define PATH 802
#define PARENT 803

#define TEXT 1
#define JSON 2

#define REGSEARCH 1
#define WILDSEARCH 2
#define DELMSEARCH 3

#define STANDARD 1
#define OVERWRITE 2
#define APPEND 3

template< class T > void SafeDelete( T*& pVal )
{
  if (pVal)
    delete pVal;
  pVal = NULL;
}

template< class T > void SafeDeleteArray( T*& pVal )
{
  if (pVal)
    delete[] pVal;
  pVal = NULL;
}

//template< typename T > void CountArray( T& array )
//{
//  return array.size();
//}


struct DataTypeStruct{
  short int datatype=UNKNOWN;
  string extrainfo="";  // for DATE type only so far, the format of the DATE 

  // We only need these operators when this struct is being used as Key of Map
  //bool operator==(const DataTypeStruct& x) const {
  //  return (this->datatype==x.datatype && this->extrainfo.compare(x.extrainfo)==0);
  //}

  //bool operator<(const DataTypeStruct& x) const {
  //  return (this->datatype<x.datatype || (this->datatype==x.datatype && this->extrainfo.compare(x.extrainfo)<0));
  //}
};

struct TreeNode;
struct TreeNode{
  int rowid;
  short int level;
  vector <string> rowValues;
  vector <string> treeKeys;
  vector <string> parentKeys;
  TreeNode* parentNode=NULL;
  TreeNode* firstChild=NULL;
  TreeNode* nextSibling=NULL;
  
  bool bInTheTree=false;
  unordered_map< string,string > mFuncVals;
  unordered_map< string,string > mParentVals; // reserved value for children
};

class GlobalVars{
public:
  GlobalVars();
  ~GlobalVars();
  
  void initVars();
  void setVars(size_t inputbuffer = 16384, short tracelevel=FATAL, bool printheader=true);

  size_t g_inputbuffer;
  short int g_tracelevel;
  bool g_printheader;
  bool g_showprogress;
  short int g_ouputformat;
  ofstream* g_logfile;
  
  bool g_consolemode;
  bool g_recursiveread;
};

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    seed ^= hash<T>{}(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template <typename Container> // we can make this generic for any container [1]
struct hash_container 
{
  size_t operator()(Container const& V) const 
  {
    std::size_t hash = V.size();
    for (auto i = V.begin(); i != V.end(); ++i)
      hash_combine(hash, *i);
    return hash;
  }

};

struct GroupProp{
public:
  short int funcID = UNKNOWN;
  long count;
  long double sum;
  string max;
  string min;
  //std::set <string> * uniquec;
  vector <string> * varray;
  bool inited = false;
  
  void init();
  GroupProp();
  GroupProp(const GroupProp& other);
  ~GroupProp();
  GroupProp& operator=(const GroupProp& other);
  void clear();
};

extern GlobalVars gv;

class TimeZone{
public:
  TimeZone();
  ~TimeZone();
  
  void init();
  void dump();
  
  static unordered_map< string, int > m_nameoffmap;
};

void trace(short level, const char *fmt, ...);
void exitProgram(short int code);
void clearGroupPropMap(unordered_map< string,GroupProp > & aggProps); // manually free the memory of GroupProp, as it's not freed in the destructor to improve performance. 

//string string_format( const string& format, Args ... args );
string readQuotedStr(const string & str, size_t& pos, const string & quoters, const char & escape = '\\'); // return most outer quoted string. pos is start pos and return the position of next char of the end of the quoted string.  
int matchQuoters(const string & listStr, const size_t & offset, const string & quoters); // detect if quoters matched.
vector<string> split(const string & str, string delim = " ", string quoters = "''", char escape = '\\', std::set<char> nestedQuoters={'(',')'}, bool repeatable=false, bool skipemptyelement=false); // split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
vector<string> split(const string & str, char delim = ' ', string quoters = "''", char escape = '\\', std::set<char> nestedQuoters={'(',')'}, bool repeatable=false, bool skipemptyelement=false); // split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
vector<string> split(const string & str, std::set<char> delims = {' ','\t','\n','\r'}, string quoters = "''\"\"", char escape = '\\', std::set<char> nestedQuoters={'(',')'}, bool repeatable=false, bool skipemptyelement=false); // split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
size_t findNthCharacter(const string & str, const std::set<char> & lookfor, const size_t & pos=0, const int & seq=1, const bool & forward=true, const string & quoters = "''()", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}); // find the Nth(seq) position of the any character in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
size_t findFirstCharacter(const string & str, const std::set<char> & lookfor, const size_t & pos=0, const string & quoters = "''()", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}); // find the first position of the any character in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
size_t findNthSub(const string & str, const string & lookfor, const size_t & pos=0, const int & seq=1, const bool & forward=true, const string & quoters = "''()", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}, bool casesensitive=true); // find the Nth(seq) position of a substring in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
size_t findFirstSub(const string & str, const string & lookfor, const size_t & pos=0, const string & quoters = "''()", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}, bool casesensitive=true); // find the first position of a substring in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
string readLine(string str, size_t & pos); // read a line
vector<string> matchWildcard(const string & str, const string & wildStr, string quoters = "\"\"", char escape = '\\', std::set<char> nestedQuoters={'(',')'}); // match wildcard to transfer a string to a vector
string readWordTillStop(const string & str, size_t & pos, char stopper='*', char escape = '\\'); // read the first word until reaches stopper
string concatArray(const vector<string> & array, const string & delim = ""); // concatenate an array to string

int startsWithWords(string str, vector<string> words, int offset); // detect if string start with special words
int startsWithWords(string str, vector<string> words); // detect if string start with special words
string removeSpace(string originalStr, string keepPattern); //remove space
string getFirstToken(string str, string token); //get the first matched regelar token from a string
vector <string> getAllTokens(string str, string token); //get all matched regelar token from a string
bool matchToken(string str, string token); // check if matched regelar token

string trim_pair(const string & str, const string & pair);
string trim_left(const string & str, char c = ' ', bool repeat=false);
string trim_right(const string & str, char c = ' ', bool repeat=false);
string trim_one(const string & str, char c = ' ');
string trim(const string & str, char c = ' ', bool repeat=true);
string trim_copy(const string & str);
string upper_copy(const string & str);
string lower_copy(const string & str);
char upper_char(const char & c);
char lower_char(const char & c);
string camelstr(const string & str); // convert a string to camel string
string snakestr(const string & str); // convert a string to snake string
string revertstr(const string & str);
int countstr(const string & str, const string & substr); // count occurences of substr in str
void replacestr(string & sRaw, const string & sReplace, const string & sNew);
void replacestr(string & sRaw, const vector<string> & vReplace, const vector<string> & vNew);
void replaceunquotedstr(string & str, const string & sReplace, const string & sNew, string quoters = "''", char escape = '\\', std::set<char> nestedQuoters={'(',')'});
void regreplacestr(string & sRaw, const string & sPattern, const string & sNew);
void regmatchstr(const string & sRaw, const string & sPattern, string & sExpr);

bool isNumber(const string& str);
bool isInt(const string& str);
bool isLong(const string& str);
bool isFloat(const string& str);
bool isDouble(const string& str);
bool isDate(const string& str, int & iOffSet, string& fmt);
string intToStr(const int val);
string longToStr(const long val);
string floatToStr(const float val);
string doubleToStr(const double val);
string dateToStr(struct tm val, int iOffSet = 888888, string fmt = DATEFMT);
string dectohex(const int & val);
int hextodec(const string& str);
string dectobin(const int & val);
int bintodec(const string& str);

bool strToDate(string str, struct tm & tm, int & iOffSet, string fmt=DATEFMT);
int localOffset();
struct tm zonetime(time_t t1, string zone);
struct tm zonetime(time_t t1, int iOffSet);
int dateFormatLen(string fmt);
string stripTimeZone(string str, int & iOffSet, string & sTimeZone);
struct tm now();
long int curtime();
double timediff(struct tm & tm1, struct tm & tm2);
void addhours(struct tm & tm, int hours);
void addseconds(struct tm & tm, int seconds);
void addtime(struct tm & tm, int diff, char unit='S');
string truncdate(const string & datesrc, const string & fmt, const int & seconds);
struct tm convertzone(const struct tm & tm, const string & fromzone, const string & tozone);
string convertzone(const string & sdate, const string & sFmt, const string & fromzone, const string & tozone);

int random(int min=1, int max=100);
string randstr(int len=8, const string & flags="uld");

bool like(string str1, string str2); // str1 like str2 (containing wildcard)
bool reglike(string str, string regstr); 
bool in(string str1, string str2); 
//char getch();
//size_t getstr(char * buffer, const size_t len);

string decodeJunction(int junction);
string decodeComparator(int comparator);
string decodeDatatype(int datatype);
string decodeOperator(int op);
string decodeExptype(int exptype);
string decodeTracelevel(int level);
short int encodeComparator(string str);
short int encodeDatatype(string str);
short int encodeJunction(string str);
short int encodeOperator(string str);
short int encodeTracelevel(string str);
short int encodeFunction(string str);
short int operatorPriority(int iOperator);
int findStrArrayId(const vector<string> array, const string member);

int anyDataCompare(string str1, string str2, DataTypeStruct dts);  // @return int str1 < str2: -1; str1 == str2:0; str1 > str2: 1
int anyDataCompare(string str1, int comparator, string str2, DataTypeStruct dts);
int compareVector(vector<string> array1, vector<string> array2);

bool evalString(string str1, int operate, string str2, string& result);
bool evalLong(string str1, int operate, string str2, long& result);
bool evalInteger(string str1, int operate, string str2, int& result);
bool evalDouble(string str1, int operate, string str2, double& result);
bool evalDate(string str1, int operate, string str2, string fmt, struct tm& result);
bool anyDataOperate(string str1, int operate, string str2, DataTypeStruct dts, string& result); // return true if operated successfully, result returns result

int detectDataType(string str, string & extrainfo); // detect the data type of an expression string
DataTypeStruct getCompatibleDataType(const DataTypeStruct & ldatatype, const DataTypeStruct & rdatatype); // get the compatible data type from two data types

//vector<string> split(const string& str, const string& delim); // split str to an array
vector<int> genlist(const int& start, const int& end, const int& step); // generate a number array

bool appendFile(const string & sContent, const string & sFile); // append content to a file

void dumpVector(vector<string> v);
void dumpMap(map<string, string> m);

#endif // __COMMFUNCS_H
