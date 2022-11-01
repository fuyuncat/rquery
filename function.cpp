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
#include <math.h>
#include "expression.h"
#include "filter.h"
#include "function.h"

void FunctionC::init()
{
  m_datatype.datatype = UNKNOWN;   // 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
  m_datatype.extrainfo = "";
  m_expStr = "";          // it's the full function string, including function name and parameters
  m_funcName = "";        // analyzed function name, upper case
  m_funcID = UNKNOWN;
  m_metaDataAnzlyzed = false; // analyze column name to column id.
  m_expstrAnalyzed = false;
  m_fieldnames = NULL;
  m_fieldtypes = NULL;
  m_rawDatatype = NULL;
  m_bDistinct = false;
  m_anaParaNums.clear();
  m_params.clear();       // parameter expressions
  m_filters.clear();
}

FunctionC::FunctionC()
{
  init();
}

// Rule one Copy Constructor
FunctionC::FunctionC(const FunctionC& other)
{
  if (this != &other){
    init();
    other.copyTo(this);
  }
}

FunctionC::FunctionC(string expStr)
{
  init();
  setExpStr(expStr);
}

// Rule two Destructor
FunctionC::~FunctionC()
{
  clear();
}

// Rule one Copy Assignment Operator
FunctionC& FunctionC::operator=(const FunctionC& other)
{
  if (this != &other){
    clear();
    other.copyTo(this);
  }
  return *this;
}

void FunctionC::copyTo(FunctionC* node) const 
{
  if (!node || node==this)
    return;
  else{
    node->clear();
    node->m_metaDataAnzlyzed = m_metaDataAnzlyzed;
    node->m_expstrAnalyzed = m_expstrAnalyzed;
    node->m_datatype = m_datatype;
    node->m_expStr = m_expStr;
    node->m_funcName = m_funcName;
    node->m_funcID = m_funcID;
    node->m_params = m_params;
    node->m_filters = m_filters;
    node->m_anaParaNums = m_anaParaNums;
    node->m_bDistinct = m_bDistinct;
    node->m_fieldnames = m_fieldnames;
    node->m_fieldtypes = m_fieldtypes;
    node->m_rawDatatype = m_rawDatatype;
  }
}

// clear expression
void FunctionC::clear(){
  m_anaParaNums.clear();
  m_params.clear();
  m_filters.clear();
  init();
}

void FunctionC::setExpStr(string expStr)
{
  clear();
  m_expStr = expStr;
  if (!analyzeExpStr()){
    m_funcName = "";
    m_funcID = UNKNOWN;
    m_params.clear();
    m_filters.clear();
  }
}

bool FunctionC::isConst() const
{
  if (isAggFunc())
    return false;
  if (m_expstrAnalyzed){
    for (int i=0; i<m_params.size(); i++){
      if (m_params[i].m_expType != CONST)
        return false;
    }
    return true;
  }
  return false;
}

bool FunctionC::isAggFunc() const
{
  return (m_funcID==SUM || m_funcID==COUNT || m_funcID==UNIQUECOUNT || m_funcID==MAX || m_funcID==MIN || m_funcID==AVERAGE || m_funcID==GROUPLIST);
}

bool FunctionC::isMacro() const
{
  return (m_funcID==FOREACH || m_funcID==ANYCOL || m_funcID==ALLCOL);
}

bool FunctionC::isAnalytic() const
{
  return (m_funcID==RANK || m_funcID==DENSERANK || m_funcID==PREVIOUS || m_funcID==NEXT || m_funcID==NEARBY || m_funcID==SEQNUM || m_funcID==SUMA || m_funcID==COUNTA || m_funcID==UNIQUECOUNTA || m_funcID==MAXA || m_funcID==MINA || m_funcID==AVERAGEA);
}

bool FunctionC::isTree() const
{
  return (m_funcID==ROOT || m_funcID==PATH || m_funcID==PARENT);
}

bool FunctionC::containRefVar() const
{
  for (int i=0;i<m_params.size();i++)
    if (m_params[i].containRefVar())
      return true;
  return false;
}

// analyze expression string to get the function name (upper case) and parameter expression (classes)
bool FunctionC::analyzeExpStr()
{
  trace(DEBUG, "Analyzing function from '%s'\n", m_expStr.c_str());
  m_expStr = trim_copy(m_expStr);
  if (m_expStr.empty()){
    trace(ERROR, "Empty function expression string!\n");
    m_expstrAnalyzed = false;
    return false;
  }
  size_t iPos = 0;
  string strParams = readQuotedStr(m_expStr, iPos, "()", '\0');
  //if (iPos<0 || strParams.empty()){
  //  trace(ERROR, "No quoted parameters found in '%s'!\n", m_expStr.c_str());
  //  m_expstrAnalyzed = false;
  //  return false;
  //}
  m_funcName = trim_copy(upper_copy(m_expStr.substr(0, m_expStr.find("("))));
  m_funcID = encodeFunction(m_funcName);
  m_expStr = m_funcName+"("+strParams+")";
  //strParams = trim_pair(strParams, "()");
  vector<string> vParams = split(strParams,',',"''()",'\\',{'(',')'},false,true);
  if (isAnalytic()){ // analyze analytic function parameters: (group1[;group2]...[,sort1 [asc|desc][;sort2 [asc|desc]]...]). sParam of analytic function can be empty
    vParams = split(strParams,';',"''()",'\\',{'(',')'},false,false);
    if (vParams.size()<2){
      trace(ERROR, "There should be at least two part parameters for an analytic function '%s', sort key is compulsory, for example rank(;sortkey)!\n",m_funcName.c_str());
      m_expstrAnalyzed = false;
      return false;
    }
    for (int i=0; i<vParams.size(); i++){
      vector<string> vAnaPara = split(trim_copy(vParams[i]),',',"''()",'\\',{'(',')'},false,true);
      m_anaParaNums.push_back(vAnaPara.size());
      ExpressionC eParam;
      for (int j=0;j<vAnaPara.size();j++){
        if (i==1){// The second part should always be sort keys for ALL analytic functions.
          vector<string> vSortPara = split(trim_copy(vAnaPara[j]),' ',"''()",'\\',{'(',')'},true,true);
          eParam = ExpressionC(trim_copy(vSortPara[0]));
          m_params.push_back(eParam);
          if (vSortPara.size()>1){ // sort direction; 1:asc;-1:desc
            eParam = ExpressionC(upper_copy(trim_copy(vSortPara[1])).compare("DESC")==0?"-1":"1");
            m_params.push_back(eParam);
          }else{
            eParam = ExpressionC("1"); // default is asc
            m_params.push_back(eParam);
          }
        }else{
          ExpressionC eParam = ExpressionC(trim_copy(vAnaPara[j]));
          m_params.push_back(eParam);
        }
      }
    }
    trace(DEBUG, "FunctionC: The analytic function '%s' group size is %d, param size %d \n", m_expStr.c_str(), m_anaParaNums[0], m_params.size());
  }else{
    ExpressionC eParam;
    for (int i=0; i<vParams.size(); i++){
      trace(DEBUG, "Processing parameter(%d) '%s'!\n", i, vParams[i].c_str());
      string sParam = trim_copy(vParams[i]);
      if (sParam.empty()){
        trace(ERROR, "Empty parameter string!\n");
        m_expstrAnalyzed = false;
        return false;
      }
      if (m_funcID==WHEN && i<vParams.size()-1 && i%2==0){
        FilterC tmpFilter(sParam);
        m_filters.push_back(tmpFilter);
      }else{
        if (m_funcID == GROUPLIST){ // GROUPLIST([distinct ]expr[,delimiter][,asc|desc])
          if (i == 0){ // check distinct keyword for GROUPLIST
            vector<string> vGLExprPara = split(trim_copy(sParam),' ',"''()",'\\',{'(',')'},true,true);
            if (vGLExprPara.size() == 2){
              if (upper_copy(trim_copy(vGLExprPara[0])).compare("DISTINCT") == 0)
                m_bDistinct = true;
              else
                trace(WARNING, "'%s' is not a correct keywork, do you mean DISTINCT?\n", vGLExprPara[0].c_str());
              sParam = trim_copy(vGLExprPara[1]);
            }
            eParam = ExpressionC(sParam);
            m_params.push_back(eParam);
          }else if(i == 1){ // delimiter for GROUPLIST
            eParam = ExpressionC(sParam);
            if (eParam.m_type!=LEAF || eParam.m_expType!=CONST){
              trace(WARNING, "delimiter for GROUPLIST only accept const, '%s' is not a const. Will use a SPACE as delimiter!\n", sParam.c_str());
              eParam = ExpressionC(" ");
            }
            m_params.push_back(eParam);
          }else{
            eParam = ExpressionC(sParam);
            m_params.push_back(eParam);
          }
        }else{
          eParam = ExpressionC(sParam);
          m_params.push_back(eParam);
        }
      }
    }
  }
  switch(m_funcID){
    case UPPER:
    case LOWER:
    case SUBSTR:
    case REPLACE:
    case REGREPLACE:
    case REGMATCH:
    case DATEFORMAT:
    case PAD:
    case GETWORD:
    case GETPART:
    case RANDSTR:
    case TRIMLEFT:
    case TRIMRIGHT:
    case TRIM:
    case DATATYPE:
    case CAMELSTR:
    case SNAKESTR:
    case REVERTSTR:
    case GROUPLIST:
    case ASCII:
    case TOSTR:
    case DECTOHEX:
    case DECTOBIN:
    case FIELDNAME:
    case CONCAT:
    case CONCATCOL:
    case ROOT:
    case PATH:
    case PARENT:
      m_datatype.datatype = STRING;
      break;
    case FLOOR:
    case CEIL:
    case INSTR:
    case COMPARESTR:
    case NOCASECOMPARESTR:
    case COMPARENUM:
    case COMPAREDATE:
    case STRLEN:
    case COUNT:
    case UNIQUECOUNT:
    case ISNULL:
    case COUNTWORD:
    case COUNTSTR:
    case RANDOM:
    case RANK:
    case DENSERANK:
    case SEQNUM:
    case FINDNTH:
    case COUNTA:
    case UNIQUECOUNTA:
    case CHAR:
    case MOD:
    case TOLONG:
    case TOINT:
    case HEXTODEC:
    case BINTODEC:
    case APPENDFILE:
      m_datatype.datatype = LONG;
      break;
    case ROUND:
    case TIMEDIFF:
    case LOG:
    case AVERAGE:
    case SUM:
    case AVERAGEA:
    case SUMA:
    case ABS:
    case TOFLOAT:
    case CALCOL:
    case SUMALL:
      m_datatype.datatype = DOUBLE;
      break;
    case NOW:
    case TRUNCDATE:
    case ZONECONVERT:
    case ADDTIME:
    case TODATE:
      m_datatype.datatype = DATE;
      break;
    case MAX:
    case MIN:
    case SWITCH:
    case GREATEST:
    case LEAST: // MAX and MIN could be any data type
    case PREVIOUS:
    case NEXT:
    case NEARBY:
    case MAXA:
    case MINA:
    case FOREACH:
    case COLTOROW:
    case ANYCOL:
    case ALLCOL:
    case WHEN:
      m_datatype.datatype = ANY;
      break;
    default:{
      trace(ERROR, "Function(1) '%s' is not supported yet!\n", m_funcName.c_str());
      m_datatype.datatype = UNKNOWN;
    }
  }
  m_expstrAnalyzed = true;
  return true;
}

void FunctionC::dump(){
  trace(DUMP,"%s\n", m_expStr.c_str());
}

// analyze column ID & name from metadata, return data type of current node
// decide current node data type by checking children's data type
DataTypeStruct FunctionC::analyzeColumns(vector<string>* fieldnames, vector<DataTypeStruct>* fieldtypes, DataTypeStruct* rawDatatype, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes)
{
  trace(DEBUG, "Analyzing columns in function '%s'\n", m_expStr.c_str());
  if (!fieldnames || !fieldtypes){
    trace(ERROR, "(Function)fieldnames or fieldtypes is NULL!\n");
    DataTypeStruct dts;
    dts.datatype = UNKNOWN;
    return dts;
  }
  m_metaDataAnzlyzed = true;
  m_fieldnames = fieldnames;
  m_fieldtypes = fieldtypes;
  m_rawDatatype = rawDatatype;
  for (int i=0; i<m_params.size(); i++){
    m_params[i].analyzeColumns(m_fieldnames, m_fieldtypes, rawDatatype, sideDatatypes);
    trace(DEBUG2, "Analyzing parameter '%s' in function '%s' (%d)\n", m_params[i].getEntireExpstr().c_str(), m_expStr.c_str(),m_params[i].columnsAnalyzed());
  }
  for (int i=0; i<m_filters.size(); i++){
    m_filters[i].analyzeColumns(m_fieldnames, m_fieldtypes, rawDatatype, sideDatatypes);
    trace(DEBUG2, "Analyzing filter '%s' in function '%s' (%d)\n", m_filters[i].m_expStr.c_str(), m_expStr.c_str(),m_filters[i].columnsAnalyzed());
  }
  if (m_funcID == TRUNCDATE || m_funcID==GROUPLIST)
    m_datatype = m_params[0].m_datatype;
  return m_datatype;
}

bool FunctionC::columnsAnalyzed(){
    return m_metaDataAnzlyzed;
}

bool FunctionC::expstrAnalyzed(){
    return m_expstrAnalyzed;
}

FunctionC* FunctionC::cloneMe(){
  FunctionC* node = new FunctionC();
  node->m_metaDataAnzlyzed = m_metaDataAnzlyzed;
  node->m_expstrAnalyzed = m_expstrAnalyzed;
  node->m_datatype = m_datatype;
  node->m_expStr = m_expStr;
  node->m_funcName = m_funcName;
  node->m_funcID = m_funcID;
  node->m_params = m_params;
  node->m_filters = m_filters;
  node->m_anaParaNums = m_anaParaNums;
  node->m_bDistinct = m_bDistinct;
  node->m_fieldnames = m_fieldnames;
  node->m_fieldtypes = m_fieldtypes;
  node->m_rawDatatype = m_rawDatatype;

  return node;
}

// remove a node from prediction. Note: the input node is the address of the node contains in current prediction
//   0                      0                  2                 1
//  1  2  (remove 3) =>   4   2 (remove 1) =>      (remove 2)  3   4
//3  4
bool FunctionC::remove(FunctionC* node){
  bool removed = false;
  if (this == node){
    clear();
    return true;
  }else
    return removed;
}

bool FunctionC::runIsnull(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isnull() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true);
  if (gotResult){
    dts.datatype = INTEGER;
    sResult = intToStr(sResult.length()==0?1:0);
    return true;
  }else
    return false;
}

bool FunctionC::runUpper(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "Upper() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true);
  if (gotResult){
    sResult = upper_copy(sResult);
    return true;
  }else
    return false;
}

bool FunctionC::runLower(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "lower() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true);
  if (gotResult){
    sResult = lower_copy(sResult);
    return true;
  }else{
    trace(ERROR, "Failed to run lower(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runSubstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "substr(str, startPos, len) function accepts only two or three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sPos, sLen; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sRaw, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sPos, dts, true) && isInt(sPos)){
    int iPos = atoi(sPos.c_str());
    if (iPos<0 || iPos>=sRaw.length()){
      trace(ERROR, "%s is out of length of %s!\n", sPos.c_str(), m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() == 3){
      if (m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sLen, dts, true) && isInt(sLen)){
        int iLen = atoi(sLen.c_str());
        if (iLen<0 || iLen+iPos>sRaw.length()){
          trace(ERROR, "%s is out of length of %s starting from %s!\n", sLen.c_str(), m_params[0].getEntireExpstr().c_str(), sPos.c_str());
          return false;
        }
        sResult = sRaw.substr(iPos,iLen);
        dts.datatype = STRING;
        return true;
      }else{
        trace(ERROR, "(1)Failed to run substr(%s, %s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
        return false;
      }
    }else{
      dts.datatype = STRING;
      sResult = sRaw.substr(iPos);
      return true;
    }      
  }else{
    trace(ERROR, "(2)Failed to run substr(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runInstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "instr(str, sub) function accepts only two parameters.\n");
    return false;
  }
  string sRaw, sSub; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sRaw, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sSub, dts, true)){
    trace(DEBUG2,"Searching '%s' in '%s'\n",sSub.c_str(),sRaw.c_str());
    sResult = intToStr(sRaw.find(sSub));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run instr(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runStrlen(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "strlen() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true);
  if (gotResult){
    sResult = intToStr(sResult.length());
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run strlen(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFindnth(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "findnth() function accepts only two or three parameter.\n");
    return false;
  }
  string str, sub, sNth; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, str, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sub, dts, true)){
    int iNth = 1;
    if (m_params.size() == 3 && m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sNth, dts, true) && isInt(sNth)){
      iNth = atoi(sNth.c_str());
    }else{
      trace(ERROR, "Failed to run findnth(%s, %s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
      return false;
    }
    size_t pos = findNthSub(str, sub, iNth<0?str.length()-1:0, iNth<0?iNth*-1:iNth,iNth<0?false:true, "", '\0', {}, true);
    sResult = intToStr(pos==string::npos?-1:(int)pos);
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run findnth(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runComparestr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "comparestr(str1, str2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, str1, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, str2, dts, true)){
    sResult = intToStr(str1.compare(str2));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run comparestr(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runNoCaseComparestr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "nocasecomparestr(str1, str2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, str1, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, str2, dts, true)){
    str1=upper_copy(str1);
    str2=upper_copy(str2);
    sResult = intToStr(str1.compare(str2));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run nocasecomparestr(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runRevertstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "revertstr(str) function accepts only one parameter.\n");
    return false;
  }
  string sStr; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sStr, dts, true)){
    sResult = revertstr(sStr);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run revertstr(%s, %s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runReplace(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 3){
    trace(ERROR, "replace(str, tobereplaced, newstr) function accepts at least three parameters.\n");
    return false;
  }
  if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true)){
    trace(ERROR, "(%s)Failed to run replace()!\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }
  vector<string> vReplace, vNew;
  string sStr;
  for (int i=1;i<m_params.size();i++){
    if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sStr, dts, true)){
      trace(ERROR, "(%d-%s)Failed to run replace()!\n", i,m_params[i].getEntireExpstr().c_str());
      return false;
    }
    if (i%2==1)
      vReplace.push_back(sStr);
    else
      vNew.push_back(sStr);
  }
  if (vNew.size()<vReplace.size()){
    trace(WARNING, "The replace strings does not pair to the new strings, the last one will not be replaced!\n");
    vNew.push_back(vReplace[vReplace.size()-1]);
  }
  replacestr(sResult, vReplace, vNew);
  dts.datatype = STRING;
  return true;
}

bool FunctionC::runRegreplace(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 3){
    trace(ERROR, "regreplace(str, regex_pattern, newstr) function accepts only three parameters.\n");
    return false;
  }
  string sReplace, sNew; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sReplace, dts, true) && m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sNew, dts, true)){
    regreplacestr(sResult, sReplace, sNew);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run regreplace(%s, %s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runRegmatch(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 3){
    trace(ERROR, "regmatch(str, regex_pattern, expr) function accepts only three parameters.\n");
    return false;
  }
  string str, sPattern; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, str, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sPattern, dts, true) && m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true)){
    regmatchstr(str, sPattern, sResult);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run regmatch(%s, %s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runCountword(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2){
    trace(ERROR, "countword(str,[ingnore_quoters]) function accepts only one or two parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sQuoters=""; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sRaw, dts, true)){
    if (m_params.size() == 2){
      if (!m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sQuoters, dts, true)){
        trace(ERROR, "(2)Failed to run countword(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
    }
    std::set<char> delims = {' ','\t','\n','\r',',','.','!','?',';'};
    vector<string> vWords = split(sRaw,delims,sQuoters,'\0',{},true,true);
    sResult = intToStr(vWords.size());
    return true;
  }else{
    trace(ERROR, "(2)Failed to run countword(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runGetword(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "getword(str,wordnum,[ingnore_quoters]) function accepts only two or three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sPos, sQuoters=""; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sRaw, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sPos, dts, true) && isInt(sPos)){
    int iPos = atoi(sPos.c_str());
    if (iPos<=0){
      trace(ERROR, "%s cannot be zero a negative number in getword()!\n", sPos.c_str(), m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() == 3){
      if (!m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sQuoters, dts, true)){
        trace(ERROR, "(1)Failed to run getword(%s,%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
        return false;
      }
    }
    std::set<char> delims = {' ','\t','\n','\r',',','.','!','?',';'};
    vector<string> vWords = split(sRaw,delims,sQuoters,'\0',{},true,true);
    if (vWords.size() == 0){
      trace(ERROR, "No word found in '%s'!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (iPos>0 && iPos<=vWords.size()){
      sResult = vWords[iPos-1];
      return true;
    }else{
      trace(ERROR, "%s is out of range of the word list in '%s'!\n", m_params[1].getEntireExpstr().c_str(), m_params[0].getEntireExpstr().c_str());
      return false;
    }
  }else{
    trace(ERROR, "(2)Failed to run getword(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runGetpart(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 3){
    trace(ERROR, "getpart(str,delimiter,part_index) function accepts only three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sPos, sDelm; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sRaw, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sDelm, dts, true) && m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sPos, dts, true) && isInt(sPos)){
    int iPos = atoi(sPos.c_str());
    if (iPos<=0){
      trace(ERROR, "%s cannot be zero a negative number in getpart()!\n", sPos.c_str(), m_params[0].getEntireExpstr().c_str());
      return false;
    }
    vector<string> vWords = split(sRaw,sDelm,"",'\0',{},true,true);
    if (vWords.size() == 0){
      trace(ERROR, "'%s' cannot be splited by '%s' in getpart()!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
      return false;
    }
    if (iPos>0 && iPos<=vWords.size()){
      sResult = vWords[iPos-1];
      return true;
    }else{
      trace(ERROR, "%s is out of range of the part list in '%s'!\n", m_params[2].getEntireExpstr().c_str(), m_params[0].getEntireExpstr().c_str());
      return false;
    }
  }else{
    trace(ERROR, "(2)Failed to run getpart(%s,%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runCountstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "countstr(str,substr) function accepts only two parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sSub; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sRaw, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sSub, dts, true)){
    sResult = intToStr(countstr(sRaw,sSub));
    return true;
  }else{
    trace(ERROR, "(2)Failed to run countstr(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFieldname(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "fieldname(fieldid) function accepts only one parameter(%d).\n",m_params.size());
    return false;
  }
  string sFieldid; 
  if (m_fieldnames && m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sFieldid, dts, true) && isInt(sFieldid)){
    int iFieldid = atoi(sFieldid.c_str());
    if (!fieldvalues || iFieldid<1 || iFieldid>fieldvalues->size())
      return false;
    if (iFieldid<1 || iFieldid>m_fieldnames->size()){
      trace(ERROR, "%s is out of range of the fields!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    sResult = (*m_fieldnames)[iFieldid-1];
    return true;
  }else{
    trace(ERROR, "Failed to run fieldname(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runConcat(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() <= 1){
    trace(ERROR, "concat() function accepts at least two parameters.\n");
    return false;
  }
  sResult = "";
  DataTypeStruct dts1, dts2;
  if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts1, true)){
    trace(ERROR, "(%d-%s)Failed to run concat()\n",0, m_params[0].getEntireExpstr().c_str());
    return false;
  }
  string scomp;
  for (int i=1;i<m_params.size();i++){
    if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, scomp, dts2, true)){
      trace(ERROR, "(%d-%s)Failed to run concat()\n",i,m_params[i].getEntireExpstr().c_str());
      return false;
    }
    sResult+=scomp;
  }
  return true;
}

bool FunctionC::runConcatcol(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 3){
    trace(ERROR, "concatcol(start,end,expr[,step,delm]) function accepts at least three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sDelm="";
  sResult = "";
  if (m_params.size() >= 5){
    if (!m_params[4].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sDelm, dts, true))
      sDelm = "";
  }
  vector<ExpressionC> vExpandedExpr = expandForeach(fieldvalues->size());
  for (int i=0; i<vExpandedExpr.size(); i++){
    vExpandedExpr[i].analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, sideDatatypes);
    if (!vExpandedExpr[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sRaw, dts, true)){
      trace(ERROR, "(%d-%s)Failed to run concatcol(start,end,expr[,step,delm])!\n",i,vExpandedExpr[i].getEntireExpstr().c_str());
    }
    sResult+=sRaw;
    if (i<vExpandedExpr.size()-1)
      sResult+=sDelm;
  }
  return true;
}

bool FunctionC::runCalcol(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 3){
    trace(ERROR, "calcol(start,end,expr[,step,operation]) function accepts at least three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sOp="SUM";
  short int iOp=SUM;
  sResult = "";
  if (m_params.size() >= 5){
    if (m_params[4].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sOp, dts, true))
      iOp = encodeFunction(upper_copy(trim_copy(sOp)));
  }
  double dSum=0,dVal=0;
  std::set <string> tmpSet;
  vector<ExpressionC> vExpandedExpr = expandForeach(fieldvalues->size());
  for (int i=0; i<vExpandedExpr.size(); i++){
    vExpandedExpr[i].analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, sideDatatypes);
    if (!vExpandedExpr[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sRaw, dts, true) || !isDouble(trim_copy(sRaw))){
      trace(ERROR, "(%d-%s)Failed to run calcol(start,end,expr[,step,operation])!\n",i,vExpandedExpr[i].getEntireExpstr().c_str());
    }
    double dTmp = atof(trim_copy(sRaw).c_str());
    switch (iOp){
      case MAX:
        dVal = i==0?dTmp:max(dTmp,dVal);
        break;
      case MIN:
        dVal = i==0?dTmp:min(dTmp,dVal);
        break;
      case UNIQUECOUNT:
        tmpSet.insert(trim_copy(sRaw));
        break;
      default:
        dSum+=dTmp;
        break;
    }
  }
  switch (iOp){
    case MAX:
    case MIN:
      sResult = doubleToStr(dVal);
      break;
    case AVERAGE:
      sResult = doubleToStr(dSum/(double)vExpandedExpr.size());
      break;
    case UNIQUECOUNT:
      sResult = intToStr(tmpSet.size());
      break;
    case COUNT:
      sResult = intToStr(vExpandedExpr.size());
      break;
    default:
      sResult = doubleToStr(dSum);
      break;
  }
  return true;
}

bool FunctionC::runAppendfile(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "appendfile(content,file) function accepts only two parameters(%d).\n",m_params.size());
    return false;
  }
  string sContent, sFile;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sContent, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sFile, dts, true)){
    sResult = intToStr(appendFile(sContent, sFile));
    return true;
  }else{
    trace(ERROR, "Failed to run appendfile(%s,%s)\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAscii(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "ascii() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true);
  if (sResult.length() == 0){
    trace(ERROR, "ascii() function accepts only one letter as parameter.\n");
    return false;
  }
  if (sResult.length()>1)
    trace(WARNING, "The parameter '%s' of ascii() is too long, will return the ascii code of the first letter only! \n", sResult.c_str());
  if (gotResult){
    dts.datatype = INTEGER;
    sResult = intToStr(int(sResult[0]));
    return true;
  }else{
    trace(ERROR, "Failed to run ascii(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runChar(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "char() function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true) && isInt(sResult)){
    dts.datatype = INTEGER;
    char a = char(atoi(sResult.c_str()));
    sResult.clear();
    sResult.push_back(a);
    return true;
  }else{
    trace(ERROR, "Failed to run char(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runComparenum(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "comparenum(num1, num2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, str1, dts, true) && isDouble(str1) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, str2, dts, true) && isDouble(str2)){
    double num1 = atof(str1.c_str()), num2 = atof(str2.c_str());
    sResult = intToStr(num1>num2?1:(num1==num2?0:-1));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run comparenum(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runComparedate(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "comparedater(date1, date2[, date_format]) function accepts only two or three parameters.\n");
    return false;
  }
  string date1, date2, sFmt; 
  DataTypeStruct dts1, dts2;
  int offSet;
  if (m_params.size() == 3 && m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sFmt, dts, true)){
    dts1.extrainfo = sFmt;
    dts2.extrainfo = sFmt;
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, date1, dts, true) && isDate(date1, offSet, dts1.extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, date2, dts, true) && isDate(date2, offSet, dts2.extrainfo)){
    if (dts1.extrainfo.compare(dts2.extrainfo)!=0){
      trace(ERROR, "Date format %s of %s doesnot match date format %s of %s!\n", dts1.extrainfo.c_str(), m_params[0].getEntireExpstr().c_str(), dts2.extrainfo.c_str(), m_params[1].getEntireExpstr().c_str());
      return false;
    }
    dts1.datatype = DATE;
    sResult = intToStr(anyDataCompare(date1, date2, dts1));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run comparedater(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runMod(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "mod() function accepts only two parameters.\n");
    return false;
  }
  string sBase, sMod;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sBase, dts, true) && isInt(sBase) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sMod, dts, true) && isInt(sMod)){
    dts.datatype = INTEGER;
    sResult = intToStr(atoi(sBase.c_str())%atoi(sMod.c_str()));
    return true;
  }else{
    trace(ERROR, "Failed to run mod(%s, %s)\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAbs(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "abs() function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true) && isDouble(sResult)){
    dts.datatype = DOUBLE;
    double dNum = atof(sResult.c_str());
    sResult = doubleToStr(dNum<0?dNum*-1:dNum);
    return true;
  }else{
    trace(ERROR, "Failed to run abs(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runDatatype(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "datatype() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true);
  if (gotResult){
    string extrainfo;
    if (dts.datatype == UNKNOWN)
      sResult = decodeDatatype(detectDataType(sResult, extrainfo));
    else
      sResult = decodeDatatype(dts.datatype);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run datatype(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runToint(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "toint(str) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true) && isInt(sResult)){
    dts.datatype = INTEGER;
    return true;
  }else{
    trace(ERROR, "Failed to run toint(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTolong(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "tolong(str) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true) && isInt(sResult)){
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run tolong(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTofloat(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "tofloat(str) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true) && isDouble(sResult)){
    dts.datatype = DOUBLE;
    return true;
  }else{
    trace(ERROR, "Failed to run tofloat(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTostr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "tostr(expr) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true)){
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run tostr(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTodate(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2){
    trace(ERROR, "runTodate(str,[dateformat]) function accepts only one or two parameters(%d).\n",m_params.size());
    return false;
  }
  DataTypeStruct dts1;
  int offSet;
  if (m_params.size() != 2 || !m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, dts.extrainfo, dts1, true))
    dts.extrainfo = "";
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true) && isDate(sResult, offSet, dts.extrainfo)){
    return true;
  }else{
    trace(ERROR, "Failed to run runTodate(str,[dateformat])\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runDectohex(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "dectohex(num) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true) && isInt(sResult)){
    sResult = dectohex(atoi(sResult.c_str()));
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run dectohex(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runHextodec(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "hextodec(str) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true)){
    sResult = intToStr(hextodec(sResult));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run dectohex(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runDectobin(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "dectobin(num) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true) && isInt(sResult)){
    sResult = dectobin(atoi(sResult.c_str()));
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run dectohex(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runBintodec(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "bintodec(str) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true)){
    sResult = intToStr(bintodec(sResult));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run dectohex(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFloor(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "floor(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sNum, dts, true) && isDouble(sNum)){
    sResult = intToStr(floor(atof(sNum.c_str())));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run floor(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runCeil(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "ceil(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sNum, dts, true) && isDouble(sNum)){
    sResult = intToStr(ceil(atof(sNum.c_str())));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run ceil(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTimediff(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "timediff(tm1, tm2) function accepts only two parameters.\n");
    return false;
  }
  string sTm1, sTm2; 
  DataTypeStruct dts1, dts2;
  int offSet1, offSet2;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sTm1, dts1, true) && isDate(sTm1, offSet1, dts1.extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sTm2, dts2, true) && isDate(sTm2, offSet2, dts2.extrainfo)){
    struct tm tm1, tm2;
    if (strToDate(sTm1, tm1, offSet1, dts1.extrainfo) && strToDate(sTm2, tm2, offSet2, dts2.extrainfo)){
      //time_t t1 = mktime(&tm1);
      //time_t t2 = mktime(&tm2);
      //sResult = doubleToStr(difftime(t1, t2));
      sResult = doubleToStr(timediff(tm1, tm2));
      dts.datatype = DOUBLE;
      return true;
    }else{
      trace(ERROR, "Failed to run timediff(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
      return false;
    }
  }else{
    trace(ERROR, "Failed to run timediff(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAddtime(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "addtime(date, number, unit) function accepts only two or three parameters.\n");
    return false;
  }
  string sTm, sNum, sUnit; 
  DataTypeStruct dts1, dts2;
  int offSet;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sTm, dts1, true) && isDate(sTm, offSet, dts1.extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sNum, dts2, true) && isInt(sNum)){
    char unit = 'S';
    if (m_params.size() == 3 && m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sUnit, dts2, true)){
      sUnit = upper_copy(sUnit);
      if (sUnit.length() == 1 & (sUnit[0]=='S' || sUnit[0]=='M' || sUnit[0]=='H' || sUnit[0]=='D' || sUnit[0]=='N' || sUnit[0]=='Y'))
        unit = sUnit[0];
      else{
        trace(ERROR, "'%s' is not a valid unit!\n", m_params[2].getEntireExpstr().c_str());
        return false;
      }
    }
    struct tm tm;
    if (strToDate(sTm, tm, offSet, dts1.extrainfo)){
      tm.tm_gmtoff = offSet*36;
      addtime(tm, atoi(sNum.c_str()), unit);
      addhours(tm, offSet/100);
      char buffer [256];
      if (strftime(buffer,256,dts1.extrainfo.c_str(),&tm))
        sResult=string(buffer);
      //sResult = dateToStr(tm, offSet, dts1.extrainfo);
      dts.datatype = DATE;
      return true;
    }else{
      trace(ERROR, "Failed to run addtime(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
      return false;
    }
  }else{
    trace(ERROR, "Failed to run addtime(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runRound(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2){
    trace(ERROR, "round(num) function accepts only one or two parameters.\n");
    return false;
  }
  string sNum;
  int scale=0;
  if (m_params.size() == 2){
    if (m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sNum, dts, true) && isInt(sNum)){
      scale = max(0,atoi(sNum.c_str()));
    }else{
      trace(ERROR, "Failed to run round(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
      return false;
    }
  }
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sNum, dts, true) && isDouble(sNum)){
    char buf[1024];
    int bsize = 0;
    memset( buf, '\0', sizeof(char)*1024 );
    bsize = sprintf(buf, ("%."+intToStr(scale)+"f").c_str(), atof(sNum.c_str()));
    sResult = string(buf);
    dts.datatype = DOUBLE;
    return true;
  }else{
    trace(ERROR, "Failed to run round(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runLog(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "log(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sNum, dts, true) && isDouble(sNum)){
    sResult = doubleToStr(log10(atof(sNum.c_str())));
    dts.datatype = DOUBLE;
    return true;
  }else{
    trace(ERROR, "Failed to run log(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runRandom(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() > 2){
    trace(ERROR, "random([min,][max]) function accepts only one parameter.\n");
    return false;
  }
  string sMin="1", sMax="100";
  if (m_params.size() > 0){
    if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sMax, dts, true) || !isDouble(sMax)){ // the parameter is the maximum range if only one parameter provided
      trace(ERROR, "Failed to run random(%s)!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() > 1){ // the first parameter is the minimum range and the second parameter is the maximum range if two parameters provided
      sMin = sMax;
      if (!m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sMax, dts, true) || !isDouble(sMax)){ 
        trace(ERROR, "Failed to run random(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
    }
  }
  sResult = intToStr(random(atoi(sMin.c_str()),atoi(sMax.c_str())));
  return true;
}

bool FunctionC::runRandstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() > 2){
    trace(ERROR, "runRandstr([min,][max]) function accepts two parameters at most.\n");
    return false;
  }
  string sLen="8", sFlags="uld";
  if (m_params.size() > 0){
    if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sLen, dts, true) || !isDouble(sLen)){ 
      trace(ERROR, "Failed to run runRandstr(%s)!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() > 1){ 
      if (!m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sFlags, dts, true)){ 
        trace(ERROR, "Failed to run runRandstr(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
    }
  }
  sResult = randstr(atoi(sLen.c_str()),sFlags);
  return true;
}

bool FunctionC::runTrimleft(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "trimleft(str[,char][,repeat(1|0)]) function accepts only one or two or three parameters.\n");
    return false;
  }
  string sStr, sChar=" ", sRepeat;
  bool repeat=true;
  if (m_params.size() > 0){
    if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sStr, dts, true)){ 
      trace(ERROR, "Failed to run trimleft(%s)!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() > 1){ 
      if (!m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sChar, dts, true)){ 
        trace(ERROR, "Failed to run trimleft(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
      if (sChar.length()!=1){
        trace(ERROR, "The second parameter of trimleft should only be one char!\n");
        return false;
      }
      if (m_params.size() > 2){ 
        if (!m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sRepeat, dts, true)){ 
          trace(ERROR, "Failed to run trimleft(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
          return false;
        }
        repeat=(sRepeat.compare("0")!=0);
      }
    }
  }
  sResult = trim_left(sStr, sChar[0], repeat);
  return true;
}

bool FunctionC::runTrimright(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "trimright(str[,char][,repeat(1|0)]) function accepts only one or two or three parameters.\n");
    return false;
  }
  string sStr, sChar=" ", sRepeat;
  bool repeat=true;
  if (m_params.size() > 0){
    if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sStr, dts, true)){ 
      trace(ERROR, "Failed to run trimright(%s)!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() > 1){ 
      if (!m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sChar, dts, true)){ 
        trace(ERROR, "Failed to run trimright(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
      if (sChar.length()!=1){
        trace(ERROR, "The second parameter of trimright should only be one char!\n");
        return false;
      }
      if (m_params.size() > 2){ 
        if (!m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sRepeat, dts, true)){ 
          trace(ERROR, "Failed to run trimleft(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
          return false;
        }
        repeat=(sRepeat.compare("0")!=0);
      }
    }
  }
  sResult = trim_right(sStr, sChar[0], repeat);
  return true;
}

bool FunctionC::runTrim(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "trim(str[,char][,repeat(1|0)]) function accepts only one or two or three parameters.\n");
    return false;
  }
  string sStr, sChar=" ", sRepeat;
  bool repeat=true;
  if (m_params.size() > 0){
    if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sStr, dts, true)){ 
      trace(ERROR, "Failed to run trim(%s)!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() > 1){ 
      if (!m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sChar, dts, true)){ 
        trace(ERROR, "Failed to run trim(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
      if (sChar.length()!=1){
        trace(ERROR, "The second parameter of trim should only be one char!\n");
        return false;
      }
      if (m_params.size() > 2){ 
        if (!m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sRepeat, dts, true)){ 
          trace(ERROR, "Failed to run trimleft(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
          return false;
        }
        repeat=(sRepeat.compare("0")!=0);
      }
    }
  }
  sResult = trim(sStr, sChar[0], repeat);
  return true;
}

bool FunctionC::runCamelstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "camelstr(str) function accepts only one or two parameters.\n");
    return false;
  }
  string sStr;
  if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sStr, dts, true)){ 
    trace(ERROR, "Failed to run camelstr(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
  sResult = camelstr(sStr);
  return true;
}

bool FunctionC::runSnakestr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "snakestr(str) function accepts only one or two parameters.\n");
    return false;
  }
  string sStr;
  if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sStr, dts, true)){ 
    trace(ERROR, "Failed to run snakestr(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
  sResult = snakestr(sStr);
  return true;
}

bool FunctionC::runTruncdate(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "truncdate(date, fmt) function accepts only two parameters.\n");
    return false;
  }
  string sTm, sSeconds;
  DataTypeStruct tmpDts;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sTm, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sSeconds, tmpDts, true) && isInt(sSeconds)){
    int iSeconds = atoi(sSeconds.c_str());
    sResult = truncdate(sTm, dts.extrainfo, iSeconds);
    dts.datatype = DATE;
    //trace(DEBUG, "Truncating seconds %d from '%s'(%u) get '%s'(%u), format:%s\n", iSeconds, sTm.c_str(), (long)t1, sResult.c_str(), (long)t2, dts.extrainfo.c_str());
    return !sResult.empty();
  }else{
    trace(ERROR, "Failed to run truncdate(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }  
}

bool FunctionC::runDateformat(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2){
    trace(ERROR, "dateformat(date[, fmt]) function accepts only one or two parameters.\n");
    return false;
  }
  string sTm, sFmt;
  DataTypeStruct tmpDts;
  int iOffSet;
  if (m_params.size() == 2 && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sFmt, tmpDts, true))
    dts.extrainfo = sFmt;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sTm, dts, true) && isDate(sTm, iOffSet, dts.extrainfo)){
    struct tm tm;
    if (strToDate(sTm, tm, iOffSet, dts.extrainfo)){
      sResult = dateToStr(tm, iOffSet, m_params.size()==1?dts.extrainfo:sFmt);
      dts.datatype = STRING;
      return !sResult.empty();
    }else{
      trace(ERROR, "(1)Failed to run dateformat(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
      return false;
    }
  }else{
    trace(ERROR, "(2)Failed to run dateformat(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), dts.extrainfo.c_str());
    return false;
  }  
}

bool FunctionC::runNow(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 0){
    trace(ERROR, "now() function does not accept any parameter.\n");
    return false;
  }
  struct tm curtime = now();
  sResult = dateToStr(curtime);
  dts.datatype = DATE;
  return true;
}

// switch(input,case1,return1[,case2,result2...][,default]): if input equal to case1, then return return1, etc.. If none matched, return default or return input if no default provided.
bool FunctionC::runSwitch(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() <= 2){
    trace(ERROR, "switch() function accepts at least three parameters.\n");
    return false;
  }
  DataTypeStruct dts1, dts2, dts3;
  if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts1, true)){
    trace(ERROR, "(0)Eval expression '%s' failed.\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }
  string scase,sreturn;
  for (int i=1;i<m_params.size();i++){
    if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, scase, dts2, true)){
      trace(ERROR, "(1)Eval expression '%s' failed.\n",m_params[i].getEntireExpstr().c_str());
      return false;
    }
    if (i+1<m_params.size()){
      if (!m_params[i+1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sreturn, dts, true)){
        trace(ERROR, "(2)Eval expression '%s' failed.\n",m_params[i+1].getEntireExpstr().c_str());
        return false;
      }
      i++;
    }else{
      //trace(DEBUG2,"Retruning default '%s'\n",scase.c_str());
      sResult = scase;
      dts = dts2;
      return true;
    }
    //trace(DEBUG2,"Comparing '%s' '%s'\n",sResult.c_str(),scase.c_str());
    dts = getCompatibleDataType(dts1, dts2);
    if (dts.datatype == ANY)
      dts.datatype = STRING;
    if (anyDataCompare(sResult,scase,dts) == 0){
      sResult = sreturn;
      dts = dts3;
      return true;
    }
  }
  //trace(DEBUG2,"Retruning original '%s'\n",sResult.c_str());
  dts = dts1;
  return true;
}

// when(condition1,return1[,condition1,result2...],else): if input equal to case1, then return return1, etc.. If none matched, return default or return input if no default provided.
bool FunctionC::runWhen(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 2 || m_filters.size()<1){
    trace(ERROR, "when() function accepts at least three parameters.\n");
    return false;
  }
  if (m_params.size() != m_filters.size() + 1){
    trace(ERROR, "Return expression number (%d) does not match condtion number (%d) in when() function!\n", m_params.size(), m_filters.size());
    return false;
  }
  vector< vector< unordered_map<string,string> > > sideDatasets;
  unordered_map< int,int > sideMatchedRowIDs;
  for (int i=0; i<m_filters.size(); i++){
    if (m_filters[i].compareExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, &sideDatasets, sideDatatypes, sideMatchedRowIDs) && m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true))
      return true;
  }
  if (m_params[m_params.size()-1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true))
    return true;
  else{
    trace(ERROR, "Failed to run when()!\n");
    return false;
  }
}

bool FunctionC::runPad(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "pad() function accepts only two parameters.\n");
    return false;
  }
  string seed, sLen;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, seed, dts, true) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sLen, dts, true)){
    if (!isInt(sLen))
      return false;
    sResult = "";
    for (int i=0;i<atoi(sLen.c_str());i++)
      sResult.append(seed);
    dts.datatype = STRING;
    return true;
  }else
    return false;
}

bool FunctionC::runGreatest(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 1){
    trace(ERROR, "greatest() function accepts at least one parameter.\n");
    return false;
  }
  string scomp;
  DataTypeStruct dts1, dts2;
  for (int i=0;i<m_params.size();i++){
    if (m_params[i].m_type == LEAF && m_params[i].m_expType == FUNCTION && m_params[i].m_Function && m_params[i].m_Function->m_funcID==FOREACH){
      vector<ExpressionC> vExpandedExpr = m_params[i].m_Function->expandForeach(fieldvalues->size());
      for (int j=0; j<vExpandedExpr.size(); j++){
        vExpandedExpr[j].analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, sideDatatypes);
        if (!vExpandedExpr[j].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, scomp, dts2, true)){
          trace(ERROR, "(%d-%d-%s)Failed to run greatest()!\n",i,j,vExpandedExpr[i].getEntireExpstr().c_str());
          return false;
        }
        if (i==0 && j==0){
          dts1 = dts2;
          sResult = scomp;
        }else{
          dts = getCompatibleDataType(dts1, dts2);
          if (dts.datatype == ANY || dts.datatype == UNKNOWN)
            dts.datatype = STRING;
          
          //trace(DEBUG2,"Comparing '%s'(%d) '%s'(%d) => %d \n",scomp.c_str(),dts2.datatype,sResult.c_str(),dts1.datatype,anyDataCompare(scomp,sResult,dts));
          if (anyDataCompare(scomp,sResult,dts)>0)
            sResult = scomp;
        }
      }
    }else{
      if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, scomp, dts2, true)){
        trace(ERROR, "greatest() function failed to get the value of '%s'.\n",m_params[i].getEntireExpstr().c_str());
        return false;
      }
      if (i==0){
        dts1 = dts2;
        sResult = scomp;
      }else{
        dts = getCompatibleDataType(dts1, dts2);
        if (dts.datatype == ANY || dts.datatype == UNKNOWN)
          dts.datatype = STRING;
        
        //trace(DEBUG2,"Comparing '%s'(%d) '%s'(%d) => %d \n",scomp.c_str(),dts2.datatype,sResult.c_str(),dts1.datatype,anyDataCompare(scomp,sResult,dts));
        if (anyDataCompare(scomp,sResult,dts)>0)
          sResult = scomp;
      }
    }
  }

  dts.datatype = ANY;
  return true;
}

bool FunctionC::runLeast(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 1){
    trace(ERROR, "least() function accepts at least one parameter.\n");
    return false;
  }
  string scomp;
  DataTypeStruct dts1, dts2;
  for (int i=0;i<m_params.size();i++){
    if (m_params[i].m_type == LEAF && m_params[i].m_expType == FUNCTION && m_params[i].m_Function && m_params[i].m_Function->m_funcID==FOREACH){
      vector<ExpressionC> vExpandedExpr = m_params[i].m_Function->expandForeach(fieldvalues->size());
      for (int j=0; j<vExpandedExpr.size(); j++){
        vExpandedExpr[j].analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, sideDatatypes);
        if (!vExpandedExpr[j].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, scomp, dts2, true)){
          trace(ERROR, "(%d-%d-%s)Failed to run least()!\n",i,j,vExpandedExpr[i].getEntireExpstr().c_str());
          return false;
        }
        if (i==0 && j==0){
          dts1 = dts2;
          sResult = scomp;
        }else{
          dts = getCompatibleDataType(dts1, dts2);
          if (dts.datatype == ANY || dts.datatype == UNKNOWN)
            dts.datatype = STRING;
          
          //trace(DEBUG2,"Comparing '%s'(%d) '%s'(%d) => %d \n",scomp.c_str(),dts2.datatype,sResult.c_str(),dts1.datatype,anyDataCompare(scomp,sResult,dts));
          if (anyDataCompare(scomp,sResult,dts)<0)
            sResult = scomp;
        }
      }
    }else{
      if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, scomp, dts2, true)){
        trace(ERROR, "least() function failed to get the value of '%s'.\n",m_params[i].getEntireExpstr().c_str());
        return false;
      }
      if (i==0){
        dts1 = dts2;
        sResult = scomp;
      }else{
        dts = getCompatibleDataType(dts1, dts2);
        if (dts.datatype == ANY || dts.datatype == UNKNOWN)
          dts.datatype = STRING;
        
        //trace(DEBUG2,"Comparing '%s'(%d) '%s'(%d) => %d \n",scomp.c_str(),dts2.datatype,sResult.c_str(),dts1.datatype,anyDataCompare(scomp,sResult,dts));
        if (anyDataCompare(scomp,sResult,dts)<0)
          sResult = scomp;
      }
    }
  }

  dts.datatype = ANY;
  return true;
}

bool FunctionC::runSumall(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 1){
    trace(ERROR, "sumall() function accepts at least one parameter.\n");
    return false;
  }
  string scomp;
  double dResult=0;
  DataTypeStruct dts1, dts2;
  for (int i=0;i<m_params.size();i++){
    if (m_params[i].m_type == LEAF && m_params[i].m_expType == FUNCTION && m_params[i].m_Function && m_params[i].m_Function->m_funcID==FOREACH){
      vector<ExpressionC> vExpandedExpr = m_params[i].m_Function->expandForeach(fieldvalues->size());
      for (int j=0; j<vExpandedExpr.size(); j++){
        vExpandedExpr[j].analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, sideDatatypes);
        if (!vExpandedExpr[j].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, scomp, dts2, true) || !isDouble(scomp)){
          trace(ERROR, "(%d-%d-%s)Failed to run sumall()!\n",i,j,vExpandedExpr[i].getEntireExpstr().c_str());
          return false;
        }
        if (i==0 && j==0){
          dResult = atof(scomp.c_str());
        }else{
          dResult += atof(scomp.c_str());
        }
      }
    }else{
      if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, scomp, dts2, true) || !isDouble(scomp)){
        trace(ERROR, "sumall() function failed to get the value of '%s'.\n",m_params[i].getEntireExpstr().c_str());
        return false;
      }
      if (i==0){
        dResult = atof(scomp.c_str());
      }else{
        dResult += atof(scomp.c_str());
      }
    }
  }

  dts.datatype = DOUBLE;
  sResult = doubleToStr(dResult);
  return true;
}

// expand foreach to a vector of expression
// foreach(beginid,endid,macro_expr[,step]). $ stands for field, # stands for field sequence, % stands for the largest field sequence ID.
vector<ExpressionC> FunctionC::expandForeach(int maxFieldNum)
{
  vector<ExpressionC> vExpr;
  trace(DEBUG,"(1)Expanding foreach expression '%s'\n",m_expStr.c_str());
  if (m_funcID!=FOREACH && m_funcID!=ANYCOL && m_funcID!=ALLCOL && m_funcID!=CONCATCOL && m_funcID!=CALCOL){
    trace(ERROR, "(1)'%s' is not foreach macro function!\n", m_funcName.c_str());
    return vExpr;
  }
  if (m_params.size()<3){
    trace(ERROR, "(1)Foreach macro function requires at least 3 parameters!\n", m_funcName.c_str());
    return vExpr;
  }
  int begin = 0, end = 0;
  if (m_params[0].getEntireExpstr().find("%") != string::npos){
    string expStr = m_params[0].getEntireExpstr();
    replacestr(expStr,"%",intToStr(maxFieldNum));
    ExpressionC constExp(expStr);
    if (!isInt(constExp.m_expStr)){
      trace(ERROR, "(1a)'%s' is not a valid number expresion!\n", m_params[0].getEntireExpstr().c_str());
      return vExpr;
    }
    begin = max(1,atoi(constExp.m_expStr.c_str()));
  }else if (isInt(m_params[0].m_expStr))
    begin = max(1,min(maxFieldNum,atoi(m_params[0].m_expStr.c_str())));
  else{
    trace(ERROR, "(1)%s is an invalid begin ID for foreach macro function!\n", m_params[0].getEntireExpstr().c_str());
    return vExpr;
  }
  if (m_params[1].getEntireExpstr().find("%") != string::npos){
    string expStr = m_params[1].getEntireExpstr();
    replacestr(expStr,"%",intToStr(maxFieldNum));
    ExpressionC constExp(expStr);
    if (!isInt(constExp.m_expStr)){
      trace(ERROR, "(1b)'%s' is not a valid number expresion!\n", m_params[1].getEntireExpstr().c_str());
      return vExpr;
    }
    end = max(1,atoi(constExp.m_expStr.c_str()));
  }else if (isInt(m_params[1].m_expStr))
    end = max(1,min(maxFieldNum,atoi(m_params[1].m_expStr.c_str())));
  else{
    trace(ERROR, "(1)Invalid end ID for foreach macro function!\n", m_params[1].getEntireExpstr().c_str());
    return vExpr;
  }
  int iStep = 1;
  if (m_params.size()>3 && isInt(trim_copy(m_params[3].m_expStr))) // step
    iStep = atoi(trim_copy(m_params[3].m_expStr).c_str());
  iStep = iStep==0?1:iStep; // step cannot be 0
  iStep = iStep*(begin<end?(iStep<0?-1:1):(iStep<0?1:-1)); // if begin less than end, step should be a positive number, otherwise, it is a negative number
  for (int i=begin; begin<end?i<=end:i>=end; i+=iStep){
    trace(DEBUG,"(1)Expanding foreach element '%s'\n",m_params[2].getEntireExpstr().c_str());
    string sNew = m_params[2].getEntireExpstr();
    replaceunquotedstr(sNew,"$","@field"+intToStr(i),"''",'\\',{'(',')'});
    replaceunquotedstr(sNew,"#",intToStr(i),"''",'\\',{'(',')'});
    ExpressionC expr(sNew);
    vExpr.push_back(expr);
    trace(DEBUG,"(1)Expanded foreach element '%s'\n",expr.getEntireExpstr().c_str());
  }
  return vExpr;
}

vector<ExpressionC> FunctionC::expandForeach(vector<ExpressionC> vExps)
{
  vector<ExpressionC> vExpr;
  trace(DEBUG,"(2)Expanding foreach expression '%s'\n",m_expStr.c_str());
  if (m_funcID!=FOREACH && m_funcID!=ANYCOL && m_funcID!=ALLCOL){
    trace(ERROR, "(2)'%s' is not foreach macro function!\n", m_funcName.c_str());
    return vExpr;
  }
  if (m_params.size()<3){
    trace(ERROR, "(2)Foreach macro function requires 3 parameters!\n", m_funcName.c_str());
    return vExpr;
  }
  int begin = 0, end = 0;
    if (m_params[0].getEntireExpstr().find("%") != string::npos){
    string expStr = m_params[0].getEntireExpstr();
    replacestr(expStr,"%",intToStr(vExps.size()));
    ExpressionC constExp(expStr);
    if (!isInt(constExp.m_expStr)){
      trace(ERROR, "(2a)'%s' is not a valid number expresion!\n", m_params[0].getEntireExpstr().c_str());
      return vExpr;
    }
    begin = max(1,atoi(constExp.m_expStr.c_str()));
  }else if (isInt(m_params[0].m_expStr))
    begin = max(1,min((int)vExps.size(),atoi(m_params[0].m_expStr.c_str())));
  else{
    trace(ERROR, "(2)%s is an invalid begin ID for foreach macro function!\n", m_params[0].getEntireExpstr().c_str());
    return vExpr;
  }
  if (m_params[1].getEntireExpstr().find("%") != string::npos){
    string expStr = m_params[1].getEntireExpstr();
    replacestr(expStr,"%",intToStr(vExps.size()));
    ExpressionC constExp(expStr);
    if (!isInt(constExp.m_expStr)){
      trace(ERROR, "(2b)'%s' is not a valid number expresion!\n", m_params[1].getEntireExpstr().c_str());
      return vExpr;
    }
    end = atoi(constExp.m_expStr.c_str());;
  }else if (isInt(m_params[1].m_expStr))
    end = max(0,min((int)vExps.size(),atoi(m_params[1].m_expStr.c_str())));
  else{
    trace(ERROR, "(2)Invalid end ID for foreach macro function!\n", m_params[1].getEntireExpstr().c_str());
    return vExpr;
  }
  for (int i=begin; begin<end?i<=end:i>=end; begin<end?i++:i--){
    trace(DEBUG,"(2)Expanding foreach element '%s'\n",m_params[2].getEntireExpstr().c_str());
    string sNew = m_params[2].getEntireExpstr();
    replaceunquotedstr(sNew,"$",vExps[i-1].getEntireExpstr(),"''",'\\',{'(',')'});
    replaceunquotedstr(sNew,"#",intToStr(i),"''",'\\',{'(',')'});
    ExpressionC expr(sNew);
    vExpr.push_back(expr);
    trace(DEBUG,"(2)Expanded foreach element '%s'\n",expr.getEntireExpstr().c_str());
  }
  return vExpr;
}

// run function and get result
bool FunctionC::runFunction(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts)
{
  bool getResult = false;
    switch(m_funcID){
    case UPPER:
      getResult = runUpper(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case LOWER:
      getResult = runLower(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case SUBSTR:
      getResult = runSubstr(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case FLOOR:
      getResult = runFloor(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case CEIL:
      getResult = runCeil(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case TIMEDIFF:
      getResult = runTimediff(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case ADDTIME:
      getResult = runAddtime(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case ISNULL:
      getResult = runIsnull(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case INSTR:
      getResult = runInstr(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case STRLEN:
      getResult = runStrlen(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case COMPARESTR:
      getResult = runComparestr(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case COMPARENUM:
      getResult = runComparenum(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case COMPAREDATE:
      getResult = runComparedate(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case NOCASECOMPARESTR:
      getResult = runNoCaseComparestr(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case REPLACE:
      getResult = runReplace(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case REGREPLACE:
      getResult = runRegreplace(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case REGMATCH:
      getResult = runRegmatch(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case COUNTWORD:
      getResult = runCountword(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case COUNTSTR:
      getResult = runCountstr(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case FIELDNAME:
      getResult = runFieldname(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case CONCAT:
      getResult = runConcat(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case CONCATCOL:
      getResult = runConcatcol(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case CALCOL:
      getResult = runCalcol(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case APPENDFILE:
      getResult = runAppendfile(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case GETWORD:
      getResult = runGetword(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case GETPART:
      getResult = runGetpart(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case RANDSTR:
      getResult = runRandstr(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case TRIMLEFT:
      getResult = runTrimleft(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case TRIMRIGHT:
      getResult = runTrimright(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case TRIM:
      getResult = runTrim(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case PAD:
      getResult = runPad(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case CAMELSTR:
      getResult = runCamelstr(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case SNAKESTR:
      getResult = runSnakestr(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case REVERTSTR:
      getResult = runRevertstr(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case FINDNTH:
      getResult = runFindnth(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case ASCII:
      getResult = runAscii(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case CHAR:
      getResult = runChar(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case MOD:
      getResult = runMod(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case ABS:
      getResult = runAbs(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case DATATYPE:
      getResult = runDatatype(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case TOINT:
      getResult = runToint(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case TOLONG:
      getResult = runTolong(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case TOFLOAT:
      getResult = runTofloat(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case TOSTR:
      getResult = runTostr(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case TODATE:
      getResult = runTodate(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case DECTOHEX:
      getResult = runDectohex(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case HEXTODEC:
      getResult = runHextodec(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case DECTOBIN:
      getResult = runDectobin(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case BINTODEC:
      getResult = runBintodec(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case SWITCH:
      getResult = runSwitch(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case WHEN:
      getResult = runWhen(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case GREATEST:
      getResult = runGreatest(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case LEAST:
      getResult = runLeast(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case SUMALL:
      getResult = runSumall(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case ROUND:
      getResult = runRound(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case LOG:
      getResult = runLog(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case RANDOM:
      getResult = runRandom(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case DATEFORMAT:
      getResult = runDateformat(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case TRUNCDATE:
      getResult = runTruncdate(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case NOW:
      getResult = runNow(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts);
      break;
    case ROOT:
    case PATH:
    case PARENT:
      getResult=true;
      break;
    case SUM:
    case COUNT:
    case UNIQUECOUNT:
    case MAX:
    case MIN:
    case AVERAGE: {// aggregation function eval parameter expression only!
      if (m_params.size() != 1){
        trace(ERROR, "Aggregation (%s) function accepts only one parameter.\n", m_funcName.c_str());
        return false;
      }
      if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true))
        return true;
      else{
        trace(ERROR, "Failed to eval aggregation (%s) function parameter (%s).\n", m_funcName.c_str(),m_params[0].getEntireExpstr().c_str());
        return false;
      }
      break;
    }
    case GROUPLIST:
    case RANK:
    case DENSERANK:
    case NEARBY:
    case PREVIOUS:
    case NEXT:
    case SUMA:
    case COUNTA:
    case UNIQUECOUNTA:
    case MAXA:
    case MINA:
    case AVERAGEA:
    case SEQNUM:{
      for (int i=0; i<m_params.size(); i++)
        if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sideDatarow, sideDatatypes, sResult, dts, true)){
          trace(ERROR, "Failed to eval function (%s) N.O. %d parameter (%s).\n", m_funcName.c_str(),i,m_params[i].getEntireExpstr().c_str());
          return false;
        }
      break;
    }
    case FOREACH:
    case COLTOROW:
      break;
      //trace(ERROR, "Internal error: Macro function '%s' is not parsed yet!\n", m_funcName.c_str());
      //return false;
    default:{
      trace(ERROR, "Function(2) '%s' is not supported yet!\n", m_funcName.c_str());
      return false;
    }
  }

  return getResult;
}
