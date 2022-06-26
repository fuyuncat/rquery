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

QuerierC::QuerierC(string regexp)
{
  init();
  setregexp(regexp);
}

QuerierC::~QuerierC()
{

}

void QuerierC::init()
{
}

void QuerierC::setregexp(string regexp)
{
  m_regexp = regexp;
  m_rexp = sregex::compile(m_regexp);
}

void QuerierC::setrawstr(string rawstr)
{
  m_rawstr = rawstr;
}

int QuerierC::boostmatch(vector<string> *result)
{
  namesaving_smatch matches(m_regexp);
  //smatch matches;
  //boost::match_results<std::string::const_iterator>  matches;
  if ( result != NULL ) {
    printf("Matching %s => %s\n",m_rawstr.c_str(), m_regexp.c_str());
    result->clear();
    //if (boost::regex_match(m_rawstr, matches, m_rexp, boost::match_perl|boost::match_extra)) {
    if (regex_match(m_rawstr, matches, m_rexp)) {
      printf("Matched %d!", matches.size());
      //for (std::vector<std::string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      //  printf("%s: %s\n",string(*it).c_str(),matches[*it].str().c_str());
      for (int i=1; i<matches.size(); i++){
        //result->push_back(matches[i].str());
        printf("Matching %s => %s\n",matches[i].first, matches[i].second);
        result->push_back(matches[i]);
      }
    }
    //BOOST_THROW_EXCEPTION(
    //    regex_error(regex_constants::error_badmark, "invalid named back-reference")
    //);
  }
}

int QuerierC::boostmatch(map<string,string> & result)
{
  namesaving_smatch matches(m_regexp);
  printf("Matching %s => %s\n",m_rawstr.c_str(), m_regexp.c_str());
  if (regex_match(m_rawstr, matches, m_rexp)) {
    for (std::vector<std::string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      result[string(*it)] = matches[*it].str();
  }
}
