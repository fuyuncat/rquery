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

#define VERSION "v1.221"

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
#define PERFM 6
#define ERROR 12
#define WARNING 13
#define INFO 14
#define DUMP 15
#define DEBUG2 98
#define DEBUG1 99
#define DEBUG 100

#define DATEFMT "%Y-%m-%d %H:%M:%S"

#define CONST 1
#define COLUMN 2
#define VARIABLE 3
#define FUNCTION 4
#define MACROPARA 5

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
#define COMPARENUM 54
#define COMPAREDATE 55
#define COUNTSTR 56
#define FIELDNAME 57
#define CONCAT 58
#define CONCATCOL 59
#define CALCOL 60
#define APPENDFILE 61
#define WHEN 62
#define GETPART 63
#define SUMALL 64
#define EVAL 65
#define RCOUNT 66
#define RMEMBER 67
#define RMEMBERID 68
#define EXEC 69
#define DETECTDT 70
#define ISLONG 71
#define ISDOUBLE 72
#define ISDATE 73
#define ISSTRING 74
#define COUNTPART 75
#define REGCOUNT 76
#define REGGET 77
#define GETPARTS 78
#define ISLEAP 79
#define WEEKDAY 80
#define MONTHFIRSTDAY 81
#define MONTHFIRSTMONDAY 82
#define YEARWEEK 83
#define DATETOLONG 84
#define LONGTODATE 85
#define URLENCODE 86
#define URLDECODE 87
#define BASE64ENCODE 88
#define BASE64DECODE 89
#define MD5 90
#define HASH 91
#define ISIP 92
#define ISIPV6 93
#define ISMAC 94
#define MYIPS 95
#define HOSTNAME 96
#define RMEMBERS 97
#define ISFILE 98
#define ISFOLDER 99
#define FILEEXIST 100
#define ACOS 101
#define ACOSH 102
#define ASIN 103
#define ASINH 104
#define ATAN 105
#define ATAN2 106
#define ATANH 107
#define CBRT 108
#define COPYSIGN 109
#define COS 110
#define COSH 111
#define ERF 112
#define EXP 113
#define EXP2 114
#define FMA 115
#define FMOD 116
#define FPCLASSIFY 117
#define HYPOT 118
#define ILOGB 119
#define ISFINITE 120
#define ISINF 121
#define ISNORMAL 122
#define LGAMMA 123
#define LOG10 124
#define LOG2 125
#define POW 126
#define REMAINDER 127
#define SCALBLN 128
#define SCALBN 129
#define SIN 130
#define SINH 131
#define SQRT 132
#define TAN 133
#define TANH 134
#define TGAMMA 135
#define PI 136
#define TRUNCDATEU 137
#define YEARDAY 138
#define LOCALTIME 139
#define GMTIME 140
#define RMFILE 141
#define RENAMEFILE 142
#define FILESIZE 143
#define FILEATTRS 144
#define ISEXECUTABLE 145
#define ISSYMBLINK 146
#define GETSYMBLINK 147
#define DUPLICATE 148
#define SUM 201
#define COUNT 202
#define UNIQUECOUNT 203
#define MAX 204
#define MIN 205
#define AVERAGE 206
#define GROUPLIST 207
#define RANK 301
#define PREVIOUS 302
#define NEXT 303
#define SEQNUM 304
#define DENSERANK 305
#define NEARBY 306
#define SUMA 307
#define COUNTA 308
#define UNIQUECOUNTA 309
#define MAXA 310
#define MINA 311
#define AVERAGEA 312
#define FOREACH 501
#define COLTOROW 502
#define ANYCOL 503
#define ALLCOL 504
#define ROOT 601
#define PATH 602
#define PARENT 603
#define USERMACROFUNC 888

#define PI_VAL 3.14159265358979323846

#define TEXT 1
#define JSON 2

#define REGSEARCH 1
#define WILDSEARCH 2
#define DELMSEARCH 3
#define LINESEARCH 4

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
  short int g_tracelevel = FATAL;
  bool g_printheader;
  bool g_showprogress;
  bool g_fileheaderonly;
  bool g_statonly;
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

//struct MacroParaStruct{
//  string name; // parameter name
//  int seq; // sequence id of the parameter
//  string initval; // initial value of the parameter.
//};

class ExpressionC;

struct MacroFuncStruct{
  string sFuncName; // function name
  ExpressionC* funcExpr; // function expression
  vector <string> vParaNames; // parameter name
};

// runtime data struct. The memory of the pointers pointing to should never be freed from this struct.
struct RuntimeDataStruct{
  vector<string>* fieldvalues = NULL;
  unordered_map<string,string>* varvalues = NULL;
  unordered_map< string,GroupProp >* aggFuncs = NULL;
  unordered_map< string,vector<string> >* anaFuncs = NULL;
  vector< vector< unordered_map<string,string> > >* sideDatasets = NULL;
  unordered_map< string, unordered_map<string,string> >* sideDatarow = NULL;
  unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes = NULL;
  unordered_map< string,string >* macroFuncParas = NULL;
  unordered_map< string,MacroFuncStruct >* macroFuncExprs = NULL;
};

extern GlobalVars gv;
extern std::set<string> g_userMacroFuncNames; // the global array is used for identify the user defined macro functions

#ifdef __DEBUG__
extern long int g_strtodatetime;
extern long int g_datetostrtime;
extern long int g_zonetimetime;
extern long int g_truncdatetime;
extern long int g_evalexprtime;
extern long int g_evalexprconsttime;
extern long int g_evalexprfunctime;
extern long int g_evalexprvartime;
extern long int g_evalexprcoltime;
extern long int g_evalexprmacpatime;
extern long int g_evalexprcaltime;
#endif // __DEBUG__

class TimeZone{
public:
  TimeZone();
  ~TimeZone();
  
  void init();
  void dump();
  
  static unordered_map< string, int > m_nameoffmap;
};

void trace(const short & level, const char *fmt, ...);
void exitProgram(const short int & code);
void clearGroupPropMap(unordered_map< string,GroupProp > & aggProps); // manually free the memory of GroupProp, as it's not freed in the destructor to improve performance. 
#if defined unix || defined __unix || defined __unix__ || defined __APPLE__ || defined __MACH__ || defined __linux__ || defined linux || defined __linux || defined __FreeBSD__ || defined __ANDROID__
string exec(const string & cmd);
#endif

//string string_format( const string& format, Args ... args );
string readQuotedStr(const string & str, size_t& pos, const string & targetquoters, const string & quoters = "", const char & escape = '\\', const std::set<char> & nestedQuoters={}); // return most outer quoted string. pos is start pos and return the position of next char of the end of the quoted string.  
int matchQuoters(const string & listStr, const size_t & offset, const string & quoters); // detect if quoters matched.
void split(vector<string> & v, const string & str, string & delim, const string & quoters = "''", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}, const bool & repeatable=false, const bool & skipemptyelement=false, const bool & delimcasesensive=false); // split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
void split(vector<string> & v, const string & str, const char & delim = ' ', const string & quoters = "''", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}, const bool & repeatable=false, const bool & skipemptyelement=false); // split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
void split(vector<string> & v, const string & str, const std::set<char> & delims = {' ','\t','\n','\r'}, const string & quoters = "''\"\"", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}, const bool & repeatable=false, const bool & skipemptyelement=false); // split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
size_t findNthCharacter(const string & str, const std::set<char> & lookfor, const size_t & pos=0, const int & seq=1, const bool & forward=true, const string & quoters = "''()", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}); // find the Nth(seq) position of the any character in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
size_t findFirstCharacter(const string & str, const std::set<char> & lookfor, const size_t & pos=0, const string & quoters = "''()", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}); // find the first position of the any character in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
size_t findNthSub(const string & str, const string & lookfor, const size_t & pos=0, const int & seq=1, const bool & forward=true, const string & quoters = "''()", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}, const bool & casesensitive=true); // find the Nth(seq) position of a substring in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
size_t findFirstSub(const string & str, const string & lookfor, const size_t & pos=0, const string & quoters = "''()", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}, const bool & casesensitive=true); // find the first position of a substring in a given string, return -1 if not found.  The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting
void readLine(const string & str, size_t & pos, string & sline); // read a line
void matchWildcard(vector<string> & matches, const string & str, const string & wildStr, const string & quoters = "\"\"", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'}); // match wildcard to transfer a string to a vector
string readWordTillStop(const string & str, size_t & pos, const char & stopper='*', const char & escape = '\\'); // read the first word until reaches stopper
string concatArray(const vector<string> & array, const string & delim = ""); // concatenate an array to string
void eliminateDups(vector<string> & array); // eliminate duplicates

int startsWithWords(const string & str, const vector<string> & words, const int & offset); // detect if string start with special words
int startsWithWords(const string & str, const vector<string> & words); // detect if string start with special words
string removeSpace(const string & originalStr, const string & keepPattern); //remove space
string getFirstToken(const string & str, const string & token); //get the first matched regelar token from a string
void getAllTokens(vector < vector <string> > & findings, const string & str, const string & token); //get all matched regelar token from a string
bool matchToken(const string & str, const string & token); // check if matched regelar token

string trim_pair(const string & str, const string & pair);
string trim_left(const string & str, const char & c = ' ', const bool & repeat=false);
string trim_right(const string & str, const char & c = ' ', const bool & repeat=false);
string trim_one(const string & str, const char & c = ' ');
string trim(const string & str, const char & c = ' ', const bool & repeat=true);
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
void replaceunquotedstr(string & str, const string & sReplace, const string & sNew, const string & quoters = "''", const char & escape = '\\', const std::set<char> & nestedQuoters={'(',')'});
void regreplacestr(string & sRaw, const string & sPattern, const string & sNew);
void regmatchstr(const string & sRaw, const string & sPattern, string & sExpr);
char from_hex(const char & ch);
char to_hex(const char & code);
string urlencode(const string & sUrl);
string urldecode(const string & sEncoded);
string base64encode(const string & str);
string base64decode(const string & sEncoded);
string md5(const string & str);
string hashstr(const string & str);

bool isNumber(const string& str);
bool isInt(const string& str);
bool isLong(const string& str);
bool isFloat(const string& str);
bool isDouble(const string& str);
bool isDate(const string& str, int & iOffSet, string& fmt);
string intToStr(const int & val);
string longToStr(const long & val);
string longlongToStr(const long long & val);
string longuintToStr(const long unsigned int & val);
string floatToStr(const float & val);
string doubleToStr(const double & val);
string dateToStr(struct tm & val, const int & iOffSet = 888888, const string & fmt = DATEFMT);
string dectohex(const int & val);
int hextodec(const string& str);
string dectobin(const int & val);
int bintodec(const string& str);

void cleanuptm(struct tm & tm); // clean up the messy date in tm, e.g. 2022 10 18 839979404 32764 839979424 => 2022 10 18 0 0 0
bool strToDate(const string & str, struct tm & tm, int & iOffSet, const string & fmt=DATEFMT);
int localOffset();
struct tm zonetime(const time_t & t1, const string & zone);
struct tm zonetime(const time_t & t1, const int & iOffSet);
int dateFormatLen(const string & fmt);
string stripTimeZone(const string & str, int & iOffSet, string & sTimeZone);
struct tm now();
long int curtime();
double timediff(struct tm & tm1, struct tm & tm2);
void addhours(struct tm & tm, const int & hours);
void addseconds(struct tm & tm, const int & seconds);
void addtime(struct tm & tm, const int & diff, const char & unit='S');
string truncdate(const string & datesrc, const string & fmt, const int & seconds);
string truncdateu(const string & datesrc, const string & fmt, const char & unit);
string monthfirstmonday(const string & datesrc, const string & fmt);
int yearweek(const string & datesrc, const string & fmt);
int yearday(const string & datesrc, const string & fmt);
string rqlocaltime(const string & datesrc, const string & fmt);
string rqgmtime(const string & datesrc, const string & fmt, const double & gmtoff);
struct tm convertzone(const struct tm & tm, const string & fromzone, const string & tozone);
string convertzone(const string & sdate, const string & sFmt, const string & fromzone, const string & tozone);

int random(const int & min=1, const int & max=100);
string randstr(const int & len=8, const string & flags="uld");

bool like(const string & str1, const string & str2); // str1 like str2 (containing wildcard)
bool reglike(const string & str, const string & regstr); 
bool in(const string & str1, const string & str2); 
//char getch();
//size_t getstr(char * buffer, const size_t len);

string decodeJunction(const int & junction);
string decodeComparator(const int & comparator);
string decodeDatatype(const int & datatype);
string decodeOperator(const int & op);
string decodeExptype(const int & exptype);
string decodeTracelevel(const int & level);
short int encodeComparator(const string & str);
short int encodeDatatype(const string & str);
short int encodeJunction(const string & str);
short int encodeOperator(const string & str);
short int encodeTracelevel(const string & str);
short int encodeFunction(const string & str);
short int operatorPriority(const int & iOperator);
int findStrArrayId(const vector<string> & array, const string & member);

int anyDataCompare(const string & str1, const string & str2, const DataTypeStruct & dts1, const DataTypeStruct & dts2);  // @return int str1 < str2: -1; str1 == str2:0; str1 > str2: 1
int anyDataCompare(const string & str1, const int & comparator, const string & str2, const DataTypeStruct & dts1, const DataTypeStruct & dts2);
int compareVector(const vector<string> & array1, const vector<string> & array2);

bool evalString(const string & str1, const int & operate, const string & str2, string& result);
bool evalLong(const string & str1, const int & operate, const string & str2, long& result);
bool evalInteger(const string & str1, const int & operate, const string & str2, int& result);
bool evalDouble(const string & str1, const int & operate, const string & str2, double& result);
bool evalDate(const string & str1, const int & operate, const string & str2, const string & fmt, struct tm& result);
bool anyDataOperate(const string & str1, const int & operate, const string & str2, const DataTypeStruct & dts, string& result); // return true if operated successfully, result returns result

int detectDataType(const string & str, string & extrainfo); // detect the data type of an expression string
DataTypeStruct getCompatibleDataType(const DataTypeStruct & ldatatype, const DataTypeStruct & rdatatype); // get the compatible data type from two data types

//vector<string> split(const string& str, const string& delim); // split str to an array
void genlist(vector<int> & vLiet, const int& start, const int& end, const int& step); // generate a number array

string hostname();
unordered_map< string,string > getmyips();

short int checkReadMode(const string & sContent);
short int getReadMode(const string & filepath);
string getFileModeStr(const string & filepath);
size_t getFileSize(const string &  filepath);
bool fileexist(const string & filepath);
bool appendFile(const string & sContent, const string & sFile); // append content to a file
bool renameFile(const string & oldname, const string & newname);
bool rmFile(const string & filename);
bool issymblink(const string & filepath);
string getsymblink(const string & filepath);
bool issymlkloop(const string & filepath);
bool isexecutable(const string & filepath);

void dumpVector(const vector<string> & v);
void dumpMap(map<string, string> & m);

#endif // __COMMFUNCS_H
