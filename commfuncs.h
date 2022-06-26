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

using namespace std;
using namespace boost::xpressive;

class namesaving_smatch : public smatch
{
public:
    namesaving_smatch(const std::string pattern);

    ~namesaving_smatch();

    std::vector<std::string>::const_iterator names_begin() const;

    std::vector<std::string>::const_iterator names_end() const;

private:
    std::vector<std::string> m_names;
};

vector<string> split(string str, char delim = ' ', char quoter = '\"', char escape = '\\');

#endif // __COMMFUNCS_H
