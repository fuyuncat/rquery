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
#include "expression.h"
#include <boost/algorithm/string.hpp>
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
  m_filename = "";
  m_line = 0;
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
  m_filter->dump();
}

void QuerierC::appendrawstr(string rawstr)
{
  m_rawstr.append(rawstr);
}

void QuerierC::setrawstr(string rawstr)
{
  m_rawstr = rawstr;
}

bool QuerierC::assignSelString(string selstr)
{
  vector<string> vSelections = split(selstr,',',"//''{}",'\\');
  for (int i=0; i<vSelections.size(); i++){
    trace(DEBUG, "Processing selection(%d) '%s'!\n", i, vSelections[i].c_str());
    string sSel = boost::algorithm::trim_copy<string>(vSelections[i]);
    if (sSel.empty()){
      trace(ERROR, "Empty selection string!\n");
      return false;
    }
    ExpressionC eSel(sSel);
    m_selections.push_back(eSel);
  }
  return true;
}

void QuerierC::pairFiledNames(namesaving_smatch matches)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 

  for (int i=1; i<matches.size(); i++){
    bool foundName = false;
    for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      if (&(matches[i]) == &(matches[*it])){
        m_fieldnames.push_back(boost::to_upper_copy<string>(string(*it)));
        foundName = true;
      }
    if (!foundName){
      try{
        m_fieldnames.push_back("@FIELD"+boost::lexical_cast<std::string>(i));
      }catch (bad_lexical_cast &){
        m_fieldnames.push_back("?");
      }
    }
  }
}

void QuerierC::setFieldDatatype(string field, int datetype)
{
  string fname = boost::to_upper_copy<string>(field);
  if (m_fieldntypes.find(fname) != m_fieldntypes.end())
    m_fieldntypes[fname] = datetype;
  else
    m_fieldntypes.insert( pair<string, int>(fname,datetype) );
  for (int i=0;i<m_fieldnames.size();i++){
    if (fname.compare(m_fieldnames[i]) == 0 && i<m_fieldtypes.size())
      m_fieldtypes[i] = datetype;
  }
}

void QuerierC::analyzeFiledTypes(namesaving_smatch matches)
{
  m_fieldtypes.clear();
  for (int i=1; i<matches.size(); i++){
    if (m_fieldnames.size()>i-1 && m_fieldntypes.find(m_fieldnames[i-1]) != m_fieldntypes.end()) 
      m_fieldtypes.push_back(m_fieldntypes[m_fieldnames[i-1]]);
    else if (m_fieldntypes.find("@FIELD"+intToStr(i)) != m_fieldntypes.end())
      m_fieldtypes.push_back(m_fieldntypes["@FIELD"+intToStr(i)]);
    else{
      int iType = detectDataType(matches[i]);
      m_fieldtypes.push_back(iType==UNKNOWN?STRING:iType); // set UNKNOWN (real) data as STRING
    }
    trace(DEBUG, "Detected column '%s' data type '%s'\n", m_fieldnames[i-1].c_str(), decodeDatatype(m_fieldtypes[m_fieldtypes.size()-1]).c_str());
  }
}

// filt a row data by filter. no predication mean true. comparasion failed means alway false
bool QuerierC::matchFilter(vector<string> rowValue, FilterC* filter)
{
  if (!filter){
    //trace(INFO, "No filter defined\n");
    return true;
  }
  if (rowValue.size() != m_fieldnames.size() + 3){ // field name number + 3 variables (@raw @line @row)
    trace(ERROR, "Filed number %d and value number %d dont match!\n", m_fieldnames.size(), rowValue.size());
    dumpVector(m_fieldnames);
    dumpVector(rowValue);
    return false;
  }
  bool matched = false; 
  vector<string> fieldValues;
  map<string, string> varValues;
  for (int i=0; i<m_fieldnames.size(); i++)
    fieldValues.push_back(rowValue[i+1]);
    //fieldValues.insert( pair<string,string>(boost::algorithm::to_upper_copy<string>(m_fieldnames[i]),rowValue[i+1]));
  varValues.insert( pair<string,string>("@RAW",rowValue[0]));
  varValues.insert( pair<string,string>("@FILE",m_filename));
  varValues.insert( pair<string,string>("@LINE",rowValue[m_fieldnames.size()+1]));
  varValues.insert( pair<string,string>("@ROW",rowValue[m_fieldnames.size()+2]));
  bool bMatchedbMatched = filter->compareExpression(&m_fieldnames, &fieldValues, &varValues);
  if (bMatchedbMatched){
    if (m_selections.size()>0){
      vector<string> vResults;
      vResults.push_back(rowValue[0]);
      for (int i=0; i<m_selections.size(); i++){
        string sResult;
        m_selections[0].evalExpression(&m_fieldnames, &fieldValues, &varValues, sResult);
        vResults.push_back(sResult);
      }
      m_results.push_back(vResults);
    }else
      m_results.push_back(rowValue);
  }
  return bMatchedbMatched;

  //if (filter->m_type == BRANCH){
  //  if (!filter->m_leftNode || !filter->m_rightNode){
  //    return false;
  //  }
  //  if (filter->m_junction == AND)  // and
  //    matched = matchFilter(rowValue, filter->m_leftNode) && matchFilter(rowValue, filter->m_rightNode);
  //  else // or
  //    matched = matchFilter(rowValue, filter->m_leftNode) || matchFilter(rowValue, filter->m_rightNode);
  //}else if (filter->m_type == LEAF){
  //  if (filter->m_leftColId < 0) // filter is expression
  //    return !filter->m_leftExpStr.empty()?!filter->m_rightExpStr.empty():filter->m_leftExpStr.compare(filter->m_rightExpStr)==0;
  //  if (rowValue.size() == 0 || filter->m_leftColId > rowValue.size()-1)
  //    return false;
  //  //printf("left:%s %s right:%s (%s)\n",rowValue[filter->m_leftColId].c_str(),decodeComparator(filter->m_comparator).c_str(),filter->m_rightExpStr.c_str(),decodeDatatype(filter->m_datatype).c_str());
  //  return anyDataCompare(rowValue[filter->m_leftColId], filter->m_comparator, filter->m_rightExpStr, filter->m_datatype) == 1;
  //}else{ // no predication means alway true
  //  return false;
  //}
  return matched;
}

int QuerierC::searchNext()
{
  //string::const_iterator start = m_rawstr.begin(), end = m_rawstr.end();
  namesaving_smatch matches(m_regexstr);
  //smatch matches;
  int found = 0;
  m_line++;
  try {
    if(regex_search(m_rawstr, matches, m_regexp)){
      //if(m_results.size()>0)
      //  formatoutput(m_results[0]);
      vector<string> matcheddata;
      for (int i=0; i<matches.size(); i++)
        matcheddata.push_back(matches[i]);
      if (m_fieldnames.size() == 0){
        pairFiledNames(matches);
        analyzeFiledTypes(matches);
        if (m_filter){
          m_filter->analyzeColumns(&m_fieldnames, &m_fieldtypes);
          //m_filter->mergeExprConstNodes();
        }
        for (int i=0; i<m_selections.size(); i++)
          m_selections[i].analyzeColumns(&m_fieldnames, &m_fieldtypes);
      }
      // append variables
      //matcheddata.push_back(m_filename);
      matcheddata.push_back(intToStr(m_line));
      matcheddata.push_back(intToStr(m_matchcount+1));
      if (matchFilter(matcheddata, m_filter)){
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
  }catch (exception& e) {
    trace(ERROR, "Regular search exception: %s\n", e.what());
    return found;
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
  //printf("%d: ", m_outputrow);
  if (m_selections.size()==0)
    printf("%s\n", datas[0].c_str());
  else{
    for (int i=1; i<datas.size(); i++)
      printf("%s\t", datas[i].c_str());
    printf("\n");
  }
}

void QuerierC::printFieldNames()
{
  //for (int i=1; i<m_fieldnames.size(); i++)
  //  printf("%s\t",m_fieldnames[i].c_str());
  if (m_selections.size()>0){
    for (int i=0; i<m_selections.size(); i++)
      printf("%s\t",m_selections[i].getEntireExpstr().c_str());
    printf("\n");
    for (int i=0; i<m_selections.size(); i++)
      printf("%s\t",string(m_selections[i].getEntireExpstr().length(),'-').c_str());
  }else{
    printf("Row\n"); 
    printf("%s",string(58,'-').c_str());
  }
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
    try{
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
    }catch (exception& e) {
      trace(ERROR, "Regular search exception: %s\n", e.what());
      return -1;
    }
    return result->size();
  }else
    return -1;
}

int QuerierC::boostmatch(map<string,string> & result)
{
  namesaving_smatch matches(m_regexstr);
  //printf("Matching %s => %s\n",m_rawstr.c_str(), m_regexstr.c_str());
  try{
    if (regex_match(m_rawstr, matches, m_regexp)) {
      for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
        result[string(*it)] = matches[*it].str();
    }
  }catch (exception& e) {
    trace(ERROR, "Regular match exception: %s\n", e.what());
    return -1;
  }
  return result.size();
}

long QuerierC::getMatchedCount()
{
  return m_matchcount;
}