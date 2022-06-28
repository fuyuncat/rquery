/*******************************************************************************
//
//        File: parser.cpp
// Description: Parser class defination
//       Usage: parser.cpp
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
#include <boost/algorithm/string.hpp>
#include "parser.h"

ParserC::ParserC()
{
  init();
}

ParserC::~ParserC()
{

}

vector<string> ParserC::junctionWords;
vector<string> ParserC::junctionSplitors;
vector<string> ParserC::comparators;

void ParserC::init()
{
  analyzedPos = 0;
  ParserC::junctionWords.push_back("AND"); ParserC::junctionWords.push_back("OR");
  ParserC::junctionSplitors.push_back(" AND ");ParserC::junctionSplitors.push_back(" OR ");
  ParserC::comparators.push_back("=");ParserC::comparators.push_back("!=");ParserC::comparators.push_back(">=");ParserC::comparators.push_back("<=");ParserC::comparators.push_back(">");ParserC::comparators.push_back("<"); // ">=", "<=" should be before ">", "<"
}

// return operation type: -1 error; 0: unused; 1: parse; 2:select; 3: filter; 4: group; 5: sort
map<string,string> ParserC::parseparam(string parameterstr)
{
  //printf("Original string: %s\n", parameterstr.c_str());
  vector<string> params = split(parameterstr,'|','/','\\');
  for (int i = 0; i < params.size(); ++i){
    string trimmedstr = boost::algorithm::trim_copy<string>(params[i]);
    size_t found = params[i].find_first_of(" ");
    //printf("Parameter %d: %s. Space at %d\n", i+1, params[i].c_str(),found);
    if  (found!=string::npos){
      //printf("Operation %s: %s\n", boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(params[i].substr(0,found))).c_str(), boost::algorithm::trim_copy<string>(params[i].substr(found+1)).c_str());
      m_queryparts.insert( pair<string,string>(boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(params[i].substr(0,found))),boost::algorithm::trim_copy<string>(params[i].substr(found+1))) );
    }
  }
  return m_queryparts;
}

//map<string,string> ParserC::parsequery(string raw)
//{
//  map<string,string> query;
//  query.insert( pair<string,string>("parse",raw) );
//  return query;
//}

