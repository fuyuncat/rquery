/*******************************************************************************
//
//        File: filter.cpp
// Description: Filter class defination
//       Usage: filter.cpp
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "filter.h"

void FilterC::init()
{
  type = UNKNOWN;       // 1: branch; 2: leaf
  junction = UNKNOWN;   // if type is BRANCH, 1: and; 2: or. Otherwise, it's meaningless
  comparator = UNKNOWN; // if type is LEAF, 1: ==; 2: >; 3: <; 4: !=; 5: >=; 6: <=. Otherwise, it's meaningless
  leftColId = -1;              // if type is LEAF, it's id of column on the left to be predicted. Otherwise, it's meaningless
  rightColId = -1;             // if type is LEAF, it's id of column on the right to be predicted. Otherwise, it's meaningless
  leftExpression = "";    // if type is LEAF, it's id of column to be predicted. Otherwise, it's meaningless
  rightExpression = "";   // if type is LEAF, it's data to be predicted. Otherwise, it's meaningless
  leftNode = NULL;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
  rightNode = NULL;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
  parentNode = NULL;    // for all types except the root, it links to parent node. Otherwise, it's meaningless
  
  metaDataAnzlyzed = false; // analyze column name to column id.
}

FilterC::FilterC()
{
  init();
}

FilterC::~FilterC()
{

}

FilterC::FilterC(FilterC* node)
{
  init();

  type = node->type;
  junction = node->junction;
  comparator = node->comparator;
  leftColId = node->leftColId;
  rightExpression = node->rightExpression;
  leftExpression = node->leftExpression;
  leftNode = node->leftNode;
  rightNode = node->rightNode;
  parentNode = node->parentNode;
  metaDataAnzlyzed = node->metaDataAnzlyzed;
  //predStr = node.predStr;
}

FilterC::FilterC(int junction, FilterC* leftNode, FilterC* rightNode)
{
  init();
  type = BRANCH;
  junction = junction;
  leftNode = leftNode;
  rightNode = rightNode;
  //leftNode = leftNode==null?null:new Prediction(leftNode);
  //rightNode = rightNode==null?null:new Prediction(rightNode);;
}

FilterC::FilterC(int comparator, int colId, string data)
{
  init();
  type = LEAF;
  comparator = comparator;
  leftColId = colId;
  rightExpression = data;
}

// return operation type: -1 error; 0: unused; 1: parse; 2:select; 3: filter; 4: group; 5: sort
map<string,string> FilterC::parseparam(string parameterstr)
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

map<string,string> FilterC::parsequery(string raw)
{
  map<string,string> query;
  query.insert( pair<string,string>("parse",raw) );
  return query;
}

