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

vector<string> ParserC::m_junctionWords;
vector<string> ParserC::m_junctionSplitors;

void ParserC::init()
{
  analyzedPos = 0;
  m_junctionWords.push_back("AND"); m_junctionWords.push_back("OR");
  m_junctionSplitors.push_back(" AND ");m_junctionSplitors.push_back(" OR ");
}

//map<string,string> ParserC::parsequery(string raw)
//{
//  map<string,string> query;
//  query.insert( pair<string,string>("parse",raw) );
//  return query;
//}

bool ParserC::isJunctionWord(const string & word)
{
  for (int i=0;i<m_junctionWords.size();i++)
    if (upper_copy(m_junctionWords[i]).compare(upper_copy(word))==0)
      return true;
  return false;
}

// return operation type: -1 error; 0: unused; 1: parse; 2:select; 3: filter; 4: group; 5: sort
map<string,string> ParserC::parseparam(const string & parameterstr)
{
  //trace(DEBUG, "Original string: %s\n", parameterstr.c_str());
  vector<string> params;
  split(params,parameterstr,'|',"''()//",'\\',{'(',')'},false,true);
  //dumpVector(params);
  for (int i = 0; i < params.size(); ++i){
    string trimmedstr = trim_copy(params[i]);
    if (trimmedstr.empty())
      continue;
    else if (trimmedstr[0]=='>'){
      if (trimmedstr.length()>1 && trimmedstr[1]=='>')
        m_queryparts.insert( pair<string,string>(">>",trim_copy(trimmedstr.substr(2))));
      else
        m_queryparts.insert( pair<string,string>(">",trim_copy(trimmedstr.substr(1))));
    }else{
      //trace(DEBUG, "Param %d:%s;\n",i,trimmedstr.c_str());
      size_t found = trimmedstr.find_first_of(" ");
      found = found==string::npos?trimmedstr.find_first_of("\t"):found;
      //trace(DEBUG, "Parameter %d: %s. Space at %d\n", i+1, params[i].c_str(),found);
      if  (found!=string::npos){
        //trace(DEBUG, "Operation %s: %s\n", lower_copy(trim_copy(params[i].substr(0,found))).c_str(), trim_copy(params[i].substr(found+1)).c_str());
        m_queryparts.insert( pair<string,string>(lower_copy(trimmedstr.substr(0,found)),trimmedstr.substr(found+1)) );
      }else
        m_queryparts.insert( pair<string,string>(lower_copy(trimmedstr),""));
    }
  }

  return m_queryparts;
}

void ParserC::dumpQueryparts()
{
  dumpMap(m_queryparts);
  //for (map<string,string>::iterator it=m_queryparts.begin(); it!=m_queryparts.end(); ++it)
  //  printf("%s: %s\n", it->first.c_str(), it->second.c_str());
}