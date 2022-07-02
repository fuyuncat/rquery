/*******************************************************************************
//
//        File: function.cpp
// Description: Function class defination
//       Usage: function.cpp
//     Created: 28/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  28/06/2022: Created
//
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <boost/algorithm/string.hpp>
#include "expression.h"
#include "function.h"

void FunctionC::init()
{
  m_type = UNKNOWN;       // 1: branch; 2: leaf
  m_operate = UNKNOWN; // if type is LEAF, 1: ==; 2: >; 3: <; 4: !=; 5: >=; 6: <=. Otherwise, it's meaningless
  m_datatype = UNKNOWN;   // 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
  m_expType = UNKNOWN;    // if type is LEAF, the expression string type. 1: CONST, 2: COLUMN, 3: FUNCTION
  m_colId = -1;      // if type is LEAF, and the expression string type is COLUMN. it's id of column. Otherwise, it's meaningless
  m_expStr = "";    // if type is LEAF, the expression string, either be a CONST, COLUMN or FUNCTION
  m_leftNode = NULL;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
  m_rightNode = NULL;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
  m_parentNode = NULL;    // for all types except the root, it links to parent node. Otherwise, it's meaningless

  m_metaDataAnzlyzed = false; // analyze column name to column id.
}

FunctionC::FunctionC()
{
  init();
}

FunctionC::FunctionC(string expString)
{
  init();
  m_expStr = expString;
  checkDataType();
}

FunctionC::~FunctionC()
{

}

FunctionC::FunctionC(FunctionC* node)
{
  init();

  m_type = node->m_type;
  m_operate = node->m_operate;
  m_colId = node->m_colId;
  m_datatype = node->m_datatype;
  m_expType = node->m_expType;
  m_expStr = node->m_expStr;
  m_leftNode = node->m_leftNode;
  m_rightNode = node->m_rightNode;
  m_parentNode = node->m_parentNode;
  m_metaDataAnzlyzed = node->m_metaDataAnzlyzed;
  //predStr = node.predStr;
}

FunctionC::FunctionC(FunctionC* m_leftNode, FunctionC* m_rightNode)
{
  init();
  m_type = BRANCH;
  m_leftNode = m_leftNode;
  m_rightNode = m_rightNode;
  //m_leftNode = m_leftNode==NULL?NULL:new Prediction(m_leftNode);
  //m_rightNode = m_rightNode==NULL?NULL:new Prediction(m_rightNode);;
}

FunctionC::FunctionC(int operate, int colId, string data)
{
  init();
  m_type = LEAF;
  m_operate = operate;
  m_colId = colId;
  m_expStr = data;
}

// detect function return type
void FunctionC::checkDataType()
{
  m_datatype = UNKNOWN;
}

// get left tree Height
int FunctionC::getLeftHeight(){
  int height = 1;
  if (m_type == BRANCH && m_leftNode)
    height += m_leftNode->getLeftHeight();

  return height;
}

// get left tree Height
int FunctionC::getRightHeight(){
  int height = 1;
  if (m_type == BRANCH && m_rightNode)
    height += m_rightNode->getRightHeight();
  
  return height;
}

// add a NEW expression into tree
void FunctionC::add(FunctionC* node, int op, bool leafGrowth, bool addOnTop){
  // not add any null or UNKNOWN node
  if (node || node->m_type ==  UNKNOWN) 
      return;
  if (m_type ==  UNKNOWN){ // not assinged
      node->copyTo(this);
  }else if (m_type == LEAF){
    FunctionC* existingNode = new FunctionC();
    copyTo(existingNode);
    m_type = BRANCH;
    m_operate = op;   
    m_colId = -1;
    m_expStr = "";
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
      FunctionC* existingNode = new FunctionC();
      copyTo(existingNode);
      m_type = BRANCH;
      m_operate = op;   
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
          m_leftNode->add(node, op, leafGrowth, addOnTop);
        else 
          m_rightNode->add(node, op, leafGrowth, addOnTop);
      }else{
        if (m_rightNode)
          m_rightNode->add(node, op, leafGrowth, addOnTop);
        else 
          m_leftNode->add(node, op, leafGrowth, addOnTop);
      }
    }
  }
}

void FunctionC::dump(int deep){
  if (m_type == BRANCH){
    trace(INFO,"%s(%d)\n",decodeOperator(m_operate).c_str(),deep);
    trace(INFO,"L-");
    m_leftNode->dump(deep+1);
    trace(INFO,"R-");
    m_rightNode->dump(deep+1);
  }else{
    trace(INFO,"(%d)%s(%d)\n",deep,m_expStr.c_str(),m_colId);
  }
}

void FunctionC::dump(){
  dump(0);
}

// detect if predication contains special colId    
bool FunctionC::containsColId(int colId){
  bool contain = false;
  if (m_type == BRANCH){
    contain = contain || m_leftNode->containsColId(colId);
    contain = contain || m_rightNode->containsColId(colId);
  }else
    contain = (m_colId == colId);

  return contain;
}

// detect if predication contains special colId    
FunctionC* FunctionC::getFirstPredByColId(int colId, bool leftFirst){
  FunctionC* node;
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
    if (m_colId == colId)
      node = this;

  return node;
}

// analyze column ID & name from metadata, return data type of current node
// decide current node data type by checking children's data type
int FunctionC::analyzeColumns(vector<string> fieldnames, vector<int> fieldtypes)
{
  m_metaDataAnzlyzed = true;
  if (m_type == BRANCH){
    int rdatatype = m_rightNode?m_rightNode->analyzeColumns(fieldnames, fieldtypes):UNKNOWN;
    int ldatatype = m_leftNode?m_leftNode->analyzeColumns(fieldnames, fieldtypes):UNKNOWN;
    if (ldatatype == STRING || rdatatype == STRING)
      if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP){ // incompatible types
        trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(STRING).c_str(), decodeDatatype(ldatatype==STRING?rdatatype:ldatatype).c_str());
        return UNKNOWN;
      }else
        return STRING;
    else if (ldatatype == DOUBLE || rdatatype == DOUBLE)
      if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP || ldatatype == STRING || rdatatype == STRING){ // incompatible types
        trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(DOUBLE).c_str(), decodeDatatype(ldatatype==DOUBLE?rdatatype:ldatatype).c_str());
        return UNKNOWN;
      }else
        return DOUBLE;
    else if (ldatatype == LONG || rdatatype == LONG)
      if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP || ldatatype == STRING || rdatatype == STRING || ldatatype == DOUBLE || rdatatype == DOUBLE){ // incompatible types
        trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(LONG).c_str(), decodeDatatype(ldatatype==LONG?rdatatype:ldatatype).c_str());
        return UNKNOWN;
      }else
        return LONG;
    else if (ldatatype == INTEGER || rdatatype == INTEGER)
      if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP || ldatatype == STRING || rdatatype == STRING || ldatatype == DOUBLE || rdatatype == DOUBLE || ldatatype == LONG || rdatatype == LONG){ // incompatible types
        trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(INTEGER).c_str(), decodeDatatype(ldatatype==INTEGER?rdatatype:ldatatype).c_str());
        return UNKNOWN;
      }else
        return INTEGER;
    else if (ldatatype == BOOLEAN || rdatatype == BOOLEAN)
      if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP || ldatatype == STRING || rdatatype == STRING || ldatatype == DOUBLE || rdatatype == DOUBLE || ldatatype == LONG || rdatatype == LONG || ldatatype == INTEGER || rdatatype == INTEGER){ // incompatible types
        trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(BOOLEAN).c_str(), decodeDatatype(ldatatype==BOOLEAN?rdatatype:ldatatype).c_str());
        return UNKNOWN;
      }else
        return BOOLEAN;
    else if (ldatatype == DATE || rdatatype == DATE || ldatatype == TIMESTAMP || rdatatype == TIMESTAMP)
      if (ldatatype == STRING || rdatatype == STRING || ldatatype == DOUBLE || rdatatype == DOUBLE || ldatatype == LONG || rdatatype == LONG || ldatatype == INTEGER || rdatatype == INTEGER || ldatatype == BOOLEAN || rdatatype == BOOLEAN){ // incompatible types
        trace(ERROR, "Datatype %s is incompatible to %s. ", decodeDatatype(DATE).c_str(), decodeDatatype((ldatatype==DATE||ldatatype==TIMESTAMP)?rdatatype:ldatatype).c_str());
        return UNKNOWN;
      }else
        return DATE;
    else
      return UNKNOWN;
  }else{
    if (fieldnames.size() != fieldtypes.size()){
      trace(ERROR,"Field name number %d does not match field type number %d.\n", fieldnames.size(), fieldtypes.size());
      return UNKNOWN;
    }
    m_expStr = boost::algorithm::trim_copy<string>(m_expStr);
    // check if it is a variable
    if (m_expStr.size()>0 && m_expStr[0]=='@'){
      m_expType = VARIABLE;
      string strLowName = boost::to_lower_copy<string>(m_expStr);
      if (strLowName.compare("@raw") == 0 || strLowName.compare("@file") == 0)
        m_datatype = STRING;
      else if (strLowName.compare("@line") == 0 || strLowName.compare("@row") == 0 || strLowName.compare("@rowsorted") == 0)
        m_datatype = LONG;
      else if (strLowName.find("@field") == 0){
        string sColId = strLowName.substr(string("@field").size());
        if (isInt(sColId) && atoi(sColId.c_str()) < fieldtypes.size()){
          m_expType = COLUMN;
          m_datatype = fieldtypes[atoi(sColId.c_str())];
        }else{
          trace(ERROR, "Unrecognized variable %s .\n", m_expStr.c_str());
          m_expType = UNKNOWN;
          m_datatype = UNKNOWN;
        }
      }
      else{
        trace(ERROR, "Unrecognized variable %s .\n", m_expStr.c_str());
        m_expType = UNKNOWN;
        m_datatype = UNKNOWN;
      }
      return m_datatype;
    }
    // check if it is a function FUNCNAME(...)
    int lefParPos = m_expStr.find("(");
    if (m_expStr.size()>2 && m_expStr[0] != '\'' && lefParPos>0 && m_expStr[m_expStr.size()-1] == ')'){
      //m_expType = FUNCTION;
      //m_datatype = funcReturnType(m_expStr.substr(0,lefParPos));
      return m_datatype;
    }
    // check if it is a column
    for (int i=0; i<fieldnames.size(); i++){
      if (boost::to_upper_copy<string>(m_expStr).compare(fieldnames[i]) == 0){
        m_expType = COLUMN;
        m_datatype = fieldtypes[i];
        return m_datatype;
      }
    }
    m_expType = CONST;
    m_datatype = detectDataType(m_expStr);
    return m_datatype;
  }
}

bool FunctionC::columnsAnalyzed(){
    return m_metaDataAnzlyzed;
}

FunctionC* FunctionC::cloneMe(){
  FunctionC* node = new FunctionC();
  node->m_metaDataAnzlyzed = m_metaDataAnzlyzed;
  //node->predStr = predStr;
  node->m_type = m_type;
  node->m_operate = m_operate;
  node->m_colId = m_colId;
  node->m_datatype = m_datatype;
  node->m_expType = m_expType;
  node->m_expStr = m_expStr;
  if (m_type == BRANCH){
    node->m_leftNode = new FunctionC();
    node->m_leftNode = m_leftNode->cloneMe();
    node->m_rightNode = new FunctionC();
    node->m_rightNode = m_rightNode->cloneMe();
    node->m_leftNode->m_parentNode = node;
    node->m_rightNode->m_parentNode = node;
  }else{
    node->m_leftNode = NULL;
    node->m_rightNode = NULL;
  }
  return node;
}

void FunctionC::copyTo(FunctionC* node){
  if (!node)
    return;
  else{
    node->m_metaDataAnzlyzed = m_metaDataAnzlyzed;
    //node->predStr = predStr;
    node->m_type = m_type;
    node->m_operate = m_operate;
    node->m_colId = m_colId;
    node->m_datatype = m_datatype;
    node->m_expType = m_expType;
    node->m_expStr = m_expStr;
    if (m_type == BRANCH){
      if (m_leftNode){
        node->m_leftNode = new FunctionC();
        m_leftNode->copyTo(node->m_leftNode);
        node->m_leftNode->m_parentNode = node;
      }else
        node->m_leftNode = NULL;
      
      if (m_rightNode){
        node->m_rightNode = new FunctionC();
        m_rightNode->copyTo(node->m_rightNode);
        node->m_rightNode->m_parentNode = node;
      }else
        node->m_rightNode = NULL;
    }
  }
}

// get all involved colIDs in this expression
std::set<int> FunctionC::getAllColIDs(int side){
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
    if (m_colId>=0)
      colIDs.insert(m_colId);
  }
  return colIDs;
}

// build the expression as a HashMap
map<int,string> FunctionC::buildMap(){
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
    if (m_colId>=0)
      datas.insert( pair<int,string>(m_colId,m_expStr) );
  }
  return datas;
}

// get all involved colIDs in this prediction
int FunctionC::size(){
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

// clear expression
void FunctionC::clear(){
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
  m_operate = UNKNOWN;
  m_colId = -1;
  m_expStr = "";
}

// remove a node from prediction. Note: the input node is the address of the node contains in current prediction
//   0                      0                  2                 1
//  1  2  (remove 3) =>   4   2 (remove 1) =>      (remove 2)  3   4
//3  4
bool FunctionC::remove(FunctionC* node){
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
void FunctionC::fillDataForColumns(map <string, string> & dataList, vector <string> columns){
  if (columns.size() == 0)
    return;
  if (m_type == BRANCH){
    if (m_leftNode)
      m_leftNode->fillDataForColumns(dataList, columns);
    if (m_rightNode)
      m_rightNode->fillDataForColumns(dataList, columns);
  }else if (m_type == LEAF && m_colId >= 0)
    dataList.insert( pair<string,string>(columns[m_colId],m_expStr) );
}

// run function and get result
bool FunctionC::runFunction(string & sResult)
{
  return false;
}

// run function and get result
bool FunctionC::runFunction(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  return false;
}