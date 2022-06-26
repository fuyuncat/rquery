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

using namespace std;

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

