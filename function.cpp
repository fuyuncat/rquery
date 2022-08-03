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
#include "function.h"

void FunctionC::init()
{
  m_datatype.datatype = UNKNOWN;   // 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
  m_datatype.extrainfo = "";
  m_expStr = "";          // it's the full function string, including function name and parameters
  m_funcName = "";        // analyzed function name, upper case
  m_funcID = UNKNOWN;
  m_params.clear();       // parameter expressions
  m_expstrAnalyzed = false;
  m_fieldnames = NULL;
  m_fieldtypes = NULL;

  m_metaDataAnzlyzed = false; // analyze column name to column id.
}

FunctionC::FunctionC()
{
  init();
}

FunctionC::FunctionC(string expStr)
{
  init();
  setExpStr(expStr);
}

FunctionC::~FunctionC()
{

}

void FunctionC::setExpStr(string expStr)
{
  clear();
  m_expStr = expStr;
  if (!analyzeExpStr()){
    m_funcName = "";
    m_funcID = UNKNOWN;
    m_params.clear();
  }
}

bool FunctionC::isConst()
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

bool FunctionC::isAggFunc()
{
  return (m_funcID==SUM || m_funcID==COUNT || m_funcID==UNIQUECOUNT || m_funcID==MAX || m_funcID==MIN || m_funcID==AVERAGE);
}

bool FunctionC::isMacro()
{
  return (m_funcID==FOREACH);
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
  int iPos = 0;
  string strParams = readQuotedStr(m_expStr, iPos, "()", '\0');
  //if (iPos<0 || strParams.empty()){
  //  trace(ERROR, "No quoted parameters found in '%s'!\n", m_expStr.c_str());
  //  m_expstrAnalyzed = false;
  //  return false;
  //}
  m_funcName = trim_copy(upper_copy(m_expStr.substr(0, m_expStr.find("("))));
  m_expStr = m_funcName+"("+strParams+")";
  //strParams = trim_pair(strParams, "()");
  vector<string> vParams = split(strParams,',',"''()",'\\',{'(',')'});
  for (int i=0; i<vParams.size(); i++){
    trace(DEBUG, "Processing parameter(%d) '%s'!\n", i, vParams[i].c_str());
    string sParam = trim_copy(vParams[i]);
    if (sParam.empty()){
      trace(ERROR, "Empty parameter string!\n");
      m_expstrAnalyzed = false;
      return false;
    }
    ExpressionC eParam = ExpressionC(sParam);
    //trace(DEBUG2,"'%s' merged const to '%s'.\n",sParam.c_str(),eParam.getEntireExpstr().c_str());
    //eParam.analyzeColumns(m_fieldnames, m_fieldtypes, rawDatatype);
    m_params.push_back(eParam);
  }
  m_funcID = encodeFunction(m_funcName);
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
      m_datatype.datatype = STRING;
      break;
    case FLOOR:
    case CEIL:
    case ROUND:
    case INSTR:
    case COMPARESTR:
    case NOCASECOMPARESTR:
    case STRLEN:
    case COUNT:
    case UNIQUECOUNT:
    case ISNULL:
    case COUNTWORD:
      m_datatype.datatype = LONG;
      break;
    case TIMEDIFF:
    case LOG:
    case AVERAGE:
    case SUM:
      m_datatype.datatype = DOUBLE;
      break;
    case NOW:
    case TRUNCDATE:
      m_datatype.datatype = DATE;
      break;
    case MAX:
    case MIN:
    case SWITCH:
    case GREATEST:
    case LEAST: // MAX and MIN could be any data type
    case FOREACH:
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
DataTypeStruct FunctionC::analyzeColumns(vector<string>* fieldnames, vector<DataTypeStruct>* fieldtypes, DataTypeStruct* rawDatatype)
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
  for (int i=0; i<m_params.size(); i++){
    m_params[i].analyzeColumns(m_fieldnames, m_fieldtypes, rawDatatype);
    //trace(DEBUG2, "Analyzing parameter '%s' in function '%s' (%d)\n", m_params[i].getEntireExpstr().c_str(), m_expStr.c_str(),m_params[i].columnsAnalyzed());
  }
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
  node->m_fieldnames = m_fieldnames;
  node->m_fieldtypes = m_fieldtypes;

  return node;
}

void FunctionC::copyTo(FunctionC* node){
  if (!node)
    return;
  else{
    node->m_metaDataAnzlyzed = m_metaDataAnzlyzed;
    node->m_expstrAnalyzed = m_expstrAnalyzed;
    node->m_datatype = m_datatype;
    node->m_expStr = m_expStr;
    node->m_funcName = m_funcName;
    node->m_funcID = m_funcID;
    node->m_params = m_params;
    node->m_fieldnames = m_fieldnames;
    node->m_fieldtypes = m_fieldtypes;
  }
}

// clear expression
void FunctionC::clear(){
  m_datatype.datatype = UNKNOWN;
  m_datatype.extrainfo = "";
  m_expStr = "";
  m_funcName = "";
  m_funcID = UNKNOWN;
  m_params.clear();
  m_metaDataAnzlyzed = false;
  m_expstrAnalyzed = false;
  m_fieldnames = NULL;
  m_fieldtypes = NULL;
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

bool FunctionC::runIsnull(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "isnull() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts);
  if (gotResult){
    dts.datatype = INTEGER;
    sResult = intToStr(sResult.length()==0?1:0);
    return true;
  }else
    return false;
}

bool FunctionC::runUpper(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "Upper() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts);
  if (gotResult){
    sResult = upper_copy(sResult);
    return true;
  }else
    return false;
}

bool FunctionC::runLower(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "lower() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts);
  if (gotResult){
    sResult = lower_copy(sResult);
    return true;
  }else{
    trace(ERROR, "Failed to run lower(%s)\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runSubstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "substr(str, startPos, len) function accepts only two or three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sPos, sLen; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sRaw, dts) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sPos, dts) && isInt(sPos)){
    int iPos = atoi(sPos.c_str());
    if (iPos<0 || iPos>=sRaw.length()){
      trace(ERROR, "%s is out of length of %s!\n", sPos.c_str(), m_params[0].m_expStr.c_str());
      return false;
    }
    if (m_params.size() == 3){
      if (m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, sLen, dts) && isInt(sLen)){
        int iLen = atoi(sLen.c_str());
        if (iLen<0 || iLen+iPos>sRaw.length()){
          trace(ERROR, "%s is out of length of %s starting from %s!\n", sLen.c_str(), m_params[0].m_expStr.c_str(), sPos.c_str());
          return false;
        }
        sResult = sRaw.substr(iPos,iLen);
        dts.datatype = STRING;
        return true;
      }else{
        trace(ERROR, "(1)Failed to run substr(%s, %s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str(), m_params[2].m_expStr.c_str());
        return false;
      }
    }else{
      dts.datatype = STRING;
      sResult = sRaw.substr(iPos);
      return true;
    }      
  }else{
    trace(ERROR, "(2)Failed to run substr(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runInstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "instr(str, sub) function accepts only two parameters.\n");
    return false;
  }
  string sRaw, sSub; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sRaw, dts) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sSub, dts)){
    trace(DEBUG2,"Searching '%s' in '%s'\n",sSub.c_str(),sRaw.c_str());
    sResult = intToStr(sRaw.find(sSub));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run instr(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runStrlen(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "strlen() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts);
  if (gotResult){
    sResult = intToStr(sResult.length());
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run strlen(%s)\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runComparestr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "comparestr(str1, str2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, str1, dts) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, str2, dts)){
    sResult = intToStr(str1.compare(str2));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run comparestr(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runNoCaseComparestr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "nocasecomparestr(str1, str2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, str1, dts) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, str2, dts)){
    str1=upper_copy(str1);
    str2=upper_copy(str2);
    sResult = intToStr(str1.compare(str2));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run nocasecomparestr(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runReplace(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 3){
    trace(ERROR, "replace(str, tobereplaced, newstr) function accepts only three parameters.\n");
    return false;
  }
  string sReplace, sNew; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sReplace, dts) && m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, sNew, dts)){
    replacestr(sResult, sReplace, sNew);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run replace(%s, %s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str(), m_params[2].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runRegreplace(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 3){
    trace(ERROR, "regreplace(str, regex_pattern, newstr) function accepts only three parameters.\n");
    return false;
  }
  string sReplace, sNew; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sReplace, dts) && m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, sNew, dts)){
    regreplacestr(sResult, sReplace, sNew);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run regreplace(%s, %s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str(), m_params[2].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runRegmatch(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 3){
    trace(ERROR, "regmatch(str, regex_pattern, expr) function accepts only three parameters.\n");
    return false;
  }
  string str, sPattern; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, str, dts) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sPattern, dts) && m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts)){
    regmatchstr(str, sPattern, sResult);
    dts.datatype = STRING;
    return true;
  }else{
    trace(ERROR, "Failed to run regmatch(%s, %s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str(), m_params[2].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runCountword(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2){
    trace(ERROR, "countword(str,[ingnore_quoters]) function accepts only one or two parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sQuoters=""; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sRaw, dts)){
    if (m_params.size() == 2){
      if (!m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sQuoters, dts)){
        trace(ERROR, "(2)Failed to run countword(%s,%s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
        return false;
      }
    }
    std::set<char> delims = {' ','\t','\n','\r',',','.','!','?',';'};
    vector<string> vWords = split(sRaw,delims,sQuoters,'\0',{},true);
    sResult = intToStr(vWords.size());
    return true;
  }else{
    trace(ERROR, "(2)Failed to run countword(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runGetword(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "getword(str,wordnum,[ingnore_quoters]) function accepts only two or three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sPos, sQuoters=""; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sRaw, dts) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sPos, dts) && isInt(sPos)){
    int iPos = atoi(sPos.c_str());
    if (iPos<=0){
      trace(ERROR, "%s cannot be zero a negative number!\n", sPos.c_str(), m_params[0].m_expStr.c_str());
      return false;
    }
    if (m_params.size() == 3){
      if (!m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, sQuoters, dts)){
        trace(ERROR, "(1)Failed to run getword(%s,%s,%s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str(), m_params[2].m_expStr.c_str());
        return false;
      }
    }
    std::set<char> delims = {' ','\t','\n','\r',',','.','!','?',';'};
    vector<string> vWords = split(sRaw,delims,sQuoters,'\0',{},true);
    if (vWords.size() == 0){
      trace(WARNING, "No word found in '%s'!\n", m_params[0].m_expStr.c_str());
      return false;
    }
    if (iPos>0 && iPos<=vWords.size()){
      sResult = vWords[iPos-1];
      return true;
    }else{
      trace(WARNING, "%s is out of range of the word list in '%s'!\n", m_params[1].m_expStr.c_str(), m_params[0].m_expStr.c_str());
      return false;
    }
  }else{
    trace(ERROR, "(2)Failed to run getword(%s,%s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runFloor(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "floor(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sNum, dts) && isDouble(sNum)){
    sResult = intToStr(floor(atof(sNum.c_str())));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run floor(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runCeil(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "ceil(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sNum, dts) && isDouble(sNum)){
    sResult = intToStr(ceil(atof(sNum.c_str())));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run ceil(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runTimediff(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "timediff(tm1, tm2) function accepts only two parameters.\n");
    return false;
  }
  string sTm1, sTm2; 
  DataTypeStruct dts1, dts2;
  int offSet1, offSet2;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sTm1, dts1) && isDate(sTm1, offSet1, dts1.extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sTm2, dts2) && isDate(sTm2, offSet2, dts2.extrainfo)){
    struct tm tm1, tm2;
    if (strToDate(sTm1, tm1, offSet1, dts1.extrainfo) && strToDate(sTm2, tm2, offSet2, dts2.extrainfo)){
      time_t t1 = mktime(&tm1);
      time_t t2 = mktime(&tm2);
      sResult = doubleToStr(difftime(t1, t2));
      dts.datatype = DOUBLE;
      return true;
    }else{
      trace(ERROR, "Failed to run timediff(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
      return false;
    }
  }else{
    trace(ERROR, "Failed to run timediff(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runRound(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "round(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sNum, dts) && isDouble(sNum)){
    sResult = intToStr(round(atof(sNum.c_str())));
    dts.datatype = LONG;
    return true;
  }else{
    trace(ERROR, "Failed to run round(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runLog(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1){
    trace(ERROR, "log(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sNum, dts) && isDouble(sNum)){
    sResult = doubleToStr(log10(atof(sNum.c_str())));
    dts.datatype = DOUBLE;
    return true;
  }else{
    trace(ERROR, "Failed to run log(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runTruncdate(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "truncdate(date, fmt) function accepts only two parameters.\n");
    return false;
  }
  string sTm, sSeconds;
  struct tm tm;
  DataTypeStruct tmpDts;
  int iOffSet;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sTm, dts) && strToDate(sTm, tm, iOffSet, dts.extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sSeconds, tmpDts) && isInt(sSeconds)){
    long iSeconds = atol(sSeconds.c_str());
    time_t t1 = mktime(&tm) - timezone; // adjust timezone
    time_t t2 = (time_t)(trunc((long double)t1/iSeconds))*iSeconds - timezone; // adjust timezone for gmtime
    trace(DEBUG2, "(a)Truncating '%s' (%d) %d %d %d %d %d %d; t1: %d iSeconds: %d(%s) t2: %d timezone: %d \n",sTm.c_str(), iOffSet,tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, (long)t1, iSeconds, sSeconds.c_str(), (long) t2, timezone);
    //tm = *(localtime(&t2));
    tm = *(gmtime(&t2));
    //tm = zonetime(t2, 0); // as tm returned from strToDate is GMT time
    sResult = dateToStr(tm, iOffSet, dts.extrainfo);
    trace(DEBUG2, "(b)Truncating '%s' (%d) => '%s' (%d %d %d %d %d %d) \n",sTm.c_str(),iOffSet, sResult.c_str(), tm.tm_year+1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    dts.datatype = DATE;
    //trace(DEBUG, "Truncating seconds %d from '%s'(%u) get '%s'(%u), format:%s\n", iSeconds, sTm.c_str(), (long)t1, sResult.c_str(), (long)t2, dts.extrainfo.c_str());
    return !sResult.empty();
  }else{
    trace(ERROR, "Failed to run truncdate(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }  
}

bool FunctionC::runDateformat(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 1 && m_params.size() != 2){
    trace(ERROR, "dateformat(date[, fmt]) function accepts only one or two parameters.\n");
    return false;
  }
  string sTm, sFmt;
  DataTypeStruct tmpDts;
  int iOffSet;
  if (m_params.size() == 2 && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sFmt, tmpDts))
    dts.extrainfo = sFmt;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sTm, dts) && isDate(sTm, iOffSet, dts.extrainfo)){
    struct tm tm;
    if (strToDate(sTm, tm, iOffSet, dts.extrainfo)){
      sResult = dateToStr(tm, iOffSet, m_params.size()==1?dts.extrainfo:sFmt);
      dts.datatype = STRING;
      return !sResult.empty();
    }else{
      trace(ERROR, "(1)Failed to run dateformat(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
      return false;
    }
  }else{
    trace(ERROR, "(2)Failed to run dateformat(%s, %s)!\n", m_params[0].m_expStr.c_str(), dts.extrainfo.c_str());
    return false;
  }  
}

bool FunctionC::runNow(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
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
bool FunctionC::runSwitch(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() <= 2){
    trace(ERROR, "switch() function accepts at least three parameters.\n");
    return false;
  }
  DataTypeStruct dts1, dts2, dts3;
  if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts1)){
    trace(ERROR, "(0)Eval expression '%s' failed.\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }
  string scase,sreturn;
  for (int i=1;i<m_params.size();i++){
    if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, scase, dts2)){
      trace(ERROR, "(1)Eval expression '%s' failed.\n",m_params[i].getEntireExpstr().c_str());
      return false;
    }
    if (i+1<m_params.size()){
      if (!m_params[i+1].evalExpression(fieldvalues, varvalues, aggFuncs, sreturn, dts)){
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

bool FunctionC::runPad(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() != 2){
    trace(ERROR, "pad() function accepts only two parameters.\n");
    return false;
  }
  string seed, sLen;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, seed, dts) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sLen, dts)){
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

bool FunctionC::runGreatest(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() <= 1){
    trace(ERROR, "greatest() function accepts at least two parameters.\n");
    return false;
  }
  DataTypeStruct dts1, dts2;
  if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts1))
    return false;
  string scomp;
  for (int i=1;i<m_params.size();i++){
    if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, scomp, dts2))
      return false;
    dts = getCompatibleDataType(dts1, dts2);
    if (dts.datatype == ANY || dts.datatype == UNKNOWN)
      dts.datatype = STRING;
    
    //trace(DEBUG2,"Comparing '%s'(%d) '%s'(%d) => %d \n",scomp.c_str(),dts2.datatype,sResult.c_str(),dts1.datatype,anyDataCompare(scomp,sResult,dts));
    if (anyDataCompare(scomp,sResult,dts)>0)
      sResult = scomp;
  }
  dts.datatype = ANY;
  return true;
}

bool FunctionC::runLeast(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  if (m_params.size() <= 1){
    trace(ERROR, "least() function accepts at least two parameters.\n");
    return false;
  }
  DataTypeStruct dts1, dts2;
  if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts1))
    return false;
  string scomp;
  for (int i=1;i<m_params.size();i++){
    if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, scomp, dts2))
      return false;
    dts = getCompatibleDataType(dts1, dts2);
    if (dts.datatype == ANY)
      dts.datatype = STRING;
    if (anyDataCompare(scomp,sResult,dts)<0)
      sResult = scomp;
  }
  dts.datatype = ANY;
  return true;
}

// expand foreach to a vector of expression
// foreach(beginid,endid,macro_expr). $ stands for field, # stands for field sequence, % stands for the largest field sequence ID.
vector<ExpressionC> FunctionC::expandForeach(int maxFieldNum)
{
  vector<ExpressionC> vExpr;
  trace(DEBUG,"(1)Expanding foreach expression '%s'\n",m_expStr.c_str());
  if (m_funcID!=FOREACH){
    trace(ERROR, "(1)'%s' is not foreach macro function!\n", m_funcName.c_str());
    return vExpr;
  }
  if (m_params.size()<3){
    trace(ERROR, "(1)Foreach macro function requires 3 parameters!\n", m_funcName.c_str());
    return vExpr;
  }
  int begin = 0, end = 0;
  if (m_params[0].m_expStr.compare("%") == 0)
    begin = maxFieldNum;
  else if (isInt(m_params[0].m_expStr))
    begin = min(maxFieldNum,atoi(m_params[0].m_expStr.c_str()));
  else{
    trace(ERROR, "(1)Invalid begin ID for foreach macro function!\n", m_params[0].m_expStr.c_str());
    return vExpr;
  }
  if (m_params[1].m_expStr.compare("%") == 0)
    end = maxFieldNum;
  else if (isInt(m_params[1].m_expStr))
    end = min(maxFieldNum,atoi(m_params[1].m_expStr.c_str()));
  else{
    trace(ERROR, "(1)Invalid end ID for foreach macro function!\n", m_params[1].m_expStr.c_str());
    return vExpr;
  }
  for (int i=begin; begin<end?i<=end:i>=end; begin<end?i++:i--){
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
  if (m_funcID!=FOREACH){
    trace(ERROR, "(2)'%s' is not foreach macro function!\n", m_funcName.c_str());
    return vExpr;
  }
  if (m_params.size()<3){
    trace(ERROR, "(2)Foreach macro function requires 3 parameters!\n", m_funcName.c_str());
    return vExpr;
  }
  int begin = 0, end = 0;
  if (m_params[0].m_expStr.compare("%") == 0)
    begin = vExps.size();
  else if (isInt(m_params[0].m_expStr))
    begin = min((int)vExps.size(),atoi(m_params[0].m_expStr.c_str()));
  else{
    trace(ERROR, "(2)Invalid begin ID for foreach macro function!\n", m_params[0].m_expStr.c_str());
    return vExpr;
  }
  if (m_params[1].m_expStr.compare("%") == 0)
    end = vExps.size();
  else if (isInt(m_params[1].m_expStr))
    end = min((int)vExps.size(),atoi(m_params[1].m_expStr.c_str()));
  else{
    trace(ERROR, "(2)Invalid end ID for foreach macro function!\n", m_params[1].m_expStr.c_str());
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
bool FunctionC::runFunction(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, DataTypeStruct & dts)
{
  bool getResult = false;
    switch(m_funcID){
    case UPPER:
      getResult = runUpper(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case LOWER:
      getResult = runLower(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case SUBSTR:
      getResult = runSubstr(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case FLOOR:
      getResult = runFloor(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case CEIL:
      getResult = runCeil(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case TIMEDIFF:
      getResult = runTimediff(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case ISNULL:
      getResult = runIsnull(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case INSTR:
      getResult = runInstr(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case STRLEN:
      getResult = runStrlen(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case COMPARESTR:
      getResult = runComparestr(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case NOCASECOMPARESTR:
      getResult = runNoCaseComparestr(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case REPLACE:
      getResult = runReplace(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case REGREPLACE:
      getResult = runRegreplace(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case REGMATCH:
      getResult = runRegmatch(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case COUNTWORD:
      getResult = runCountword(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case GETWORD:
      getResult = runGetword(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case SWITCH:
      getResult = runSwitch(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case PAD:
      getResult = runPad(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case GREATEST:
      getResult = runGreatest(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case LEAST:
      getResult = runLeast(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case ROUND:
      getResult = runRound(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case LOG:
      getResult = runLog(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case DATEFORMAT:
      getResult = runDateformat(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case TRUNCDATE:
      getResult = runTruncdate(fieldvalues, varvalues, aggFuncs, sResult, dts);
      break;
    case NOW:
      getResult = runNow(fieldvalues, varvalues, aggFuncs, sResult, dts);
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
      if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts))
        return true;
      else{
        trace(ERROR, "Failed to eval aggregation (%s) function parameter (%s).\n", m_funcName.c_str(),m_params[0].getEntireExpstr().c_str());
        return false;
      }
      break;
    }
    case FOREACH:
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
