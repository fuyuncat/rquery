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
#define NOT 8
#define IN 9
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

vector<string> split(string str, char delim = ' ', char quoter = '\"', char escape = '\\');
string trim_one(string str, char c = ' ');
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
int findStrArrayId(const vector<string> array, const string member);
int anyDataCompare(string str1, string str2, int type);
int anyDataCompare(string str1, int comparator, string str2, int type);

#endif // __COMMFUNCS_H
