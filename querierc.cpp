/*******************************************************************************
//
//        File: querierc.cpp
// Description: Querier class defination
//       Usage: querierc.cpp
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
#include "commfuncs.h"
#include "querierc.h"

QuerierC::QuerierC()
{
  init();
}


QuerierC::~QuerierC()
{
  regfree( &preg );
  free( regex );
}


void QuerierC::init()
{
}

int QuerierC::boostmatch(const string regexp, const string str, vector<string> *result)
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

int QuerierC::boostmatch(const string regexp, const string str, map<string,string> & result)
{
  sregex rexp = sregex::compile(regexp);
  namesaving_smatch matches(regexp);
  if (regex_match(str, matches, rexp)) {
    for (std::vector<std::string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      result[string(*it)] = matches[*it].str();
  }
}
