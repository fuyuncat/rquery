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

FunctionC::FunctionC(const string & expStr)
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

void FunctionC::setExpStr(const string & expStr)
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

bool FunctionC::isConst()
{
  if (isAggFunc() || m_funcID==RCOUNT || m_funcID==RANDOM || m_funcID==RANDSTR || (m_funcID==NOW && m_params.size()>0)) // RCOUNT/RANDOM accept 0 parameter, RANDSTR accept const parameter, unlike NOW(), it cannot be counted as a const function
    return false;
  if (m_expstrAnalyzed){
    for (size_t i=0; i<m_params.size(); i++){
      if (m_params[i].m_expType != CONST)
        return false;
    }
    for (size_t i=0; i<m_filters.size(); i++){
      if (!m_filters[i].isConst())
        return false;
    }
    return true;
  }
  return false;
}

bool FunctionC::isAggFunc()
{
  return (m_funcID==SUM || m_funcID==COUNT || m_funcID==UNIQUECOUNT || m_funcID==MAX || m_funcID==MIN || m_funcID==AVERAGE || m_funcID==GROUPLIST);
}

bool FunctionC::isMacro()
{
  return (m_funcID==FOREACH || m_funcID==ANYCOL || m_funcID==ALLCOL);
}

bool FunctionC::isAnalytic()
{
  return (m_funcID==RANK || m_funcID==DENSERANK || m_funcID==PREVIOUS || m_funcID==NEXT || m_funcID==NEARBY || m_funcID==SEQNUM || m_funcID==SUMA || m_funcID==COUNTA || m_funcID==UNIQUECOUNTA || m_funcID==MAXA || m_funcID==MINA || m_funcID==AVERAGEA);
}

bool FunctionC::isTree()
{
  return (m_funcID==ROOT || m_funcID==PATH || m_funcID==PARENT);
}

bool FunctionC::containRefVar()
{
  for (size_t i=0;i<m_params.size();i++)
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
  string strParams = readQuotedStr(m_expStr, iPos, "()", "''", '\0', {});
  //if (iPos<0 || strParams.empty()){
  //  trace(ERROR, "No quoted parameters found in '%s'!\n", m_expStr.c_str());
  //  m_expstrAnalyzed = false;
  //  return false;
  //}
  m_funcName = trim_copy(upper_copy(m_expStr.substr(0, m_expStr.find("("))));
  m_funcID = encodeFunction(m_funcName);
  m_expStr = m_funcName+"("+strParams+")";
  //strParams = trim_pair(strParams, "()");
  vector<string> vParams;
  if (isAnalytic()){ // analyze analytic function parameters: (group1[;group2]...[,sort1 [asc|desc][;sort2 [asc|desc]]...]). sParam of analytic function can be empty
    split(vParams,strParams,';',"''()",'\\',{'(',')'},false,false);
    if (vParams.size()<2){
      trace(ERROR, "There should be at least two part parameters for an analytic function '%s', sort key is compulsory, for example rank(;sortkey)!\n",m_funcName.c_str());
      m_expstrAnalyzed = false;
      return false;
    }
    for (size_t i=0; i<vParams.size(); i++){
      vector<string> vAnaPara;
      split(vAnaPara,trim_copy(vParams[i]),',',"''()",'\\',{'(',')'},false,true);
      m_anaParaNums.push_back(vAnaPara.size());
      for (size_t j=0;j<vAnaPara.size();j++){
        if (i==1){// The second part should always be sort keys for ALL analytic functions.
          vector<string> vSortPara;
          split(vSortPara,trim_copy(vAnaPara[j]),' ',"''()",'\\',{'(',')'},true,true);
          m_params.push_back(ExpressionC(trim_copy(vSortPara[0])));
          if (vSortPara.size()>1){ // sort direction; 1:asc;-1:desc
            m_params.push_back(ExpressionC(upper_copy(trim_copy(vSortPara[1])).compare("DESC")==0?"-1":"1"));
          }else{
            m_params.push_back(ExpressionC("1"));
          }
        }else{
          m_params.push_back(ExpressionC(trim_copy(vAnaPara[j])));
        }
      }
    }
    trace(DEBUG, "FunctionC: The analytic function '%s' group size is %d, param size %d \n", m_expStr.c_str(), m_anaParaNums[0], m_params.size());
  }else{
    split(vParams,strParams,',',"''()",'\\',{'(',')'},false,true);
    for (size_t i=0; i<vParams.size(); i++){
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
            vector<string> vGLExprPara;
            split(vGLExprPara,trim_copy(sParam),' ',"''()",'\\',{'(',')'},true,true);
            if (vGLExprPara.size() == 2){
              if (upper_copy(trim_copy(vGLExprPara[0])).compare("DISTINCT") == 0)
                m_bDistinct = true;
              else
                trace(WARNING, "'%s' is not a correct keywork, do you mean DISTINCT?\n", vGLExprPara[0].c_str());
              sParam = trim_copy(vGLExprPara[1]);
            }
            m_params.push_back(ExpressionC(sParam));
          }else if(i == 1){ // delimiter for GROUPLIST
            m_params.push_back(ExpressionC(sParam));
            if (m_params[m_params.size()-1].m_type!=LEAF || m_params[m_params.size()-1].m_expType!=CONST){
              trace(WARNING, "delimiter for GROUPLIST only accept const, '%s' is not a const. Will use a SPACE as delimiter!\n", sParam.c_str());
              m_params[m_params.size()-1] = ExpressionC(" ");
            }
          }else{
            m_params.push_back(ExpressionC(sParam));
          }
        }else{
          m_params.push_back(ExpressionC(sParam));
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
    case GETPARTS:
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
    case EXEC:
    case DETECTDT:
    case REGGET:
    case URLENCODE:
    case URLDECODE:
    case BASE64ENCODE:
    case BASE64DECODE:
    case MD5:
    case HASH:
    case MYIPS:
    case HOSTNAME:
    case RMEMBERS:
    case FPCLASSIFY:
    case FILEATTRS:
    case GETSYMBLINK:
    case DUPLICATE:
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
    case RCOUNT:
    case RMEMBERID:
    case ISLONG:
    case ISDOUBLE:
    case ISDATE:
    case ISSTRING:
    case COUNTPART:
    case REGCOUNT:
    case ISLEAP:
    case WEEKDAY:
    case YEARWEEK:
    case YEARDAY:
    case DATETOLONG:
    case ISIP:
    case ISIPV6:
    case ISMAC:
    case ISFILE:
    case ISFOLDER:
    case FILEEXIST:
    case RMFILE:
    case RENAMEFILE:
    case FILESIZE:
    case ISFINITE:
    case ISINF:
    case ISNORMAL:
    case ISSYMBLINK:
    case ISEXECUTABLE:
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
    case ACOS:
    case ACOSH:
    case ASIN:
    case ASINH:
    case ATAN:
    case ATAN2:
    case ATANH:
    case CBRT:
    case COPYSIGN:
    case COS:
    case COSH:
    case ERF:
    case EXP:
    case EXP2:
    case FMA:
    case FMOD:
    case HYPOT:
    case ILOGB:
    case LGAMMA:
    case LOG10:
    case LOG2:
    case POW:
    case REMAINDER:
    case SCALBLN:
    case SCALBN:
    case SIN:
    case SINH:
    case SQRT:
    case TAN:
    case TANH:
    case TGAMMA:
    case PI:
      m_datatype.datatype = DOUBLE;
      break;
    case NOW:
    case TRUNCDATE:
    case TRUNCDATEU:
    case ZONECONVERT:
    case ADDTIME:
    case TODATE:
    case MONTHFIRSTDAY:
    case MONTHFIRSTMONDAY:
    case LONGTODATE:
    case LOCALTIME:
    case GMTIME:
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
    case EVAL:
    case RMEMBER:
    case USERMACROFUNC:
      m_datatype.datatype = ANY;
      break;
    default:{
      trace(FATAL, "Function(1) '%s' is not supported yet!\n", m_funcName.c_str());
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
  for (size_t i=0; i<m_params.size(); i++){
    m_params[i].analyzeColumns(m_fieldnames, m_fieldtypes, rawDatatype, sideDatatypes);
    //trace(DEBUG2, "Analyzing parameter '%s' in function '%s' (%d)\n", m_params[i].getEntireExpstr().c_str(), m_expStr.c_str(),m_params[i].columnsAnalyzed());
  }
  for (size_t i=0; i<m_filters.size(); i++){
    m_filters[i].analyzeColumns(m_fieldnames, m_fieldtypes, rawDatatype, sideDatatypes);
    //trace(DEBUG2, "Analyzing filter '%s' in function '%s' (%d)\n", m_filters[i].m_expStr.c_str(), m_expStr.c_str(),m_filters[i].columnsAnalyzed());
  }
  if (m_funcID == TRUNCDATE || m_funcID == TRUNCDATEU || m_funcID==GROUPLIST)
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

bool FunctionC::runIsnull(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isnull() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(rds, sResult, dts, true);
  if (gotResult){
    dts.datatype = INTEGER;
    sResult = intToStr(sResult.length()==0?1:0);
    return true;
  }else
    return false;
}

bool FunctionC::runUpper(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "Upper() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(rds, sResult, dts, true);
  if (gotResult){
    sResult = upper_copy(sResult);
    dts.datatype = STRING;
    return true;
  }else
    return false;
}

bool FunctionC::runLower(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "lower() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(rds, sResult, dts, true);
  if (gotResult){
    sResult = lower_copy(sResult);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run lower(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runSubstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "substr(str, startPos, len) function accepts only two or three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sPos, sLen; 
  if (m_params[0].evalExpression(rds, sRaw, dts, true) && m_params[1].evalExpression(rds, sPos, dts, true) && isInt(sPos)){
    int iPos = atoi(sPos.c_str());
    if (iPos<0 || iPos>=sRaw.length()){
      trace(ERROR, "%s is out of length of %s!\n", sPos.c_str(), m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() == 3){
      if (m_params[2].evalExpression(rds, sLen, dts, true) && isInt(sLen)){
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

bool FunctionC::runInstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "instr(str, sub) function accepts only two parameters.\n");
    return false;
  }
  string sRaw, sSub; 
  if (m_params[0].evalExpression(rds, sRaw, dts, true) && m_params[1].evalExpression(rds, sSub, dts, true)){
    trace(DEBUG2,"Searching '%s' in '%s'\n",sSub.c_str(),sRaw.c_str());
    sResult = intToStr(sRaw.find(sSub));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run instr(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runStrlen(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "strlen() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(rds, sResult, dts, true);
  if (gotResult){
    sResult = intToStr(sResult.length());
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run strlen(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFindnth(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "findnth() function accepts only two or three parameter.\n");
    return false;
  }
  string str, sub, sNth; 
  if (m_params[0].evalExpression(rds, str, dts, true) && m_params[1].evalExpression(rds, sub, dts, true)){
    int iNth = 1;
    if (m_params.size() == 3 && m_params[2].evalExpression(rds, sNth, dts, true) && isInt(sNth)){
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

bool FunctionC::runComparestr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "comparestr(str1, str2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(rds, str1, dts, true) && m_params[1].evalExpression(rds, str2, dts, true)){
    sResult = intToStr(str1.compare(str2));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run comparestr(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runNoCaseComparestr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "nocasecomparestr(str1, str2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(rds, str1, dts, true) && m_params[1].evalExpression(rds, str2, dts, true)){
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

bool FunctionC::runRevertstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "revertstr(str) function accepts only one parameter.\n");
    return false;
  }
  string sStr; 
  if (m_params[0].evalExpression(rds, sStr, dts, true)){
    sResult = revertstr(sStr);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run revertstr(%s, %s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runReplace(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 3){
    trace(ERROR, "replace(str, tobereplaced, newstr) function accepts at least three parameters.\n");
    return false;
  }
  if (!m_params[0].evalExpression(rds, sResult, dts, true)){
    trace(ERROR, "(%s)Failed to run replace()!\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }
  vector<string> vReplace, vNew;
  string sStr;
  for (size_t i=1;i<m_params.size();i++){
    if (!m_params[i].evalExpression(rds, sStr, dts, true)){
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

bool FunctionC::runRegreplace(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 3){
    trace(ERROR, "regreplace(str, regex_pattern, newstr) function accepts only three parameters.\n");
    return false;
  }
  string sReplace, sNew; 
  if (m_params[0].evalExpression(rds, sResult, dts, true) && m_params[1].evalExpression(rds, sReplace, dts, true) && m_params[2].evalExpression(rds, sNew, dts, true)){
    regreplacestr(sResult, sReplace, sNew);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run regreplace(%s, %s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runRegmatch(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 3){
    trace(ERROR, "regmatch(str, regex_pattern, expr) function accepts only three parameters.\n");
    return false;
  }
  string str, sPattern; 
  if (m_params[0].evalExpression(rds, str, dts, true) && m_params[1].evalExpression(rds, sPattern, dts, true) && m_params[2].evalExpression(rds, sResult, dts, true)){
    regmatchstr(str, sPattern, sResult);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run regmatch(%s, %s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runRegcount(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "regcount(str, regex_pattern) function accepts only two parameters.\n");
    return false;
  }
  string str, sPattern; 
  if (m_params[0].evalExpression(rds, str, dts, true) && m_params[1].evalExpression(rds, sPattern, dts, true)){
    vector < vector <string> > results;
    getAllTokens(results, str, sPattern);
    sResult = intToStr(results.size());
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run regcount(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runRegget(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 3){
    trace(ERROR, "regget(str, regex_pattern, idxnum[, matchseq]) function accepts only three or four parameters.\n");
    return false;
  }
  string str, sPattern, sNum, sSeq="0"; 
  if (m_params[0].evalExpression(rds, str, dts, true) && m_params[1].evalExpression(rds, sPattern, dts, true) && m_params[2].evalExpression(rds, sNum, dts, true) && isInt(sNum)){
    if (m_params.size() >= 4){
      if (!m_params[3].evalExpression(rds, sSeq, dts, true) || !isInt(sSeq)){
        trace(WARNING, "%s is not a valid match sequence number for regget(str, regex_pattern, idxnum[, matchseq]), will use the defualt value.\n", m_params[3].getEntireExpstr().c_str());
      }
    }
    vector < vector <string> > results;
    getAllTokens(results, str, sPattern);
    if (results.size()==0){
      sResult = "";
      dts.datatype = STRING;
      return true;
    }
    int iNum = atoi(sNum.c_str()), iSeq = atoi(sSeq.c_str());
    iNum = iNum>0?iNum-1:iNum+results.size();
    if (iNum>=results.size()){
      trace(WARNING, "%d is out of range of the number of matched string in regget()!\n", iNum);
      return false;
    }
    iSeq = iSeq>=0?iSeq:iSeq+results[iNum].size();
    if (iSeq>=results[iNum].size()){
      trace(WARNING, "%d is out of range of the number of matches in regget()!\n", iSeq);
      iSeq = results[iNum].size()-1;
    }
    sResult = results[iNum][iSeq];
    
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run regget(%s, %s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runCountword(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2){
    trace(ERROR, "countword(str,[ingnore_quoters]) function accepts only one or two parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sQuoters=""; 
  if (m_params[0].evalExpression(rds, sRaw, dts, true)){
    if (m_params.size() == 2){
      if (!m_params[1].evalExpression(rds, sQuoters, dts, true)){
        trace(ERROR, "(2)Failed to run countword(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
    }
    std::set<char> delims = {' ','\t','\n','\r',',','.','!','?',';'};
    vector<string> vWords;
    split(vWords, sRaw,delims,sQuoters,'\0',{},true,true);
    sResult = intToStr(vWords.size());
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "(2)Failed to run countword(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runGetword(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "getword(str,wordnum,[ingnore_quoters]) function accepts only two or three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sPos, sQuoters=""; 
  if (m_params[0].evalExpression(rds, sRaw, dts, true) && m_params[1].evalExpression(rds, sPos, dts, true) && isInt(sPos)){
    int iPos = atoi(sPos.c_str());
    if (iPos==0){
      trace(ERROR, "%s cannot be zero a negative number in getword()!\n", sPos.c_str(), m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() == 3){
      if (!m_params[2].evalExpression(rds, sQuoters, dts, true)){
        trace(ERROR, "(1)Failed to run getword(%s,%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
        return false;
      }
    }
    std::set<char> delims = {' ','\t','\n','\r',',','.','!','?',';'};
    vector<string> vWords;
    split(vWords, sRaw,delims,sQuoters,'\0',{},true,false);
    dts.datatype = STRING;
    if (vWords.size() == 0){
      //trace(ERROR, "No word found in '%s'!\n", m_params[0].getEntireExpstr().c_str());
      //return false;
      sResult = "";
      return true;
    }
    if (iPos>0 && iPos<=vWords.size()){
      sResult = vWords[iPos-1];
      return true;
    }else if (iPos<0 && abs(iPos)<=vWords.size()){
      sResult = vWords[iPos+vWords.size()];
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

bool FunctionC::runGetpart(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 3){
    trace(ERROR, "getpart(str,delimiter,part_index[,quoters]) function accepts only three or four parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sPos, sDelm, sQuoters; 
  if (m_params[0].evalExpression(rds, sRaw, dts, true) && m_params[1].evalExpression(rds, sDelm, dts, true) && m_params[2].evalExpression(rds, sPos, dts, true) && isInt(sPos)){
    int iPos = atoi(sPos.c_str());
    if (iPos==0){
      trace(ERROR, "%s cannot be zero a negative number in getpart()!\n", sPos.c_str());
      return false;
    }
    if (m_params.size()>=4)
      m_params[3].evalExpression(rds, sQuoters, dts, true);
    vector<string> vWords;
    split(vWords, sRaw,sDelm,sQuoters,'\0',{},true,false,false);
    dts.datatype = STRING;
    if (vWords.size() == 0){
      trace(ERROR, "'%s' cannot be splited by '%s' in getpart()!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
      return false;
    }
    if (iPos>0 && iPos<=vWords.size()){
      sResult = vWords[iPos-1];
      return true;
    }else if (iPos<0 && abs(iPos)<=vWords.size()){
      sResult = vWords[iPos+vWords.size()];
      return true;
    }else{
      sResult = "";
      trace(WARNING, "%s is out of range of the part list in getpart()!\n", m_params[2].getEntireExpstr().c_str());
      return false;
    }
  }else{
    trace(ERROR, "(2)Failed to run getpart(%s,%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runGetparts(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 3){
    trace(ERROR, "getparts(str,delimiter,startidx[,endidx][,quoters]) function accepts only three or four or five parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sDelm, sStart, sEnd, sQuoters; 
  if (m_params[0].evalExpression(rds, sRaw, dts, true) && m_params[1].evalExpression(rds, sDelm, dts, true) && m_params[2].evalExpression(rds, sStart, dts, true) && isInt(sStart)){
    int iStart = atoi(sStart.c_str());
    if (iStart==0){
      trace(ERROR, "%s cannot be zero a negative number in getparts()!\n", sStart.c_str());
      return false;
    }
    int iEnd=-1;
    if (m_params.size()>=4){
      if (m_params[3].evalExpression(rds, sEnd, dts, true) && isInt(sEnd)){
        iEnd = atoi(sEnd.c_str());
      }else
        trace(WARNING, "%s is not a valid index number in getparts()!\n", m_params[0].getEntireExpstr().c_str());
    }
    if (m_params.size()>=5)
      m_params[4].evalExpression(rds, sQuoters, dts, true);
    vector<string> vWords;
    split(vWords,sRaw,sDelm,sQuoters,'\0',{},true,false,false);
    dts.datatype = STRING;
    if (vWords.size() == 0){
      trace(ERROR, "'%s' cannot be splited by '%s' in getparts()!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
      return false;
    }
    iStart = iStart>0?iStart-1:iStart+vWords.size();
    if (iStart>=vWords.size()){
      trace(WARNING, "%d is out of range of part number in getparts()!\n", iStart);
      iStart = vWords.size()-1;
    }
    iEnd = iEnd>0?iEnd-1:iEnd+vWords.size();
    if (iEnd>=vWords.size()){
      trace(WARNING, "%d is out of range of part number in getparts()!\n", iEnd);
      iEnd = vWords.size()-1;
    }
    sResult = "";
    for (size_t i=iStart; iStart<=iEnd?i<=iEnd:i>=iEnd; iStart<=iEnd?i++:i--)
      sResult+=vWords[i]+(i!=iEnd?sDelm:"");
    return true;
  }else{
    trace(ERROR, "(2)Failed to run getparts(%s,%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runCountpart(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 2){
    trace(ERROR, "countpart(str,delimiter[,quoters]) function accepts only two, three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sDelm, sQuoters=""; 
  if (m_params[0].evalExpression(rds, sRaw, dts, true) && m_params[1].evalExpression(rds, sDelm, dts, true)){
    if (m_params.size()>=3)
      m_params[2].evalExpression(rds, sQuoters, dts, true);
    vector<string> vWords;
    split(vWords,sRaw,sDelm,sQuoters,'\0',{},true,false,false);
    if (vWords.size() == 0){
      trace(ERROR, "'%s' cannot be splited by '%s' in countpart()!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
      return false;
    }
    dts.datatype = LONG;
    sResult = intToStr(vWords.size());
    return true;
  }else{
    trace(ERROR, "(2)Failed to run countpart(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runCountstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "countstr(str,substr) function accepts only two parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sSub; 
  if (m_params[0].evalExpression(rds, sRaw, dts, true) && m_params[1].evalExpression(rds, sSub, dts, true)){
    sResult = intToStr(countstr(sRaw,sSub));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "(2)Failed to run countstr(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFieldname(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "fieldname(fieldid) function accepts only one parameter(%d).\n",m_params.size());
    return false;
  }
  string sFieldid; 
  if (m_fieldnames && m_params[0].evalExpression(rds, sFieldid, dts, true) && isInt(sFieldid)){
    int iFieldid = atoi(sFieldid.c_str());
    if (!rds.fieldvalues || iFieldid<1 || iFieldid>rds.fieldvalues->size())
      return false;
    if (iFieldid<1 || iFieldid>m_fieldnames->size()){
      trace(ERROR, "%s is out of range of the fields!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    dts.datatype = STRING;
    sResult = (*m_fieldnames)[iFieldid-1];
    return true;
  }else{
    trace(ERROR, "Failed to run fieldname(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runConcat(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() <= 1){
    trace(ERROR, "concat() function accepts at least two parameters.\n");
    return false;
  }
  sResult = "";
  DataTypeStruct dts1, dts2;
  if (!m_params[0].evalExpression(rds, sResult, dts1, true)){
    trace(ERROR, "(%d-%s)Failed to run concat()\n",0, m_params[0].getEntireExpstr().c_str());
    return false;
  }
  string scomp;
  for (size_t i=1;i<m_params.size();i++){
    if (!m_params[i].evalExpression(rds, scomp, dts2, true)){
      trace(ERROR, "(%d-%s)Failed to run concat()\n",i,m_params[i].getEntireExpstr().c_str());
      return false;
    }
    sResult+=scomp;
  }
  dts.datatype = STRING;
  return true;
}

bool FunctionC::runConcatcol(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 3){
    trace(ERROR, "concatcol(start,end,expr[,step,delm]) function accepts at least three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sDelm="";
  sResult = "";
  if (m_params.size() >= 5){
    if (!m_params[4].evalExpression(rds, sDelm, dts, true))
      sDelm = "";
  }
  vector<ExpressionC> vExpandedExpr;
  expandForeach(vExpandedExpr, rds.fieldvalues->size());
  for (size_t i=0; i<vExpandedExpr.size(); i++){
    vExpandedExpr[i].analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, rds.sideDatatypes);
    if (!vExpandedExpr[i].evalExpression(rds, sRaw, dts, true)){
      trace(ERROR, "(%d-%s)Failed to run concatcol(start,end,expr[,step,delm])!\n",i,vExpandedExpr[i].getEntireExpstr().c_str());
    }
    sResult+=sRaw;
    if (i<vExpandedExpr.size()-1)
      sResult+=sDelm;
  }
  dts.datatype = STRING;
  return true;
}

bool FunctionC::runCalcol(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 3){
    trace(ERROR, "calcol(start,end,expr[,step,operation]) function accepts at least three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sOp="SUM";
  short int iOp=SUM;
  sResult = "";
  if (m_params.size() >= 5){
    if (m_params[4].evalExpression(rds, sOp, dts, true))
      iOp = encodeFunction(upper_copy(trim_copy(sOp)));
  }
  double dSum=0,dVal=0;
  std::set <string> tmpSet;
  vector<ExpressionC> vExpandedExpr;
  expandForeach(vExpandedExpr, rds.fieldvalues->size());
  for (size_t i=0; i<vExpandedExpr.size(); i++){
    vExpandedExpr[i].analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, rds.sideDatatypes);
    if (!vExpandedExpr[i].evalExpression(rds, sRaw, dts, true) || !isDouble(trim_copy(sRaw))){
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
  dts.datatype = DOUBLE;
  return true;
}

bool FunctionC::runAppendfile(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "appendfile(content,file) function accepts only two parameters(%d).\n",m_params.size());
    return false;
  }
  string sContent, sFile;
  if (m_params[0].evalExpression(rds, sContent, dts, true) && m_params[1].evalExpression(rds, sFile, dts, true)){
    sResult = intToStr(appendFile(sContent, sFile));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run appendfile(%s,%s)\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAscii(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "ascii() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(rds, sResult, dts, true);
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

bool FunctionC::runChar(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "char() function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(rds, sResult, dts, true) && isInt(sResult)){
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

bool FunctionC::runUrlencode(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "urlencode() function accepts only one parameter.\n");
    return false;
  }
  string url;
  if (m_params[0].evalExpression(rds, url, dts, true)){
    dts.datatype = STRING;
    sResult = urlencode(url);
    return true;
  }else{
    trace(ERROR, "Failed to run urlencode(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runUrldecode(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "urldecode() function accepts only one parameter.\n");
    return false;
  }
  string encoded;
  if (m_params[0].evalExpression(rds, encoded, dts, true)){
    dts.datatype = STRING;
    sResult = urldecode(encoded);
    return true;
  }else{
    trace(ERROR, "Failed to run urldecode(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runBase64encode(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "base64encode() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = STRING;
    sResult = base64encode(str);
    return true;
  }else{
    trace(ERROR, "Failed to run base64encode(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runBase64decode(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "base64decode() function accepts only one parameter.\n");
    return false;
  }
  string encoded;
  if (m_params[0].evalExpression(rds, encoded, dts, true)){
    dts.datatype = STRING;
    sResult = base64decode(encoded);
    return true;
  }else{
    trace(ERROR, "Failed to run base64decode(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runDuplicate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 2){
    trace(ERROR, "base64decode() function accepts only two or three parameters.\n");
    return false;
  }
  string str, num, delim=",";
  if (m_params[0].evalExpression(rds, str, dts, true) && m_params[1].evalExpression(rds, num, dts, true) && isInt(num)){
    int iDup = max(1,atoi(num.c_str()));
    dts.datatype = STRING;
    if (m_params.size() >= 3)
      m_params[2].evalExpression(rds, delim, dts, true);
    sResult ="";
    for (int i=0; i<iDup; i++){
      sResult += str;
      if (i<iDup-1)
        sResult += delim;
    }
    return true;
  }else{
    trace(ERROR, "Failed to run base64decode(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runMd5(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "md5() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = STRING;
    sResult = md5(str);
    return true;
  }else{
    trace(ERROR, "Failed to run md5(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runHash(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "hash() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = STRING;
    sResult = hashstr(str);
    return true;
  }else{
    trace(ERROR, "Failed to run hash(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIsip(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isip() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = LONG;
    sResult = intToStr(getFirstToken(str,"^((25[0-5]|(2[0-4]|1\\d|[1-9]|)\\d)\\.?\\b){4}$").compare(str) == 0);
    return true;
  }else{
    trace(ERROR, "Failed to run isip(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIsipv6(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isipv6() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    //string IPV4SEG  = "(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])";
    //string IPV4ADDR = "("+IPV4SEG+"\\.){3,3}"+IPV4SEG;
    //string IPV6SEG  = "[0-9a-fA-F]{1,4}";
    //string IPV6ADDR = "(("+IPV6SEG+":){7,7}"+IPV6SEG+"|("+IPV6SEG+":){1,7}:|("+IPV6SEG+":){1,6}:"+IPV6SEG+"|("+IPV6SEG+":){1,5}(:"+IPV6SEG+"){1,2}|("+IPV6SEG+":){1,4}(:"+IPV6SEG+"){1,3}|("+IPV6SEG+":){1,3}(:"+IPV6SEG+"){1,4}|("+IPV6SEG+":){1,2}(:"+IPV6SEG+"){1,5}|"+IPV6SEG+":((:"+IPV6SEG+"){1,6})|:((:"+IPV6SEG+"){1,7}|:)|fe80:(:"+IPV6SEG+"){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}"+IPV4ADDR+"|("+IPV6SEG+":){1,4}:"+IPV4ADDR+")";

    dts.datatype = LONG;
    sResult = intToStr(getFirstToken(str,"(?:^|(?<=\\s))(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))(?=\\s|$)").compare(str) == 0);
    return true;
  }else{
    trace(ERROR, "Failed to run isipv6(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIsmac(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "ismac() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = LONG;
    sResult = intToStr(getFirstToken(str,"^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$").compare(str) == 0);
    return true;
  }else{
    trace(ERROR, "Failed to run ismac(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runMyips(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  string sStartid="1", sEndid, sDelm="|";
  if (m_params.size() >= 1 && (!m_params[0].evalExpression(rds, sStartid, dts, true) || !isInt(sStartid))){
    trace(WARNING, "'%s' is not ab IP id for myips(), set it to 1!\n",m_params[0].getEntireExpstr().c_str());
    sStartid="1";
  }
  sEndid = sStartid;
  if (m_params.size() >= 2 && (!m_params[1].evalExpression(rds, sEndid, dts, true) || !isInt(sEndid))){
    trace(WARNING, "'%s' is not ab IP id for myips(), set it to %s!\n",m_params[1].getEntireExpstr().c_str(), sStartid.c_str());
    sEndid=sStartid;
  }
  unordered_map< string,string > IPs = getmyips();
  if (IPs.size() == 0){
    trace(ERROR, "Failed to run myips()!\n");
    return false;
  }
  int iStartid = atoi(sStartid.c_str());
  iStartid = iStartid>0?iStartid-1:iStartid+IPs.size();
  if (iStartid>=IPs.size()){
    trace(WARNING, "%d is out of range of part number in myips()!\n", iStartid);
    iStartid = IPs.size()-1;
  }
  int iEndid = atoi(sEndid.c_str());
  iEndid = iEndid>0?iEndid-1:iEndid+IPs.size();
  if (iEndid>=IPs.size()){
    trace(WARNING, "%d is out of range of part number in myips()!\n", iEndid);
    iEndid = IPs.size()-1;
  }
  if (m_params.size() >= 3)
    m_params[2].evalExpression(rds, sDelm, dts, true);
  dts.datatype = STRING;  
  sResult = "";
  unordered_map< string,string >::iterator it=IPs.begin();
  for (size_t i=iStartid; (iStartid<=iEndid?i<=iEndid:i>=iEndid) && it!=IPs.end(); iStartid<=iEndid?i++:i--){
    sResult+=it->second+(i!=iEndid?sDelm:"");
    it++;
  }

  string str;
  dts.datatype = STRING;
  return true;
}

bool FunctionC::runHostname(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 0){
    trace(ERROR, "hostname() function does not accepts any parameter.\n");
    return false;
  }
  string str;
  dts.datatype = STRING;
  sResult = hostname();
  return true;
}

bool FunctionC::runIsfile(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isfile() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = LONG;
    sResult = intToStr(getReadMode(str) == SINGLEFILE);
    return true;
  }else{
    trace(ERROR, "Failed to run isfile(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIsfolder(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isfolder() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = LONG;
    sResult = intToStr(getReadMode(str) == FOLDER);
    return true;
  }else{
    trace(ERROR, "Failed to run isfolder(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFileexist(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "fileexist() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = LONG;
    sResult = intToStr(fileexist(str));
    return true;
  }else{
    trace(ERROR, "Failed to run fileexist(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runRmfile(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "rmfile() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = LONG;
    short int iMode = getReadMode(str);
    if (iMode != SINGLEFILE){
      trace(WARNING, "Failed to run rmfile(%s), '%s' is not a file or does not exist!\n", m_params[0].getEntireExpstr().c_str(), str.c_str());
      sResult = "0";
    }else
      sResult = intToStr(rmFile(str));
    return true;
  }else{
    trace(ERROR, "Failed to run rmfile(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runRenamefile(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "renamefile() function accepts only two parameters.\n");
    return false;
  }
  string oldfile, newfile;
  if (m_params[0].evalExpression(rds, oldfile, dts, true) && m_params[1].evalExpression(rds, newfile, dts, true)){
    dts.datatype = LONG;
    short int iOldMode = getReadMode(oldfile);
    short int iNewMode = getReadMode(newfile);
    if (iOldMode != SINGLEFILE && iOldMode != FOLDER){
      trace(WARNING, "Failed to run renamefile(), '%s' is not a file or does not exist!\n", m_params[0].getEntireExpstr().c_str());
      sResult = "0";
    }else if (iNewMode == SINGLEFILE || iNewMode == FOLDER){
      trace(WARNING, "Failed to run renamefile(), '%s' already exist!\n", m_params[1].getEntireExpstr().c_str());
      sResult = "0";
    }else
      sResult = intToStr(renameFile(oldfile, newfile));
    return true;
  }else{
    trace(ERROR, "Failed to run renamefile(%s, %s)\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFilesize(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "filesize() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = LONG;
    short int iMode = getReadMode(str);
    if (iMode != SINGLEFILE){
      trace(WARNING, "Failed to run filesize(%s), '%s' is not a file or does not exist!\n", m_params[0].getEntireExpstr().c_str(), str.c_str());
      sResult = "-1";
    }else
      sResult = intToStr(getFileSize(str));
    return true;
  }else{
    trace(ERROR, "Failed to run filesize(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFileattrs(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "fileattrs() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = STRING;
    sResult = getFileModeStr(str);
    return true;
  }else{
    trace(ERROR, "Failed to run fileattrs(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIsexecutable(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isexecutable() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = LONG;
    sResult = intToStr(isexecutable(str));
    return true;
  }else{
    trace(ERROR, "Failed to run isexecutable(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIssymblink(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "issymblink() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = LONG;
    sResult = intToStr(issymblink(str));
    return true;
  }else{
    trace(ERROR, "Failed to run issymblink(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runGetsymblink(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "getsymblink() function accepts only one parameter.\n");
    return false;
  }
  string str;
  if (m_params[0].evalExpression(rds, str, dts, true)){
    dts.datatype = STRING;
    sResult = getsymblink(str);
    return true;
  }else{
    trace(ERROR, "Failed to run getsymblink(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runComparenum(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "comparenum(num1, num2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(rds, str1, dts, true) && isDouble(str1) && m_params[1].evalExpression(rds, str2, dts, true) && isDouble(str2)){
    double num1 = atof(str1.c_str()), num2 = atof(str2.c_str());
    sResult = intToStr(num1>num2?1:(num1==num2?0:-1));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run comparenum(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runComparedate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "comparedater(date1, date2[, date_format]) function accepts only two or three parameters.\n");
    return false;
  }
  string date1, date2, sFmt; 
  DataTypeStruct dts1, dts2;
  int offSet;
  if (m_params.size() == 3 && m_params[2].evalExpression(rds, sFmt, dts, true)){
    dts1.extrainfo = sFmt;
    dts2.extrainfo = sFmt;
  }
  if (m_params[0].evalExpression(rds, date1, dts, true) && isDate(date1, offSet, dts1.extrainfo) && m_params[1].evalExpression(rds, date2, dts, true) && isDate(date2, offSet, dts2.extrainfo)){
    if (dts1.extrainfo.compare(dts2.extrainfo)!=0){
      trace(ERROR, "Date format %s of %s doesnot match date format %s of %s!\n", dts1.extrainfo.c_str(), m_params[0].getEntireExpstr().c_str(), dts2.extrainfo.c_str(), m_params[1].getEntireExpstr().c_str());
      return false;
    }
    sResult = intToStr(anyDataCompare(date1, date2, dts1, dts2));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run comparedater(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runMod(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "mod() function accepts only two parameters.\n");
    return false;
  }
  string sBase, sMod;
  if (m_params[0].evalExpression(rds, sBase, dts, true) && isInt(sBase) && m_params[1].evalExpression(rds, sMod, dts, true) && isInt(sMod)){
    dts.datatype = INTEGER;
    sResult = intToStr(atoi(sBase.c_str())%atoi(sMod.c_str()));
    return true;
  }else{
    trace(ERROR, "Failed to run mod(%s, %s)\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAbs(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "abs() function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(rds, sResult, dts, true) && isDouble(sResult)){
    dts.datatype = DOUBLE;
    double dNum = atof(sResult.c_str());
    sResult = doubleToStr(dNum<0?dNum*-1:dNum);
    return true;
  }else{
    trace(ERROR, "Failed to run abs(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAcos(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "acos() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(acos(x));
    return true;
  }else{
    trace(ERROR, "Failed to run acos(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAcosh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "acosh() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(acosh(x));
    return true;
  }else{
    trace(ERROR, "Failed to run acosh(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAsin(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "asin() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(asin(x));
    return true;
  }else{
    trace(ERROR, "Failed to run asin(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAsinh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "asinh() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(asinh(x));
    return true;
  }else{
    trace(ERROR, "Failed to run asinh(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAtan(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "atan() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(atan(x));
    return true;
  }else{
    trace(ERROR, "Failed to run atan(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAtan2(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "atan2() function accepts only two parameters.\n");
    return false;
  }
  string sX, sY;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX) && m_params[1].evalExpression(rds, sY, dts, true) && isDouble(sY)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str()), y = atof(sY.c_str());
    sResult = doubleToStr(atan2(x,y));
    return true;
  }else{
    trace(ERROR, "Failed to run atan2(%s)\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runAtanh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "atanh() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(atanh(x));
    return true;
  }else{
    trace(ERROR, "Failed to run atanh(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runCbrt(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "cbrt() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(cbrt(x));
    return true;
  }else{
    trace(ERROR, "Failed to run cbrt(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runCopysign(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "copysign() function accepts only two parameters.\n");
    return false;
  }
  string sX, sY;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX) && m_params[1].evalExpression(rds, sY, dts, true) && isDouble(sY)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str()), y = atof(sY.c_str());
    sResult = doubleToStr(copysign(x,y));
    return true;
  }else{
    trace(ERROR, "Failed to run copysign(%s)\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runCos(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "cos() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(cos(x));
    return true;
  }else{
    trace(ERROR, "Failed to run cos(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runCosh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "cosh() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(cosh(x));
    return true;
  }else{
    trace(ERROR, "Failed to run cosh(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runErf(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "erf() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(erf(x));
    return true;
  }else{
    trace(ERROR, "Failed to run erf(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runExp(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "exp() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(exp(x));
    return true;
  }else{
    trace(ERROR, "Failed to run exp(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runExp2(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "exp2() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(exp2(x));
    return true;
  }else{
    trace(ERROR, "Failed to run exp2(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFma(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 3){
    trace(ERROR, "fma() function accepts only three parameters.\n");
    return false;
  }
  string sX,sY,sZ;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX) && m_params[1].evalExpression(rds, sY, dts, true) && isDouble(sY) && m_params[2].evalExpression(rds, sZ, dts, true) && isDouble(sZ)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str()), y = atof(sY.c_str()), z = atof(sZ.c_str());
    sResult = doubleToStr(fma(x,y,z));
    return true;
  }else{
    trace(ERROR, "Failed to run fma(%s,%s,%s)\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str(), m_params[2].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFmod(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "fmod() function accepts only two parameters.\n");
    return false;
  }
  string sX,sY;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX) && m_params[1].evalExpression(rds, sY, dts, true) && isDouble(sY)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str()), y = atof(sY.c_str());
    sResult = doubleToStr(fmod(x, y));
    return true;
  }else{
    trace(ERROR, "Failed to run fmod(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFpclassify(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "fpclassify() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = STRING;
    double x = atof(sX.c_str());
    switch(fpclassify(x)){
    case FP_INFINITE:
      sResult = "INFINITE";
      break;
    case FP_NAN:
      sResult = "NAN";
      break;
    case FP_ZERO:
      sResult = "ZERO";
      break;
    case FP_SUBNORMAL:
      sResult = "SUBNORMAL";
      break;
    case FP_NORMAL:
    default:
      sResult = "NORMAL";
      break;
    }
    return true;
  }else{
    trace(ERROR, "Failed to run fpclassify(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runHypot(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "hypot() function accepts only two parameters.\n");
    return false;
  }
  string sX,sY;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX) && m_params[1].evalExpression(rds, sY, dts, true) && isDouble(sY)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str()), y = atof(sY.c_str());
    sResult = doubleToStr(hypot(x, y));
    return true;
  }else{
    trace(ERROR, "Failed to run hypot(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIlogb(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "ilogb() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(ilogb(x));
    return true;
  }else{
    trace(ERROR, "Failed to run ilogb(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIsfinite(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isfinite() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = LONG;
    double x = atof(sX.c_str());
    sResult = intToStr(isfinite(x));
    return true;
  }else{
    trace(ERROR, "Failed to run isfinite(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIsinf(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isinf() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = LONG;
    double x = atof(sX.c_str());
    sResult = intToStr(isinf(x));
    return true;
  }else{
    trace(ERROR, "Failed to run isinf(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIsnormal(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isnormal() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = LONG;
    double x = atof(sX.c_str());
    sResult = intToStr(isnormal(x));
    return true;
  }else{
    trace(ERROR, "Failed to run isnormal(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runLgamma(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "lgamma() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(lgamma(x));
    return true;
  }else{
    trace(ERROR, "Failed to run lgamma(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runLog10(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "log10() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(log10(x));
    return true;
  }else{
    trace(ERROR, "Failed to run log10(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runLog2(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "log2() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(log2(x));
    return true;
  }else{
    trace(ERROR, "Failed to run log2(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runPow(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "pow() function accepts only two parameter.\n");
    return false;
  }
  string sX,sY;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX) && m_params[1].evalExpression(rds, sY, dts, true) && isDouble(sY)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str()), y = atof(sY.c_str());
    sResult = doubleToStr(pow(x, y));
    return true;
  }else{
    trace(ERROR, "Failed to run pow(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runRemainder(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "remainder() function accepts only two parameters.\n");
    return false;
  }
  string sX,sY;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX) && m_params[1].evalExpression(rds, sY, dts, true) && isDouble(sY)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str()), y = atof(sY.c_str());
    sResult = doubleToStr(remainder(x, y));
    return true;
  }else{
    trace(ERROR, "Failed to run remainder(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runScalbln(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "scalbln() function accepts only two parameters.\n");
    return false;
  }
  string sX,sY;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX) && m_params[1].evalExpression(rds, sY, dts, true) && isLong(sY)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str()); long y = atol(sY.c_str());
    sResult = doubleToStr(scalbln(x, y));
    return true;
  }else{
    trace(ERROR, "Failed to run scalbln(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runScalbn(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "scalbn() function accepts only two parameter.\n");
    return false;
  }
  string sX,sY;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX) && m_params[1].evalExpression(rds, sY, dts, true) && isLong(sY)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str()); long y = atol(sY.c_str());
    sResult = doubleToStr(scalbn(x, y));
    return true;
  }else{
    trace(ERROR, "Failed to run scalbn(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runSin(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "sin() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(sin(x));
    return true;
  }else{
    trace(ERROR, "Failed to run sin(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runSinh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "sinh() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(sinh(x));
    return true;
  }else{
    trace(ERROR, "Failed to run sinh(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runSqrt(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "sqrt() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(sqrt(x));
    return true;
  }else{
    trace(ERROR, "Failed to run sqrt(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTan(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "tan() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(tan(x));
    return true;
  }else{
    trace(ERROR, "Failed to run tan(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTanh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "tanh() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(tanh(x));
    return true;
  }else{
    trace(ERROR, "Failed to run tanh(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTgamma(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "tgamma() function accepts only one parameter.\n");
    return false;
  }
  string sX;
  if (m_params[0].evalExpression(rds, sX, dts, true) && isDouble(sX)){
    dts.datatype = DOUBLE;
    double x = atof(sX.c_str());
    sResult = doubleToStr(tgamma(x));
    return true;
  }else{
    trace(ERROR, "Failed to run tgamma(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runPi(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 0){
    trace(ERROR, "pi() function does not accept any parameter.\n");
    return false;
  }
  dts.datatype = DOUBLE;
  sResult = doubleToStr(PI_VAL);
  return true;
}

bool FunctionC::runDatatype(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "datatype() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(rds, sResult, dts, true);
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

bool FunctionC::runDetectdt(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "detectdt() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(rds, sResult, dts, true);
  if (gotResult){
    string extrainfo;
    short int iType = detectDataType(sResult, extrainfo);
    sResult = decodeDatatype(iType==UNKNOWN?STRING:iType);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run detectdt(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIslong(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "islong() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(rds, sResult, dts, true);
  if (gotResult){
    sResult = intToStr(isLong(sResult));
    dts.datatype = LONG;
    return true;
  }else{
    sResult = "0";
    trace(ERROR, "Failed to run islong(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIsdouble(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isdouble() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(rds, sResult, dts, true);
  if (gotResult){
    sResult = intToStr(isDouble(sResult));
    dts.datatype = LONG;
    return true;
  }else{
    sResult = "0";
    trace(ERROR, "Failed to run isdouble(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIsdate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isdate() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(rds, sResult, dts, true);
  if (gotResult){
    string extrainfo = "";
    int iOffSet;
    sResult = intToStr(isDate(sResult, iOffSet, extrainfo));
    dts.datatype = LONG;
    return true;
  }else{
    sResult = "0";
    trace(ERROR, "Failed to run isdate(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runIsstring(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isstring() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(rds, sResult, dts, true);
  if (gotResult){
    sResult = "1";
    dts.datatype = LONG;
    return true;
  }else{
    sResult = "0";
    trace(ERROR, "Failed to run isstring(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runToint(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "toint(str) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(rds, sResult, dts, true) && isInt(sResult)){
    dts.datatype = INTEGER;
    return true;
  }else{
    trace(ERROR, "Failed to run toint(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTolong(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "tolong(str) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(rds, sResult, dts, true) && isInt(sResult)){
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run tolong(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTofloat(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "tofloat(str) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(rds, sResult, dts, true) && isDouble(sResult)){
    dts.datatype = DOUBLE;
    return true;
  }else{
    trace(ERROR, "Failed to run tofloat(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTostr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "tostr(expr) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(rds, sResult, dts, true)){
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run tostr(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTodate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2){
    trace(ERROR, "runTodate(str,[dateformat]) function accepts only one or two parameters(%d).\n",m_params.size());
    return false;
  }
  DataTypeStruct dts1;
  int offSet;
  if (m_params.size() != 2 || !m_params[1].evalExpression(rds, dts.extrainfo, dts1, true))
    dts.extrainfo = "";
  if (m_params[0].evalExpression(rds, sResult, dts, true) && isDate(sResult, offSet, dts.extrainfo)){
    dts.datatype = DATE;
    m_datatype = dts;
    return true;
  }else{
    trace(ERROR, "Failed to run runTodate(str,[dateformat])\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runDectohex(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "dectohex(num) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(rds, sResult, dts, true) && isInt(sResult)){
    sResult = dectohex(atoi(sResult.c_str()));
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run dectohex(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runHextodec(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "hextodec(str) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(rds, sResult, dts, true)){
    sResult = intToStr(hextodec(sResult));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run dectohex(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runDectobin(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "dectobin(num) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(rds, sResult, dts, true) && isInt(sResult)){
    sResult = dectobin(atoi(sResult.c_str()));
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run dectohex(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runBintodec(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "bintodec(str) function accepts only one parameter.\n");
    return false;
  }
  if (m_params[0].evalExpression(rds, sResult, dts, true)){
    sResult = intToStr(bintodec(sResult));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run dectohex(%s)\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runFloor(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "floor(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(rds, sNum, dts, true) && isDouble(sNum)){
    sResult = intToStr(floor(atof(sNum.c_str())));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run floor(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runCeil(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "ceil(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(rds, sNum, dts, true) && isDouble(sNum)){
    sResult = intToStr(ceil(atof(sNum.c_str())));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run ceil(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runTimediff(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "timediff(tm1, tm2) function accepts only two parameters.\n");
    return false;
  }
  string sTm1, sTm2; 
  DataTypeStruct dts1, dts2;
  int offSet1, offSet2;
  if (m_params[0].evalExpression(rds, sTm1, dts1, true) && isDate(sTm1, offSet1, dts1.extrainfo) && m_params[1].evalExpression(rds, sTm2, dts2, true) && isDate(sTm2, offSet2, dts2.extrainfo)){
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

bool FunctionC::runAddtime(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "addtime(date, number, unit) function accepts only two or three parameters.\n");
    return false;
  }
  string sTm, sNum, sUnit; 
  DataTypeStruct dts1, dts2;
  int offSet;
  if (m_params[0].evalExpression(rds, sTm, dts1, true) && isDate(sTm, offSet, dts1.extrainfo) && m_params[1].evalExpression(rds, sNum, dts2, true) && isInt(sNum)){
    char unit = 'S';
    if (m_params.size() == 3 && m_params[2].evalExpression(rds, sUnit, dts2, true)){
      sUnit = upper_copy(sUnit);
      if (sUnit.length() == 1 && (sUnit[0]=='S' || sUnit[0]=='M' || sUnit[0]=='H' || sUnit[0]=='D' || sUnit[0]=='N' || sUnit[0]=='Y'))
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
      dts.extrainfo = dts1.extrainfo;
      m_datatype = dts;
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

bool FunctionC::runRound(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2){
    trace(ERROR, "round(num) function accepts only one or two parameters.\n");
    return false;
  }
  string sNum;
  int scale=0;
  if (m_params.size() == 2){
    if (m_params[1].evalExpression(rds, sNum, dts, true) && isInt(sNum)){
      scale = max(0,atoi(sNum.c_str()));
    }else{
      trace(ERROR, "Failed to run round(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
      return false;
    }
  }
  if (m_params[0].evalExpression(rds, sNum, dts, true) && isDouble(sNum)){
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

bool FunctionC::runLog(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "log(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(rds, sNum, dts, true) && isDouble(sNum)){
    sResult = doubleToStr(log(atof(sNum.c_str())));
    dts.datatype = DOUBLE;
    return true;
  }else{
    trace(ERROR, "Failed to run log(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
}

bool FunctionC::runRandom(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() > 2){
    trace(ERROR, "random([min,][max]) function accepts only one parameter.\n");
    return false;
  }
  string sMin="1", sMax="100";
  if (m_params.size() > 0){
    if (!m_params[0].evalExpression(rds, sMax, dts, true) || !isDouble(sMax)){ // the parameter is the maximum range if only one parameter provided
      trace(ERROR, "Failed to run random(%s)!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() > 1){ // the first parameter is the minimum range and the second parameter is the maximum range if two parameters provided
      sMin = sMax;
      if (!m_params[1].evalExpression(rds, sMax, dts, true) || !isDouble(sMax)){ 
        trace(ERROR, "Failed to run random(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
    }
  }
  dts.datatype = LONG;
  sResult = intToStr(random(atoi(sMin.c_str()),atoi(sMax.c_str())));
  return true;
}

bool FunctionC::runRandstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() > 2){
    trace(ERROR, "runRandstr([min,][max]) function accepts two parameters at most.\n");
    return false;
  }
  string sLen="8", sFlags="uld";
  if (m_params.size() > 0){
    if (!m_params[0].evalExpression(rds, sLen, dts, true) || !isDouble(sLen)){ 
      trace(ERROR, "Failed to run runRandstr(%s)!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() > 1){ 
      if (!m_params[1].evalExpression(rds, sFlags, dts, true)){ 
        trace(ERROR, "Failed to run runRandstr(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
    }
  }
  dts.datatype = STRING;
  sResult = randstr(atoi(sLen.c_str()),sFlags);
  return true;
}

bool FunctionC::runTrimleft(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "trimleft(str[,char][,repeat(1|0)]) function accepts only one or two or three parameters.\n");
    return false;
  }
  string sStr, sChar=" ", sRepeat;
  bool repeat=true;
  if (m_params.size() > 0){
    if (!m_params[0].evalExpression(rds, sStr, dts, true)){ 
      trace(ERROR, "Failed to run trimleft(%s)!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() > 1){ 
      if (!m_params[1].evalExpression(rds, sChar, dts, true)){ 
        trace(ERROR, "Failed to run trimleft(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
      if (sChar.length()!=1){
        trace(ERROR, "The second parameter of trimleft should only be one char!\n");
        return false;
      }
      if (m_params.size() > 2){ 
        if (!m_params[2].evalExpression(rds, sRepeat, dts, true)){ 
          trace(ERROR, "Failed to run trimleft(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
          return false;
        }
        repeat=(sRepeat.compare("0")!=0);
      }
    }
  }
  dts.datatype = STRING;
  sResult = trim_left(sStr, sChar[0], repeat);
  return true;
}

bool FunctionC::runTrimright(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "trimright(str[,char][,repeat(1|0)]) function accepts only one or two or three parameters.\n");
    return false;
  }
  string sStr, sChar=" ", sRepeat;
  bool repeat=true;
  if (m_params.size() > 0){
    if (!m_params[0].evalExpression(rds, sStr, dts, true)){ 
      trace(ERROR, "Failed to run trimright(%s)!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() > 1){ 
      if (!m_params[1].evalExpression(rds, sChar, dts, true)){ 
        trace(ERROR, "Failed to run trimright(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
      if (sChar.length()!=1){
        trace(ERROR, "The second parameter of trimright should only be one char!\n");
        return false;
      }
      if (m_params.size() > 2){ 
        if (!m_params[2].evalExpression(rds, sRepeat, dts, true)){ 
          trace(ERROR, "Failed to run trimleft(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
          return false;
        }
        repeat=(sRepeat.compare("0")!=0);
      }
    }
  }
  dts.datatype = STRING;
  sResult = trim_right(sStr, sChar[0], repeat);
  return true;
}

bool FunctionC::runTrim(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "trim(str[,char][,repeat(1|0)]) function accepts only one or two or three parameters.\n");
    return false;
  }
  string sStr, sChar=" ", sRepeat;
  bool repeat=true;
  if (m_params.size() > 0){
    if (!m_params[0].evalExpression(rds, sStr, dts, true)){ 
      trace(ERROR, "Failed to run trim(%s)!\n", m_params[0].getEntireExpstr().c_str());
      return false;
    }
    if (m_params.size() > 1){ 
      if (!m_params[1].evalExpression(rds, sChar, dts, true)){ 
        trace(ERROR, "Failed to run trim(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
        return false;
      }
      if (sChar.length()!=1){
        trace(ERROR, "The second parameter of trim should only be one char!\n");
        return false;
      }
      if (m_params.size() > 2){ 
        if (!m_params[2].evalExpression(rds, sRepeat, dts, true)){ 
          trace(ERROR, "Failed to run trimleft(%s,%s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
          return false;
        }
        repeat=(sRepeat.compare("0")!=0);
      }
    }
  }
  dts.datatype = STRING;
  sResult = trim(sStr, sChar[0], repeat);
  return true;
}

bool FunctionC::runCamelstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "camelstr(str) function accepts only one or two parameters.\n");
    return false;
  }
  string sStr;
  if (!m_params[0].evalExpression(rds, sStr, dts, true)){ 
    trace(ERROR, "Failed to run camelstr(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
  dts.datatype = STRING;
  sResult = camelstr(sStr);
  return true;
}

bool FunctionC::runSnakestr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "snakestr(str) function accepts only one or two parameters.\n");
    return false;
  }
  string sStr;
  if (!m_params[0].evalExpression(rds, sStr, dts, true)){ 
    trace(ERROR, "Failed to run snakestr(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
  dts.datatype = STRING;
  sResult = snakestr(sStr);
  return true;
}

bool FunctionC::runTruncdate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "truncdate(date,seconds) function accepts only two parameters.\n");
    return false;
  }
  string sTm, sSeconds;
  DataTypeStruct tmpDts;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && m_params[1].evalExpression(rds, sSeconds, tmpDts, true) && isInt(sSeconds)){
    int iSeconds = atoi(sSeconds.c_str());
    sResult = truncdate(sTm, dts.extrainfo, iSeconds);
    dts.datatype = DATE;
    m_datatype = dts;
    //trace(DEBUG, "Truncating seconds %d from '%s'(%u) get '%s'(%u), format:%s\n", iSeconds, sTm.c_str(), (long)t1, sResult.c_str(), (long)t2, dts.extrainfo.c_str());
    return !sResult.empty();
  }else{
    trace(ERROR, "Failed to run truncdate(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }  
}

bool FunctionC::runTruncdateu(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "truncdateu(date, unit) function accepts only two parameters.\n");
    return false;
  }
  string sTm, sUnit;
  DataTypeStruct tmpDts;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && m_params[1].evalExpression(rds, sUnit, tmpDts, true) && !sUnit.empty()){
    sResult = truncdateu(sTm, dts.extrainfo, sUnit[0]);
    dts.datatype = DATE;
    m_datatype = dts;
    //trace(DEBUG, "Truncating from '%s'(%u) get '%s'(%u), format:%s\n", sTm.c_str(), (long)t1, sResult.c_str(), (long)t2, dts.extrainfo.c_str());
    return !sResult.empty();
  }else{
    trace(ERROR, "Failed to run truncdate(%s, %s)!\n", m_params[0].getEntireExpstr().c_str(), m_params[1].getEntireExpstr().c_str());
    return false;
  }  
}

bool FunctionC::runDateformat(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2){
    trace(ERROR, "dateformat(date[, fmt]) function accepts only one or two parameters.\n");
    return false;
  }
  string sTm, sFmt;
  DataTypeStruct tmpDts;
  int iOffSet;
  if (m_params.size() == 2 && m_params[1].evalExpression(rds, sFmt, tmpDts, true))
    dts.extrainfo = sFmt;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && isDate(sTm, iOffSet, dts.extrainfo)){
    struct tm tm;
    if (strToDate(sTm, tm, iOffSet, dts.extrainfo)){
      addhours(tm, -timezone/3600);
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

bool FunctionC::runNow(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 1){
    trace(ERROR, "now() function accepts 0 or one parameter.\n");
    return false;
  }
  struct tm curtime = now();
  dts.extrainfo = string(DATEFMT)+" %z";
  sResult = dateToStr(curtime, curtime.tm_gmtoff/36, dts.extrainfo);
  //sResult = dateToStr(curtime);
  dts.datatype = DATE;
  m_datatype = dts;
  return true;
}

bool FunctionC::runIsleap(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isleap() function accepts only one parameter.\n");
    return false;
  }
  string sTm;
  int offSet;
  DataTypeStruct tmpDts;
  struct tm tm1;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && isDate(sTm, offSet, tmpDts.extrainfo) && strToDate(sTm, tm1, offSet, tmpDts.extrainfo)){
    sResult = intToStr(tm1.tm_year%400==0 || (tm1.tm_year%4==0 && tm1.tm_year%100!=0));
  }else{
    trace(ERROR, "Failed to run isleap(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }  
  dts.datatype = LONG;
  return true;
}

bool FunctionC::runWeekday(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "weekday() function accepts only one parameter.\n");
    return false;
  }
  string sTm;
  int offSet;
  DataTypeStruct tmpDts;
  struct tm tm1;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && isDate(sTm, offSet, tmpDts.extrainfo) && strToDate(sTm, tm1, offSet, tmpDts.extrainfo)){
    sResult = intToStr(tm1.tm_wday==0?7:tm1.tm_wday);
  }else{
    trace(ERROR, "Failed to run weekday(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }  
  dts.datatype = LONG;
  return true;
}

bool FunctionC::runMonthfirstday(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "monthfirstday() function accepts only one parameter.\n");
    return false;
  }
  string sTm;
  int offSet;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && isDate(sTm, offSet, dts.extrainfo)){
    sResult = truncdateu(sTm, dts.extrainfo, 'd');
  }else{
    trace(ERROR, "Failed to run weekday(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }  
  dts.datatype = DATE;
  m_datatype = dts;
  return true;
}

bool FunctionC::runMonthfirstmonday(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "monthfirstmonday() function accepts only one parameter.\n");
    return false;
  }
  string sTm;
  int offSet;
  struct tm tm1;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && isDate(sTm, offSet, dts.extrainfo) && strToDate(sTm, tm1, offSet, dts.extrainfo)){
    sResult = monthfirstmonday(sTm, dts.extrainfo);
  }else{
    trace(ERROR, "Failed to run monthfirstmonday(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }  
  dts.datatype = DATE;
  m_datatype = dts;
  return true;
}

bool FunctionC::runYearweek(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "yearweek() function accepts only one parameter.\n");
    return false;
  }
  string sTm;
  int offSet;
  DataTypeStruct tmpDts;
  struct tm tm1;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && isDate(sTm, offSet, tmpDts.extrainfo) && strToDate(sTm, tm1, offSet, tmpDts.extrainfo)){
    sResult = intToStr(yearweek(sTm, dts.extrainfo));
  }else{
    trace(ERROR, "Failed to run yearweek(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }  
  dts.datatype = LONG;
  return true;
}

bool FunctionC::runYearday(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "yearday() function accepts only one parameter.\n");
    return false;
  }
  string sTm;
  int offSet;
  DataTypeStruct tmpDts;
  struct tm tm1;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && isDate(sTm, offSet, tmpDts.extrainfo) && strToDate(sTm, tm1, offSet, tmpDts.extrainfo)){
    sResult = intToStr(yearday(sTm, dts.extrainfo));
  }else{
    trace(ERROR, "Failed to run yearday(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }  
  dts.datatype = LONG;
  return true;
}

bool FunctionC::runDatetolong(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "datetolong() function accepts only one parameter.\n");
    return false;
  }
  string sTm;
  int offSet;
  DataTypeStruct tmpDts;
  struct tm tm1;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && isDate(sTm, offSet, tmpDts.extrainfo) && strToDate(sTm, tm1, offSet, tmpDts.extrainfo)){
    sResult = intToStr((long)mktime(&tm1) - timezone);
  }else{
    trace(ERROR, "Failed to run datetolong(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }  
  dts.datatype = LONG;
  return true;
}

bool FunctionC::runLongtodate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "longtodate() function accepts only one parameter.\n");
    return false;
  }
  string sLong;
  struct tm tm1;
  if (m_params[0].evalExpression(rds, sLong, dts, true) && isLong(sLong)){
    time_t t1 = (time_t)atol(sLong.c_str());
    tm1 = *(localtime(&t1));
    sResult = dateToStr(tm1, 0, DATEFMT);;
  }else{
    trace(ERROR, "Failed to run longtodate(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }
  dts.extrainfo = DATEFMT;
  dts.datatype = DATE;
  m_datatype = dts;
  return true;
}

bool FunctionC::runLocaltime(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "localtime() function accepts only one parameter.\n");
    return false;
  }
  string sTm;
  int offSet;
  struct tm tm1;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && isDate(sTm, offSet, dts.extrainfo)){
    sResult = rqlocaltime(sTm, dts.extrainfo);;
  }else{
    trace(ERROR, "Failed to run localtime(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }  
  dts.datatype = DATE;
  m_datatype = dts;
  return true;
}

bool FunctionC::runGmtime(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "gmtime() function accepts only two parameters.\n");
    return false;
  }
  string sTm, sGmtOff;
  int offSet;
  struct tm tm1;
  if (m_params[0].evalExpression(rds, sTm, dts, true) && isDate(sTm, offSet, dts.extrainfo) && m_params[1].evalExpression(rds, sGmtOff, dts, true) && isDouble(sGmtOff)){
    double dGmtOff = atof(sGmtOff.c_str());
    if (dGmtOff>12 || dGmtOff<-12){
      trace(ERROR, "The valid GMT offset range is from -12 to 12!\n");
      return false;
    }
    sResult = rqgmtime(sTm, dts.extrainfo, dGmtOff);
  }else{
    trace(ERROR, "Failed to run gmtime(%s)!\n", m_params[0].getEntireExpstr().c_str());
    return false;
  }  
  dts.datatype = DATE;
  m_datatype = dts;
  return true;
}

// switch(input,case1,return1[,case2,result2...][,default]): if input equal to case1, then return return1, etc.. If none matched, return default or return input if no default provided.
bool FunctionC::runSwitch(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() <= 2){
    trace(ERROR, "switch() function accepts at least three parameters.\n");
    return false;
  }
  DataTypeStruct dts1, dts2;
  if (!m_params[0].evalExpression(rds, sResult, dts1, true)){
    trace(ERROR, "(0)Eval expression '%s' failed.\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }
  string scase,sreturn;
  for (size_t i=1;i<m_params.size();i++){
    if (!m_params[i].evalExpression(rds, scase, dts2, true)){
      trace(ERROR, "(1)Eval expression '%s' failed.\n",m_params[i].getEntireExpstr().c_str());
      return false;
    }
    if (i+1<m_params.size()){
      if (!m_params[i+1].evalExpression(rds, sreturn, dts, true)){
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
    if (dts1.datatype != dts2.datatype || dts1.datatype != dts.datatype){
      dts1 = dts;
      dts2 = dts;
    }
    if (anyDataCompare(sResult,scase,dts1,dts2) == 0){
      sResult = sreturn;
      m_datatype = dts;
      return true;
    }
  }
  //trace(DEBUG2,"Retruning original '%s'\n",sResult.c_str());
  dts = dts1;
  return true;
}

// when(condition1,return1[,condition1,result2...],else): if input equal to case1, then return return1, etc.. If none matched, return default or return input if no default provided.
bool FunctionC::runWhen(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 2 || m_filters.size()<1){
    trace(ERROR, "when() function accepts at least three parameters.\n");
    return false;
  }
  if (m_params.size() != m_filters.size() + 1){
    trace(ERROR, "Return expression number (%d) does not match condtion number (%d) in when() function!\n", m_params.size(), m_filters.size());
    return false;
  }
  vector< unordered_map< int,int > > sideMatchedRowIDs;
  for (size_t i=0; i<m_filters.size(); i++){
    if (m_filters[i].compareExpression(rds, sideMatchedRowIDs) && m_params[i].evalExpression(rds, sResult, dts, true)){
      m_datatype = dts;
      return true;
    }
  }
  if (m_params[m_params.size()-1].evalExpression(rds, sResult, dts, true))
    return true;
  else{
    trace(ERROR, "Failed to run when()!\n");
    return false;
  }
}

bool FunctionC::runPad(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "pad() function accepts only two parameters.\n");
    return false;
  }
  string seed, sLen;
  if (m_params[0].evalExpression(rds, seed, dts, true) && m_params[1].evalExpression(rds, sLen, dts, true)){
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

bool FunctionC::runGreatest(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 1){
    trace(ERROR, "greatest() function accepts at least one parameter.\n");
    return false;
  }
  string scomp;
  DataTypeStruct dts1, dts2;
  for (size_t i=0;i<m_params.size();i++){
    if (m_params[i].m_type == LEAF && m_params[i].m_expType == FUNCTION && m_params[i].m_Function && m_params[i].m_Function->m_funcID==FOREACH){
      vector<ExpressionC> vExpandedExpr;
      m_params[i].m_Function->expandForeach(vExpandedExpr, rds.fieldvalues->size());
      for (size_t j=0; j<vExpandedExpr.size(); j++){
        vExpandedExpr[j].analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, rds.sideDatatypes);
        if (!vExpandedExpr[j].evalExpression(rds, scomp, dts2, true)){
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
          if (dts1.datatype != dts2.datatype || dts1.datatype != dts.datatype){
            dts1 = dts;
            dts2 = dts;
          }
          
          //trace(DEBUG2,"Comparing '%s'(%d) '%s'(%d) => %d \n",scomp.c_str(),dts2.datatype,sResult.c_str(),dts1.datatype,anyDataCompare(scomp,sResult,dts1,dts2));
          if (anyDataCompare(scomp,sResult,dts1,dts2)>0)
            sResult = scomp;
        }
      }
    }else{
      if (!m_params[i].evalExpression(rds, scomp, dts2, true)){
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
        if (dts1.datatype != dts2.datatype || dts1.datatype != dts.datatype){
          dts1 = dts;
          dts2 = dts;
        }

        //trace(DEBUG2,"Comparing '%s'(%d) '%s'(%d) => %d \n",scomp.c_str(),dts2.datatype,sResult.c_str(),dts1.datatype,anyDataCompare(scomp,sResult,dts,dts));
        if (anyDataCompare(scomp,sResult,dts1,dts2)>0)
          sResult = scomp;
      }
    }
  }

  dts.datatype = ANY;
  return true;
}

bool FunctionC::runLeast(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 1){
    trace(ERROR, "least() function accepts at least one parameter.\n");
    return false;
  }
  string scomp;
  DataTypeStruct dts1, dts2;
  for (size_t i=0;i<m_params.size();i++){
    if (m_params[i].m_type == LEAF && m_params[i].m_expType == FUNCTION && m_params[i].m_Function && m_params[i].m_Function->m_funcID==FOREACH){
      vector<ExpressionC> vExpandedExpr;
      m_params[i].m_Function->expandForeach(vExpandedExpr, rds.fieldvalues->size());
      for (size_t j=0; j<vExpandedExpr.size(); j++){
        vExpandedExpr[j].analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, rds.sideDatatypes);
        if (!vExpandedExpr[j].evalExpression(rds, scomp, dts2, true)){
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
          if (dts1.datatype != dts2.datatype || dts1.datatype != dts.datatype){
            dts1 = dts;
            dts2 = dts;
          }
          //trace(DEBUG2,"Comparing '%s'(%d) '%s'(%d) => %d \n",scomp.c_str(),dts2.datatype,sResult.c_str(),dts1.datatype,anyDataCompare(scomp,sResult,dts1,dts2));
          if (anyDataCompare(scomp,sResult,dts1,dts2)<0)
            sResult = scomp;
        }
      }
    }else{
      if (!m_params[i].evalExpression(rds, scomp, dts2, true)){
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
        if (dts1.datatype != dts2.datatype || dts1.datatype != dts.datatype){
          dts1 = dts;
          dts2 = dts;
        }
        
        //trace(DEBUG2,"Comparing '%s'(%d) '%s'(%d) => %d \n",scomp.c_str(),dts2.datatype,sResult.c_str(),dts1.datatype,anyDataCompare(scomp,sResult,dts1, dts2));
        if (anyDataCompare(scomp,sResult,dts1, dts2)<0)
          sResult = scomp;
      }
    }
  }

  dts.datatype = ANY;
  return true;
}

bool FunctionC::runSumall(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() < 1){
    trace(ERROR, "sumall() function accepts at least one parameter.\n");
    return false;
  }
  string scomp;
  double dResult=0;
  DataTypeStruct dts1, dts2;
  for (size_t i=0;i<m_params.size();i++){
    if (m_params[i].m_type == LEAF && m_params[i].m_expType == FUNCTION && m_params[i].m_Function && m_params[i].m_Function->m_funcID==FOREACH){
      vector<ExpressionC> vExpandedExpr;
      m_params[i].m_Function->expandForeach(vExpandedExpr, rds.fieldvalues->size());
      for (size_t j=0; j<vExpandedExpr.size(); j++){
        vExpandedExpr[j].analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, rds.sideDatatypes);
        if (!vExpandedExpr[j].evalExpression(rds, scomp, dts2, true) || !isDouble(scomp)){
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
      if (!m_params[i].evalExpression(rds, scomp, dts2, true) || !isDouble(scomp)){
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

bool FunctionC::runEval(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "eval() function accepts only one parameter.\n");
    return false;
  }
  string sExpr;
  if (!m_params[0].evalExpression(rds, sExpr, dts, true)){
    trace(ERROR, "Failed to get '%s'.\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }
  ExpressionC expr = ExpressionC(sExpr);
  expr.analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, rds.sideDatatypes);
  if (!expr.evalExpression(rds, sResult, dts, true)){
    trace(ERROR, "Failed to run eval(%s)!\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }

  m_datatype = dts;
  return true;
}

bool FunctionC::runExec(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "exec() function accepts only one parameter.\n");
    return false;
  }
  string sCmd;
  if (!m_params[0].evalExpression(rds, sCmd, dts, true)){
    trace(ERROR, "Failed to get '%s' for exec().\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }
  sResult = exec(sCmd);
  dts.datatype = STRING;

  return true;
}

bool FunctionC::runUsermacro(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  unordered_map< string,string > mFuncParas;
  if (!rds.macroFuncExprs || rds.macroFuncExprs->find(m_funcName)==rds.macroFuncExprs->end()){
    trace(ERROR, "Failed to get the macro function '%s' definition.\n",m_funcName.c_str());
    return false;
  }
  for (size_t i=0; i<m_params.size(); i++){
    string paraVal;
    if (!m_params[i].evalExpression(rds, paraVal, dts, true)){
      trace(ERROR, "Failed to get the parameter value of '%s'.\n",m_funcName.c_str());
      return false;
    }
    if (i<(*rds.macroFuncExprs)[m_funcName].vParaNames.size())
      mFuncParas.insert(pair< string,string >((*rds.macroFuncExprs)[m_funcName].vParaNames[i],paraVal));
    else
      trace(WARNING, "The passed in parameter number is more the defined number!\n",m_funcName.c_str());
  }
  rds.macroFuncParas = &mFuncParas;
  if (!(*rds.macroFuncExprs)[m_funcName].funcExpr || !(*rds.macroFuncExprs)[m_funcName].funcExpr->evalExpression(rds, sResult, dts, true)){
    trace(ERROR, "Failed to get the result of the macro function '%s' definition.\n",m_funcName.c_str());
    return false;
  }

  m_datatype = dts;
  return true;
}

bool FunctionC::runRcount(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (!rds.sideDatasets){
    trace(ERROR, "Something wrong with the side word data set, failed to run rcount().\n");
    return false;
  }
  
  string s1, s2, memval;
  if (m_params.size() == 0){
    sResult = intToStr(rds.sideDatasets->size());
    return true;
  }else{
    if (!m_params[0].evalExpression(rds, s1, dts, true) || !isInt(s1)){
      trace(ERROR, "'%s' is not a valid id of side work data set for rcount()!\n",m_params[0].getEntireExpstr().c_str());
      return false;
    }
    int iS1 = atoi(s1.c_str())-1;
    if (iS1 < 0 || iS1 >= rds.sideDatasets->size()){
      trace(ERROR, "%d is out of range of side work data set (%d) for rcount()!\n",iS1, rds.sideDatasets->size());
      return false;
    }
    if (m_params.size() == 1){
      sResult = intToStr((int)(*rds.sideDatasets)[iS1].size());
      return true;
    }else{
      if (!m_params[1].evalExpression(rds, s2, dts, true)){
        trace(ERROR, "'%s' is not a valid id of field for rcount()!\n",m_params[1].getEntireExpstr().c_str());
        return false;
      }
      if (m_params.size() == 2){
        sResult = intToStr((int)(*rds.sideDatasets)[iS1].size()); // size of each field actually equal to size of each data set.
        return true;
      }else{
        if (!m_params[2].evalExpression(rds, memval, dts, true)){
          trace(ERROR, "'%s' is not a valid member value for rcount()!\n",m_params[2].getEntireExpstr().c_str());
          return false;
        }
        int nCount=0;
        for (size_t i=0; i<(*rds.sideDatasets)[iS1].size(); i++){
          if ((*rds.sideDatasets)[iS1][i].find(s2)!=(*rds.sideDatasets)[iS1][i].end() && (*rds.sideDatasets)[iS1][i][s2].compare(memval)==0)
            nCount++;
        }
        sResult = intToStr(nCount); // size of each field actually equal to size of each data set.
        return true;
      }
    }
  }

  dts.datatype = LONG;
  return true;
}

bool FunctionC::runRmember(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (!rds.sideDatasets){
    trace(ERROR, "Something wrong with the side word data set, failed to run rmember().\n");
    return false;
  }
  
  if (m_params.size() < 3){
    trace(ERROR, "rmember() function accepts three parameters.\n");
    return false;
  }
  string s1, s2, mid;
  if (!m_params[0].evalExpression(rds, s1, dts, true) || !isInt(s1)){
    trace(ERROR, "'%s' is not a valid id of side work data set for rmember()!\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }
  if (!m_params[1].evalExpression(rds, s2, dts, true)){
    trace(ERROR, "'%s' is not a valid id of field for rmember()!\n",m_params[1].getEntireExpstr().c_str());
    return false;
  }
  if (!m_params[2].evalExpression(rds, mid, dts, true) || !isInt(mid)){
    trace(ERROR, "'%s' is not a valid id of member for rmember()!\n",m_params[2].getEntireExpstr().c_str());
    return false;
  }
  int iS1 = atoi(s1.c_str())-1;
  if (iS1 < 0 || iS1 >= rds.sideDatasets->size()){
    trace(ERROR, "%d is out of range of side work data set (%d) for rmember()!\n",iS1, rds.sideDatasets->size());
    return false;
  }
  int nid = atoi(mid.c_str());
  if (nid > 0 && nid <= (*rds.sideDatasets)[iS1].size()){
    if ((*rds.sideDatasets)[iS1][nid-1].find(s2)!=(*rds.sideDatasets)[iS1][nid-1].end()){
      sResult = (*rds.sideDatasets)[iS1][nid-1][s2];
      return true;
    }else{
      sResult = ""; // return empty if not found.
      return true;
    }
  }else if (nid < 0 && abs(nid) <= (*rds.sideDatasets)[iS1].size()){
    if ((*rds.sideDatasets)[iS1][nid+(*rds.sideDatasets)[iS1].size()].find(s2)!=(*rds.sideDatasets)[iS1][nid+(*rds.sideDatasets)[iS1].size()].end()){
      sResult = (*rds.sideDatasets)[iS1][nid+(*rds.sideDatasets)[iS1].size()][s2];
      m_datatype = dts;
      return true;
    }else{
      sResult = ""; // return empty if not found.
      return true;
    }
  }else{
    trace(ERROR, "%d is out of range of side work data result set (%d) for rmember()!\n", nid, (*rds.sideDatasets)[iS1].size());
    return false;
  }
}


bool FunctionC::runRmembers(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (!rds.sideDatasets){
    trace(ERROR, "Something wrong with the side word data set, failed to run rmembers().\n");
    return false;
  }
  
  if (m_params.size() < 3){
    trace(ERROR, "rmembers() function accepts three or four parameters.\n");
    return false;
  }
  string s1, s2, sStartid, sEndid, sDelm="|";
  if (!m_params[0].evalExpression(rds, s1, dts, true) || !isInt(s1)){
    trace(ERROR, "'%s' is not a valid id of side work data set for rmembers()!\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }
  if (!m_params[1].evalExpression(rds, s2, dts, true)){
    trace(ERROR, "'%s' is not a valid id of field for rmembers()!\n",m_params[1].getEntireExpstr().c_str());
    return false;
  }
  if (!m_params[2].evalExpression(rds, sStartid, dts, true) || !isInt(sStartid)){
    trace(ERROR, "'%s' is not a valid id of member for rmembers()!\n",m_params[2].getEntireExpstr().c_str());
    return false;
  }
  int iS1 = atoi(s1.c_str())-1;
  if (iS1 < 0 || iS1 >= rds.sideDatasets->size()){
    trace(ERROR, "%d is out of range of side work data set (%d) for rmembers()!\n",iS1, rds.sideDatasets->size());
    return false;
  }
  sEndid = (m_params.size() <= 3 || !m_params[3].evalExpression(rds, sEndid, dts, true) || !isInt(sEndid))?sStartid:sEndid;

  int iStartid = atoi(sStartid.c_str());
  iStartid = iStartid>0?iStartid-1:iStartid+(*rds.sideDatasets)[iS1].size();
  if (iStartid>=(*rds.sideDatasets)[iS1].size()){
    trace(WARNING, "%d is out of range of part number in rmembers()!\n", iStartid);
    iStartid = (*rds.sideDatasets)[iS1].size()-1;
  }
  int iEndid = atoi(sEndid.c_str());
  iEndid = iEndid>0?iEndid-1:iEndid+(*rds.sideDatasets)[iS1].size();
  if (iEndid>=(*rds.sideDatasets)[iS1].size()){
    trace(WARNING, "%d is out of range of part number in rmembers()!\n", iEndid);
    iEndid = (*rds.sideDatasets)[iS1].size()-1;
  }
  if (m_params.size() >= 5)
    m_params[4].evalExpression(rds, sDelm, dts, true);
  dts.datatype = STRING;  
  sResult = "";
  for (size_t i=iStartid; iStartid<=iEndid?i<=iEndid:i>=iEndid; iStartid<=iEndid?i++:i--)
    sResult+=(*rds.sideDatasets)[iS1][i][s2]+(i!=iEndid?sDelm:"");
  return true;
}

bool FunctionC::runRmemberid(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  if (!rds.sideDatasets){
    trace(ERROR, "Something wrong with the side word data set, failed to run rmemberid().\n");
    return false;
  }
  
  if (m_params.size() < 3){
    trace(ERROR, "rmemberid() function accepts three parameters.\n");
    return false;
  }
  string s1, s2, memval;
  if (!m_params[0].evalExpression(rds, s1, dts, true) || !isInt(s1)){
    trace(ERROR, "'%s' is not a valid id of side work data set for rmemberid()!\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }
  if (!m_params[1].evalExpression(rds, s2, dts, true)){
    trace(ERROR, "'%s' is not a valid id of field for rmemberid()!\n",m_params[1].getEntireExpstr().c_str());
    return false;
  }
  if (!m_params[2].evalExpression(rds, memval, dts, true)){
    trace(ERROR, "'%s' is not a valid id of member for rmemberid()!\n",m_params[2].getEntireExpstr().c_str());
    return false;
  }
  int iS1 = atoi(s1.c_str())-1;
  if (iS1 < 0 || iS1 >= rds.sideDatasets->size()){
    trace(ERROR, "%d is out of range of side work data set (%d) for rmemberid()!\n",iS1, rds.sideDatasets->size());
    return false;
  }
  dts.datatype = LONG;
  for (size_t i=0; i<(*rds.sideDatasets)[iS1].size(); i++){
    if ((*rds.sideDatasets)[iS1][i].find(s2)!=(*rds.sideDatasets)[iS1][i].end() && (*rds.sideDatasets)[iS1][i][s2].compare(memval)==0){
      sResult = intToStr(i+1);
      return true;
    }
  }
  sResult = "-1"; // return empty if not found.
  return true;
}

// expand foreach to a vector of expression
// foreach(beginid,endid,macro_expr[,step]). $ stands for field, # stands for field sequence, % stands for the largest field sequence ID.
void FunctionC::expandForeach(vector<ExpressionC> & vExpr, const int & maxFieldNum)
{
  trace(DEBUG,"(1)Expanding foreach expression '%s'\n",m_expStr.c_str());
  if (m_funcID!=FOREACH && m_funcID!=ANYCOL && m_funcID!=ALLCOL && m_funcID!=CONCATCOL && m_funcID!=CALCOL){
    trace(ERROR, "(1)'%s' is not foreach macro function!\n", m_funcName.c_str());
    return;
  }
  if (m_params.size()<3){
    trace(ERROR, "(1)Foreach macro function requires at least 3 parameters!\n", m_funcName.c_str());
    return;
  }
  int begin = 0, end = 0;
  if (m_params[0].getEntireExpstr().find("%") != string::npos){
    string expStr = m_params[0].getEntireExpstr();
    replacestr(expStr,"%",intToStr(maxFieldNum));
    ExpressionC constExp(expStr);
    if (!isInt(constExp.m_expStr)){
      trace(ERROR, "(1a)'%s' is not a valid number expresion!\n", m_params[0].getEntireExpstr().c_str());
      return;
    }
    begin = max(1,atoi(constExp.m_expStr.c_str()));
  }else if (isInt(m_params[0].m_expStr))
    begin = max(1,min(maxFieldNum,atoi(m_params[0].m_expStr.c_str())));
  else{
    trace(ERROR, "(1)%s is an invalid begin ID for foreach macro function!\n", m_params[0].getEntireExpstr().c_str());
    return;
  }
  if (m_params[1].getEntireExpstr().find("%") != string::npos){
    string expStr = m_params[1].getEntireExpstr();
    replacestr(expStr,"%",intToStr(maxFieldNum));
    ExpressionC constExp(expStr);
    if (!isInt(constExp.m_expStr)){
      trace(ERROR, "(1b)'%s' is not a valid number expresion!\n", m_params[1].getEntireExpstr().c_str());
      return;
    }
    end = max(1,atoi(constExp.m_expStr.c_str()));
  }else if (isInt(m_params[1].m_expStr))
    end = max(1,min(maxFieldNum,atoi(m_params[1].m_expStr.c_str())));
  else{
    trace(ERROR, "(1)Invalid end ID for foreach macro function!\n", m_params[1].getEntireExpstr().c_str());
    return;
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
    vExpr.push_back(ExpressionC(sNew));
    trace(DEBUG,"(1)Expanded foreach element '%s'\n",vExpr[vExpr.size()-1].getEntireExpstr().c_str());
  }
}

void FunctionC::expandForeach(vector<ExpressionC> & vExpr, const vector<ExpressionC> & vExps)
{
  trace(DEBUG,"(2)Expanding foreach expression '%s'\n",m_expStr.c_str());
  if (m_funcID!=FOREACH && m_funcID!=ANYCOL && m_funcID!=ALLCOL){
    trace(ERROR, "(2)'%s' is not foreach macro function!\n", m_funcName.c_str());
    return;
  }
  if (m_params.size()<3){
    trace(ERROR, "(2)Foreach macro function requires 3 parameters!\n", m_funcName.c_str());
    return;
  }
  int begin = 0, end = 0;
    if (m_params[0].getEntireExpstr().find("%") != string::npos){
    string expStr = m_params[0].getEntireExpstr();
    replacestr(expStr,"%",intToStr(vExps.size()));
    ExpressionC constExp(expStr);
    if (!isInt(constExp.m_expStr)){
      trace(ERROR, "(2a)'%s' is not a valid number expresion!\n", m_params[0].getEntireExpstr().c_str());
      return;
    }
    begin = max(1,atoi(constExp.m_expStr.c_str()));
  }else if (isInt(m_params[0].m_expStr))
    begin = max(1,min((int)vExps.size(),atoi(m_params[0].m_expStr.c_str())));
  else{
    trace(ERROR, "(2)%s is an invalid begin ID for foreach macro function!\n", m_params[0].getEntireExpstr().c_str());
    return;
  }
  if (m_params[1].getEntireExpstr().find("%") != string::npos){
    string expStr = m_params[1].getEntireExpstr();
    replacestr(expStr,"%",intToStr(vExps.size()));
    ExpressionC constExp(expStr);
    if (!isInt(constExp.m_expStr)){
      trace(ERROR, "(2b)'%s' is not a valid number expresion!\n", m_params[1].getEntireExpstr().c_str());
      return;
    }
    end = atoi(constExp.m_expStr.c_str());;
  }else if (isInt(m_params[1].m_expStr))
    end = max(0,min((int)vExps.size(),atoi(m_params[1].m_expStr.c_str())));
  else{
    trace(ERROR, "(2)Invalid end ID for foreach macro function!\n", m_params[1].getEntireExpstr().c_str());
    return;
  }
  for (int i=begin; begin<end?i<=end:i>=end; begin<end?i++:i--){
    trace(DEBUG,"(2)Expanding foreach element '%s'\n",m_params[2].getEntireExpstr().c_str());
    string sNew = m_params[2].getEntireExpstr();
    replaceunquotedstr(sNew,"$",vExps[i-1].getEntireExpstr(),"''",'\\',{'(',')'});
    replaceunquotedstr(sNew,"#",intToStr(i),"''",'\\',{'(',')'});
    vExpr.push_back(ExpressionC(sNew));
    trace(DEBUG,"(2)Expanded foreach element '%s'\n",vExpr[vExpr.size()-1].getEntireExpstr().c_str());
  }
}

// run function and get result
bool FunctionC::runFunction(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts)
{
  bool getResult = false;
  switch(m_funcID){
    case UPPER:
      getResult = runUpper(rds, sResult, dts);
      break;
    case LOWER:
      getResult = runLower(rds, sResult, dts);
      break;
    case SUBSTR:
      getResult = runSubstr(rds, sResult, dts);
      break;
    case FLOOR:
      getResult = runFloor(rds, sResult, dts);
      break;
    case CEIL:
      getResult = runCeil(rds, sResult, dts);
      break;
    case TIMEDIFF:
      getResult = runTimediff(rds, sResult, dts);
      break;
    case ADDTIME:
      getResult = runAddtime(rds, sResult, dts);
      break;
    case ISNULL:
      getResult = runIsnull(rds, sResult, dts);
      break;
    case INSTR:
      getResult = runInstr(rds, sResult, dts);
      break;
    case STRLEN:
      getResult = runStrlen(rds, sResult, dts);
      break;
    case COMPARESTR:
      getResult = runComparestr(rds, sResult, dts);
      break;
    case COMPARENUM:
      getResult = runComparenum(rds, sResult, dts);
      break;
    case COMPAREDATE:
      getResult = runComparedate(rds, sResult, dts);
      break;
    case NOCASECOMPARESTR:
      getResult = runNoCaseComparestr(rds, sResult, dts);
      break;
    case REPLACE:
      getResult = runReplace(rds, sResult, dts);
      break;
    case REGREPLACE:
      getResult = runRegreplace(rds, sResult, dts);
      break;
    case REGMATCH:
      getResult = runRegmatch(rds, sResult, dts);
      break;
    case COUNTWORD:
      getResult = runCountword(rds, sResult, dts);
      break;
    case COUNTSTR:
      getResult = runCountstr(rds, sResult, dts);
      break;
    case FIELDNAME:
      getResult = runFieldname(rds, sResult, dts);
      break;
    case CONCAT:
      getResult = runConcat(rds, sResult, dts);
      break;
    case CONCATCOL:
      getResult = runConcatcol(rds, sResult, dts);
      break;
    case CALCOL:
      getResult = runCalcol(rds, sResult, dts);
      break;
    case APPENDFILE:
      getResult = runAppendfile(rds, sResult, dts);
      break;
    case GETWORD:
      getResult = runGetword(rds, sResult, dts);
      break;
    case GETPART:
      getResult = runGetpart(rds, sResult, dts);
      break;
    case GETPARTS:
      getResult = runGetparts(rds, sResult, dts);
      break;
    case RANDSTR:
      getResult = runRandstr(rds, sResult, dts);
      break;
    case TRIMLEFT:
      getResult = runTrimleft(rds, sResult, dts);
      break;
    case TRIMRIGHT:
      getResult = runTrimright(rds, sResult, dts);
      break;
    case TRIM:
      getResult = runTrim(rds, sResult, dts);
      break;
    case PAD:
      getResult = runPad(rds, sResult, dts);
      break;
    case CAMELSTR:
      getResult = runCamelstr(rds, sResult, dts);
      break;
    case SNAKESTR:
      getResult = runSnakestr(rds, sResult, dts);
      break;
    case REVERTSTR:
      getResult = runRevertstr(rds, sResult, dts);
      break;
    case FINDNTH:
      getResult = runFindnth(rds, sResult, dts);
      break;
    case ASCII:
      getResult = runAscii(rds, sResult, dts);
      break;
    case CHAR:
      getResult = runChar(rds, sResult, dts);
      break;
    case URLENCODE:
      getResult = runUrlencode(rds, sResult, dts);
      break;
    case URLDECODE:
      getResult = runUrldecode(rds, sResult, dts);
      break;
    case BASE64ENCODE:
      getResult = runBase64encode(rds, sResult, dts);
      break;
    case BASE64DECODE:
      getResult = runBase64decode(rds, sResult, dts);
      break;
    case DUPLICATE:
      getResult = runDuplicate(rds, sResult, dts);
      break;
    case MD5:
      getResult = runMd5(rds, sResult, dts);
      break;
    case HASH:
      getResult = runHash(rds, sResult, dts);
      break;
    case ISIP:
      getResult = runIsip(rds, sResult, dts);
      break;
    case ISIPV6:
      getResult = runIsipv6(rds, sResult, dts);
      break;
    case ISMAC:
      getResult = runIsmac(rds, sResult, dts);
      break;
    case MYIPS:
      getResult = runMyips(rds, sResult, dts);
      break;
    case HOSTNAME:
      getResult = runHostname(rds, sResult, dts);
      break;
    case RMEMBERS:
      getResult = runRmembers(rds, sResult, dts);
      break;
    case ISFILE:
      getResult = runIsfile(rds, sResult, dts);
      break;
    case ISFOLDER:
      getResult = runIsfolder(rds, sResult, dts);
      break;
    case FILEEXIST:
      getResult = runFileexist(rds, sResult, dts);
      break;
    case RMFILE:
      getResult = runRmfile(rds, sResult, dts);
      break;
    case RENAMEFILE:
      getResult = runRenamefile(rds, sResult, dts);
      break;
    case FILESIZE:
      getResult = runFilesize(rds, sResult, dts);
      break;
    case FILEATTRS:
      getResult = runFileattrs(rds, sResult, dts);
      break;
    case ISEXECUTABLE:
      getResult = runIsexecutable(rds, sResult, dts);
      break;
    case ISSYMBLINK:
      getResult = runIssymblink(rds, sResult, dts);
      break;
    case GETSYMBLINK:
      getResult = runGetsymblink(rds, sResult, dts);
      break;
    case MOD:
      getResult = runMod(rds, sResult, dts);
      break;
    case ABS:
      getResult = runAbs(rds, sResult, dts);
      break;
    case ACOS:
      getResult = runAcos(rds, sResult, dts);
      break;
    case ACOSH:
      getResult = runAcosh(rds, sResult, dts);
      break;
    case ASIN:
      getResult = runAsin(rds, sResult, dts);
      break;
    case ASINH:
      getResult = runAsinh(rds, sResult, dts);
      break;
    case ATAN:
      getResult = runAtan(rds, sResult, dts);
      break;
    case ATAN2:
      getResult = runAtan2(rds, sResult, dts);
      break;
    case ATANH:
      getResult = runAtanh(rds, sResult, dts);
      break;
    case CBRT:
      getResult = runCbrt(rds, sResult, dts);
      break;
    case COPYSIGN:
      getResult = runCopysign(rds, sResult, dts);
      break;
    case COS:
      getResult = runCos(rds, sResult, dts);
      break;
    case COSH:
      getResult = runCosh(rds, sResult, dts);
      break;
    case ERF:
      getResult = runErf(rds, sResult, dts);
      break;
    case EXP:
      getResult = runExp(rds, sResult, dts);
      break;
    case EXP2:
      getResult = runExp2(rds, sResult, dts);
      break;
    case FMA:
      getResult = runFma(rds, sResult, dts);
      break;
    case FMOD:
      getResult = runFmod(rds, sResult, dts);
      break;
    case FPCLASSIFY:
      getResult = runFpclassify(rds, sResult, dts);
      break;
    case HYPOT:
      getResult = runHypot(rds, sResult, dts);
      break;
    case ILOGB:
      getResult = runIlogb(rds, sResult, dts);
      break;
    case ISFINITE:
      getResult = runIsfinite(rds, sResult, dts);
      break;
    case ISINF:
      getResult = runIsinf(rds, sResult, dts);
      break;
    case ISNORMAL:
      getResult = runIsnormal(rds, sResult, dts);
      break;
    case LGAMMA:
      getResult = runLgamma(rds, sResult, dts);
      break;
    case LOG10:
      getResult = runLog10(rds, sResult, dts);
      break;
    case LOG2:
      getResult = runLog2(rds, sResult, dts);
      break;
    case POW:
      getResult = runPow(rds, sResult, dts);
      break;
    case REMAINDER:
      getResult = runRemainder(rds, sResult, dts);
      break;
    case SCALBLN:
      getResult = runScalbln(rds, sResult, dts);
      break;
    case SCALBN:
      getResult = runScalbn(rds, sResult, dts);
      break;
    case SIN:
      getResult = runSin(rds, sResult, dts);
      break;
    case SINH:
      getResult = runSinh(rds, sResult, dts);
      break;
    case SQRT:
      getResult = runSqrt(rds, sResult, dts);
      break;
    case TAN:
      getResult = runTan(rds, sResult, dts);
      break;
    case TANH:
      getResult = runTanh(rds, sResult, dts);
      break;
    case TGAMMA:
      getResult = runTgamma(rds, sResult, dts);
      break;
    case PI:
      getResult = runPi(rds, sResult, dts);
      break;
    case DATATYPE:
      getResult = runDatatype(rds, sResult, dts);
      break;
    case TOINT:
      getResult = runToint(rds, sResult, dts);
      break;
    case TOLONG:
      getResult = runTolong(rds, sResult, dts);
      break;
    case TOFLOAT:
      getResult = runTofloat(rds, sResult, dts);
      break;
    case TOSTR:
      getResult = runTostr(rds, sResult, dts);
      break;
    case TODATE:
      getResult = runTodate(rds, sResult, dts);
      break;
    case DECTOHEX:
      getResult = runDectohex(rds, sResult, dts);
      break;
    case HEXTODEC:
      getResult = runHextodec(rds, sResult, dts);
      break;
    case DECTOBIN:
      getResult = runDectobin(rds, sResult, dts);
      break;
    case BINTODEC:
      getResult = runBintodec(rds, sResult, dts);
      break;
    case SWITCH:
      getResult = runSwitch(rds, sResult, dts);
      break;
    case WHEN:
      getResult = runWhen(rds, sResult, dts);
      break;
    case GREATEST:
      getResult = runGreatest(rds, sResult, dts);
      break;
    case LEAST:
      getResult = runLeast(rds, sResult, dts);
      break;
    case SUMALL:
      getResult = runSumall(rds, sResult, dts);
      break;
    case EVAL:
      getResult = runEval(rds, sResult, dts);
      break;
    case RCOUNT:
      getResult = runRcount(rds, sResult, dts);
      break;
    case RMEMBER:
      getResult = runRmember(rds, sResult, dts);
      break;
    case RMEMBERID:
      getResult = runRmemberid(rds, sResult, dts);
      break;
    case ROUND:
      getResult = runRound(rds, sResult, dts);
      break;
    case LOG:
      getResult = runLog(rds, sResult, dts);
      break;
    case RANDOM:
      getResult = runRandom(rds, sResult, dts);
      break;
    case DATEFORMAT:
      getResult = runDateformat(rds, sResult, dts);
      break;
    case TRUNCDATE:
      getResult = runTruncdate(rds, sResult, dts);
      break;
    case TRUNCDATEU:
      getResult = runTruncdateu(rds, sResult, dts);
      break;
    case YEARDAY:
      getResult = runYearday(rds, sResult, dts);
      break;
    case NOW:
      getResult = runNow(rds, sResult, dts);
      break;
    case ISLEAP:
      getResult = runIsleap(rds, sResult, dts);
      break;
    case WEEKDAY:
      getResult = runWeekday(rds, sResult, dts);
      break;
    case MONTHFIRSTDAY:
      getResult = runMonthfirstday(rds, sResult, dts);
      break;
    case MONTHFIRSTMONDAY:
      getResult = runMonthfirstmonday(rds, sResult, dts);
      break;
    case YEARWEEK:
      getResult = runYearweek(rds, sResult, dts);
      break;
    case DATETOLONG:
      getResult = runDatetolong(rds, sResult, dts);
      break;
    case LONGTODATE:
      getResult = runLongtodate(rds, sResult, dts);
      break;
    case LOCALTIME:
      getResult = runLocaltime(rds, sResult, dts);
      break;
    case GMTIME:
      getResult = runGmtime(rds, sResult, dts);
      break;
    case EXEC:
      getResult = runExec(rds, sResult, dts);
      break;
    case DETECTDT:
      getResult = runDetectdt(rds, sResult, dts);
      break;
    case ISLONG:
      getResult = runIslong(rds, sResult, dts);
      break;
    case ISDOUBLE:
      getResult = runIsdouble(rds, sResult, dts);
      break;
    case ISDATE:
      getResult = runIsdate(rds, sResult, dts);
      break;
    case ISSTRING:
      getResult = runIsstring(rds, sResult, dts);
      break;
    case COUNTPART:
      getResult = runCountpart(rds, sResult, dts);
      break;
    case REGCOUNT:
      getResult = runRegcount(rds, sResult, dts);
      break;
    case REGGET:
      getResult = runRegget(rds, sResult, dts);
      break;
    case USERMACROFUNC:
      getResult = runUsermacro(rds, sResult, dts);
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
      if (m_params[0].evalExpression(rds, sResult, dts, true))
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
      for (size_t i=0; i<m_params.size(); i++)
        if (!m_params[i].evalExpression(rds, sResult, dts, true)){
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
