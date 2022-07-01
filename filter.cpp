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
#include <boost/algorithm/string.hpp>
#include "filter.h"

std::set<char> FilterC::m_operators;

void FilterC::init()
{
  m_type = UNKNOWN;       // 1: branch; 2: leaf
  m_junction = UNKNOWN;   // if type is BRANCH, 1: and; 2: or. Otherwise, it's meaningless
  m_comparator = UNKNOWN; // if type is LEAF, 1: ==; 2: >; 3: <; 4: !=; 5: >=; 6: <=. Otherwise, it's meaningless
  m_datatype = UNKNOWN;   // if type is LEAF, 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
  m_leftColId = -1;              // if type is LEAF, it's id of column on the left to be predicted. Otherwise, it's meaningless
  m_rightColId = -1;             // if type is LEAF, it's id of column on the right to be predicted. Otherwise, it's meaningless
  m_leftExpStr = "";    // if type is LEAF, it's id of column to be predicted. Otherwise, it's meaningless
  m_rightExpStr = "";   // if type is LEAF, it's data to be predicted. Otherwise, it's meaningless
  m_leftExpression = NULL; // meaningful only if type is LEAF
  m_rightExpression = NULL; // meaningful only if type is LEAF
  m_leftNode = NULL;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
  m_rightNode = NULL;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
  m_parentNode = NULL;    // for all types except the root, it links to parent node. Otherwise, it's meaningless
  
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

  m_type = node->m_type;
  m_junction = node->m_junction;
  m_comparator = node->m_comparator;
  m_datatype = node->m_datatype;
  m_leftColId = node->m_leftColId;
  m_rightColId = node->m_rightColId;
  m_leftExpStr = node->m_leftExpStr;
  m_rightExpStr = node->m_rightExpStr;
  m_leftExpression = node->m_leftExpression;
  m_rightExpression = node->m_rightExpression;
  m_leftNode = node->m_leftNode;
  m_rightNode = node->m_rightNode;
  m_parentNode = node->m_parentNode;
  metaDataAnzlyzed = node->metaDataAnzlyzed;
  //predStr = node.predStr;

  //m_operators = {'^','*','/','+','-'};
  m_operators.insert('^');m_operators.insert('*');m_operators.insert('/');m_operators.insert('+');m_operators.insert('-'); // "^", "*", "/" should be before "+", "-"
}

FilterC::FilterC(int junction, FilterC* leftNode, FilterC* rightNode)
{
  init();
  m_type = BRANCH;
  m_junction = junction;
  m_leftNode = leftNode;
  m_rightNode = rightNode;
  //m_leftNode = leftNode==NULL?NULL:new Prediction(leftNode);
  //m_rightNode = rightNode==NULL?NULL:new Prediction(rightNode);;
}

FilterC::FilterC(int comparator, int colId, string data)
{
  init();
  m_type = LEAF;
  m_comparator = comparator;
  m_leftColId = colId;
  m_rightExpStr = data;
}

// split input command line into pieces; \ is escape char, " could be escaped.
// \" is character '"'
// splitString({ad cas asdfa}, ' ', {'{','}'}, true)  => {ad,cas,asdfa}
// splitString("ad "cas\" asdfa"", ' ', {'"','"'}, true) => {ad,cas\" asdfa}
bool FilterC::buildExpression(ExpressionC* node, string initialString)
{
  //System.out.println(String.format("%d",deep) +":"+initialString);
  if (initialString.empty()){
    trace(ERROR, "Error: No statement found!\n");
    return false;
  }else
    initialString = boost::algorithm::trim_copy<string>(initialString);

  int nextPos=0,strStart=0;

  // checking reg expr str. the whole string is quoted by "//"
  if (initialString[0] == '/'){
    if (initialString[initialString.size()-1] == '/'){
      node->m_type = LEAF;
      node->m_operate = UNKNOWN;
      node->m_datatype = STRING;
      node->m_expType = CONST;
      node->m_expStr = initialString;
      node->m_colId = -1;
      node->m_leftNode = NULL;
      node->m_rightNode = NULL;
      node->m_parentNode = NULL;
      return true;
    }else{
      trace(ERROR, "Regular expression is not closed. \n");
      return false;
    }
  }else if (initialString[0] == '('){ // checking quoted expression
    string sStr = readQuotedStr(initialString, nextPos, "()", '\0');
    if (sStr.size() > 1){
      if (nextPos == initialString.size()) { // whole string is a quoted string
        return buildExpression(node, initialString.substr(1,initialString.size()-2));
      }else{
        while (initialString[nextPos] == ' ') // skip space
          nextPos++;
        if (nextPos < initialString.size()-1 && m_operators.find(initialString[nextPos]) != m_operators.end()){
          ExpressionC* rightNode = new ExpressionC();
          if (buildExpression(rightNode, initialString.substr(nextPos+1))){
            ExpressionC* leftNode = new ExpressionC();
            if (buildExpression(leftNode, sStr)){
              node->m_type = BRANCH;
              node->m_operate = encodeOperator(initialString.substr(nextPos,1));
              node->m_datatype = DATE;
              node->m_expType = UNKNOWN;
              node->m_expStr = initialString.substr(nextPos,1);
              node->m_colId = -1;
              node->m_parentNode = NULL;
              node->m_leftNode = leftNode;
              node->m_rightNode = rightNode;
              rightNode->m_parentNode = node;
              return true;
            }else{
              delete leftNode;
              return false;
            }
          }else{
            delete rightNode;
            return false;
          }
        }else{
          trace(ERROR, "Invalide expression involved DATE string. \n");
          return false;
        }
      }
    }else{
      trace(ERROR, "DATE string is not closed. \n");
      return false;
    }
  }else if (initialString[0] == '{'){ // checking DATE string
    string sStr = readQuotedStr(initialString, nextPos, "{}", '\0');
    if (sStr.size() > 1){
      if (nextPos == initialString.size()) { // whole string is a date string
        node->m_type = LEAF;
        node->m_operate = UNKNOWN;
        node->m_datatype = DATE;
        node->m_expType = CONST;
        node->m_expStr = sStr;
        node->m_colId = -1;
        node->m_leftNode = NULL;
        node->m_rightNode = NULL;
        node->m_parentNode = NULL;
        return true;
      }else{
        while (initialString[nextPos] == ' ') // skip space
          nextPos++;
        if (nextPos < initialString.size()-1 && m_operators.find(initialString[nextPos]) != m_operators.end()){
          ExpressionC* rightNode = new ExpressionC();
          if (buildExpression(rightNode, initialString.substr(nextPos+1))){
            ExpressionC* leafNode = new ExpressionC();
            leafNode->m_type = LEAF;
            leafNode->m_operate = UNKNOWN;
            leafNode->m_datatype = DATE;
            leafNode->m_expType = CONST;
            leafNode->m_expStr = sStr;
            leafNode->m_colId = -1;
            leafNode->m_leftNode = NULL;
            leafNode->m_rightNode = NULL;
            leafNode->m_parentNode = node;

            node->m_type = BRANCH;
            node->m_operate = encodeOperator(initialString.substr(nextPos,1));
            node->m_datatype = DATE;
            node->m_expType = UNKNOWN;
            node->m_expStr = initialString.substr(nextPos,1);
            node->m_colId = -1;
            node->m_parentNode = NULL;
            node->m_leftNode = leafNode;
            node->m_rightNode = rightNode;
            
            rightNode->m_parentNode = node;
            return true;
          }else{
            delete rightNode;
            return false;
          }
        }else{
          trace(ERROR, "Invalide expression involved DATE string. \n");
          return false;
        }
      }
    }else{
      trace(ERROR, "DATE string is not closed. \n");
      return false;
    }
  }else if (initialString[0] == '\''){ // checking STRING string
    string sStr = readQuotedStr(initialString, nextPos, "''", '\\');
    if (sStr.size() > 1){
      if (nextPos == initialString.size()) { // whole string is a date string
        node->m_type = LEAF;
        node->m_operate = UNKNOWN;
        node->m_datatype = STRING;
        node->m_expType = CONST;
        node->m_expStr = sStr;
        node->m_colId = -1;
        node->m_leftNode = NULL;
        node->m_rightNode = NULL;
        node->m_parentNode = NULL;
        return true;
      }else{
        while (initialString[nextPos] == ' ') // skip space
          nextPos++;
        if (nextPos < initialString.size()-1 && m_operators.find(initialString[nextPos]) != m_operators.end()){
          ExpressionC* rightNode = new ExpressionC();
          if (buildExpression(rightNode, initialString.substr(nextPos+1))){
            ExpressionC* leafNode = new ExpressionC();
            leafNode->m_type = LEAF;
            leafNode->m_operate = UNKNOWN;
            leafNode->m_datatype = STRING;
            leafNode->m_expType = CONST;
            leafNode->m_expStr = sStr;
            leafNode->m_colId = -1;
            leafNode->m_leftNode = NULL;
            leafNode->m_rightNode = NULL;
            leafNode->m_parentNode = node;

            node->m_type = BRANCH;
            node->m_operate = encodeOperator(initialString.substr(nextPos,1));
            node->m_datatype = STRING;
            node->m_expType = UNKNOWN;
            node->m_expStr = initialString.substr(nextPos,1);
            node->m_colId = -1;
            node->m_parentNode = NULL;
            node->m_leftNode = leafNode;
            node->m_rightNode = rightNode;
            
            rightNode->m_parentNode = node;
            return true;
          }else{
            delete rightNode;
            return false;
          }
        }else{
          trace(ERROR, "Invalide expression involved STRING string. \n");
          return false;
        }
      }
    }else{
      trace(ERROR, "STRING string is not closed. \n");
      return false;
    }
  }else{
    while (initialString[nextPos] != ' ' && m_operators.find(initialString[nextPos]) == m_operators.end()) {// moving forward until reach the first operator
      if (initialString[nextPos] == '\'' || initialString[nextPos] == '{' || initialString[nextPos] == '/' || initialString[nextPos] == '}'){
        trace(ERROR, "Invalid character detected. \n");
        return false;
      }
      nextPos++;
    }
    if (nextPos == 0){
      trace(ERROR, "Expression cannot start with an operator \n");
      return false;
    }
    string sExpStr = initialString.substr(0,nextPos);
    while (initialString[nextPos] == ' ') // skip space
      nextPos++;
    if (nextPos < initialString.size()-1 && m_operators.find(initialString[nextPos]) != m_operators.end()){
      ExpressionC* rightNode = new ExpressionC();
      if (buildExpression(rightNode, initialString.substr(nextPos+1))){
        ExpressionC* leafNode = new ExpressionC();
        leafNode->m_type = LEAF;
        leafNode->m_operate = UNKNOWN;
        leafNode->m_datatype = UNKNOWN;
        leafNode->m_expType = CONST;
        leafNode->m_expStr = sExpStr;
        leafNode->m_colId = -1;
        leafNode->m_leftNode = NULL;
        leafNode->m_rightNode = NULL;
        leafNode->m_parentNode = node;

        node->m_type = BRANCH;
        node->m_operate = encodeOperator(initialString.substr(nextPos,1));
        node->m_datatype = UNKNOWN;
        node->m_expType = UNKNOWN;
        node->m_expStr = initialString.substr(nextPos,1);
        node->m_colId = -1;
        node->m_parentNode = NULL;
        node->m_leftNode = leafNode;
        node->m_rightNode = rightNode;
        
        rightNode->m_parentNode = node;
        return true;
      }else{
        delete rightNode;
        return false;
      }
    }else{
      trace(ERROR, "Invalide expression string. \n");
      return false;
    }
  }
  return false;
}

ExpressionC* FilterC::buildExpression(string initialString)
{
  //printf("building filter: %s", initialString.c_str());
  ExpressionC* node = new ExpressionC(initialString);
  if (!buildExpression(node, initialString)){
    delete node;
    trace(ERROR, "Building expression from %s failed!\n", initialString.c_str());
    return NULL;
  }
  //node.dump();
  //printf(" completed!\n");
  return node;
}

// get left tree Height
int FilterC::getLeftHeight(){
  int height = 1;
  if (m_type == BRANCH && m_leftNode)
    height += m_leftNode->getLeftHeight();

  return height;
}

// get left tree Height
int FilterC::getRightHeight(){
  int height = 1;
  if (m_type == BRANCH && m_rightNode)
    height += m_rightNode->getRightHeight();
  
  return height;
}

// add a NEW filter into tree
void FilterC::add(FilterC* node, int junction, bool leafGrowth, bool addOnTop){
  // not add any null or UNKNOWN node
  if (node || node->m_type ==  UNKNOWN) 
      return;
  if (m_type ==  UNKNOWN){ // not assinged
      node->copyTo(this);
  }else if (m_type == LEAF){
    FilterC* existingNode = new FilterC();
    copyTo(existingNode);
    m_type = BRANCH;
    m_junction = junction;
    m_comparator = UNKNOWN;   
    m_leftColId = -1;  
    m_rightColId = -1;    
    m_leftExpStr = "";
    m_rightExpStr = "";
    m_leftExpression = NULL;
    m_rightExpression = NULL;
    if (leafGrowth){
      m_rightNode = existingNode;
      m_rightNode->m_parentNode = this;
      m_leftNode = node;
      m_leftNode->m_parentNode = this;
    }else{
      m_leftNode = existingNode;
      m_leftNode->m_parentNode = this;
      m_rightNode = node;
      m_rightNode->m_parentNode = this;
    }
  }else{
    if (addOnTop){
      FilterC* existingNode = new FilterC();
      copyTo(existingNode);
      m_junction = junction;
      if (leafGrowth){
        m_leftNode = node;
        m_leftNode->m_parentNode = this;
        m_rightNode = existingNode;
        m_rightNode->m_parentNode = this;
      }else{
        m_leftNode = existingNode;
        m_leftNode->m_parentNode = this;
        m_rightNode = node;
        m_rightNode->m_parentNode = this;
      }
    }else{
      if (leafGrowth){
        if (m_leftNode)
          m_leftNode->add(node, junction, leafGrowth, addOnTop);
        else 
          m_rightNode->add(node, junction, leafGrowth, addOnTop);
      }else{
        if (m_rightNode)
          m_rightNode->add(node, junction, leafGrowth, addOnTop);
        else 
          m_leftNode->add(node, junction, leafGrowth, addOnTop);
      }
    }
  }
}

void FilterC::dump(int deep){
  if (m_type == BRANCH){
    trace(INFO,"(%d)%s\n",deep,decodeJunction(junction).c_str());
    trace(INFO,"L-");
    m_leftNode->dump(deep+1);
    trace(INFO,"R-");
    m_rightNode->dump(deep+1);
  }else{
    trace(INFO,"(%d)%s(%d)",deep,m_leftExpStr.c_str(),m_leftColId);
    trace(INFO,"%s",decodeComparator(m_comparator).c_str());
    trace(INFO,"%s\n",m_rightExpStr.c_str());
  }
}

void FilterC::dump(){
  dump(0);
}

// detect if predication contains special colId    
bool FilterC::containsColId(int colId){
  bool contain = false;
  if (m_type == BRANCH){
    contain = contain || m_leftNode->containsColId(colId);
    contain = contain || m_rightNode->containsColId(colId);
  }else
    contain = (m_leftColId == colId);

  return contain;
}

// detect if predication contains special colId    
FilterC* FilterC::getFirstPredByColId(int colId, bool leftFirst){
  FilterC* node;
  if (m_type == BRANCH){
    if (leftFirst){
      if (m_leftNode)
        node = m_leftNode->getFirstPredByColId(colId, leftFirst);
      if (!node)
        node = m_rightNode->getFirstPredByColId(colId, leftFirst);
    }else{
      if (m_rightNode)
        node = m_rightNode->getFirstPredByColId(colId, leftFirst);
      if (!node)
        node = m_leftNode->getFirstPredByColId(colId, leftFirst);
    }
  }else if (m_type == LEAF)
    if (m_leftColId == colId)
      node = this;

  return node;
}

// analyze column ID & name from metadata
bool FilterC::analyzeColumns(vector<string> m_fieldnames1, vector<string> m_fieldnames2){
  if (m_type == BRANCH){
    metaDataAnzlyzed = true;
    if (m_leftNode)
        metaDataAnzlyzed = metaDataAnzlyzed && m_leftNode->analyzeColumns(m_fieldnames1, m_fieldnames2);
    if (!metaDataAnzlyzed)
        return metaDataAnzlyzed;
    if (m_rightNode)
        metaDataAnzlyzed = metaDataAnzlyzed &&  m_rightNode->analyzeColumns(m_fieldnames1, m_fieldnames2);
  }else if (m_type == LEAF){
    m_datatype = detectDataType(m_rightExpStr);
    if (m_datatype == UNKNOWN)
      m_datatype = detectDataType(m_leftExpStr);

    if (m_fieldnames1.size()>0){
      if (m_leftExpStr[0] == '"') {// quoted, treat as expression, otherwise, as columns
        m_leftExpStr = trim_one(m_leftExpStr,'"'); // remove quoters
        m_leftColId = -1;
      }else {
        if (isInt(m_leftExpStr)){ // check if the name is ID already
          m_leftColId = atoi(m_leftExpStr.c_str());
          m_leftExpStr = m_fieldnames1[m_leftColId];
        }else{
          m_leftColId = findStrArrayId(m_fieldnames1, m_leftExpStr);
        }
      }
    }
    if (m_fieldnames2.size()>0){
      if (m_rightExpStr[0] == '"') {// quoted, treat as expression, otherwise, as columns
        m_rightExpStr = trim_one(m_rightExpStr,'"'); // remove quoters
        m_rightColId = -1;
      }else {
        if (isInt(m_rightExpStr)){ // check if the name is ID already
          m_rightColId = atoi(m_rightExpStr.c_str());
          m_rightExpStr = m_fieldnames2[m_rightColId];
        }else{
          m_rightColId = findStrArrayId(m_fieldnames2, m_rightExpStr);
        }
      }
    }
    if(m_leftColId != -1 && m_rightColId != -1){
      //if (metaData1.getColumnType(m_leftColId) != metaData2.getColumnType(m_rightColId)){
      //  //dtrace.trace(254);
      //  return false;
      //}else
        return true;
    }else
      return true;
  }
  return metaDataAnzlyzed;
}

bool FilterC::columnsAnalyzed(){
    return metaDataAnzlyzed;
}

FilterC* FilterC::cloneMe(){
  FilterC* node = new FilterC();
  node->metaDataAnzlyzed = metaDataAnzlyzed;
  //node->predStr = predStr;
  node->m_type = m_type;
  node->m_datatype = m_datatype;
  node->m_junction = m_junction;
  node->m_comparator = m_comparator;
  node->m_leftColId = m_leftColId;
  node->m_rightColId = m_rightColId;
  node->m_rightExpStr = m_rightExpStr;
  node->m_leftExpStr = m_leftExpStr;
  if (m_type == BRANCH){
    node->m_leftExpression = new ExpressionC(node->m_leftExpStr);
    node->m_leftExpression = m_leftExpression->cloneMe();
    node->m_rightExpression = new ExpressionC(node->m_rightExpStr);
    node->m_rightExpression = m_rightExpression->cloneMe();
    node->m_leftNode = new FilterC();
    node->m_leftNode = m_leftNode->cloneMe();
    node->m_rightNode = new FilterC();
    node->m_rightNode = m_rightNode->cloneMe();
    node->m_leftNode->m_parentNode = node;
    node->m_rightNode->m_parentNode = node;
  }else{
    node->m_leftExpression = NULL;
    node->m_rightExpression = NULL;
    node->m_leftNode = NULL;
    node->m_rightNode = NULL;
  }
  return node;
}

void FilterC::copyTo(FilterC* node){
  if (!node)
    return;
  else{
    node->metaDataAnzlyzed = metaDataAnzlyzed;
    //node->predStr = predStr;
    node->m_type = m_type;
    node->m_datatype = m_datatype;
    node->m_junction = m_junction;
    node->m_comparator = m_comparator;
    node->m_leftColId = m_leftColId;
    node->m_rightColId = m_rightColId;
    node->m_rightExpStr = m_rightExpStr;
    node->m_leftExpStr = m_leftExpStr;
    if (m_type == BRANCH){
      if (m_leftNode){
        node->m_leftNode = new FilterC();
        m_leftNode->copyTo(node->m_leftNode);
        node->m_leftNode->m_parentNode = node;
      }else
        node->m_leftNode = NULL;

      if (m_rightNode){
        node->m_rightNode = new FilterC();
        m_rightNode->copyTo(node->m_rightNode);
        node->m_rightNode->m_parentNode = node;
      }else
        node->m_rightNode = NULL;
      node->m_leftExpression = NULL;
      node->m_rightExpression = NULL;
    }else{
      if (m_leftExpression){
        node->m_leftExpression = new ExpressionC(m_leftExpStr);
        m_leftExpression->copyTo(node->m_leftExpression);
      }
      if (m_rightExpression){
        node->m_rightExpression = new ExpressionC(m_rightExpStr);
        m_rightExpression->copyTo(node->m_rightExpression);
      }
    }
  }
}

// get all involved colIDs in this prediction
std::set<int> FilterC::getAllColIDs(int side){
  std::set<int> colIDs;
  if (m_type == BRANCH){
    if (m_leftNode){
      std::set<int> foo = m_leftNode->getAllColIDs(side);
      colIDs.insert(foo.begin(), foo.end());
    }
    if (m_rightNode){
      std::set<int> foo = m_rightNode->getAllColIDs(side);
      colIDs.insert(foo.begin(), foo.end());
    }
  }else if(m_type == LEAF){
    if (side == LEFT && m_leftColId>=0)
      colIDs.insert(m_leftColId);
    else if (side == RIGHT && m_rightColId>=0)
      colIDs.insert(m_rightColId);
  }
  return colIDs;
}

// build the prediction as a HashMap
map<int,string> FilterC::buildMap(){
  map<int,string> datas;
  if (m_type == BRANCH){
    if (m_leftNode){
      map<int,string> foo = m_leftNode->buildMap();
      datas.insert(foo.begin(), foo.end());
    }
    if (m_rightNode){
      map<int,string> foo = m_rightNode->buildMap();
      datas.insert(foo.begin(), foo.end());
    }
  }else if(m_type == LEAF){
    if (m_leftColId>=0)
      datas.insert( pair<int,string>(m_leftColId,m_rightExpStr) );
  }
  return datas;
}

// calculate an expression prediction
bool FilterC::compareExpression(){
  bool result=true;
  if (m_type == BRANCH){
    if (!m_leftNode || !m_rightNode)
      return false;
    if (m_junction == AND)
      result = m_leftNode->compareExpression() && m_rightNode->compareExpression();
    else
      result = m_leftNode->compareExpression() || m_rightNode->compareExpression();
  }else if(m_type == LEAF){
    return anyDataCompare(m_leftExpStr, m_comparator, m_rightExpStr, STRING) == 1;
  }else{ // no predication means alway true
    return true;
  }
  return result;
}

// get all involved colIDs in this prediction
int FilterC::size(){
  int size = 0;
  if (m_type == BRANCH){
    if (m_leftNode)
      size += m_leftNode->size();
    if (m_rightNode)
      size += m_rightNode->size();
  }else if (m_type == LEAF)
    size = 1;
  else 
    size = 0;
  return size;
}

// clear predictin
void FilterC::clear(){
  if (m_leftNode){
    m_leftNode->clear();
    delete m_leftNode;
    m_leftNode = NULL;
  }
  if (m_rightNode){
    m_rightNode->clear();
    delete m_rightNode;
    m_rightNode = NULL;
  }
  m_type = UNKNOWN;
  m_datatype = UNKNOWN;
  m_junction = UNKNOWN;
  m_comparator = UNKNOWN;
  m_leftColId = -1;
  m_rightColId = -1;
  m_rightExpStr = "";
  m_leftExpStr = "";
  metaDataAnzlyzed = false;
}

// remove a node from prediction. Note: the input node is the address of the node contains in current prediction
//   0                      0                  2                 1
//  1  2  (remove 3) =>   4   2 (remove 1) =>      (remove 2)  3   4
//3  4
bool FilterC::remove(FilterC* node){
  bool removed = false;
  if (m_leftNode){
    if (m_leftNode == node){
      m_leftNode->clear();
      delete m_leftNode;
      m_leftNode = NULL;
      return true;
    }else{
      removed = removed || m_leftNode->remove(node);
      if (removed)
        return removed;
    }
  }
  if (m_rightNode){
    if (m_rightNode == node){
      m_rightNode->clear();
      delete m_rightNode;
      m_rightNode = NULL;
      return true;
    }else{
      removed = removed || m_rightNode->remove(node);
      if (removed)
        return removed;
    }
  }
  if (this == node){
    clear();
    return true;
  }else
    return removed;

  /*if (this == node){
      if (this.m_parentNode != null){
          if (this.m_parentNode.m_leftNode == this ) // this is m_leftNode
              this.m_parentNode.m_rightNode.copyTo(this); // assign right brother as parent
          else if (this.m_parentNode.m_rightNode == this ) // this is rihtnode
               this.m_parentNode.m_leftNode.copyTo(this); // assign left brother as parent
      }
      return true;
  }else if (m_type == Consts.BRANCH){
      return (m_leftNode != null && m_leftNode.remove(node)) || (m_rightNode != null && m_rightNode.remove(node));
  }
  return false;//*/
}

// build a data list for a set of column, keeping same sequence, fill the absent column with NULL
void FilterC::fillDataForColumns(map <string, string> & dataList, vector <string> columns){
  if (columns.size() == 0)
    return;
  if (m_type == BRANCH){
    if (m_leftNode)
      m_leftNode->fillDataForColumns(dataList, columns);
    if (m_rightNode)
      m_rightNode->fillDataForColumns(dataList, columns);
  }else if (m_type == LEAF && m_leftColId >= 0)
    dataList.insert( pair<string,string>(columns[m_leftColId],m_rightExpStr) );
}
