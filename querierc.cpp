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
#include "querierc.h"

QuerierC::QuerierC()
{
  init();
}

QuerierC::QuerierC(string regexstr)
{
  init();
  setregexp(regexstr);
}

QuerierC::~QuerierC()
{

}

void QuerierC::init()
{
  m_searchflags = regex_constants::match_default;
}

void QuerierC::setregexp(string regexstr)
{
  m_regexstr = regexstr;
  m_regexp = sregex::compile(m_regexstr);
}

void QuerierC::setrawstr(string rawstr)
{
  m_rawstr = rawstr;
}

int QuerierC::searchNext()
{
  //string::const_iterator start = m_rawstr.begin(), end = m_rawstr.end();
  namesaving_smatch matches(m_regexstr);
  int found = 0;
  if(regex_search(m_rawstr, matches, m_regexp)){
    m_results.push_back(matches);
    printf("matched: %s; start pos: %d; len: %d\n",string(matches[0]).c_str(),m_rawstr.find(string(matches[0])),string(matches[0]).length());
    printf("orig: %s\n",m_rawstr.c_str());
    m_rawstr = m_rawstr.substr(m_rawstr.find(matches[0])+matches[0].length());
    printf("new: %s\n",m_rawstr.c_str());
    formatoutput(matches);
    //m_rawstr.emplace_back( start, matches[0].first );
    //auto start = distance(m_rawstr.begin(),start);
    //auto len   = distance(start, matches[0].first);
    //auto sub_str = m_rawstr.substr(start,len);
    found++;
  }
  return found;
}

int QuerierC::searchAll()
{
  int totalfound = 0;
  int found = searchNext();
  while (found>0){
    found = searchNext();
    totalfound+=found;
  }
  return totalfound;
}

void QuerierC::formatoutput(namesaving_smatch matches)
{
  //for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
  //  printf("%s\t", matches[*it].str().c_str());
  for (int i=1; i<matches.size(); i++){
    printf("%s\t",matches[i].str().c_str());
  printf("\n");
}

void QuerierC::output()
{
  printf("Result Num: %d\n",m_results.size());
  for (int i=0; i<m_results.size(); i++){
    formatoutput(m_results[i]);
  }
}

void QuerierC::outputAndClean()
{
  output();
  m_results.clear();
}

int QuerierC::boostmatch(vector<string> *result)
{
  namesaving_smatch matches(m_regexstr);
  //smatch matches;
  //boost::match_results<string::const_iterator>  matches;
  if ( result != NULL ) {
    //printf("Matching %s => %s\n",m_rawstr.c_str(), m_regexstr.c_str());
    result->clear();
    //if (boost::regex_match(m_rawstr, matches, m_regexp, boost::match_perl|boost::match_extra)) {
    if (regex_match(m_rawstr, matches, m_regexp)) {
      //printf("Matched %d!\n", matches.size());
      //for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      //  printf("%s: %s\n",string(*it).c_str(),matches[*it].str().c_str());
      for (int i=1; i<matches.size(); i++){
        //result->push_back(matches[i].str());
        //printf("Matching %s => %s\n",matches[i].first, matches[i].second);
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
  namesaving_smatch matches(m_regexstr);
  //printf("Matching %s => %s\n",m_rawstr.c_str(), m_regexstr.c_str());
  if (regex_match(m_rawstr, matches, m_regexp)) {
    for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      result[string(*it)] = matches[*it].str();
  }
}
