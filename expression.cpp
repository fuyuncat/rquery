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
#include <algorithm>
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
  m_rawDatatype = NULL;
  m_Function = NULL;
  m_macroParaDefExpr = NULL;

  m_metaDataAnzlyzed = false; // analyze column name to column id.
  m_expstrAnalyzed = false;   // if expression string analyzed

  //m_operators = {'^','*','/','+','-'};
  //m_macroParaDefault.clear();
  m_operators.clear();
  m_operators.insert('^');m_operators.insert('*');m_operators.insert('/');m_operators.insert('+');m_operators.insert('-'); // "^", "*", "/" should be before "+", "-"
}

ExpressionC::ExpressionC()
{
  init();
}

ExpressionC::ExpressionC(const string & expString)
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
  node->m_rawDatatype = m_rawDatatype;
  if (m_Function)
    node->m_Function = m_Function->cloneMe();
  else
    node->m_Function = NULL;
  if (m_macroParaDefExpr)
    node->m_macroParaDefExpr = m_macroParaDefExpr->cloneMe();
  else
    node->m_macroParaDefExpr = NULL;
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
    node->m_rawDatatype = m_rawDatatype;
    if(m_Function){
      if (node->m_Function){
        node->m_Function->clear();
        SafeDelete(node->m_Function);
      }
      node->m_Function = new FunctionC();
      m_Function->copyTo(node->m_Function);
    }else
      node->m_Function = NULL;
    if(m_macroParaDefExpr){
      if (node->m_macroParaDefExpr){
        node->m_macroParaDefExpr->clear();
        SafeDelete(node->m_macroParaDefExpr);
      }
      node->m_macroParaDefExpr = new ExpressionC();
      m_macroParaDefExpr->copyTo(node->m_macroParaDefExpr);
    }else
      node->m_macroParaDefExpr = NULL;
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
  if (m_macroParaDefExpr){
    m_macroParaDefExpr->clear();
    SafeDelete(m_macroParaDefExpr);
  }
  //m_macroParaDefault.clear();
  
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

ExpressionC::ExpressionC(const int & operate, const int & colId, const string & data)
{
  clear();
  m_type = LEAF;
  m_operate = operate;
  m_colId = colId;
  m_expStr = data;
}

void ExpressionC::setExpstr(const string & expString)
{
  m_expStr = expString;
  buildExpression();
  //dump();
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


New method (v0.951)
8+7*(8+8)+3*16*16

 +
8 7*(8+8)+3*16*16

  +
8  *
  7  (8+8)+3*16*16

  +
8   +N
  *    3*16*16
7 (8+8)

  +
8   +
  *    3*16*16
7  +
  8 8

  +
8    +
  *    *
7  +  3 16*16
  8 8

  +
8    +
  *     *
7  +  3   *
  8 8   16 16

**********************************************************/
ExpressionC* ExpressionC::BuildTree(string expStr, ExpressionC* newNode, ExpressionC* parentNode, bool isLeftChild)
{
  if (!newNode){
    trace(ERROR, "Error: node is not allocated!\n");
    return NULL;
  }
  if (expStr.empty()){
    trace(ERROR, "Error: No statement found!\n");
    return NULL;
  }else
    expStr = trim_copy(expStr);
  trace(DEBUG, "Building BTREE from '%s'\n", expStr.c_str());
  int nextPos=0,strStart=0;
  try{
    newNode->m_expstrAnalyzed = true;
    newNode->m_parentNode = parentNode;
    int iPos = findFirstCharacter(expStr, m_operators, 0, "''()", '\\',{'(',')'});
    // check if is a scientific notation number, e.g.1.58e+8
    if(iPos>1 && iPos<expStr.length()-1 && (expStr[iPos-1]=='e' || expStr[iPos-1]=='E') && (expStr[iPos]=='+' || expStr[iPos] == '-')){
      //trace(DEBUG, "Checking scientific notation number for base '%s'\n", expStr.substr(0,iPos-1).c_str());
      if (isDouble(expStr.substr(0,iPos-1))){ //check left of +/1 is a double number
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
        if (!BuildTree(expStr.substr(1,expStr.length()-2),newNode,NULL,true))
          return NULL;
        newNode=newNode->getTopParent();
        newNode->m_parentNode = parentNode;
        if (parentNode && isLeftChild){
          if (parentNode->m_rightNode!=newNode)
            parentNode->m_leftNode = newNode;
        }else{
          if (parentNode->m_leftNode!=newNode)
            parentNode->m_rightNode = newNode;
        }
      }else{ // atom expression to be built as a leaf
        buildLeafNode(expStr, newNode);
      }
    }else{ // got an operator, building a branch
      //trace(DEBUG, "Found '%s'(%d) in '%s'\n",expStr.substr(iPos,1).c_str(),iPos,expStr.c_str());
      if (iPos == expStr.length() - 1){ // operator should NOT be the end
        //newNode->clear();
        //SafeDelete(newNode);
        trace(ERROR, "Operator should NOT be the end of the expression!\n");
        return NULL;
      }
      newNode->m_type = BRANCH;
      newNode->m_operate = encodeOperator(expStr.substr(iPos,1));
      newNode->m_expStr = expStr.substr(iPos,1);
      if (parentNode){
        if (operatorPriority(newNode->m_operate)>operatorPriority(parentNode->m_operate)){
          //parentNode->m_rightNode = newNode;
          ExpressionC* tmpNode = new ExpressionC();
          newNode->m_leftNode = tmpNode;
          if (!BuildTree(expStr.substr(0,iPos), tmpNode, newNode,true))
            return NULL;
        }else{
          ExpressionC* tmpNode = new ExpressionC();
          parentNode->m_rightNode = tmpNode;
          if (!BuildTree(expStr.substr(0,iPos), tmpNode, parentNode,false))
            return NULL;
          ExpressionC* pNode = parentNode;
          while (pNode && operatorPriority(newNode->m_operate)<=operatorPriority(pNode->m_operate)){
            if (!pNode->m_parentNode){
              newNode->m_leftNode = pNode;
              pNode->m_parentNode = newNode;
              newNode->m_parentNode = NULL;
              break;
            }
            pNode = pNode->m_parentNode;
            if (operatorPriority(newNode->m_operate)>operatorPriority(pNode->m_operate)){
              newNode->m_leftNode = pNode->m_rightNode;
              pNode->m_rightNode->m_parentNode = newNode;
              newNode->m_parentNode = pNode;
              pNode->m_rightNode = newNode;
              break;
            }
          }
        }
      }else{
        ExpressionC* tmpNode = new ExpressionC();
        newNode->m_leftNode = tmpNode;
        if (!BuildTree(expStr.substr(0,iPos), tmpNode, newNode,true))
          return NULL;
      }
      //if (!parentNode || operatorPriority(newNode->m_operate)<operatorPriority(parentNode->m_operate)){
      //  if (parentNode){
      //    parentNode->m_rightNode = newNode;
      //  }
      //  newNode->m_leftNode = BuildTree(expStr.substr(0,iPos), NULL);
      //}else{
      //  parentNode->m_rightNode = BuildTree(expStr.substr(0,iPos), NULL); 
      //  parentNode->m_datatype = getCompatibleDataType(parentNode->m_leftNode->m_datatype, parentNode->m_rightNode->m_datatype);
      //  newNode->m_leftNode = parentNode;
      //}
      ExpressionC* tmpNode = new ExpressionC();
      newNode->m_rightNode = tmpNode;
      if (!BuildTree(expStr.substr(iPos+1), tmpNode, newNode,false))
        return NULL;
      newNode->m_datatype = getCompatibleDataType(newNode->m_leftNode->m_datatype, newNode->m_rightNode->m_datatype);
      if (parentNode)
        parentNode->m_datatype = getCompatibleDataType(parentNode->m_leftNode->m_datatype, parentNode->m_rightNode->m_datatype);
    }
    return newNode;
    //return newNode->getTopParent();
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
  if (expStr.empty()){
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
    node->m_rawDatatype = NULL;
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
        else if (node->m_expStr.compare("@LINE") == 0 || node->m_expStr.compare("@FILELINE") == 0 || node->m_expStr.compare("@FILEID") == 0 || node->m_expStr.compare("@ROW") == 0 || node->m_expStr.compare("@ROWSORTED") == 0 || node->m_expStr.compare("@%") == 0 || node->m_expStr.compare("@LEVEL") == 0 || node->m_expStr.compare("@NODEID") == 0 || node->m_expStr.compare("@ISLEAF") == 0 || node->m_expStr.compare("@DUPID") == 0)
          node->m_datatype.datatype = LONG;
        else if (node->m_expStr.find("@FIELD") == 0){
          string sColId = m_expStr.substr(string("@FIELD").length());
          node->m_colId = isInt(sColId)?atoi(sColId.c_str())-1:-1;
        }else if (isInt(node->m_expStr.substr(1))){ // @N is abbreviasion of @fieldN
          string sColId = node->m_expStr.substr(1);
          node->m_colId = isInt(sColId)?atoi(sColId.c_str())-1:-1;
          node->m_expStr = "@FIELD"+sColId;
        }else{
          node->m_datatype.datatype = UNKNOWN;
        }
        return true;
      }else{
        trace(ERROR, "Invalid variable name '%s'! \n", expStr.c_str());
        node->m_expstrAnalyzed = false;
        return false;
      }
    }else if (expStr[0] == '~'){ // checking User Defined Macro Function Parameter ~nam[=val]~
      size_t pos=0;
      string sParastr = trim_copy(readQuotedStr(expStr, pos, "~~", "''", '\0', {}));
      vector<string> vMacroNamVal;
      split(vMacroNamVal,sParastr,'=',"''()",'\\',{'(',')'},false,true);
      if (vMacroNamVal.size()==0 || trim_copy(vMacroNamVal[0]).empty()){
        trace(FATAL, "(2) Macro function parameter '%s' has an invalid definition! \n", sParastr.c_str());
        return false;
      }
      node->m_expStr = upper_copy(trim_copy(vMacroNamVal[0]));
      node->m_expType = MACROPARA;
      node->m_expstrAnalyzed = true;
      node->m_datatype.datatype = ANY;
      if (vMacroNamVal.size()>1)
        node->m_macroParaDefExpr = new ExpressionC(trim_copy(vMacroNamVal[1]));
      return true;
    }else if (expStr[0] == '\''){ // checking STRING string
      if (expStr.length()>1 && expStr[expStr.length()-1] == '\''){ // whole string is a STRING string
        size_t iPos = 0;
        node->m_expStr = readQuotedStr(expStr, iPos, "''", "", '\\', {});
        replacestr(node->m_expStr,{"\\\\","\\'","\\t","\\v","\\n","\\r"},{"\\","'","\t","\v","\n","\r"});
        trace(DEBUG, "Read STRING \"%s\" . \n", node->m_expStr.c_str());
        if (isDate(node->m_expStr, node->m_datatype.extrainfo))
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
      if ((node->m_Function->m_funcID==ANYCOL || node->m_Function->m_funcID==ALLCOL) && node->m_parentNode){
        trace(FATAL, "Function '%s' cannot be a part of an expression! \n", expStr.c_str());
        return false;
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
  ExpressionC* newNode = new ExpressionC();
  if (BuildTree(m_expStr, newNode, NULL,true)){
    ExpressionC* root = newNode->getTopParent();
    root->copyTo(this);
    root->clear();
    SafeDelete(root);
    return true;
  }else{
    newNode->clear();
    SafeDelete(newNode);
    clear();
    return false;
  }
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

void ExpressionC::dump(const int & deep) const
{
  if (m_type == BRANCH){
    trace(DUMP,"%s(%d-%s)\n",decodeOperator(m_operate).c_str(),deep,decodeDatatype(m_datatype.datatype).c_str());
    trace(DUMP,"L-");
    if (m_leftNode)
      m_leftNode->dump(deep+1);
    trace(DUMP,"R-");
    if (m_rightNode)
      m_rightNode->dump(deep+1);
  }else{
    trace(DUMP,"(%d)%s(%d-%s)\n",deep,m_expStr.c_str(),m_colId,decodeDatatype(m_datatype.datatype).c_str());
  }
}

void ExpressionC::dump() const
{
  dump(0);
}

string ExpressionC::getEntireExpstr() const
{
  string expStr = m_expStr;
  if (m_type==LEAF&&m_expType==CONST&&(m_datatype.datatype==STRING||m_datatype.datatype==DATE||m_datatype.datatype==TIMESTAMP)){
    replacestr(expStr,"'","\\'");
    expStr = "'"+expStr+"'";
  }
  return (m_leftNode?m_leftNode->getEntireExpstr():"")+expStr+(m_rightNode?m_rightNode->getEntireExpstr():"");
}

// detect if predication contains special colId    
bool ExpressionC::containsColId(const int & colId){
  bool contain = false;
  if (m_type == BRANCH){
    contain = contain || m_leftNode->containsColId(colId);
    contain = contain || m_rightNode->containsColId(colId);
  }else
    contain = (m_colId == colId);

  return contain;
}

// detect if predication contains special colId    
ExpressionC* ExpressionC::getFirstPredByColId(const int & colId, const bool & leftFirst){
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
DataTypeStruct ExpressionC::analyzeColumns(vector<string>* fieldnames, vector<DataTypeStruct>* fieldtypes, DataTypeStruct* rawDatatype, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes)
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
    m_rawDatatype = rawDatatype;
    if (m_type == BRANCH){
      dts.datatype=UNKNOWN;
      DataTypeStruct rdatatype = m_rightNode?m_rightNode->analyzeColumns(fieldnames, fieldtypes, rawDatatype, sideDatatypes):dts;
      DataTypeStruct ldatatype = m_leftNode?m_leftNode->analyzeColumns(fieldnames, fieldtypes, rawDatatype, sideDatatypes):dts;
      //trace(DEBUG, "Left node: %s (%d); Right node: %s (%d)\n", m_leftNode->m_expStr.c_str(),m_leftNode->m_type, m_rightNode->m_expStr.c_str(),m_rightNode->m_type);
      //trace(DEBUG, "Getting compatible type from %s and %s\n", decodeDatatype(ldatatype.datatype).c_str(), decodeDatatype(rdatatype.datatype).c_str());
      m_datatype = getCompatibleDataType(ldatatype, rdatatype);
      //trace(DEBUG, "Getting compatible type %s\n", decodeDatatype(m_datatype.datatype).c_str());
      m_metaDataAnzlyzed = m_datatype.datatype!=UNKNOWN;
    }else{
      if (fieldnames->size() < fieldtypes->size()){
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
        else if(m_expStr.length()>=8 && m_expStr[1]=='R'&&m_expStr[2]=='['){
          size_t pos=0;
          string s1 = upper_copy(readQuotedStr(m_expStr, pos, "[]", "''", '\0', {})), s2 = upper_copy(readQuotedStr(m_expStr, pos, "[]", "''", '\0', {}));
          if (s1.empty() || s2.empty()){
            trace(ERROR, "(1)Invalide subscript in reference variable '%s'\n",m_expStr.c_str());
          }
          m_datatype = (*sideDatatypes)[s1][s2];
        }else if (m_expStr.compare("@LINE") == 0 || m_expStr.compare("@FILELINE") == 0 || m_expStr.compare("@FILEID") == 0 || m_expStr.compare("@ROW") == 0 || m_expStr.compare("@ROWSORTED") == 0 || m_expStr.compare("@%") == 0 || m_expStr.compare("@LEVEL") == 0 || m_expStr.compare("@NODEID") == 0 || m_expStr.compare("@ISLEAF") == 0 || m_expStr.compare("@DUPID") == 0)
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
            // this is a warning, because field size could be different if query multiple files.
            trace(WARNING, "Unrecognized variable(1) %s, Extracted COL ID: %d, number of fields: %d.\n", sColId.c_str(), iColID, fieldtypes->size());
            m_expType = UNKNOWN;
            m_datatype.datatype = UNKNOWN;
          }
        }else if (m_expStr.compare("@N") == 0){ // the latest field
          m_expType = COLUMN;
          m_colId = (int)fieldtypes->size()-1;
          m_datatype = (*fieldtypes)[m_colId];
          trace(DEBUG, "Tuning '%s' from VARIABLE to COLUMN(%d).\n", m_expStr.c_str(), m_colId);
        }else if (m_expStr.length()>2 && m_expStr[1] == '(' && m_expStr[2] == 'N'  && m_expStr[m_expStr.length()-1] == ')'){ // calculate @(N-x)
          string expStr = m_expStr.substr(2,m_expStr.length()-3);
          replacestr(expStr,"N",intToStr(fieldtypes->size()));
          ExpressionC constExp(expStr);
          if (!isInt(constExp.m_expStr)){
            trace(ERROR, "analyzeColumns: '%s' is not a valid number expresion!\n", m_expStr.c_str());
            return dts;
          }
          m_expType = COLUMN;
          m_colId = max(1,min((int)fieldtypes->size(),atoi(constExp.m_expStr.c_str())))-1;
          m_datatype = (*fieldtypes)[m_colId];
          trace(DEBUG, "Tuning '%s' from VARIABLE to COLUMN(%d).\n", m_expStr.c_str(), m_colId);
        }else{
          //trace(ERROR, "Unrecognized variable(2) %s .\n", m_expStr.c_str());
          //m_expType = UNKNOWN;
          m_datatype.datatype = ANY;
        }
        //trace(DEBUG, "Expression '%s' type is %s, data type is '%s'\n", m_expStr.c_str(), decodeExptype(m_expType).c_str(),decodeDatatype(m_datatype.datatype).c_str());
        return m_datatype;
      }else if (m_expType == MACROPARA){ // analy columns of the macro para default value.
        if (m_macroParaDefExpr)
          m_datatype = m_macroParaDefExpr->analyzeColumns(fieldnames, fieldtypes, rawDatatype, sideDatatypes);
      }else if (m_expType == COLUMN){ // field data type could be detected more than once.
        if (m_colId>=0 && m_colId<fieldtypes->size())
          m_datatype = (*fieldtypes)[m_colId];
        else
          m_datatype.datatype = UNKNOWN;
      }
      if (m_datatype.datatype == UNKNOWN){
        // check if it is a function FUNCNAME(...)
        int lefParPos = m_expStr.find("(");
        if (m_expStr.length()>2 && m_expStr[0] != '\'' && lefParPos>0 && m_expStr[m_expStr.length()-1] == ')')
          m_expType = FUNCTION;
        else{
          if (m_expType == CONST){
            // check if it is a time, quoted by {}
            if (m_expStr.length()>1 && m_expStr[0]=='{' && m_expStr[m_expStr.length()-1]=='}'){
              if (isDate(m_expStr.substr(1,m_expStr.length()-2),m_datatype.extrainfo)){
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
          for (size_t i=0; i<fieldnames->size(); i++){
            if (upper_copy(m_expStr).compare(upper_copy((*fieldnames)[i])) == 0){
              if (i>=fieldtypes->size()){
                trace(ERROR, "Field id %d is out range of filed data types numbers %d\n", i, fieldtypes->size());
                dts.datatype = UNKNOWN;
                return dts;
              }
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
      }
      if (m_expType == FUNCTION){
        if (!m_Function){
          m_Function = new FunctionC(m_expStr);
          trace(DEBUG, "(2)New function from '%s'\n",m_expStr.c_str());
        }
        m_datatype = m_Function->analyzeColumns(fieldnames, fieldtypes, rawDatatype, sideDatatypes);
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

// get all involved colIDs in this expression
unordered_set<int> ExpressionC::getAllColIDs(const int & side){
  unordered_set<int> colIDs;
  if (m_type == BRANCH){
    if (m_leftNode){
      unordered_set<int> foo = m_leftNode->getAllColIDs(side);
      colIDs.insert(foo.begin(), foo.end());
    }
    if (m_rightNode){
      unordered_set<int> foo = m_rightNode->getAllColIDs(side);
      colIDs.insert(foo.begin(), foo.end());
    }
  }else if(m_type == LEAF){
    if (m_colId>=0)
      colIDs.insert(m_colId);
  }
  return colIDs;
}

// build the expression as a HashMap
unordered_map<int,string> ExpressionC::buildMap(){
  unordered_map<int,string> datas;
  if (m_type == BRANCH){
    if (m_leftNode){
      unordered_map<int,string> foo = m_leftNode->buildMap();
      datas.insert(foo.begin(), foo.end());
    }
    if (m_rightNode){
      unordered_map<int,string> foo = m_rightNode->buildMap();
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
void ExpressionC::fillDataForColumns(unordered_map <string, string> & dataList, const vector <string> & columns){
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

FunctionC* ExpressionC::getAnaFunc(const string & funcExpStr)
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

bool ExpressionC::calAggFunc(const GroupProp & aggGroupProp, FunctionC* function, string & sResult)
{
  switch (aggGroupProp.funcID){
    case AVERAGE:
      sResult = doubleToStr(aggGroupProp.sum/(double)aggGroupProp.count);
      break;
    case SUM:
      sResult = doubleToStr(aggGroupProp.sum);
      break;
    case COUNT:
      sResult = longToStr(aggGroupProp.count);
      break;
    case UNIQUECOUNT:{
      //sResult = longToStr(aggGroupProp.uniquec->size());
      unordered_set <string> uniquec(aggGroupProp.varray->begin(), aggGroupProp.varray->end());
      sResult = longToStr(uniquec.size());
      break;
    }case MAX:
      sResult = aggGroupProp.max;
      break;
    case MIN:
      sResult = aggGroupProp.min;
      break;
    case GROUPLIST:{
      sResult = "";
      trace(DEBUG, "varray size: %d\n", aggGroupProp.varray->size());
      if (function->m_bDistinct){ // do distinct
        unordered_set <string> uniquec(aggGroupProp.varray->begin(), aggGroupProp.varray->end());
        aggGroupProp.varray->clear();
        std::copy(uniquec.begin(), uniquec.end(), std::back_inserter(*(aggGroupProp.varray)));
      }
      if (function->m_params.size()>2){ // sort parameter provided, do sorting
        struct SortType{
          DataTypeStruct dts;
          short int direction;
        };
        SortType sortKey;
        sortKey.dts = function->m_datatype;
        sortKey.direction = upper_copy(trim_copy(function->m_params[2].m_expStr)).compare("DESC")==0?DESC:ASC;
        auto sortVectorLambda = [sortKey] (string const& v1, string const& v2) -> bool
        {
          int iCompareRslt = anyDataCompare(v1,v2,sortKey.dts,sortKey.dts);
          return (sortKey.direction==ASC ? iCompareRslt<0 : iCompareRslt>0);
        };
        std::sort(aggGroupProp.varray->begin(), aggGroupProp.varray->end(), sortVectorLambda);
      }
      for (size_t i=0;i<aggGroupProp.varray->size();i++)
        sResult.append((i>0?(function->m_params.size()>1?function->m_params[1].m_expStr:" "):"")+(*aggGroupProp.varray)[i]);
      function->m_datatype.datatype = STRING;
      break;
    }
    default:{
      trace(ERROR, "Invalid aggregation function '%s'\n",function->m_expStr.c_str());
      return false;
    }
  }
  return true;
}

#ifdef __DEBUG__
long int g_evalexprtime;
long int g_evalexprconsttime;
long int g_evalexprfunctime;
long int g_evalexprvartime;
long int g_evalexprcoltime;
long int g_evalexprmacpatime;
long int g_evalexprcaltime;
#endif // __DEBUG__

// calculate this expression. fieldnames: column names; fieldvalues: column values; varvalues: variable values; sResult: return result. column names are upper case; skipRow: wheather skip @row or not. extrainfo so far for date format only
bool ExpressionC::evalExpression(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts, const bool & getresultonly)
{
#ifdef __DEBUG__
  long int thistime = curtime();
  long int evalstarttime = thistime;
#endif // __DEBUG__
  if (!rds.fieldvalues || !rds.fieldvalues || !rds.aggFuncs || !rds.anaFuncs){
    trace(ERROR, "Insufficient metadata!\n");
    return false;
  }
  // if (!m_metaDataAnzlyzed || !m_expstrAnalyzed){
  if (!m_metaDataAnzlyzed && (m_type != LEAF || m_expType != FUNCTION || !m_Function || !m_Function->isAggFunc())){
    trace(ERROR, "Expression '%s' is not analyzed! metaData: %d, expstr: %d \n",getEntireExpstr().c_str(),m_metaDataAnzlyzed,m_expstrAnalyzed);
    return false;
  }
  bool bResult=false;
  if (m_type == LEAF){
    if (m_expType == CONST){
      sResult = m_expStr;
      dts = m_datatype;
      bResult=true;
#ifdef __DEBUG__
  g_evalexprconsttime += curtime()-thistime;
  thistime = curtime();
#endif // __DEBUG__
    }else if (m_expType == FUNCTION){
      //trace(DEBUG2, "Expression '%s' is a fucntion.. \n",m_expStr.c_str());
      if (m_Function){
        if (m_Function->isAggFunc()){
          // looks like this part is duplicated with querierc.evalAggExpNode(), but dont change rds.aggFuncs values, as the same aggregation function may present multiple times in the selections/sorts clauses.
          unordered_map< string,GroupProp >::iterator it = rds.aggFuncs->find(m_Function->m_expStr);
          if (it != rds.aggFuncs->end()){
            if (m_Function->m_params.size()>0){
              // we dont bother with the paramter, as it has already been evaled in querierc.evalAggExpNode()
              // calculate the parameter here will cause duplicated calculation if the same aggregation function involved multiple times in the selection/sort
              //m_Function->m_params[0].evalExpression(rds, sResult, extrainfo);
              it->second.funcID = m_Function->m_funcID;
              dts = m_Function->m_datatype;
              if (calAggFunc(it->second, m_Function, sResult)){
                //m_expType = CONST;
                //SafeDelete(m_Function);
                bResult=true;
              }else
                bResult=false;
            }else
              trace(ERROR, "Missing paramters for aggregation function '%s'\n",m_Function->m_expStr.c_str());
          }else{
            trace(ERROR, "Failed to find aggregation function '%s' dataset when evaling '%s'!\n", m_Function->m_expStr.c_str(), getTopParent()->getEntireExpstr().c_str());
            bResult=false;
          }
        }else if (m_Function->isAnalytic()){
          unordered_map< string,vector<string> >::iterator it = rds.anaFuncs->find(m_Function->m_expStr);
          if (it != rds.anaFuncs->end()){
            unordered_map< string,vector<string> > * oldAnaFuncs = rds.anaFuncs;
            static unordered_map< string,vector<string> > dummyAnaFuncs;
            rds.anaFuncs = &dummyAnaFuncs;
            string tmpRslt;
            if (it->second.size()==0)// Empty vector means it's still doing raw data matching, only need to eval parameter expressions. Only retrieve once for each analytic function (identified by its expression str)
              for (size_t i=0;i<m_Function->m_params.size();i++){
                m_Function->m_params[i].evalExpression(rds, tmpRslt, dts, true);
                it->second.push_back(tmpRslt);
              }
            else // otherwise, the passed-in vector is analytic function result, need to return it to do other operation, e.g. filter
              sResult = it->second[0];
            dts = m_Function->m_datatype;
            rds.anaFuncs = oldAnaFuncs;
            bResult=true;
          }else{
            trace(ERROR, "Failed to find analytic function '%s' dataset when evaling '%s'!\n", m_Function->m_expStr.c_str(), getTopParent()->getEntireExpstr().c_str());
            bResult=false;
          }
        }else{
          bool gotResult = m_Function->runFunction(rds, sResult, dts);
          m_datatype = dts;
          if (!getresultonly && gotResult){
            m_expType = CONST;
            m_expStr = sResult;
          }
          bResult=gotResult;
        }
      }else
        bResult=false;
#ifdef __DEBUG__
  g_evalexprfunctime += curtime()-thistime;
  thistime = curtime();
#endif // __DEBUG__
    }else if (m_expType == COLUMN){
      if (m_colId >= 0 && m_colId<rds.fieldvalues->size()){
        sResult = (*rds.fieldvalues)[m_colId];
        dts = (*m_fieldtypes)[m_colId];
        m_datatype = dts;
        if (!getresultonly){
          m_expType = CONST;
          m_expStr = sResult;
        }
        bResult=true;
      }else{
        size_t i=0;
        for (i=0; i<m_fieldnames->size(); i++)
          if ((*m_fieldnames)[i].compare(m_expStr) == 0)
            break;
        if (i<rds.fieldvalues->size() && i<m_fieldtypes->size()){
          sResult = (*rds.fieldvalues)[i];
          dts = (*m_fieldtypes)[i];
          m_datatype = dts;
          if (!getresultonly){
            m_expType = CONST;
            m_expStr = sResult;
          }
          bResult=true;
        }else{
          trace(WARNING, "Cannot find COLUMN '%s'\n",m_expStr.c_str()); // change ERROR to WARNING as when field size changes, the parsed field names/types still exist.
          bResult=false;
        }
      }
#ifdef __DEBUG__
  g_evalexprcoltime += curtime()-thistime;
  thistime = curtime();
#endif // __DEBUG__
    }else if (m_expType == VARIABLE){
      //if (skipRow && (m_expStr.compare("@ROW") == 0 || m_expStr.compare("@ROWSORTED") == 0)){
      //  //trace(DEBUG, "Skip @row & @rowsorted ... \n");
      //  return true;
      //}
      if (rds.varvalues->find(m_expStr) != rds.varvalues->end()){
        //trace(DEBUG, "Assigning '%s' to '%s' ... \n", (*rds.varvalues)[m_expStr].c_str(), m_expStr.c_str());
        //dumpMap(*rds.varvalues);
        sResult = (*rds.varvalues)[m_expStr];
        dts = m_datatype;
        if (!getresultonly){
          m_expType = CONST;
          m_expStr = sResult;
        }
        bResult=true;
      }else if(m_expStr.length()>=8 && m_expStr[1]=='R'&&m_expStr[2]=='['){
        size_t pos=0;
        string s1 = upper_copy(readQuotedStr(m_expStr, pos, "[]", "''", '\0', {})), s2 = upper_copy(readQuotedStr(m_expStr, pos, "[]", "''", '\0', {}));
        if (s1.empty() || s2.empty()){
          trace(ERROR, "(2)Invalide subscript in reference variable '%s'\n",m_expStr.c_str());
          bResult=false;
        }else{
          sResult = (*rds.sideDatarow)[s1][s2];
          dts = (*rds.sideDatatypes)[s1][s2];
          m_datatype = dts;
          if (!getresultonly){
            m_expType = CONST;
            m_expStr = sResult;
          }
          bResult=true;
        }
      }else{
        trace(ERROR, "Cannot find VARIABLE '%s'\n",m_expStr.c_str());
        bResult=false;
      }
#ifdef __DEBUG__
  g_evalexprvartime += curtime()-thistime;
  thistime = curtime();
#endif // __DEBUG__
    }else if (m_expType == MACROPARA){
      if (rds.macroFuncParas && rds.macroFuncParas->find(m_expStr)!=rds.macroFuncParas->end()){
        sResult = (*rds.macroFuncParas)[m_expStr];
        dts.datatype = ANY;
        bResult=true;
      }else{
        // use default value
        if (m_macroParaDefExpr){
          if (!m_macroParaDefExpr->evalExpression(rds, sResult, dts, getresultonly)){
            trace(ERROR, "Failed to get the default value from '%s' for \n", m_expStr.c_str());
            bResult=false;
          }else
            bResult=true;
        }else{
          trace(ERROR, "Please provide the value of macro function parameter '%s'\n",m_expStr.c_str());
          bResult=false;
        }
      }
#ifdef __DEBUG__
  g_evalexprmacpatime += curtime()-thistime;
  thistime = curtime();
#endif // __DEBUG__
    }else{
      trace(ERROR, "Unknown expression type of '%s'\n",m_expStr.c_str());
      bResult=false;
    }
  }else{
    string leftRst = "", rightRst = "";
    DataTypeStruct leftDts, rightDts;
    bResult = true;
    if (!m_leftNode || !m_leftNode->evalExpression(rds, leftRst, leftDts, getresultonly)){
      trace(ERROR, "Missing leftNode '%s'\n",m_expStr.c_str());
      bResult=false;
    }
    if (bResult && (!m_rightNode || !m_rightNode->evalExpression(rds, rightRst, rightDts, getresultonly))){
      trace(ERROR, "Missing rightNode '%s'\n",m_expStr.c_str());
      bResult=false;
    }
#ifdef __DEBUG__
  thistime = curtime();
#endif // __DEBUG__
    //trace(DEBUG2,"calculating(1) (%s) '%s'%s'%s'\n", decodeExptype(m_datatype.datatype).c_str(),leftRst.c_str(),decodeOperator(m_operate).c_str(),rightRst.c_str());
    //if ((m_leftNode->m_type==LEAF && m_leftNode->m_expType==FUNCTION && m_leftNode->m_Function && m_leftNode->m_Function->isAnalytic()) || (m_rightNode->m_type==LEAF && m_rightNode->m_expType==FUNCTION && m_rightNode->m_Function && m_rightNode->m_Function->isAnalytic())) { // if left or right expression is analytic function, we dont do the operation
    if (bResult){
      if (m_leftNode->containAnaFunc() || m_rightNode->containAnaFunc()) { // if left or right expression is analytic function, we dont do the operation
        trace(DEBUG,"Skip to eval analytic function. Left: '%s'; Right: '%s'\n", m_leftNode->getEntireExpstr().c_str(),m_rightNode->getEntireExpstr().c_str());
        bResult=true;
      }else if ( anyDataOperate(leftRst, m_operate, rightRst, m_datatype, sResult)){
        trace(DEBUG,"calculating(1) (%s) '%s'%s'%s', get '%s'\n", decodeExptype(m_datatype.datatype).c_str(),leftRst.c_str(),decodeOperator(m_operate).c_str(),rightRst.c_str(),sResult.c_str());
        dts = m_datatype;
        if (!getresultonly){
          //clear();
          m_type = LEAF;
          m_expType = CONST;
          m_expStr = sResult;
          m_datatype = dts;
        }
        bResult=true;
      }else
        bResult=false;
    }
#ifdef __DEBUG__
  g_evalexprcaltime += curtime()-thistime;
#endif // __DEBUG__
  }
#ifdef __DEBUG__
  g_evalexprtime += curtime()-evalstarttime;
#endif // __DEBUG__
  return bResult;
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
          unordered_map<string,string> mvarvalues;
          unordered_map< string,GroupProp > aggFuncs;
          unordered_map< string,vector<string> > anaFuncs;
          vector< vector< unordered_map<string,string> > > sideDatasets;
          unordered_map< string, unordered_map<string,string> > sideDatarow;
          unordered_map< string, unordered_map<string,DataTypeStruct> > sideDatatypes;
          m_Function->analyzeColumns(&vfieldnames, &fieldtypes, &rawDatatype, &sideDatatypes);
          DataTypeStruct dts;
          RuntimeDataStruct rds;
          rds.fieldvalues = &vfieldvalues;
          rds.varvalues = &mvarvalues;
          rds.aggFuncs = &aggFuncs;
          rds.anaFuncs = &anaFuncs;
          rds.sideDatasets = &sideDatasets;
          rds.sideDatarow = &sideDatarow;
          rds.sideDatatypes = &sideDatatypes;
          gotResult = m_Function->runFunction(rds,sResult,dts);
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
      for (size_t i=0;i<m_Function->m_params.size();i++){
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
        for (size_t i=0; i<m_Function->m_params.size();i++)
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
      for (size_t i=0;i<m_Function->m_params.size();i++){
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

bool ExpressionC::getTreeFuncs(unordered_map< string,vector<ExpressionC> > & treeFuncs)
{
  //trace(DEBUG2,"Checking '%s'(%d %d %d)\n",getEntireExpstr().c_str(),m_type,m_expType,m_Function?m_Function->isAggFunc():-1);
  if (m_type == LEAF && m_expType == FUNCTION && m_Function){
    if (m_Function->isTree()){
      vector<ExpressionC> vParams;
      if (treeFuncs.find(m_Function->m_expStr) == treeFuncs.end()){
        vParams.clear();
        for (size_t i=0; i<m_Function->m_params.size();i++)
          vParams.push_back(m_Function->m_params[i]);
        treeFuncs.insert(pair< string,vector<ExpressionC> >(m_Function->m_expStr,vParams));
      }
      return true; // the parameter expressions of an aggregation function should not include another aggregation function
    }else { // check the paramters of normal functions
      //trace(DEBUG2,"Parameter size %d\n",m_Function->m_params.size());
      bool bGotTreeFunc = false;
      for (size_t i=0;i<m_Function->m_params.size();i++){
        //trace(DEBUG2,"Parameter '%s'\n",m_Function->m_params[i].getEntireExpstr().c_str());
        bGotTreeFunc = m_Function->m_params[i].getTreeFuncs(treeFuncs)||bGotTreeFunc;
      }
      return bGotTreeFunc;
    }
  }else{
    bool bGotTreeFunc = (m_leftNode && m_leftNode->getTreeFuncs(treeFuncs));
    bGotTreeFunc = (m_rightNode && m_rightNode->getTreeFuncs(treeFuncs));
    return bGotTreeFunc;
  }
}

void ExpressionC::setTreeFuncs(unordered_map< string,string > & treeFuncVals)
{
  if (m_type == LEAF && m_expType == FUNCTION && m_Function){
    if (m_Function->isTree() && treeFuncVals.find(m_Function->m_expStr) != treeFuncVals.end()){
      m_expType = CONST;
      m_expStr = treeFuncVals[m_Function->m_expStr];
      m_Function->clear();
      SafeDelete(m_Function);
    }else { // check the paramters of normal functions
      //trace(DEBUG2,"Parameter size %d\n",m_Function->m_params.size());
      for (size_t i=0;i<m_Function->m_params.size();i++){
        //trace(DEBUG2,"Parameter '%s'\n",m_Function->m_params[i].getEntireExpstr().c_str());
        m_Function->m_params[i].setTreeFuncs(treeFuncVals);
      }
    }
  }else{
    if (m_leftNode)
      m_leftNode->setTreeFuncs(treeFuncVals);
    if (m_rightNode)
      m_rightNode->setTreeFuncs(treeFuncVals);
  }
}

int ExpressionC::getSideWorkID()
{
  if (m_type == LEAF){
    if (m_expType == VARIABLE && m_expStr.length()>=8 && m_expStr[1]=='R' && m_expStr[2]=='['){
      size_t pos=0;
      string sID = readQuotedStr(m_expStr, pos, "[]", "''", '\0', {});
      if (isInt(sID))
        return atoi(sID.c_str());
      else
        return -1;
    }else
      return -1;
  }else{
    if (m_leftNode)
      return m_leftNode->getSideWorkID();
    if (m_rightNode)
      return m_rightNode->getSideWorkID();
    return -1;
  }
  return -1;
}

bool ExpressionC::containRefVar()
{
  if (m_type == LEAF){
    if (m_expType == FUNCTION && m_Function && m_Function->containRefVar())
      return true;
    else if (m_expType == VARIABLE && m_expStr.length()>=8 && m_expStr[1]=='R' && m_expStr[2]=='[')
      return true;
    else
      return false;
  }else{
    if ((m_leftNode && m_leftNode->containRefVar()) || (m_rightNode && m_rightNode->containRefVar()))
      return true;
    return false;
  }
  return false;
}

bool ExpressionC::containGroupFunc()
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

bool ExpressionC::containAnaFunc()
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

bool ExpressionC::groupFuncOnly()
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

bool ExpressionC::inColNamesRange(const vector<string> & fieldnames)
{
  if (m_type == LEAF){
    if (m_expType == CONST){
      //trace(DEBUG,"666666 '%s' '%s'\n", m_expStr.c_str());
      return true;
    }
    else if (m_expType == COLUMN || m_expType == VARIABLE || m_expType == UNKNOWN){
      for (size_t i=0;i<fieldnames.size();i++)
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
        for (size_t i=0;i<m_Function->m_params.size();i++)
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