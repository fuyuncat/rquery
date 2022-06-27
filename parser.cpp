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
#include "parser.h"

ParserC::ParserC()
{
  init();
}

ParserC::~ParserC()
{

}

void ParserC::init()
{

}

// return operation type: -1 error; 0: unused; 1: parse; 2:select; 3: filter; 4: group; 5: sort
map<string,string> ParserC::parseparam(string parameterstr)
{
  map<string,string> query;
  //printf("Original string: %s\n", parameterstr.c_str());
  vector<string> params = split(parameterstr,'|','/','\\');
  for (int i = 0; i < params.size(); ++i){
    string trimmedstr = boost::algorithm::trim_copy<string>(params[i]);
    size_t found = params[i].find_first_of(" ");
    //printf("Parameter %d: %s. Space at %d\n", i+1, params[i].c_str(),found);
    if  (found!=string::npos){
      //printf("Operation %s: %s\n", boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(params[i].substr(0,found))).c_str(), boost::algorithm::trim_copy<string>(params[i].substr(found+1)).c_str());
      query.insert( pair<string,string>(boost::algorithm::to_lower_copy<string>(boost::algorithm::trim_copy<string>(params[i].substr(0,found))),boost::algorithm::trim_copy<string>(params[i].substr(found+1))) );
    }
  }
  return query;
}

map<string,string> ParserC::parsequery(string raw)
{
  map<string,string> query;
  query.insert( pair<string,string>("parse",raw) );
  return query;
}

vector<string> ParserC::parsereg(string raw, string rex, int & rst )
{
  string newrex = trim_one(rex, '/');
  QuerierC re(newrex);
  vector<string> vs;
  re.setrawstr(raw);
  rst = re.boostmatch( &vs );
  return vs;
}

map<string,string> ParserC::parseregmap(string raw, string rex, int & rst )
{
  map<string,string> matches;
  string newrex = trim_one(rex, '/');

  QuerierC re(newrex);
  re.setrawstr(raw);
  rst = re.boostmatch( matches );
  return matches;
}