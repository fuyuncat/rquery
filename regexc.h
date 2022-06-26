/*******************************************************************************
//
//        File: regexc.h
// Description: RegExC class header
//       Usage: regexc.h
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/

#ifndef __REGEXC_H
#define __REGEXC_H

#define REGEXC_VERSION 100

#include <regex.h>
#include <vector>
#include <string>
//#include <boost/regex.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>

using namespace std;
using namespace boost::xpressive;

class namesaving_smatch : public smatch
{
public:
    namesaving_smatch(const std::string pattern)
    {
        std::string pattern_str = pattern;
        sregex capture_pattern = sregex::compile("\\?P?<(\\w+)>");
        sregex_iterator words_begin = sregex_iterator(pattern_str.begin(), pattern_str.end(), capture_pattern);
        sregex_iterator words_end = sregex_iterator();

        for (sregex_iterator i = words_begin; i != words_end; i++)
        {
            std::string name = (*i)[1].str();
            m_names.push_back(name);
        }
    }

    ~namesaving_smatch() { }

    std::vector<std::string>::const_iterator names_begin() const
    {
        return m_names.begin();
    }

    std::vector<std::string>::const_iterator names_end() const
    {
        return m_names.end();
    }

private:
    std::vector<std::string> m_names;
};

class RegExC
{
  public:

    RegExC();
    ~RegExC();

    // Match string "str" with regular expression "regexp".
    // flag = 'i' to ignore case (see REG_ICASE in regex.h)
    // Optional "result" contain matched strings. Index 0 whole match, 1-9 substring from ()
    // return REG_... code from regex.h. REG_NOERROR is 0.
    int match( string regexp, string str, vector<string> *result = NULL, char flag = 0 );
    int boostmatch( string regexp, string str, vector<string> *result = NULL);
    int boostmatch(const string regexp, const string str, map<string,string> & result);

    // Sed command s///.
    // Shortest help :-)  result=`echo -n "str" | sed 's/regexp/newstr/'`
    // "newstr" can contain &, \1..9 - see man sed
    int sub( string regexp, string str, string newstr, string *result = NULL, char flag = 0 );

    // cflags for regcomp and eflags for regexec (see man regex)
    int set_cflags( int mask )   { cflags |= mask; return cflags; }
    int reset_cflags( int mask ) { cflags &= ~mask; return cflags; }
    int set_eflags( int mask )   { eflags |= mask; return eflags; }
    int reset_eflags( int mask ) { eflags &= ~mask; return eflags; }

  protected:

    int cflags;
    int eflags;
    char *regex;
    regex_t preg;
    int regerr;

    void init();

    int set_regexp( const char *regexp, int recompile = 0 );
};

#endif // __REGEXC_H

