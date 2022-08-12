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
#include "function.h"

std::set<char> ExpressionC::m_operators;

void ExpressionC::init()
{
  m_type = UNKNOWN;       // 1: branch; 2: leaf
  m_operate = UNKNOWN; // if type is LEAF, 1: ==; 2: >; 3: <; 4: !=; 5: >=; 6: <=. Otherwise, it's meaningless
  m_datatype.datatype = UNKNOWN;   // 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
  m_datatype.extrainfo = "";
  m_expType = UNKNOWN;    // if type is LEAF, the expression string type. 1: CONST, 2: COLUMN, 3: FUNCTION
  m_funcID = UNKNOWN;
  m_colId = -1;      // if type is LEAF, and the expression string type is COLUMN. it's id of column. Otherwise, it's meaningless
  m_expStr = "";    // if type is LEAF, the expression string, either be a CONST, COLUMN or FUNCTION
  m_leftNode = NULL;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
  m_rightNode = NULL;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
  m_parentNode = NULL;    // for all types except the root, it links to parent node. Otherwise, it's meaningless
  m_fieldnames = NULL;
  m_fieldtypes = NULL;
  m_Function = NULL;

  m_metaDataAnzlyzed = false; // analyze column name to column id.
  m_expstrAnalyzed = false;   // if expression string analyzed

  //m_operators = {'^','*','/','+','-'};
  m_operators.clear();
  m_operators.insert('^');m_operators.insert('*');m_operators.insert('/');m_operators.insert('+');m_operators.insert('-'); // "^", "*", "/" should be before "+", "-"
}

ExpressionC::ExpressionC()
{
  init();
}

ExpressionC::ExpressionC(string expString)
{
  init();
  setExpstr(expString);
}

// Rule one Copy Constructor
ExpressionC::ExpressionC(const ExpressionC& other)
{
  if (this != &other){
    init();
    other.copyTo(this);
  }
}

// Rule two Destructor
ExpressionC::~ExpressionC()
{
  clear();
}

// Rule one Copy Assignment Operator
ExpressionC& ExpressionC::operator=(const ExpressionC& other)
{
  if (this != &other){
    clear();
    other.copyTo(this);
  }
  return *this;
}

ExpressionC::ExpressionC(ExpressionC* node)
{
  if (node && node!=this){
    init();
    node->copyTo(this);
  }
}

void ExpressionC::copyTo(ExpressionC* node) const
{
  if (!node || this==node)
    return;
  else{
    node->clear();
    node->m_metaDataAnzlyzed = m_metaDataAnzlyzed;
    node->m_expstrAnalyzed = m_expstrAnalyzed;
    node->m_type = m_type;
    node->m_operate = m_operate;
    node->m_colId = m_colId;
    node->m_datatype = m_datatype;
    node->m_expType = m_expType;
    node->m_funcID = m_funcID;
    node->m_expStr = m_expStr;
    node->m_fieldnames = m_fieldnames;
    node->m_fieldtypes = m_fieldtypes;
    if(m_Function){
      if (node->m_Function){
        node->m_Function->clear();
        SafeDelete(node->m_Function);
      }
      node->m_Function = new FunctionC();
      m_Function->copyTo(node->m_Function);
    }else
      node->m_Function = NULL;
    if (m_type == BRANCH){
      if (m_leftNode){
        if (node->m_leftNode){
          node->m_leftNode->clear();
          SafeDelete(node->m_leftNode);
        }
        node->m_leftNode = new ExpressionC();
        m_leftNode->copyTo(node->m_leftNode);
        node->m_leftNode->m_parentNode = node;
      }else
        node->m_leftNode = NULL;
      
      if (m_rightNode){
        if (node->m_rightNode){
          node->m_rightNode->clear();
          SafeDelete(node->m_rightNode);
        }
        node->m_rightNode = new ExpressionC();
        m_rightNode->copyTo(node->m_rightNode);
        node->m_rightNode->m_parentNode = node;
      }else
        node->m_rightNode = NULL;
    }
  }
}

// clear expression
void ExpressionC::clear(){
  if (m_leftNode){
    m_leftNode->clear();
    SafeDelete(m_leftNode);
  }
  if (m_rightNode){
    m_rightNode->clear();
    SafeDelete(m_rightNode);
  }
  if (m_Function){
    m_Function->clear();
    SafeDelete(m_Function);
  }
  m_operators.clear();
  
  init();
}

ExpressionC::ExpressionC(ExpressionC* m_leftNode, ExpressionC* m_rightNode)
{
  clear();
  m_type = BRANCH;
  m_leftNode = m_leftNode;
  m_rightNode = m_rightNode;
  //m_leftNode = m_leftNode==NULL?NULL:new Prediction(m_leftNode);
  //m_rightNode = m_rightNode==NULL?NULL:new Prediction(m_rightNode);;
}

ExpressionC::ExpressionC(int operate, int colId, string data)
{
  clear();
  m_type = LEAF;
  m_operate = operate;
  m_colId = colId;
  m_expStr = data;
}

void ExpressionC::setExpstr(string expString)
{
  m_expStr = expString;
  buildExpression();
  if (isMacroInExpression()){
    trace(FATAL, "A macro function cannot be a part of an expression '%s'\n", m_expStr.c_str());
    //return;
  }
  string sResult;
  if (mergeConstNodes(sResult)){
    m_expStr = sResult;
    m_expType = CONST;
  }
}

bool ExpressionC::expstrAnalyzed()
{
  return m_expstrAnalyzed;
}

ExpressionC* ExpressionC::getTopParent()
{
  if (m_parentNode)
    return m_parentNode->getTopParent();
  return this;
}

/*********** Build Btree from expression string logic ******************
Build(expStr, parentNode)
return NULL if expStr is empty
new New_Node
reading atom subExpStr or quoted subExpStr util next operator or END
if reached END
  if subExpStr is quoted
    remove quoters
    New_Node = Build(unquoted_exp,NULL)
  else
    New_Node.type = LEAF
    detect dataType and expType
if read operator
  New_Node.type = BRANCH
  if op_priority>parent_priority
    if parentNode
      parentNode->rightNode = New_Node
    New_Node->leftNode=Build(subExpStr, NULL)
    New_Node->rightNode=Build(restExpStr, NULL)
  else
    parentNode->rightNode = Build(subExpStr, NULL)
    New_Node->leftNode = parentNode
    New_Node->rightNode = Build(restExpStr, NULL)
return the top ParentNode of New_Node

Example.
a+b+c*d/(e-f)^g

Building "a+b+c*d/(e-f)^g", parentNode=NULL, As following op(+) priority>parent_priority(NULL/0), dont grow tree, create a rightNode(B1) for parentNode NULL, build(a, NULL) as leftNode of B1, then build the rest expStr Build(b+c*d/(e-f)^g, B1) as rightNode of B1
  B1(+)
a    b+c*d/(e-f)^g

Building "b+c*d/(e-f)^g", parentNode=B1
As following op(+) priority<=parent_priority, grow tree, build(b, NULL) as rightNode of B1, B1 as leftNode of new parent node B2, then build the rest expStr Build(c*d/(e-f)^g, B2) as rightNode of B2
    B2(+)
 B1(+)  c*d/(e-f)^g
a  b

Building "c*d/(e-f)^g", parentNode=B2
As following op(*) priority>parent_priority, dont grow tree, create a rightNode(B3) for parentNode B2, Build(c, NULL) as leftNode of B3, then build the rest expStr Build(d/(e-f)^g, B3) as rightNode of B3
    B2(+)
 B1(+)  B3(*)
a  b   c  d/(e-f)^g

Building "d/(e-f)^g", parentNode=B3
As following op(/) priority<=parent_priority, grow tree, Build(d, NULL) as rightNode of B3, B3 as leftNode of new parent node B4, then build the rest expStr Build((e-f)^g, B4) as rightNode of B4
    B2(+)
 B1(+)   B4(/)
a  b   B3(*) (e-f)^g
      c  d

Building "(e-f)^g", parentNode=B4
As following op(^) priority>parent_priority, dont grow tree, create a rightNode(B5) for parentNode B2, Build(e-f, NULL) as leftNode of B5, then build the rest expStr Build(g, B5) as rightNode of B3
    B2(+)
 B1(+)   B4(/)
a  b   B3(*)   B5(^)
      c  d  (e-f)   g

Building "e-f", parentNode=NULL, recrusive call this procedure, return the top Parent B6
  B6(-)
  e  f

Building "g", parentNode=B5
As no following op, return the top Branch (B2)
    B2(+)
 B1(+)   B4(/)
a  b   B3(*)   B5(^)
      c  d   B6(-)   g
            e  f

**********************************************************/
ExpressionC* ExpressionC::BuildTree(string expStr, ExpressionC* parentNode)
{
  trace(DEBUG, "Building BTREE from '%s'\n", expStr.c_str());
  if (m_expStr.empty()){
    trace(ERROR, "Error: No statement found!\n");
    return NULL;
  }else
    m_expStr = trim_copy(m_expStr);
  int nextPos=0,strStart=0;
  try{
    ExpressionC* newNode = new ExpressionC();
    newNode->m_expstrAnalyzed = true;
    if (expStr.length()>1 && expStr[0]=='/' && expStr[expStr.length()-1]=='/') { // regular expression string can NOT operate with any other expression!
      buildLeafNode(expStr, newNode);
      return newNode->getTopParent();
    }
    int iPos = findFirstCharacter(expStr, m_operators, 0, "''()", '\\',{'(',')'});
    // check if is a scientific notation number, e.g.1.58e+8
    if(iPos>1 && iPos<expStr.length()-1 && (expStr[iPos-1]=='e' || expStr[iPos-1]=='E') && (expStr[iPos]=='+' || expStr[iPos] == '-')){
      trace(DEBUG, "Checking scientific notation number for base '%s'\n", expStr.substr(0,iPos-1).c_str());
      //check left of +/1 is a double number
      if (isDouble(expStr.substr(0,iPos-1))){
        int iNextPos = findFirstCharacter(expStr, m_operators, iPos+1, "''()", '\\',{'(',')'});
        string sNotation="";
        if (iNextPos < 0)
          sNotation = expStr.substr(iPos+1);
        else
          sNotation = expStr.substr(iPos+1,iNextPos-iPos-1);
        trace(DEBUG, "Checking scientific notation number for notation '%s'\n", sNotation.c_str());
        if (isInt(sNotation)) // this is a scientific notation number
          iPos = iNextPos;
      }
    }
    if (iPos<0) { // didnt find any operator, reached the end
      if (expStr.length()>1 && expStr[0]=='(' && expStr[expStr.length()-1]==')') { // quoted expression
        newNode->clear();
        SafeDelete(newNode);
        newNode = BuildTree(expStr.substr(1,expStr.length()-2),NULL);
      }else{ // atom expression to be built as a leaf
        buildLeafNode(expStr, newNode);
      }
    }else{ // got an operator, building a branch
      //trace(DEBUG, "Found '%s'(%d) in '%s'\n",expStr.substr(iPos,1).c_str(),iPos,expStr.c_str());
      if (iPos == expStr.length() - 1){ // operator should NOT be the end
        newNode->clear();
        SafeDelete(newNode);
        trace(ERROR, "Operator should NOT be the end of the expression!\n");
        return NULL;
      }
      newNode->m_type = BRANCH;
      newNode->m_operate = encodeOperator(expStr.substr(iPos,1));
      newNode->m_expStr = expStr.substr(iPos,1);
      if (!parentNode || operatorPriority(newNode->m_operate)>operatorPriority(parentNode->m_operate)){
        if (parentNode){
          parentNode->m_rightNode = newNode;
        }
        newNode->m_leftNode = BuildTree(expStr.substr(0,iPos), NULL);
      }else{
        parentNode->m_rightNode = BuildTree(expStr.substr(0,iPos), NULL); 
        parentNode->m_datatype = getCompatibleDataType(parentNode->m_leftNode->m_datatype, parentNode->m_rightNode->m_datatype);
        newNode->m_leftNode = parentNode;
      }
      newNode->m_rightNode = BuildTree(expStr.substr(iPos+1), NULL);
      newNode->m_datatype = getCompatibleDataType(newNode->m_leftNode->m_datatype, newNode->m_rightNode->m_datatype);
      if (parentNode)
        parentNode->m_datatype = getCompatibleDataType(parentNode->m_leftNode->m_datatype, parentNode->m_rightNode->m_datatype);
    }
    return newNode->getTopParent();
  }catch (exception& e) {
    trace(ERROR, "Building expression exception: %s\n", e.what());
    return NULL;
  }
}

// Build a leaf node from an expression string
bool ExpressionC::buildLeafNode(string expStr, ExpressionC* node)
{
  if (!node)
    return false;
  trace(DEBUG, "Building Leaf Node from '%s'\n", expStr.c_str());
  if (m_expStr.empty()){
    trace(ERROR, "Error: No statement found!\n");
    return false;
  }else
    expStr = trim_copy(expStr);
  int nextPos=0,strStart=0;
  try{
    node->m_type = LEAF;
    node->m_operate = UNKNOWN;
    node->m_expStr = expStr;
    node->m_colId = -1;
    node->m_leftNode = NULL;
    node->m_rightNode = NULL;
    node->m_parentNode = NULL;
    node->m_fieldnames = NULL;
    node->m_fieldtypes = NULL;
    node->m_expstrAnalyzed = true;    
    // check if it is a variable
    if (expStr[0]=='@'){
      if (expStr.length()>1){
        node->m_datatype.datatype = UNKNOWN;
        node->m_expType = VARIABLE;
        node->m_expstrAnalyzed = true;
        node->m_expStr = upper_copy(expStr);
        if (node->m_expStr.compare("@RAW") == 0 || node->m_expStr.compare("@FILE") == 0)
          node->m_datatype.datatype = STRING;
        else if (node->m_expStr.compare("@LINE") == 0 || node->m_expStr.compare("@ROW") == 0 || node->m_expStr.compare("@ROWSORTED") == 0 || node->m_expStr.compare("@%") == 0)
          node->m_datatype.datatype = LONG;
        else if (node->m_expStr.find("@FIELD") == 0){
          string sColId = m_expStr.substr(string("@FIELD").length());
          node->m_colId = isInt(sColId)?atoi(sColId.c_str())-1:-1;
        }else if (isInt(node->m_expStr.substr(1))){ // @N is abbreviasion of @fieldN
          string sColId = node->m_expStr.substr(1);
          node->m_colId = isInt(sColId)?atoi(sColId.c_str())-1:-1;
          node->m_expStr = "@FIELD"+sColId;
        }else{
          node->m_expType = VARIABLE;
          node->m_datatype.datatype = UNKNOWN;
        }
        return true;
      }else{
        trace(ERROR, "Invalid variable name '%s'! \n", expStr.c_str());
        node->m_expstrAnalyzed = false;
        return false;
      }
    //}else if (expStr[0] == '/'){
    //  if (expStr.length()>1 && expStr[expStr.length()-1] == '/'){ // whole string is a regular expression string
    //    node->m_datatype.datatype = STRING;
    //    node->m_expType = CONST;
    //    node->m_expStr = trim_pair(expStr,"//");
    //    node->m_expstrAnalyzed = true;
    //    return true;
    //  }else{
    //    trace(ERROR, "Regular expression '%s' is not closed. \n", expStr.c_str());
    //    return false;
    //  }
    //}else if (expStr[0] == '{'){ // checking DATE string
    //  int iOffSet;
    //  if (expStr.length()>1 && expStr[expStr.length()-1] == '}'){ // whole string is a date string
    //    if (isDate(expStr.substr(1,expStr.length()-2), iOffSet, node->m_datatype.extrainfo))
    //      node->m_datatype.datatype = DATE;
    //    else{
    //      trace(ERROR, "Unrecognized date format of '%s'. \n", expStr.c_str());
    //      return false;
    //    }
    //    node->m_expType = CONST;
    //    node->m_expstrAnalyzed = true;
    //    node->m_expStr = trim_pair(expStr,"{}");
    //    return true;
    //  }else{
    //    trace(ERROR, "DATE string '%s' is not closed. \n", expStr.c_str());
    //    node->m_expstrAnalyzed = false;
    //    return false;
    //  }
    }else if (expStr[0] == '\''){ // checking STRING string
      if (expStr.length()>1 && expStr[expStr.length()-1] == '\''){ // whole string is a STRING string
        int iOffSet, iPos = 0;
        node->m_expStr = readQuotedStr(expStr, iPos, "''", '\\');
        trace(DEBUG, "Read STRING \"%s\" . \n", expStr.c_str());
        if (isDate(node->m_expStr, iOffSet, node->m_datatype.extrainfo))
          node->m_datatype.datatype = DATE;
        else
          node->m_datatype.datatype = STRING;
        node->m_expType = CONST;
        node->m_expstrAnalyzed = true;
        return true;
      }else{
        trace(ERROR, "STRING '%s' is not closed. \n", expStr.c_str());
        node->m_expstrAnalyzed = false;
        return false;
      }
    }else if (isLong(expStr)){
      node->m_datatype.datatype = LONG;
      node->m_expType = CONST;
      node->m_expstrAnalyzed = true;
      return true;
    }else if (isDouble(expStr)){
      node->m_datatype.datatype = DOUBLE;
      node->m_expType = CONST;
      node->m_expstrAnalyzed = true;
      return true;
    }else if (isInt(expStr)){
      node->m_datatype.datatype = INTEGER;
      node->m_expType = CONST;
      node->m_expstrAnalyzed = true;
      return true;
    }else if (expStr.find("(")>0 && expStr[expStr.length()-1]==')') { // checking FUNCTION
      int iQuoteStart = expStr.find("(");
      string sFuncName = upper_copy(trim_copy(expStr.substr(0,iQuoteStart)));
      string sParams = trim_copy(expStr.substr(iQuoteStart));
      node->m_expType = FUNCTION;
      node->m_expStr = sFuncName+sParams;
      //trace(DEBUG2, "Found function '%s' => ! \n", expStr.c_str(), node->m_expStr.c_str());
      if (!node->m_Function){
        node->m_Function = new FunctionC(node->m_expStr);
        trace(DEBUG, "(1)New function from '%s'\n",node->m_expStr.c_str());
      }
      trace(DEBUG, "(1)ExpressionC: The analytic function '%s' param size %d \n", node->m_Function->m_expStr.c_str(), node->m_Function->m_params.size());
      node->m_datatype = node->m_Function->m_datatype;
      node->m_funcID = node->m_Function->m_funcID;
      node->m_expstrAnalyzed = true;
      if (node->m_datatype.datatype != UNKNOWN)
        return true;
      else {
        trace(ERROR, "Function '%s' is not recognized! \n", expStr.c_str());
        node->m_expstrAnalyzed = false;
        return false;
      }
    }else{ // it could be a COLUMN or invalid expression string
      node->m_expstrAnalyzed = true;
    }
  }catch (exception& e) {
    trace(ERROR, "Building leaf node exception: %s\n", e.what());
    return false;
  }
  return false;
}

// build expression class from the expression string
bool ExpressionC::buildExpression()
{
  ExpressionC* root = BuildTree(m_expStr, NULL);
  if (root){
    root->copyTo(this);
    root->clear();
    SafeDelete(root);
    return true;
  }
  return false;
}

bool ExpressionC::isMacroInExpression()
{
  if (m_expType == FUNCTION && m_Function && m_Function->isMacro())
    if (m_parentNode || m_leftNode || m_rightNode){
      return true;
    }else
      return false;
  else{
    bool bLeftMacro = false, bRightMacro = false;
    if (m_leftNode)
      bLeftMacro = m_leftNode->isMacroInExpression();
    if (m_rightNode)
      bRightMacro = m_rightNode->isMacroInExpression();
    return bLeftMacro || bRightMacro;
  }
}

// get left tree Height
int ExpressionC::getLeftHeight(){
  int height = 1;
  if (m_type == BRANCH && m_leftNode)
    height += m_leftNode->getLeftHeight();

  return height;
}

// get left tree Height
int ExpressionC::getRightHeight(){
  int height = 1;
  if (m_type == BRANCH && m_rightNode)
    height += m_rightNode->getRightHeight();
  
  return height;
}

// add a NEW expression into tree
void ExpressionC::add(ExpressionC* node, int op, bool leafGrowth, bool addOnTop){
  // not add any null or UNKNOWN node
  if (node || node->m_type ==  UNKNOWN) 
      return;
  if (m_type ==  UNKNOWN){ // not assinged
      node->copyTo(this);
  }else if (m_type == LEAF){
    ExpressionC* existingNode = new ExpressionC();
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
      ExpressionC* existingNode = new ExpressionC();
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

void ExpressionC::dump(int deep)
{
  if (m_type == BRANCH){
    trace(DUMP,"%s(%d)\n",decodeOperator(m_operate).c_str(),deep);
    trace(DUMP,"L-");
    m_leftNode->dump(deep+1);
    trace(DUMP,"R-");
    m_rightNode->dump(deep+1);
  }else{
    trace(DUMP,"(%d)%s(%d)\n",deep,m_expStr.c_str(),m_colId);
  }
}

void ExpressionC::dump()
{
  dump(0);
}

string ExpressionC::getEntireExpstr()
{
  string expStr = m_expStr;
  if (m_type==LEAF&&m_expType==CONST&&(m_datatype.datatype==STRING||m_datatype.datatype==DATE||m_datatype.datatype==TIMESTAMP)){
    replacestr(expStr,"'","\\'");
    expStr = "'"+expStr+"'";
  }
  return (m_leftNode?m_leftNode->getEntireExpstr():"")+expStr+(m_rightNode?m_rightNode->getEntireExpstr():"");
}

// detect if predication contains special colId    
bool ExpressionC::containsColId(int colId){
  bool contain = false;
  if (m_type == BRANCH){
    contain = contain || m_leftNode->containsColId(colId);
    contain = contain || m_rightNode->containsColId(colId);
  }else
    contain = (m_colId == colId);

  return contain;
}

// detect if predication contains special colId    
ExpressionC* ExpressionC::getFirstPredByColId(int colId, bool leftFirst){
  ExpressionC* node;
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
DataTypeStruct ExpressionC::analyzeColumns(vector<string>* fieldnames, vector<DataTypeStruct>* fieldtypes, DataTypeStruct* rawDatatype)
{
  DataTypeStruct dts;
  dts.extrainfo="";
  try{
    trace(DEBUG, "Analyzing columns in expression '%s'\n", m_expStr.c_str());
    if (!fieldnames || !fieldtypes){
      trace(ERROR, "(Expression)fieldnames or fieldtypes is NULL!\n");
      dts.datatype = UNKNOWN;
      return dts;
    }
    m_metaDataAnzlyzed = true;
    m_fieldnames = fieldnames;
    m_fieldtypes = fieldtypes;
    if (m_type == BRANCH){
      dts.datatype=UNKNOWN;
      DataTypeStruct rdatatype = m_rightNode?m_rightNode->analyzeColumns(fieldnames, fieldtypes, rawDatatype):dts;
      DataTypeStruct ldatatype = m_leftNode?m_leftNode->analyzeColumns(fieldnames, fieldtypes, rawDatatype):dts;
      //trace(DEBUG, "Left node: %s (%d); Right node: %s (%d)\n", m_leftNode->m_expStr.c_str(),m_leftNode->m_type, m_rightNode->m_expStr.c_str(),m_rightNode->m_type);
      //trace(DEBUG, "Getting compatible type from %s and %s\n", decodeDatatype(ldatatype.datatype).c_str(), decodeDatatype(rdatatype.datatype).c_str());
      m_datatype = getCompatibleDataType(ldatatype, rdatatype);
      //trace(DEBUG, "Getting compatible type %s\n", decodeDatatype(m_datatype.datatype).c_str());
      m_metaDataAnzlyzed = m_datatype.datatype!=UNKNOWN;
    }else{
      if (fieldnames->size() != fieldtypes->size()){
        trace(ERROR,"Field name number %d does not match field type number %d.\n", fieldnames->size(), fieldtypes->size());
        m_metaDataAnzlyzed = false;
        dts.datatype = UNKNOWN;
        return dts;
      }
      //m_expStr = trim_copy(m_expStr);
      // check if it is a variable
      //if (m_expStr.length()>0 && m_expStr[0]=='@'){
      if (m_expType == VARIABLE){
        m_expStr = upper_copy(trim_copy(m_expStr));
        //string strVarName = upper_copy(m_expStr);
        if (m_expStr.compare("@RAW") == 0)
          m_datatype = *rawDatatype;
        else if(m_expStr.compare("@FILE") == 0)
          m_datatype.datatype = STRING;
        else if (m_expStr.compare("@LINE") == 0 || m_expStr.compare("@ROW") == 0 || m_expStr.compare("@ROWSORTED") == 0 || m_expStr.compare("@%") == 0)
          m_datatype.datatype = LONG;
        else if (m_expStr.find("@FIELD") == 0){
          string sColId = m_expStr.substr(string("@FIELD").length());
          int iColID = isInt(sColId)?atoi(sColId.c_str())-1:-1;
          if (!sColId.empty() && iColID < fieldtypes->size()){
            m_expType = COLUMN;
            //m_datatype = (*fieldtypes)[atoi(sColId.c_str())];
            m_colId = iColID;
            m_datatype = (*fieldtypes)[m_colId];
            trace(DEBUG, "Tuning '%s' from VARIABLE to COLUMN(%d).\n", m_expStr.c_str(), m_colId);
          }else{
            trace(ERROR, "Unrecognized variable(1) %s, Extracted COL ID: %d, number of fields: %d.\n", sColId.c_str(), iColID, fieldtypes->size());
            m_expType = UNKNOWN;
            m_datatype.datatype = UNKNOWN;
          }
        }else{
          //trace(ERROR, "Unrecognized variable(2) %s .\n", m_expStr.c_str());
          //m_expType = UNKNOWN;
          m_datatype.datatype = UNKNOWN;
        }
        //trace(DEBUG, "Expression '%s' type is %s, data type is '%s'\n", m_expStr.c_str(), decodeExptype(m_expType).c_str(),decodeDatatype(m_datatype.datatype).c_str());
        return m_datatype;
      }
      if (m_datatype.datatype == UNKNOWN){
        // check if it is a function FUNCNAME(...)
        int lefParPos = m_expStr.find("(");
        if (m_expStr.length()>2 && m_expStr[0] != '\'' && lefParPos>0 && m_expStr[m_expStr.length()-1] == ')')
          m_expType = FUNCTION;
        if (m_expType == CONST){
          // check if it is a time, quoted by {}
          int iOffSet;
          if (m_expStr.length()>1 && m_expStr[0]=='{' && m_expStr[m_expStr.length()-1]=='}'){
            if (isDate(m_expStr.substr(1,m_expStr.length()-2),iOffSet,m_datatype.extrainfo)){
              m_datatype.datatype = DATE;
            }else{
              trace(ERROR,"Failed to get the date format from '%s'.\n", m_expStr.c_str());
              m_metaDataAnzlyzed = false;
              dts.datatype = UNKNOWN;
              return dts;
            }
            trace(DEBUG, "Expression '%s' type is CONST, data type is DATE\n", m_expStr.c_str());
            return m_datatype;
          }
          // check if it is a string, quoted by ''
          if (m_expStr.length()>1 && m_expStr[0]=='\'' && m_expStr[m_expStr.length()-1]=='\''){
            m_datatype.datatype = STRING;
            trace(DEBUG, "Expression '%s' type is CONST, data type is STRING\n", m_expStr.c_str());
            return m_datatype;
          }
          // check if it is a regular expression string, quoted by //
          if (m_expStr.length()>1 && m_expStr[0]=='/' && m_expStr[m_expStr.length()-1]=='/'){
            m_datatype.datatype = STRING;
            trace(DEBUG, "Expression '%s' type is CONST, data type is STRING\n", m_expStr.c_str());
            return m_datatype;
          }
        }
        // check if it is a column
        for (int i=0; i<fieldnames->size(); i++){
          if (upper_copy(m_expStr).compare(upper_copy((*fieldnames)[i])) == 0){
            m_expStr = trim_copy(upper_copy(m_expStr));
            m_expType = COLUMN;
            m_colId = i;
            m_datatype = (*fieldtypes)[i];
            trace(DEBUG, "Expression '%s' type is COLUMN, data type is %s\n", m_expStr.c_str(), decodeDatatype(m_datatype.datatype).c_str());
            return m_datatype;
          }
        }
        if (isInt(m_expStr)){
          m_expType = CONST;
          m_datatype.datatype = INTEGER;
        }else if (isLong(m_expStr)){
          m_expType = CONST;
          m_datatype.datatype = LONG;
        }else if (isDouble(m_expStr)){
          m_expType = CONST;
          m_datatype.datatype = DOUBLE;
        }else{
          m_expType = UNKNOWN;
          m_datatype.datatype = UNKNOWN;
        }
      }
      if (m_expType == FUNCTION){
        if (!m_Function){
          m_Function = new FunctionC(m_expStr);
          trace(DEBUG, "(2)New function from '%s'\n",m_expStr.c_str());
        }
        m_datatype = m_Function->analyzeColumns(fieldnames, fieldtypes, rawDatatype);
        // m_datatype = m_Function->m_datatype;
        m_funcID = m_Function->m_funcID;
        //m_expStr = upper_copy(trim_copy(m_Function->m_expStr)); -- should NOT turn the parameters to UPPER case.
        m_expStr = m_Function->m_expStr;
        trace(DEBUG, "Expression '%s' type is FUNCTION, data type is %s\n", m_expStr.c_str(), decodeDatatype(m_datatype.datatype).c_str());
        return m_datatype;
      }
      trace(DEBUG, "Expression '%s' type is %s, data type is %s\n", m_expStr.c_str(), decodeExptype(m_expType).c_str(), decodeDatatype(m_datatype.datatype).c_str());
    }
    dts = m_datatype;
    return dts;
  }catch (exception& e) {
    trace(ERROR, "Unhandled exception: %s\n", e.what());
    dts.datatype = UNKNOWN;
    return dts;
  }
}

bool ExpressionC::columnsAnalyzed(){
    return m_metaDataAnzlyzed;
}

ExpressionC* ExpressionC::cloneMe(){
  ExpressionC* node = new ExpressionC();
  node->m_metaDataAnzlyzed = m_metaDataAnzlyzed;
  node->m_expstrAnalyzed = m_expstrAnalyzed;
  //node->predStr = predStr;
  node->m_type = m_type;
  node->m_operate = m_operate;
  node->m_colId = m_colId;
  node->m_datatype = m_datatype;
  node->m_expType = m_expType;
  node->m_funcID = m_funcID;
  node->m_expStr = m_expStr;
  node->m_fieldnames = m_fieldnames;
  node->m_fieldtypes = m_fieldtypes;
  if (m_Function)
    node->m_Function = m_Function->cloneMe();
  else
    node->m_Function = NULL;
  if (m_type == BRANCH){
    node->m_leftNode = new ExpressionC();
    node->m_leftNode = m_leftNode->cloneMe();
    node->m_rightNode = new ExpressionC();
    node->m_rightNode = m_rightNode->cloneMe();
    node->m_leftNode->m_parentNode = node;
    node->m_rightNode->m_parentNode = node;
  }else{
    node->m_leftNode = NULL;
    node->m_rightNode = NULL;
  }
  return node;
}

// get all involved colIDs in this expression
std::set<int> ExpressionC::getAllColIDs(int side){
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
map<int,string> ExpressionC::buildMap(){
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
int ExpressionC::size(){
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

// remove a node from prediction. Note: the input node is the address of the node contains in current prediction
//   0                      0                  2                 1
//  1  2  (remove 3) =>   4   2 (remove 1) =>      (remove 2)  3   4
//3  4
bool ExpressionC::remove(ExpressionC* node){
  bool removed = false;
  if (m_leftNode){
    if (m_leftNode == node){
      m_leftNode->clear();
      SafeDelete(m_leftNode);
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
      SafeDelete(m_rightNode);
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
void ExpressionC::fillDataForColumns(map <string, string> & dataList, vector <string> columns){
  if (columns.size() == 0)
    return;
  if (m_type == BRANCH){
    if (m_leftNode)
      m_leftNode->fillDataForColumns(dataList, columns);
    if (m_rightNode)
      m_rightNode->fillDataForColumns(dataList, columns);
  }else if (m_type == LEAF && m_colId >= 0){
    dataList.insert( pair<string,string>(columns[m_colId],m_expStr) );
  }
}

// align children datatype with current datatype
void ExpressionC::alignChildrenDataType()
{
  if (m_datatype.datatype != UNKNOWN){
    if (m_leftNode){
      m_leftNode->m_datatype = m_datatype;
      m_leftNode->alignChildrenDataType();
    }
    if (m_rightNode){
      m_rightNode->m_datatype = m_datatype;
      m_rightNode->alignChildrenDataType();
    }
  }
}

FunctionC* ExpressionC::getAnaFunc(string funcExpStr)
{
  if (m_type == LEAF && m_expType == FUNCTION && m_Function->isAnalytic() && m_Function && m_Function->m_expStr.compare(funcExpStr)==0)
    return m_Function;
  else if (m_type == BRANCH){
    FunctionC* leftF;
    if (m_leftNode){
      leftF = m_leftNode->getAnaFunc(funcExpStr);
      if (leftF)
        return leftF;
    }
    if (m_rightNode)
      return m_rightNode->getAnaFunc(funcExpStr);
  }
  return NULL;
}

bool ExpressionC::evalAnalyticFunc(unordered_map< string,string > * anaResult, string & sResult)
{
  if (m_type == LEAF){
    if (m_expType == CONST){
      sResult = m_expStr;
      return true;
    }else if (m_expType == FUNCTION && m_Function && m_Function->isAnalytic()){
      unordered_map< string,string >::iterator it = anaResult->find(m_Function->m_expStr);
      if (it == anaResult->end()){
        trace(ERROR, "Failed to find analytic function '%s' result!\n", m_Function->m_expStr.c_str());
        return false;
      }
      sResult = it->second;
      return true;
    }else{
      trace(ERROR, "This expression '%s' is not pre evaled!\n", getTopParent()->getEntireExpstr().c_str());
      return false;
    }
  }else{
    string leftRst = "", rightRst = "";
    if (!m_leftNode || !m_leftNode->evalAnalyticFunc(anaResult, leftRst)){
      trace(ERROR, "(2)Missing leftNode '%s'\n",m_expStr.c_str());
      return false;
    }
    if (!m_rightNode || !m_rightNode->evalAnalyticFunc(anaResult, rightRst)){
      trace(ERROR, "(2)Missing rightNode '%s'\n",m_expStr.c_str());
      return false;
    }
    //trace(DEBUG2,"calculating(2) (%s) '%s'%s'%s'\n", decodeExptype(m_datatype.datatype).c_str(),leftRst.c_str(),decodeOperator(m_operate).c_str(),rightRst.c_str());
    if ( anyDataOperate(leftRst, m_operate, rightRst, m_datatype, sResult)){
      //trace(DEBUG2,"calculating(2) (%s) '%s'%s'%s', get '%s'\n", decodeExptype(m_datatype.datatype).c_str(),leftRst.c_str(),decodeOperator(m_operate).c_str(),rightRst.c_str(),sResult.c_str());
      return true;
    }else
      return false;
  }
  return false;
}

// calculate this expression. fieldnames: column names; fieldvalues: column values; varvalues: variable values; sResult: return result. column names are upper case; skipRow: wheather skip @row or not. extrainfo so far for date format only
bool ExpressionC::evalExpression(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, string & sResult, DataTypeStruct & dts, bool getresultonly)
{
  if (!fieldvalues || !varvalues || !aggFuncs || !anaFuncs){
    trace(ERROR, "Insufficient metadata!\n");
    return false;
  }
  // if (!m_metaDataAnzlyzed || !m_expstrAnalyzed){
  if (!m_metaDataAnzlyzed && (m_type != LEAF || m_expType != FUNCTION || !m_Function || !m_Function->isAggFunc())){
    trace(ERROR, "Expression '%s' is not analyzed! metaData: %d, expstr: %d \n",m_expStr.c_str(),m_metaDataAnzlyzed,m_expstrAnalyzed);
    return false;
  }
  if (m_type == LEAF){
    if (m_expType == CONST){
      sResult = m_expStr;
      return true;
    }else if (m_expType == FUNCTION){
      //trace(DEBUG2, "Expression '%s' is a fucntion.. \n",m_expStr.c_str());
      if (m_Function){
        if (m_Function->isAggFunc()){
          // looks like this part is duplicated with querierc.evalAggExpNode(), but dont change aggFuncs values, as the same aggregation function may present multiple times in the selections/sorts clauses.
          unordered_map< string,GroupProp >::iterator it = aggFuncs->find(m_Function->m_expStr);
          if (it != aggFuncs->end()){
            if (m_Function->m_params.size()>0){
              // we dont bother with the paramter, as it has already been evaled in querierc.evalAggExpNode()
              // calculate the parameter here will cause duplicated calculation if the same aggregation function involved multiple times in the selection/sort
              //m_Function->m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
              switch (m_Function->m_funcID){
                case AVERAGE:
                  sResult = doubleToStr(it->second.sum/(double)it->second.count);
                  break;
                case SUM:
                  sResult = doubleToStr(it->second.sum);
                  break;
                case COUNT:
                  sResult = longToStr(it->second.count);
                  break;
                case UNIQUECOUNT:{
                  sResult = longToStr(it->second.uniquec.size());
                  //std::set <string> uniquec(it->second.varray.begin(), it->second.varray.end());
                  //sResult = longToStr(uniquec.size());
                  break;
                }case MAX:
                  sResult = it->second.max;
                  break;
                case MIN:
                  sResult = it->second.min;
                  break;
                default:{
                  trace(ERROR, "Invalid aggregation function '%s'\n",m_Function->m_expStr.c_str());
                  return false;
                }
              }
              dts = m_Function->m_datatype;
              return true;
            }else
              trace(ERROR, "Missing paramters for aggregation function '%s'\n",m_Function->m_expStr.c_str());
          }else{
            trace(ERROR, "Failed to find aggregation function '%s' dataset when evaling '%s'!\n", m_Function->m_expStr.c_str(), getTopParent()->getEntireExpstr().c_str());
            return false;
          }
        }else if (m_Function->isAnalytic()){
          unordered_map< string,vector<string> >::iterator it = anaFuncs->find(m_Function->m_expStr);
          if (it != anaFuncs->end()){
            unordered_map< string,vector<string> > dummyAnaFuncs;
            string tmpRslt;
            if (it->second.size()==0)// Empty vector means it's still doing raw data matching, only need to eval parameter expressions. Only retrieve once for each analytic function (identified by its expression str)
              for (int i=0;i<m_Function->m_params.size();i++){
                m_Function->m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, &dummyAnaFuncs, tmpRslt, dts, true);
                it->second.push_back(tmpRslt);
              }
            else // otherwise, the passed-in vector is analytic function result, need to return it to do other operation, e.g. filter
              sResult = it->second[0];
            dts = m_Function->m_datatype;
            return true;
          }else{
            trace(ERROR, "Failed to find analytic function '%s' dataset when evaling '%s'!\n", m_Function->m_expStr.c_str(), getTopParent()->getEntireExpstr().c_str());
            return false;
          }
        }else{
          bool gotResult = m_Function->runFunction(fieldvalues, varvalues, aggFuncs, anaFuncs, sResult, dts);
          m_datatype = dts;
          if (!getresultonly && gotResult){
            m_expType = CONST;
            m_expStr = sResult;
          }
          return gotResult;
        }
      }
    }else if (m_expType == COLUMN){
      if (m_colId >= 0 && m_colId<fieldvalues->size()){
        sResult = (*fieldvalues)[m_colId];
        dts = (*m_fieldtypes)[m_colId];
        m_datatype = dts;
        if (!getresultonly){
          m_expType = CONST;
          m_expStr = sResult;
        }
        return true;
      }else{
        int i=0;
        for (i=0; i<m_fieldnames->size(); i++)
          if ((*m_fieldnames)[i].compare(m_expStr) == 0)
            break;
        if (i<m_fieldnames->size()){
          sResult = (*fieldvalues)[i];
          dts = (*m_fieldtypes)[i];
          m_datatype = dts;
          if (!getresultonly){
            m_expType = CONST;
            m_expStr = sResult;
          }
          return true;
        }else{
          trace(ERROR, "Cannot find COLUMN '%s'\n",m_expStr.c_str());
          return false;
        }
      }
    }else if (m_expType == VARIABLE){
      //if (skipRow && (m_expStr.compare("@ROW") == 0 || m_expStr.compare("@ROWSORTED") == 0)){
      //  //trace(DEBUG, "Skip @row & @rowsorted ... \n");
      //  return true;
      //}
      if (varvalues->find(m_expStr) != varvalues->end()){
        //trace(DEBUG, "Assigning '%s' to '%s' ... \n", (*varvalues)[m_expStr].c_str(), m_expStr.c_str());
        //dumpMap(*varvalues);
        sResult = (*varvalues)[m_expStr];
        dts = m_datatype;
        if (!getresultonly){
          m_expType = CONST;
          m_expStr = sResult;
        }
        return true;
      }else{
        trace(ERROR, "Cannot find VARIABLE '%s'\n",m_expStr.c_str());
        return false;
      }
    }else{
      trace(ERROR, "Unknown expression type of '%s'\n",m_expStr.c_str());
      return false;
    }
  }else{
    string leftRst = "", rightRst = "";
    DataTypeStruct leftDts, rightDts;
    if (!m_leftNode || !m_leftNode->evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, leftRst, leftDts, getresultonly)){
      trace(ERROR, "Missing leftNode '%s'\n",m_expStr.c_str());
      return false;
    }
    if (!m_rightNode || !m_rightNode->evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, rightRst, rightDts, getresultonly)){
      trace(ERROR, "Missing rightNode '%s'\n",m_expStr.c_str());
      return false;
    }
    //trace(DEBUG2,"calculating(1) (%s) '%s'%s'%s'\n", decodeExptype(m_datatype.datatype).c_str(),leftRst.c_str(),decodeOperator(m_operate).c_str(),rightRst.c_str());
    //if ((m_leftNode->m_type==LEAF && m_leftNode->m_expType==FUNCTION && m_leftNode->m_Function && m_leftNode->m_Function->isAnalytic()) || (m_rightNode->m_type==LEAF && m_rightNode->m_expType==FUNCTION && m_rightNode->m_Function && m_rightNode->m_Function->isAnalytic())) { // if left or right expression is analytic function, we dont do the operation
    if (m_leftNode->containAnaFunc() || m_rightNode->containAnaFunc()) { // if left or right expression is analytic function, we dont do the operation
      trace(DEBUG,"Skip to eval analytic function. Left: '%s'; Right: '%s'\n", m_leftNode->getEntireExpstr().c_str(),m_rightNode->getEntireExpstr().c_str());
      return true;
    }else if ( anyDataOperate(leftRst, m_operate, rightRst, m_datatype, sResult)){
      trace(DEBUG2,"calculating(1) (%s) '%s'%s'%s', get '%s'\n", decodeExptype(m_datatype.datatype).c_str(),leftRst.c_str(),decodeOperator(m_operate).c_str(),rightRst.c_str(),sResult.c_str());
      dts = m_datatype;
      if (!getresultonly){
        //clear();
        m_type = LEAF;
        m_expType = CONST;
        m_expStr = sResult;
        m_datatype = dts;
      }
      return true;
    }else
      return false;
  }
  return false;
}

// merge const expression, reduce calculation during matching
bool ExpressionC::mergeConstNodes(string & sResult)
{
  trace(DEBUG,"Merging consts in expression '%s' (%s)\n", m_expStr.c_str(), decodeExptype(m_expType).c_str());
  if (m_type == LEAF){
    if (m_expType == CONST){
      sResult = m_expStr;
      trace(DEBUG,"Return CONST '%s' (%s)\n", m_expStr.c_str(), decodeDatatype(m_datatype.datatype).c_str());
      return true;
    }else if (m_expType == FUNCTION){
      //if (containGroupFunc()) // skip aggregation functions
      //  return false;
      if (m_Function){
        bool gotResult = false;
        if (m_Function->isConst()){
          vector<string> vfieldnames;
          vector<DataTypeStruct> fieldtypes;
          DataTypeStruct rawDatatype;
          vector<string> vfieldvalues;
          map<string,string> mvarvalues;
          unordered_map< string,GroupProp > aggFuncs;
          unordered_map< string,vector<string> > anaFuncs;
          m_Function->analyzeColumns(&vfieldnames, &fieldtypes, &rawDatatype);
          DataTypeStruct dts;
          gotResult = m_Function->runFunction(&vfieldvalues,&mvarvalues,&aggFuncs,&anaFuncs,sResult,dts);
          if (gotResult){
            m_expStr = sResult;
            m_expType = CONST;
            m_datatype = m_Function->m_datatype;
            trace(DEBUG,"Return function '%s' result '%s'\n", m_Function->m_expStr.c_str(), sResult.c_str());
          }
        }else
          gotResult = false;
        return gotResult;
      }
    }else{
      trace(DEBUG,"'%s' is not a CONST or FUNCTION.\n", m_expStr.c_str());
      return false;
    }
  }else{
    string leftRst = "", rightRst = "";
    if (!m_leftNode || !m_leftNode->mergeConstNodes(leftRst))
      return false;
    if (!m_rightNode || !m_rightNode->mergeConstNodes(rightRst))
      return false;
    trace(DEBUG,"calculating(2) (%s) '%s'%s'%s'\n", decodeExptype(m_datatype.datatype).c_str(),leftRst.c_str(),decodeOperator(m_operate).c_str(),rightRst.c_str());
    bool gotResult = anyDataOperate(leftRst, m_operate, rightRst, m_datatype, sResult);
    if (gotResult){
      m_leftNode->clear();
      SafeDelete(m_leftNode);
      m_rightNode->clear();
      SafeDelete(m_rightNode);
      m_expType = CONST;
      m_type = LEAF;
      m_expStr = sResult;
    }
    return gotResult;
  }
  return false;
}

void ExpressionC::getAllColumnNames(vector<string> & fieldnames)
{
  if (m_type == LEAF && (m_expType == COLUMN || m_expType == VARIABLE || m_expType == UNKNOWN))
    fieldnames.push_back(trim_copy(upper_copy(m_expStr)));
  if (m_leftNode)
    m_leftNode->getAllColumnNames(fieldnames);
  if (m_rightNode)
    m_rightNode->getAllColumnNames(fieldnames);
}

bool ExpressionC::getAggFuncs(unordered_map< string,GroupProp > & aggFuncs)
{
  //trace(DEBUG2,"Checking '%s'(%d %d %d)\n",getEntireExpstr().c_str(),m_type,m_expType,m_Function?m_Function->isAggFunc():-1);
  if (m_type == LEAF && m_expType == FUNCTION && m_Function){
    if (m_Function->isAggFunc()){
      GroupProp gp;
      if (aggFuncs.find(m_Function->m_expStr) == aggFuncs.end()){
        gp = GroupProp();
        //trace(DEBUG2,"Adding aggregation function '%s' properties \n",m_Function->m_expStr.c_str());
        aggFuncs.insert(pair<string,GroupProp>(m_Function->m_expStr,gp));
      }
      return true; // the parameter expressions of an aggregation function should not include another aggregation function
    }else { // check the paramters of normal functions
      //trace(DEBUG2,"Parameter size %d\n",m_Function->m_params.size());
      bool bGotAggFunc = false;
      for (int i=0;i<m_Function->m_params.size();i++){
        //trace(DEBUG2,"Parameter '%s'\n",m_Function->m_params[i].getEntireExpstr().c_str());
        bGotAggFunc = m_Function->m_params[i].getAggFuncs(aggFuncs)||bGotAggFunc;
      }
      return bGotAggFunc;
    }
  }else{
    bool bGotAggFunc = (m_leftNode && m_leftNode->getAggFuncs(aggFuncs));
    bGotAggFunc = (m_rightNode && m_rightNode->getAggFuncs(aggFuncs));
    return bGotAggFunc;
  }
}

bool ExpressionC::getAnaFuncs(unordered_map< string,vector<ExpressionC> > & anaFuncs, unordered_map< string, vector<int> > & anaParaNums)
{
  //trace(DEBUG2,"Checking '%s'(%d %d %d)\n",getEntireExpstr().c_str(),m_type,m_expType,m_Function?m_Function->isAggFunc():-1);
  if (m_type == LEAF && m_expType == FUNCTION && m_Function){
    if (m_Function->isAnalytic()){
      vector<ExpressionC> vParams;
      if (anaFuncs.find(m_Function->m_expStr) == anaFuncs.end()){
        vParams.clear();
        for (int i=0; i<m_Function->m_params.size();i++)
          vParams.push_back(m_Function->m_params[i]);
        //trace(DEBUG2,"Adding aggregation function '%s' properties \n",m_Function->m_expStr.c_str());
        anaFuncs.insert(pair< string,vector<ExpressionC> >(m_Function->m_expStr,vParams));
      }
      anaParaNums.insert(pair< string,vector<int> >(m_Function->m_expStr,m_Function->m_anaParaNums));
      trace(DEBUG, "(2)ExpressionC: The analytic function '%s' group size is %d, param size %d \n", m_Function->m_expStr.c_str(), m_Function->m_anaParaNums[0], m_Function->m_params.size());
      return true; // the parameter expressions of an aggregation function should not include another aggregation function
    }else { // check the paramters of normal functions
      //trace(DEBUG2,"Parameter size %d\n",m_Function->m_params.size());
      bool bGotAnaFunc = false;
      for (int i=0;i<m_Function->m_params.size();i++){
        //trace(DEBUG2,"Parameter '%s'\n",m_Function->m_params[i].getEntireExpstr().c_str());
        bGotAnaFunc = m_Function->m_params[i].getAnaFuncs(anaFuncs, anaParaNums)||bGotAnaFunc;
      }
      return bGotAnaFunc;
    }
  }else{
    bool bGotAnaFunc = (m_leftNode && m_leftNode->getAnaFuncs(anaFuncs, anaParaNums));
    bGotAnaFunc = (m_rightNode && m_rightNode->getAnaFuncs(anaFuncs, anaParaNums));
    return bGotAnaFunc;
  }
}

bool ExpressionC::containGroupFunc() const
{
  if (m_type == LEAF){
    if (m_expType == FUNCTION && m_Function && m_Function->isAggFunc())
      return true;
  }else{
    if ((m_leftNode && m_leftNode->containGroupFunc()) || (m_rightNode && m_rightNode->containGroupFunc()))
      return true;
    return false;
  }
  return false;
}

bool ExpressionC::containAnaFunc() const
{
  if (m_type == LEAF){
    if (m_expType == FUNCTION && m_Function && m_Function->isAnalytic())
      return true;
  }else{
    if ((m_leftNode && m_leftNode->containAnaFunc()) || (m_rightNode && m_rightNode->containAnaFunc()))
      return true;
    return false;
  }
  return false;
}

bool ExpressionC::groupFuncOnly() const
{
  if (m_type == LEAF){
    if (m_expType == CONST)
      return true;
    if (m_expType == FUNCTION){
      return containGroupFunc();
    }else
      return false;
  }else{
    if (!m_leftNode || !m_leftNode->groupFuncOnly())
      return false;
    if (!m_rightNode || !m_rightNode->groupFuncOnly())
      return false;
    return true;
  }
  return false;
}

bool ExpressionC::existLeafNode(ExpressionC* node)
{
  //trace(DEBUG,"Checking %d => %d; '%s' => '%s'\n", m_expType, node->m_expType, getEntireExpstr().c_str(), node->m_expStr.c_str());
  if (node->m_type != LEAF){
    //trace(DEBUG,"111111 '%s' '%s'\n", m_expStr.c_str(), node->m_expStr.c_str());
    return false;
  }
  if (m_type == LEAF){
    if (m_expType == node->m_expType && upper_copy(m_expStr).compare(upper_copy(node->m_expStr)) == 0){
      //trace(DEBUG,"222222 '%s' '%s'\n", m_expStr.c_str(), node->m_expStr.c_str());
      return true;
    }else{
      //trace(DEBUG,"000000 '%s' '%s'\n", m_expStr.c_str(), node->m_expStr.c_str());
      return false;
    }
  }else{
    if (m_leftNode && m_leftNode->existLeafNode(node)){
      //trace(DEBUG,"3333333 '%s' '%s'\n", m_expStr.c_str(), node->m_expStr.c_str());
      return true;
    }
    if (m_rightNode && m_rightNode->existLeafNode(node)){
      //trace(DEBUG,"444444 '%s' '%s'\n", m_expStr.c_str(), node->m_expStr.c_str());
      return true;
    }
    //trace(DEBUG,"555555 '%s' '%s'\n", m_expStr.c_str(), node->m_expStr.c_str());
    return false;
  }
}

bool ExpressionC::inColNamesRange(vector<string> fieldnames)  const
{
  if (m_type == LEAF){
    if (m_expType == CONST){
      //trace(DEBUG,"666666 '%s' '%s'\n", m_expStr.c_str());
      return true;
    }
    else if (m_expType == COLUMN || m_expType == VARIABLE || m_expType == UNKNOWN){
      for (int i=0;i<fieldnames.size();i++)
        if (upper_copy(m_expStr).compare(upper_copy(fieldnames[i])) == 0){
          //trace(DEBUG,"777777 '%s' \n", m_expStr.c_str());
          return true;
        }
      //trace(DEBUG,"888888 '%s' \n", m_expStr.c_str());
      return false;
    }else if (m_expType == FUNCTION){
      if (groupFuncOnly()){
        //trace(DEBUG,"999999 '%s' \n", m_expStr.c_str());
        return true;
      }
      if (m_Function){
        bool compatible = true;
        for (int i=0;i<m_Function->m_params.size();i++)
          if (!m_Function->m_params[i].inColNamesRange(fieldnames)){
            compatible = false;
            //trace(DEBUG,"AAAAAA '%s' \n", m_expStr.c_str());
            break;
          }
        //trace(DEBUG,"BBBBBB '%s' \n", m_expStr.c_str());
        return compatible;
      }
    }
  }else{
    if (!m_leftNode || !m_leftNode->inColNamesRange(fieldnames)){
      //trace(DEBUG,"CCCCCC '%s' \n", m_leftNode?m_leftNode->m_expStr.c_str():m_expStr.c_str());
      return false;
    }
    if (!m_rightNode || !m_rightNode->inColNamesRange(fieldnames)){
      //trace(DEBUG,"DDDDDD '%s' \n", m_rightNode?m_rightNode->m_expStr.c_str():m_expStr.c_str());
      return false;
    }
    //trace(DEBUG,"EEEEEE '%s' \n", m_expStr.c_str());
    return true;
  }
  trace(DEBUG,"FFFFFF '%s' \n", m_expStr.c_str());
  return false;
}
