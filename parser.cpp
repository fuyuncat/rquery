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
  junctionWords.push_back("AND"); junctionWords.push_back("OR");
  junctionSplitors.push_back(" AND ");junctionSplitors.push_back(" OR ");
  comparators.push_back("=");comparators.push_back("!=");comparators.push_back(">=");comparators.push_back("<=");comparators.push_back(">");comparators.push_back("<");comparators.push_back("LIKE");comparators.push_back("REGLIKE");comparators.push_back("NOLIKE");comparators.push_back("NOREGLIKE");comparators.push_back("IN"); // ">=", "<=" should be before ">", "<"
}

//map<string,string> ParserC::parsequery(string raw)
//{
//  map<string,string> query;
//  query.insert( pair<string,string>("parse",raw) );
//  return query;
//}

bool ParserC::isJunctionWord(string word)
{
  for (int i=0;i<junctionWords.size();i++)
    if (boost::iequals(junctionWords[i], word))
      return true;
  return false;
}

void ParserC::buildLeafNodeFromStr(FilterC* node, string str)
{
  bool quoteStarted = false;
  int lastPos = 0;
  for (int i=0;i<str.length();i++){
    if (str[i] == '"'){
      quoteStarted = !quoteStarted;
      str = str.substr(0, i)+(i<str.length()-1?str.substr(i+1):"");
      i--;
    }else if(str[i] == '\\' && i<str.length()-1 && str[i+1] == '"'){
      i++; // skip escaped " :\"
      str = str.substr(0, i-1)+str.substr(i);
    }else if(!quoteStarted && startsWithWords(str.substr(i), comparators) >= 0){ // splitor that not between quato are the real splitor
      string compStr = comparators[startsWithWords(str.substr(i), comparators)];
      node->comparator = encodeComparator(compStr);
      node->type = LEAF;
      node->leftExpStr =  boost::algorithm::trim_copy<string>(str.substr(0,i));
      node->rightExpStr = trim_one( boost::algorithm::trim_copy<string>(str.substr(i+compStr.length())),'"');
      node->leftExpression = new ExpressionC(node->leftExpStr);
      node->rightExpression = new ExpressionC(node->rightExpStr);
      return;
    }
  }
}

// split input command line into pieces; \ is escape char, " and splitor could be escaped.
// space splitor in "" should be ignored. \" is character '"'
// splitString({ad cas asdfa}, ' ', {'{','}'}, true)  => {ad,cas,asdfa}
// splitString("ad "cas\" asdfa"", ' ', {'"','"'}, true) => {ad,cas\" asdfa}
bool ParserC::buildFilter(FilterC* node, string initialString, string splitor, string quoters)
{
  //System.out.println(String.format("%d",deep) +":"+initialString);
  if (initialString.empty()){
    printf("\n");
    printf("Error: No statement found!\n");
    printf("\n");
    return false;
  }else
    initialString = boost::algorithm::trim_copy<string>(initialString);
  //initialString = initialString.trim();
  //node.predStr = new String(initialString);
  char stringQuoter = '"';
  if (quoters.empty() || quoters.length() != 2)
      quoters = "()";
  //printf("Building: %s; splitor: %s; quoters: %s\n", initialString.c_str(), splitor.c_str(), quoters.c_str());
  int quoteDeep = 0;
  int quoteStart = -1;  // top quoter start position
  int quoteEnd = -1;;   // top quoter end position
  bool stringStart = false;
  for (int i=0;i<initialString.length();i++){
    if (initialString[i] == stringQuoter &&(i==0 || (i>0 && initialString[i-1]!='\\'))){ // \ is ignore character
      stringStart = !stringStart;
      continue;
    }
    if (stringStart) // ignore all character being a string
      continue;
    if (initialString[i] == quoters[0]){
      quoteDeep++;
      if (quoteDeep == 1 && quoteStart < 0)
        quoteStart = i;
    }else if (initialString[i] == quoters[1]){
      quoteDeep--;
      if (quoteDeep == 0 && quoteEnd < 0)
        quoteEnd = i;
      if  (quoteDeep < 0){
        printf("\n");
        printf("Error: Left quoter missed!\n");
        printf("\n");;
      }
    }else if(initialString[i] == '\\' && i<initialString.length()-1 && 
            ((initialString[i+1] == quoters[0] || initialString[i+1] == quoters[1] || initialString[i+1] == ' '))){
      i++; // skip escaped " \"
      initialString = initialString.substr(0, i-1)+initialString.substr(i);
    }else if(quoteDeep == 0 && initialString[i] == ' '){ // splitor that not between quato are the real splitor
      if ( boost::to_upper_copy<string>(initialString.substr(i)).find(splitor) == 0){
        node->type = BRANCH;
        node->junction = encodeJunction(boost::algorithm::trim_copy<string>(splitor));
        node->leftNode = new FilterC();
        node->leftNode->parentNode = node;
        //printf("Building leftNode\n");
        if (!buildFilter(node->leftNode, initialString.substr(0, i)," OR ",quoters)) { // OR priority higher than AND
          delete node->leftNode;
          return false;
        }
        node->rightNode = new FilterC();
        node->rightNode->parentNode = node;
        //printf("Building rightNode\n");
        if (!buildFilter(node->rightNode, initialString.substr(i+splitor.length())," OR ",quoters)){
          delete node->rightNode;
          return false;
        }
        return true;
      }
    }
  }

  if (quoteDeep != 0){
    printf("\n");
    printf("Error: Right quoter missed!\n");
    printf("\n");
    return false;
  }

  if (quoteStart == 0 && quoteEnd == initialString.length()-1){ // sub expression quoted 
    initialString = initialString.substr(1,initialString.length()-2);  // trim the quoters
    return buildFilter(node, initialString," OR ",quoters);
  }else if (boost::to_upper_copy<string>(splitor).compare(" OR ") == 0){
    return buildFilter(node, initialString," AND ",quoters);
  }else
    buildLeafNodeFromStr(node, initialString);
  return true;
}

FilterC* ParserC::buildFilter(string initialString)
{
  //printf("building filter: %s", initialString.c_str());
  FilterC* node = new FilterC();
  if (!buildFilter(node, initialString, " OR ", "()")){
    delete node;
    //printf(" failed!\n");
    return NULL;
  }
  //node.dump();
  //printf(" completed!\n");
  return node;
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

void ParserC::dumpQueryparts()
{
  for (map<string,string>::iterator it=m_queryparts.begin(); it!=m_queryparts.end(); ++it)
    printf("%s: %s\n", it->first.c_str(), it->second.c_str());
}