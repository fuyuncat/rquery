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
#ifndef __COMMFUNCS_H
#define __COMMFUNCS_H

#include <regex.h>
#include <vector>
#include <string>
#include <set>
#include <ctime>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>

using namespace std;
using namespace boost::xpressive;

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
#define FATAL 1
#define ERROR 2
#define WARNING 3
#define INFO 4
#define DEBUG 9
#define DATEFMT "%Y-%m-%d %H:%M:%S"
#define CONST 1
#define COLUMN 2
#define VARIABLE 3
#define FUNCTION 4

class namesaving_smatch : public smatch
{
public:
  namesaving_smatch();

  namesaving_smatch(const string pattern);

  ~namesaving_smatch();
  
  void init(const string pattern);

  vector<string>::const_iterator names_begin() const;

  vector<string>::const_iterator names_end() const;

private:
  vector<string> m_names;
};

//template<typename ... Args>

class GlobalVars{
public:
  GlobalVars();
  ~GlobalVars();
  
  void initVars();
  void setVars(size_t inputbuffer = 16384, short tracelevel=FATAL, bool printheader=TRUE);

  static size_t g_inputbuffer;
  static short g_tracelevel;
  static bool g_printheader;
};

void trace(short level, const char *fmt, ...);
//string string_format( const string& format, Args ... args );
string readQuotedStr(string str, int& pos, string quoters, char escape = '\\'); // return most outer quoted string. pos is start pos and return the position of next char of the end of the quoted string.  
int matchQuoters(string listStr, int offset, string quoters); // detect if quoters matched.
vector<string> split(string str, char delim = ' ', string quoters = "''", char escape = '\\'); // split string by delim, skip the delim in the quoted part. The chars with even sequence number in quoters are left quoters, odd sequence number chars are right quoters. No nested quoting

string trim_pair(string str, string pair);
string trim_left(string str, char c = ' ');
string trim_right(string str, char c = ' ');
string trim_one(string str, char c = ' ');
string trim(string str, char c = ' ');
void replacestr(string & sRaw, string sReplace, string sNew);

bool isNumber(const string& str);
bool isInt(const string& str);
bool isLong(const string& str);
bool isFloat(const string& str);
bool isDouble(const string& str);
bool isDate(const string& str, string& fmt);
string intToStr(const int val);
string longToStr(const long val);
string floatToStr(const float val);
string doubleToStr(const double val);
string dateToStr(struct tm val, string fmt = DATEFMT);

bool strToDate(string str, struct tm & tm);
struct tm now();

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
int encodeComparator(string str);
int encodeDatatype(string str);
int encodeJunction(string str);
int encodeOperator(string str);
int findStrArrayId(const vector<string> array, const string member);

int anyDataCompare(string str1, string str2, int type);
int anyDataCompare(string str1, int comparator, string str2, int type);

bool evalString(string str1, int operate, string str2, string& result);
bool evalLong(string str1, int operate, string str2, long& result);
bool evalInteger(string str1, int operate, string str2, int& result);
bool evalDouble(string str1, int operate, string str2, double& result);
bool evalDate(string str1, int operate, string str2, struct tm& result);
bool anyDataOperate(string str1, int operate, string str2, int type, string& result); // return true if operated successfully, result returns result

int detectDataType(string str); // detect the data type of an expression string
int getCompatibleDataType(int ldatatype, int rdatatype); // get the compatible data type from two data types

int startsWithWords(string str, vector<string> words, int offset); // detect if string start with special words
int startsWithWords(string str, vector<string> words); // detect if string start with special words
string removeSpace(string originalStr, string keepPattern); //remove space
string getFirstToken(string str, string token); //get the first matched regelar token from a string
vector <string> getAllTokens(string str, string token); //get all matched regelar token from a string
bool matchToken(string str, string token); // check if matched regelar token

void dumpVector(vector<string> v);
void dumpMap(map<string, string> m);

#endif // __COMMFUNCS_H
