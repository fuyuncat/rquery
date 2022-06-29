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
  m_filter = NULL;
  m_rawstr = "";
  m_searchflags = regex_constants::match_default;
  m_matchcount = 0;
  m_outputrow = 0;
}

void QuerierC::setregexp(string regexstr)
{
  m_regexstr = regexstr;
  m_regexp = sregex::compile(m_regexstr);
}

void QuerierC::assignFilter(FilterC* filter)
{
  m_filter = filter;
}

void QuerierC::appendrawstr(string rawstr)
{
  m_rawstr.append(rawstr);
}

void QuerierC::setrawstr(string rawstr)
{
  m_rawstr = rawstr;
}

void QuerierC::pairFiledNames(namesaving_smatch matches)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 

  for (int i=0; i<matches.size(); i++){
    bool foundName = false;
    for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      if (&(matches[i]) == &(matches[*it])){
        m_fieldnames.push_back(string(*it));
        foundName = true;
      }
    if (!foundName){
      try{
        m_fieldnames.push_back("@field"+boost::lexical_cast<std::string>(i));
      }catch (bad_lexical_cast &){
        m_fieldnames.push_back("?");
      }
    }
  }
}

// filt a row data by filter
bool QuerierC::matchFilter(vector<string> rowValue, FilterC* filter)
{
  if (!filter)
    return true;
  bool matched = false; 
  if (filter->type == BRANCH){
    if (!filter->leftNode || !filter->rightNode){
      return false;
    }
    if (filter->junction == AND)  // and
      matched = matchFilter(rowValue, filter->leftNode) && matchFilter(rowValue, filter->rightNode);
    else // or
      matched = matchFilter(rowValue, filter->leftNode) || matchFilter(rowValue, filter->rightNode);
  }else if (filter->type == LEAF){
    if (filter->leftColId < 0) // filter is expression
      return !filter->leftExpression.empty()?!filter->rightExpression.empty():filter->leftExpression.compare(filter->rightExpression)==0;
    if (rowValue.size() == 0 || filter->leftColId > rowValue.size()-1)
      return false;
    //printf("left:%s %s right:%s (%s)\n",rowValue[filter->leftColId].c_str(),decodeComparator(filter->comparator).c_str(),filter->rightExpression.c_str(),decodeDatatype(filter->datatype).c_str());
    return anyDataCompare(rowValue[filter->leftColId], filter->comparator, filter->rightExpression, filter->datatype) == 1;
  }else{ // no predication means alway true
    return true;
  }
  return matched;
}

int QuerierC::searchNext()
{
  //string::const_iterator start = m_rawstr.begin(), end = m_rawstr.end();
  namesaving_smatch matches(m_regexstr);
  //smatch matches;
  int found = 0;
  if(regex_search(m_rawstr, matches, m_regexp)){
    //if(m_results.size()>0)
    //  formatoutput(m_results[0]);
    vector<string> matcheddata;
    for (int i=0; i<matches.size(); i++)
      matcheddata.push_back(matches[i]);
    if (m_fieldnames.size() == 0){
      pairFiledNames(matches);
      if (m_filter)
        m_filter->analyzeColumns(m_fieldnames, m_fieldnames);
    }
    if (matchFilter(matcheddata, m_filter)){
      m_results.push_back(matcheddata);
      m_matchcount++;
    }
    //m_results.push_back(matches);
    //vector<namesaving_smatch>::iterator p = m_results.end();
    //m_results.insert(p, matches);
    //if(m_results.size()>0)
    //  formatoutput(m_results[0]);
    //formatoutput(m_results[m_results.size()-1]);
    //printf("matched: %s; start pos: %d; len: %d\n",string(matches[0]).c_str(),m_rawstr.find(string(matches[0])),string(matches[0]).length());
    //printf("orig: %s\n",m_rawstr.c_str());
    //formatoutput(matches);

    //smatch m;
    //regex_search(m_rawstr, m, m_regexp);
    //for (int i=1; i<m.size(); i++)
    //  printf("%s\t",m[i].str().c_str());
    //printf("\n");

    size_t newlnpos = m_rawstr.find("\n",m_rawstr.find(matches[0])+matches[0].length()-1);
    newlnpos != string::npos?m_rawstr = m_rawstr.substr(newlnpos):m_rawstr = "";
    //printf("new: %s\n",m_rawstr.c_str());
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

//void QuerierC::formatoutput(namesaving_smatch matches)
//{
//  //for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
//  //  printf("%s\t", matches[*it].str().c_str());
//  for (int i=1; i<matches.size(); i++)
//    printf("%s\t",matches[i].str().c_str());
//  printf("\n");
//}

void QuerierC::formatoutput(vector<string> datas)
{
  m_outputrow++;
  for (int i=1; i<datas.size(); i++)
    printf("%d: %s\t",m_outputrow,datas[i].c_str());
  printf("\n");
}

void QuerierC::printFieldNames()
{
  for (int i=1; i<m_fieldnames.size(); i++)
    printf("%s\t",m_fieldnames[i].c_str());
  printf("\n");
}

void QuerierC::output()
{
  //printf("Result Num: %d\n",m_results.size());
  for (int i=0; i<m_results.size(); i++)
    formatoutput(m_results[i]);
}

void QuerierC::outputAndClean()
{
  output();
  m_results.clear();
}

int QuerierC::boostmatch(vector<string> *result)
{
  //namesaving_smatch matches(m_regexstr);
  smatch matches;
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
