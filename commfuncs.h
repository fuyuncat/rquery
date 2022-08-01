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
#include <ctime>
#include <fstream>

using namespace std;

#define VERSION "v0.912"

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
#define ZONECONVERT 25
#define SUM 101
#define COUNT 102
#define UNIQUECOUNT 103
#define MAX 104
#define MIN 105
#define AVERAGE 106
#define FOREACH 501

#define TEXT 1
#define JSON 2

struct DataTypeStruct{
  int datatype=UNKNOWN;
  string extrainfo="";  // for DATE type only so far, the format of the DATE 

  // We only need these operators when this struct is being used as Key of Map
  //bool operator==(const DataTypeStruct& x) const {
  //  return (this->datatype==x.datatype && this->extrainfo.compare(x.extrainfo)==0);
  //}

  //bool operator<(const DataTypeStruct& x) const {
  //  return (this->datatype<x.datatype || (this->datatype==x.datatype && this->extrainfo.compare(x.extrainfo)<0));
  //}
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
  long count;
  long double sum;
  string max;
  string min;
  std::set <string> uniquec;
  bool inited = false;
};

extern GlobalVars gv;

void trace(short level, const char *fmt, ...);
void exitProgram(short int code);
//string string_format( const string& format, Args ... args );
string readQuotedStr(string str, int& pos, string quoters, char escape = '\\'); // return most outer quoted string. pos is start pos and return the position of next char of the end of the quoted string.  
int matchQuoters(string listStr, int offset, string quoters); // detect if quoters matched.
vector<string> split(const string & str, string delim = " ", string quoters = "''", char escape = '\\', std::set<char> nestedQuoters={'(',')'}); // split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
vector<string> split(const string & str, char delim = ' ', string quoters = "''", char escape = '\\', std::set<char> nestedQuoters={'(',')'}); // split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
int findFirstCharacter(string str, std::set<char> lookfor, int pos=0, string quoters = "''{}()",  char escape = '\\', std::set<char> nestedQuoters={'(',')'}); // find the first position of the any character in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting

int startsWithWords(string str, vector<string> words, int offset); // detect if string start with special words
int startsWithWords(string str, vector<string> words); // detect if string start with special words
string removeSpace(string originalStr, string keepPattern); //remove space
string getFirstToken(string str, string token); //get the first matched regelar token from a string
vector <string> getAllTokens(string str, string token); //get all matched regelar token from a string
bool matchToken(string str, string token); // check if matched regelar token

string trim_pair(const string & str, const string & pair);
string trim_left(const string & str, char c = ' ');
string trim_right(const string & str, char c = ' ');
string trim_one(const string & str, char c = ' ');
string trim(const string & str, char c = ' ');
string trim_copy(const string & str);
string upper_copy(const string & str);
string lower_copy(const string & str);
void replacestr(string & sRaw, const string & sReplace, const string & sNew);
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

bool strToDate(string str, struct tm & tm, int & iOffSet, string fmt=DATEFMT);
struct tm zonetime(time_t t1, int iOffSet);
int dateFormatLen(string fmt);
string stripTimeZone(string str, int & iOffSet, string & sTimeZone);
struct tm now();
long int curtime();

bool like(string str1, string str2); 
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

void dumpVector(vector<string> v);
void dumpMap(map<string, string> m);

#endif // __COMMFUNCS_H