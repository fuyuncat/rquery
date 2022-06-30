/*******************************************************************************
//
//        File: expression.cpp
// Description: Expression class defination
//       Usage: expression.cpp
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
#include "expression.h"

void ExpressionC::init()
{
  type = UNKNOWN;       // 1: branch; 2: leaf
  junction = UNKNOWN;   // if type is BRANCH, 1: and; 2: or. Otherwise, it's meaningless
  operate = UNKNOWN; // if type is LEAF, 1: ==; 2: >; 3: <; 4: !=; 5: >=; 6: <=. Otherwise, it's meaningless
  datatype = UNKNOWN;   // if type is LEAF, 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
  leftColId = -1;              // if type is LEAF, it's id of column on the left to be predicted. Otherwise, it's meaningless
  rightColId = -1;             // if type is LEAF, it's id of column on the right to be predicted. Otherwise, it's meaningless
  leftExpression = "";    // if type is LEAF, it's id of column to be predicted. Otherwise, it's meaningless
  rightExpression = "";   // if type is LEAF, it's data to be predicted. Otherwise, it's meaningless
  leftNode = NULL;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
  rightNode = NULL;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
  parentNode = NULL;    // for all types except the root, it links to parent node. Otherwise, it's meaningless
  
  metaDataAnzlyzed = false; // analyze column name to column id.
}

ExpressionC::ExpressionC()
{
  init();
}

ExpressionC::~ExpressionC()
{

}

ExpressionC::ExpressionC(ExpressionC* node)
{
  init();

  type = node->type;
  junction = node->junction;
  operate = node->operate;
  leftColId = node->leftColId;
  rightExpression = node->rightExpression;
  leftExpression = node->leftExpression;
  leftNode = node->leftNode;
  rightNode = node->rightNode;
  parentNode = node->parentNode;
  metaDataAnzlyzed = node->metaDataAnzlyzed;
  //predStr = node.predStr;
}

ExpressionC::ExpressionC(int junction, ExpressionC* leftNode, ExpressionC* rightNode)
{
  init();
  type = BRANCH;
  junction = junction;
  leftNode = leftNode;
  rightNode = rightNode;
  //leftNode = leftNode==NULL?NULL:new Prediction(leftNode);
  //rightNode = rightNode==NULL?NULL:new Prediction(rightNode);;
}

ExpressionC::ExpressionC(int operate, int colId, string data)
{
  init();
  type = LEAF;
  operate = operate;
  leftColId = colId;
  rightExpression = data;
}

// get left tree Height
int ExpressionC::getLeftHeight(){
  int height = 1;
  if (type == BRANCH && leftNode)
    height += leftNode->getLeftHeight();

  return height;
}

// get left tree Height
int ExpressionC::getRightHeight(){
  int height = 1;
  if (type == BRANCH && rightNode)
    height += rightNode->getRightHeight();
  
  return height;
}

// add a NEW preiction into tree
void ExpressionC::add(ExpressionC* node, int junction, bool leafGrowth, bool addOnTop){
  // not add any null or UNKNOWN node
  if (node || node->type ==  UNKNOWN) 
      return;
  if (type ==  UNKNOWN){ // not assinged
      node->copyTo(this);
  }else if (type == LEAF){
    ExpressionC* existingNode = new ExpressionC();
    copyTo(existingNode);
    type = BRANCH;
    junction = junction;
    operate = UNKNOWN;   
    leftColId = -1;        
    leftExpression = "";
    rightExpression = "";
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
      ExpressionC* existingNode = new ExpressionC();
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

void ExpressionC::dump(int deep){
  if (type == BRANCH){
    trace(INFO,"(%d)%s\n",deep,decodeJunction(junction).c_str());
    trace(INFO,"L-");
    leftNode->dump(deep+1);
    trace(INFO,"R-");
    rightNode->dump(deep+1);
  }else{
    trace(INFO,"(%d)%s(%d)",deep,leftExpression.c_str(),leftColId);
    trace(INFO,"%s",decodeComparator(comparator).c_str());
    trace(INFO,"%s\n",rightExpression.c_str());
  }
}

void ExpressionC::dump(){
  dump(0);
}

// detect if predication contains special colId    
bool ExpressionC::containsColId(int colId){
  bool contain = false;
  if (type == BRANCH){
    contain = contain || leftNode->containsColId(colId);
    contain = contain || rightNode->containsColId(colId);
  }else
    contain = (leftColId == colId);

  return contain;
}

// detect if predication contains special colId    
ExpressionC* ExpressionC::getFirstPredByColId(int colId, bool leftFirst){
  ExpressionC* node;
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
bool ExpressionC::analyzeColumns(vector<string> m_fieldnames1, vector<string> m_fieldnames2){
  if (type == BRANCH){
    metaDataAnzlyzed = true;
    if (leftNode)
        metaDataAnzlyzed = metaDataAnzlyzed && leftNode->analyzeColumns(m_fieldnames1, m_fieldnames2);
    if (!metaDataAnzlyzed)
        return metaDataAnzlyzed;
    if (rightNode)
        metaDataAnzlyzed = metaDataAnzlyzed &&  rightNode->analyzeColumns(m_fieldnames1, m_fieldnames2);
  }else if (type == LEAF){
    datatype = detectDataType(rightExpression);
    if (datatype == UNKNOWN)
      datatype = detectDataType(leftExpression);

    if (m_fieldnames1.size()>0){
      if (leftExpression[0] == '"') {// quoted, treat as expression, otherwise, as columns
        leftExpression = trim_one(leftExpression,'"'); // remove quoters
        leftColId = -1;
      }else {
        if (isInt(leftExpression)){ // check if the name is ID already
          leftColId = atoi(leftExpression.c_str());
          leftExpression = m_fieldnames1[leftColId];
        }else{
          leftColId = findStrArrayId(m_fieldnames1, leftExpression);
        }
      }
    }
    if (m_fieldnames2.size()>0){
      if (rightExpression[0] == '"') {// quoted, treat as expression, otherwise, as columns
        rightExpression = trim_one(rightExpression,'"'); // remove quoters
        rightColId = -1;
      }else {
        if (isInt(rightExpression)){ // check if the name is ID already
          rightColId = atoi(rightExpression.c_str());
          rightExpression = m_fieldnames2[rightColId];
        }else{
          rightColId = findStrArrayId(m_fieldnames2, rightExpression);
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

bool ExpressionC::columnsAnalyzed(){
    return metaDataAnzlyzed;
}

ExpressionC* ExpressionC::cloneMe(){
  ExpressionC* node = new ExpressionC();
  node->metaDataAnzlyzed = metaDataAnzlyzed;
  //node->predStr = predStr;
  node->type = type;
  node->junction = junction;
  node->operate = operate;
  node->leftColId = leftColId;
  node->rightExpression = rightExpression;
  node->leftExpression = leftExpression;
  if (type == BRANCH){
    node->leftNode = new ExpressionC();
    node->leftNode = leftNode->cloneMe();
    node->rightNode = new ExpressionC();
    node->rightNode = rightNode->cloneMe();
    node->leftNode->parentNode = node;
    node->rightNode->parentNode = node;
  }else{
    node->leftNode = NULL;
    node->rightNode = NULL;
  }
  return node;
}

void ExpressionC::copyTo(ExpressionC* node){
  if (!node)
    return;
  else{
    node->metaDataAnzlyzed = metaDataAnzlyzed;
    //node->predStr = predStr;
    node->type = type;
    node->junction = junction;
    node->operate = operate;
    node->leftColId = leftColId;
    node->rightExpression = rightExpression;
    node->leftExpression = leftExpression;
    if (type == BRANCH){
      if (leftNode){
        node->leftNode = new ExpressionC();
        leftNode->copyTo(node->leftNode);
        node->leftNode->parentNode = node;
      }else
        node->leftNode = NULL;
      
      if (rightNode){
        node->rightNode = new ExpressionC();
        rightNode->copyTo(node->rightNode);
        node->rightNode->parentNode = node;
      }else
        node->rightNode = NULL;
    }
  }
}

// get all involved colIDs in this prediction
std::set<int> ExpressionC::getAllColIDs(int side){
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
map<int,string> ExpressionC::buildMap(){
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
      datas.insert( pair<int,string>(leftColId,rightExpression) );
  }
  return datas;
}

// calculate an expression prediction
bool ExpressionC::compareExpression(){
  bool result=true;
  if (type == BRANCH){
    if (!leftNode || !rightNode)
      return false;
    if (junction == AND)
      result = leftNode->compareExpression() && rightNode->compareExpression();
    else
      result = leftNode->compareExpression() || rightNode->compareExpression();
  }else if(type == LEAF){
    return anyDataCompare(leftExpression, operate, rightExpression, STRING) == 1;
  }else{ // no predication means alway true
    return true;
  }
  return result;
}

// get all involved colIDs in this prediction
int ExpressionC::size(){
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
void ExpressionC::clear(){
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
  junction = UNKNOWN;
  operate = UNKNOWN;
  leftColId = -1;
  rightExpression = "";
  leftExpression = "";
}

// remove a node from prediction. Note: the input node is the address of the node contains in current prediction
//   0                      0                  2                 1
//  1  2  (remove 3) =>   4   2 (remove 1) =>      (remove 2)  3   4
//3  4
bool ExpressionC::remove(ExpressionC* node){
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
void ExpressionC::fillDataForColumns(map <string, string> & dataList, vector <string> columns){
  if (columns.size() == 0)
    return;
  if (type == BRANCH){
    if (leftNode)
      leftNode->fillDataForColumns(dataList, columns);
    if (rightNode)
      rightNode->fillDataForColumns(dataList, columns);
  }else if (type == LEAF && leftColId >= 0)
    dataList.insert( pair<string,string>(columns[leftColId],rightExpression) );
}
