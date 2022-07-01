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

vector<string> FilterC::m_operators;

void FilterC::init()
{
  type = UNKNOWN;       // 1: branch; 2: leaf
  junction = UNKNOWN;   // if type is BRANCH, 1: and; 2: or. Otherwise, it's meaningless
  comparator = UNKNOWN; // if type is LEAF, 1: ==; 2: >; 3: <; 4: !=; 5: >=; 6: <=. Otherwise, it's meaningless
  datatype = UNKNOWN;   // if type is LEAF, 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
  leftColId = -1;              // if type is LEAF, it's id of column on the left to be predicted. Otherwise, it's meaningless
  rightColId = -1;             // if type is LEAF, it's id of column on the right to be predicted. Otherwise, it's meaningless
  leftExpStr = "";    // if type is LEAF, it's id of column to be predicted. Otherwise, it's meaningless
  rightExpStr = "";   // if type is LEAF, it's data to be predicted. Otherwise, it's meaningless
  leftExpression = NULL; // meaningful only if type is LEAF
  rightExpression = NULL; // meaningful only if type is LEAF
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
  datatype = node->datatype;
  leftColId = node->leftColId;
  rightColId = node->rightColId;
  leftExpStr = node->leftExpStr;
  rightExpStr = node->rightExpStr;
  leftExpression = node->leftExpression;
  rightExpression = node->rightExpression;
  leftNode = node->leftNode;
  rightNode = node->rightNode;
  parentNode = node->parentNode;
  metaDataAnzlyzed = node->metaDataAnzlyzed;
  //predStr = node.predStr;

  m_operators = {'^','*','/','+','-'};
  //m_operators.push_back('^');m_operators.push_back('*');m_operators.push_back('/');m_operators.push_back('+');m_operators.push_back('-'); // "^", "*", "/" should be before "+", "-"
}

FilterC::FilterC(int junction, FilterC* leftNode, FilterC* rightNode)
{
  init();
  type = BRANCH;
  junction = junction;
  leftNode = leftNode;
  rightNode = rightNode;
  //leftNode = leftNode==NULL?NULL:new Prediction(leftNode);
  //rightNode = rightNode==NULL?NULL:new Prediction(rightNode);;
}

FilterC::FilterC(int comparator, int colId, string data)
{
  init();
  type = LEAF;
  comparator = comparator;
  leftColId = colId;
  rightExpStr = data;
}

void FilterC::buildLeafNodeFromStr(ExpressionC* node, string str)
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
    }else if(!quoteStarted && startsWithWords(str.substr(i), operators) >= 0){ // splitor that not between quato are the real splitor
      string opStr = operators[startsWithWords(str.substr(i), operators)];
      node->m_operate = encodeOperator(opStr);
      node->m_type = LEAF;
      node->m_datatype = UNKNOWN;
      node->m_expType = UNKNOWN;
      node->m_expStr = trim_one( boost::algorithm::trim_copy<string>(str),'"');
      node->m_leftNode = NULL;
      node->m_rightNode = NULL;
      return;
    }
  }
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
    if (dateStr.size() > 1){
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
              node->m_operate = encodeOperator(string(initialString[nextPos]));
              node->m_datatype = DATE;
              node->m_expType = UNKNOWN;
              node->m_expStr = string(initialString[nextPos]);
              node->m_colId = -1;
              node->m_parentNode = NULL;
              node->m_leftNode = leftNode;
              node->m_rightNode = rightNode;
              rightNode->m_parentNode = node;
              return true;
            }else
              delete rightNode;
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
            node->m_operate = encodeOperator(string(initialString[nextPos]));
            node->m_datatype = DATE;
            node->m_expType = UNKNOWN;
            node->m_expStr = string(initialString[nextPos]);
            node->m_colId = -1;
            node->m_parentNode = NULL;
            node->m_leftNode = leafNode;
            node->rightNode = rightNode;
            
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
            node->m_operate = encodeOperator(string(initialString[nextPos]));
            node->m_datatype = STRING;
            node->m_expType = UNKNOWN;
            node->m_expStr = string(initialString[nextPos]);
            node->m_colId = -1;
            node->m_parentNode = NULL;
            node->m_leftNode = leafNode;
            node->rightNode = rightNode;
            
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
        node->m_operate = encodeOperator(string(initialString[nextPos]));
        node->m_datatype = UNKNOWN;
        node->m_expType = UNKNOWN;
        node->m_expStr = string(initialString[nextPos]);
        node->m_colId = -1;
        node->m_parentNode = NULL;
        node->m_leftNode = leafNode;
        node->rightNode = rightNode;
        
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
    trace(ERROR, "Building expression from %s failed!\n", initialString);
    return NULL;
  }
  //node.dump();
  //printf(" completed!\n");
  return node;
}

// get left tree Height
int FilterC::getLeftHeight(){
  int height = 1;
  if (type == BRANCH && leftNode)
    height += leftNode->getLeftHeight();

  return height;
}

// get left tree Height
int FilterC::getRightHeight(){
  int height = 1;
  if (type == BRANCH && rightNode)
    height += rightNode->getRightHeight();
  
  return height;
}

// add a NEW filter into tree
void FilterC::add(FilterC* node, int junction, bool leafGrowth, bool addOnTop){
  // not add any null or UNKNOWN node
  if (node || node->type ==  UNKNOWN) 
      return;
  if (type ==  UNKNOWN){ // not assinged
      node->copyTo(this);
  }else if (type == LEAF){
    FilterC* existingNode = new FilterC();
    copyTo(existingNode);
    type = BRANCH;
    junction = junction;
    comparator = UNKNOWN;   
    leftColId = -1;  
    rightColId = -1;    
    leftExpStr = "";
    rightExpStr = "";
    leftExpression = NULL;
    rightExpression = NULL;
    if (leafGrowth){
      rightNode = existingNode;
      rightNode->parentNode = this;
      leftNode = node;
      leftNode->parentNode = this;
    }else{
      leftNode = existingNode;
      leftNode->parentNode = this;
      rightNode = node;
      rightNode->parentNode = this;
    }
  }else{
    if (addOnTop){
      FilterC* existingNode = new FilterC();
      copyTo(existingNode);
      junction = junction;
      if (leafGrowth){
        leftNode = node;
        leftNode->parentNode = this;
        rightNode = existingNode;
        rightNode->parentNode = this;
      }else{
        leftNode = existingNode;
        leftNode->parentNode = this;
        rightNode = node;
        rightNode->parentNode = this;
      }
    }else{
      if (leafGrowth){
        if (leftNode)
          leftNode->add(node, junction, leafGrowth, addOnTop);
        else 
          rightNode->add(node, junction, leafGrowth, addOnTop);
      }else{
        if (rightNode)
          rightNode->add(node, junction, leafGrowth, addOnTop);
        else 
          leftNode->add(node, junction, leafGrowth, addOnTop);
      }
    }
  }
}

void FilterC::dump(int deep){
  if (type == BRANCH){
    trace(INFO,"(%d)%s\n",deep,decodeJunction(junction).c_str());
    trace(INFO,"L-");
    leftNode->dump(deep+1);
    trace(INFO,"R-");
    rightNode->dump(deep+1);
  }else{
    trace(INFO,"(%d)%s(%d)",deep,leftExpStr.c_str(),leftColId);
    trace(INFO,"%s",decodeComparator(comparator).c_str());
    trace(INFO,"%s\n",rightExpStr.c_str());
  }
}

void FilterC::dump(){
  dump(0);
}

// detect if predication contains special colId    
bool FilterC::containsColId(int colId){
  bool contain = false;
  if (type == BRANCH){
    contain = contain || leftNode->containsColId(colId);
    contain = contain || rightNode->containsColId(colId);
  }else
    contain = (leftColId == colId);

  return contain;
}

// detect if predication contains special colId    
FilterC* FilterC::getFirstPredByColId(int colId, bool leftFirst){
  FilterC* node;
  if (type == BRANCH){
    if (leftFirst){
      if (leftNode)
        node = leftNode->getFirstPredByColId(colId, leftFirst);
      if (!node)
        node = rightNode->getFirstPredByColId(colId, leftFirst);
    }else{
      if (rightNode)
        node = rightNode->getFirstPredByColId(colId, leftFirst);
      if (!node)
        node = leftNode->getFirstPredByColId(colId, leftFirst);
    }
  }else if (type == LEAF)
    if (leftColId == colId)
      node = this;

  return node;
}

// analyze column ID & name from metadata
bool FilterC::analyzeColumns(vector<string> m_fieldnames1, vector<string> m_fieldnames2){
  if (type == BRANCH){
    metaDataAnzlyzed = true;
    if (leftNode)
        metaDataAnzlyzed = metaDataAnzlyzed && leftNode->analyzeColumns(m_fieldnames1, m_fieldnames2);
    if (!metaDataAnzlyzed)
        return metaDataAnzlyzed;
    if (rightNode)
        metaDataAnzlyzed = metaDataAnzlyzed &&  rightNode->analyzeColumns(m_fieldnames1, m_fieldnames2);
  }else if (type == LEAF){
    datatype = detectDataType(rightExpStr);
    if (datatype == UNKNOWN)
      datatype = detectDataType(leftExpStr);

    if (m_fieldnames1.size()>0){
      if (leftExpStr[0] == '"') {// quoted, treat as expression, otherwise, as columns
        leftExpStr = trim_one(leftExpStr,'"'); // remove quoters
        leftColId = -1;
      }else {
        if (isInt(leftExpStr)){ // check if the name is ID already
          leftColId = atoi(leftExpStr.c_str());
          leftExpStr = m_fieldnames1[leftColId];
        }else{
          leftColId = findStrArrayId(m_fieldnames1, leftExpStr);
        }
      }
    }
    if (m_fieldnames2.size()>0){
      if (rightExpStr[0] == '"') {// quoted, treat as expression, otherwise, as columns
        rightExpStr = trim_one(rightExpStr,'"'); // remove quoters
        rightColId = -1;
      }else {
        if (isInt(rightExpStr)){ // check if the name is ID already
          rightColId = atoi(rightExpStr.c_str());
          rightExpStr = m_fieldnames2[rightColId];
        }else{
          rightColId = findStrArrayId(m_fieldnames2, rightExpStr);
        }
      }
    }
    if(leftColId != -1 && rightColId != -1){
      //if (metaData1.getColumnType(leftColId) != metaData2.getColumnType(rightColId)){
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
  node->type = type;
  node->datatype = datatype;
  node->junction = junction;
  node->comparator = comparator;
  node->leftColId = leftColId;
  node->rightColId = rightolId;
  node->rightExpStr = rightExpStr;
  node->leftExpStr = leftExpStr;
  if (type == BRANCH){
    node->leftExpression = new ExpressionC(node->leftExpStr);
    node->leftExpression = leftExpression->cloneMe();
    node->rightExpression = new ExpressionC(node->rightExpStr);
    node->rightExpression = rightExpression->cloneMe();
    node->leftNode = new FilterC();
    node->leftNode = leftNode->cloneMe();
    node->rightNode = new FilterC();
    node->rightNode = rightNode->cloneMe();
    node->leftNode->parentNode = node;
    node->rightNode->parentNode = node;
  }else{
    node->leftExpression = NULL;
    node->rightExpression = NULL;
    node->leftNode = NULL;
    node->rightNode = NULL;
  }
  return node;
}

void FilterC::copyTo(FilterC* node){
  if (!node)
    return;
  else{
    node->metaDataAnzlyzed = metaDataAnzlyzed;
    //node->predStr = predStr;
    node->type = type;
    node->datatype = datatype;
    node->junction = junction;
    node->comparator = comparator;
    node->leftColId = leftColId;
    node->rightColId = rightolId;
    node->rightExpStr = rightExpStr;
    node->leftExpStr = leftExpStr;
    if (type == BRANCH){
      if (leftNode){
        node->leftNode = new FilterC();
        leftNode->copyTo(node->leftNode);
        node->leftNode->parentNode = node;
      }else
        node->leftNode = NULL;

      if (rightNode){
        node->rightNode = new FilterC();
        rightNode->copyTo(node->rightNode);
        node->rightNode->parentNode = node;
      }else
        node->rightNode = NULL;
      node->leftExpression = NULL;
      node->rightExpression = NULL;
    }else{
      if (leftExpression){
        node->leftExpression = new ExpressionC(leftExpStr);
        leftExpression->copyTo(node->leftExpression);
      }
      if (rightExpression){
        node->rightExpression = new ExpressionC(rightExpStr);
        rightExpression->copyTo(node->rightExpression);
      }
    }
  }
}

// get all involved colIDs in this prediction
std::set<int> FilterC::getAllColIDs(int side){
  std::set<int> colIDs;
  if (type == BRANCH){
    if (leftNode){
      std::set<int> foo = leftNode->getAllColIDs(side);
      colIDs.insert(foo.begin(), foo.end());
    }
    if (rightNode){
      std::set<int> foo = rightNode->getAllColIDs(side);
      colIDs.insert(foo.begin(), foo.end());
    }
  }else if(type == LEAF){
    if (side == LEFT && leftColId>=0)
      colIDs.insert(leftColId);
    else if (side == RIGHT && rightColId>=0)
      colIDs.insert(rightColId);
  }
  return colIDs;
}

// build the prediction as a HashMap
map<int,string> FilterC::buildMap(){
  map<int,string> datas;
  if (type == BRANCH){
    if (leftNode){
      map<int,string> foo = leftNode->buildMap();
      datas.insert(foo.begin(), foo.end());
    }
    if (rightNode){
      map<int,string> foo = rightNode->buildMap();
      datas.insert(foo.begin(), foo.end());
    }
  }else if(type == LEAF){
    if (leftColId>=0)
      datas.insert( pair<int,string>(leftColId,rightExpStr) );
  }
  return datas;
}

// calculate an expression prediction
bool FilterC::compareExpression(){
  bool result=true;
  if (type == BRANCH){
    if (!leftNode || !rightNode)
      return false;
    if (junction == AND)
      result = leftNode->compareExpression() && rightNode->compareExpression();
    else
      result = leftNode->compareExpression() || rightNode->compareExpression();
  }else if(type == LEAF){
    return anyDataCompare(leftExpStr, comparator, rightExpStr, STRING) == 1;
  }else{ // no predication means alway true
    return true;
  }
  return result;
}

// get all involved colIDs in this prediction
int FilterC::size(){
  int size = 0;
  if (type == BRANCH){
    if (leftNode)
      size += leftNode->size();
    if (rightNode)
      size += rightNode->size();
  }else if (type == LEAF)
    size = 1;
  else 
    size = 0;
  return size;
}

// clear predictin
void FilterC::clear(){
  if (leftNode){
    leftNode->clear();
    delete leftNode;
    leftNode = NULL;
  }
  if (rightNode){
    rightNode->clear();
    delete rightNode;
    rightNode = NULL;
  }
  type = UNKNOWN;
  datatype = UNKNOWN;
  junction = UNKNOWN;
  comparator = UNKNOWN;
  leftColId = -1;
  rightColId = -1;
  rightExpStr = "";
  leftExpStr = "";
  metaDataAnzlyzed = false;
}

// remove a node from prediction. Note: the input node is the address of the node contains in current prediction
//   0                      0                  2                 1
//  1  2  (remove 3) =>   4   2 (remove 1) =>      (remove 2)  3   4
//3  4
bool FilterC::remove(FilterC* node){
  bool removed = false;
  if (leftNode){
    if (leftNode == node){
      leftNode->clear();
      delete leftNode;
      leftNode = NULL;
      return true;
    }else{
      removed = removed || leftNode->remove(node);
      if (removed)
        return removed;
    }
  }
  if (rightNode){
    if (rightNode == node){
      rightNode->clear();
      delete rightNode;
      rightNode = NULL;
      return true;
    }else{
      removed = removed || rightNode->remove(node);
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
      if (this.parentNode != null){
          if (this.parentNode.leftNode == this ) // this is leftnode
              this.parentNode.rightNode.copyTo(this); // assign right brother as parent
          else if (this.parentNode.rightNode == this ) // this is rihtnode
               this.parentNode.leftNode.copyTo(this); // assign left brother as parent
      }
      return true;
  }else if (type == Consts.BRANCH){
      return (leftNode != null && leftNode.remove(node)) || (rightNode != null && rightNode.remove(node));
  }
  return false;//*/
}

// build a data list for a set of column, keeping same sequence, fill the absent column with NULL
void FilterC::fillDataForColumns(map <string, string> & dataList, vector <string> columns){
  if (columns.size() == 0)
    return;
  if (type == BRANCH){
    if (leftNode)
      leftNode->fillDataForColumns(dataList, columns);
    if (rightNode)
      rightNode->fillDataForColumns(dataList, columns);
  }else if (type == LEAF && leftColId >= 0)
    dataList.insert( pair<string,string>(columns[leftColId],rightExpStr) );
}
