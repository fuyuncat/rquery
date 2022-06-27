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
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <unistd.h>
#include <termios.h>

using namespace std;
using namespace boost::xpressive;

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
char getch();

#endif // __COMMFUNCS_H
