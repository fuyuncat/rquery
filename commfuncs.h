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

#ifndef __COMMFUNCS_H
#define __COMMFUNCS_H

#include <regex.h>
#include <vector>
#include <string>
#include <set>
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
#define LEFT 1
#define RIGHT 2
#define STRING 1
#define LONG 2
#define INTEGER 3
#define DOUBLE 4
#define DATE 5
#define TIMESTAMP 6
#define BOOLEAN 7

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

extern size_t g_inputbuffer;

//string string_format( const string& format, Args ... args );
vector<string> split(string str, char delim = ' ', char quoter = '\"', char escape = '\\');
string trim_one(string str, char c = ' ');
string trim(string str, char c = ' ');
bool isNumber(const string& str);
bool isInt(const string& str);
bool isLong(const string& str);
bool isFloat(const string& str);
bool isDouble(const string& str);
bool isDate(const string& str, string& fmt);
bool like(string str1, string str2); 
bool reglike(string str, string regstr); 
//char getch();
//size_t getstr(char * buffer, const size_t len);
string decodeJunction(int junction);
string decodeComparator(int comparator);
string decodeDatatype(int datatype);
int encodeComparator(string str);
int encodeJunction(string str);
int findStrArrayId(const vector<string> array, const string member);
int anyDataCompare(string str1, string str2, int type);
int anyDataCompare(string str1, int comparator, string str2, int type);
int startsWithWords(string str, vector<string> words, int offset); // detect if string start with special words
int startsWithWords(string str, vector<string> words); // detect if string start with special words
string removeSpace(string originalStr, string keepPattern); //remove space
int matchQuoters(string listStr, int offset, string quoters); // detect if quoters matched.
string getFirstToken(string str, string token); //get the first matched regelar token from a string
vector <string> getAllTokens(string str, string token); //get all matched regelar token from a string
bool matchToken(string str, string token); // check if matched regelar token
int detectDataType(string str); // detect the data type of an expression string

#endif // __COMMFUNCS_H
