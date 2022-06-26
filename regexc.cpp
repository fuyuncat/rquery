/*******************************************************************************
//
//        File: regexc.cpp
// Description: RegExC class defination
//       Usage: regexc.cpp
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include "regexc.h"

using namespace boost::xpressive;

RegExC::RegExC()
{
  init();
}


RegExC::~RegExC()
{
  regfree( &preg );
  free( regex );
}


void RegExC::init()
{
  cflags = REG_EXTENDED;
  eflags = 0;
  regex = strdup( "" );
  regerr = regcomp( &preg, regex, 0 );
}


int RegExC::set_regexp( const char *regexp, int recompile )
{
  if ( !regexp || !*regexp ) return -1;

  if ( strcmp( regexp, regex ) )
  {
      recompile = 1;
      free( regex );
      regex = strdup( regexp );
  }

  if ( recompile ) 
  {
      regfree( &preg );
      regerr = regcomp( &preg, regex, cflags );
  }
  
  return regerr;
}


int RegExC::match( const string regexp, const string str, vector<string> *result, char flag )
{
  regmatch_t match[ 10 ];
  int cf = cflags;
  if ( flag == 'i' ) set_cflags( REG_ICASE );
  else               reset_cflags( REG_ICASE );

  if ( set_regexp( regexp.c_str(), cf != cflags ) != REG_NOERROR ) return regerr;

  int regres = regexec( &preg, str.c_str(), 10, match, eflags );

  if ( regres == REG_NOERROR && result != NULL )
  {
      result->clear();
      for ( int i = 0; match[ i ].rm_so != -1 && i < 10; i++ )
          result->push_back( str.substr( match[ i ].rm_so, match[ i ].rm_eo - match[ i ].rm_so ) );
  }
  return regres;
}

int RegExC::boostmatch(const string regexp, const string str, vector<string> *result)
{
  //boost::regex rexp(regexp);
  sregex rexp = sregex::compile(regexp);
  namesaving_smatch matches(regexp);
  //smatch matches;
  //boost::match_results<std::string::const_iterator>  matches;
  if ( result != NULL ) {
    //printf("Matching &s => %s\n",str.c_str(), regexp.c_str());
    result->clear();
    //if (boost::regex_match(str, matches, rexp, boost::match_perl|boost::match_extra)) {
    if (regex_match(str, matches, rexp)) {
      //printf("Matched %d!", matches.size());
      //for (std::vector<std::string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      //  printf("%s: %s\n",string(*it).c_str(),matches[*it].str().c_str());
      for (int i=1; i<matches.size(); i++){
        //result->push_back(matches[i].str());
        // printf("Matching &s => %s\n",matches[i].name_, matches[i].second);
        result->push_back(matches[i]);
      }
    }
    //BOOST_THROW_EXCEPTION(
    //    regex_error(regex_constants::error_badmark, "invalid named back-reference")
    //);
  }
}

int RegExC::boostmatch(const string regexp, const string str, map<string,string> & result)
{
  sregex rexp = sregex::compile(regexp);
  namesaving_smatch matches(regexp);
  if (regex_match(str, matches, rexp)) {
    for (std::vector<std::string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      result[string(*it)] = matches[*it].str();
  }
}

int RegExC::sub( string regexp, string str, string newstr, string *result, char flag )
{
  vector<string> res;

  int regres = match( regexp, str, &res, flag );

  if ( regres == REG_NOERROR && result != NULL )
  {
      int pos = 0;
      // replace all & with res[ 0 ]. Do not replace "\&"
      while ( ( pos = newstr.find( '&', pos ) ) != string::npos )
      {
          if ( pos > 0 )
              if ( newstr[ pos - 1 ] == '\\' )
              {
                  newstr.erase( pos - 1, 1 );
                  continue;
              }
          newstr.replace( pos, 1, res[ 0 ] );
          pos += res[ 0 ].size();
      }

      string substr( "\\0" );
      // replace all \1..9 with res[ 1..9 ]. Do not replace "\\N"
      for ( int i = 1; i < res.size(); i++ )
      {
          substr[ 1 ] = '0' + i;
          pos = 0;
          while ( ( pos = newstr.find( substr, pos )  ) != string::npos )
          {
              if ( pos > 0 )
                  if ( newstr[ pos - 1 ] == '\\' )
                  {
                      newstr.erase( pos, 1 );
                      continue;
                  }
              newstr.replace( pos, 2, res[ i ] );
              pos += res[ i ].size();
          }
      }
      // replace matched string in str with newstr
      pos = str.find( res[ 0 ] );
      str.replace( pos, res[ 0 ].size(), newstr );
      *result = str;
  }

  return regres;
}

