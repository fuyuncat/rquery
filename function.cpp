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
  if (iPos<0 || strParams.empty()){
    trace(ERROR, "No quoted parameters found in '%s'!\n", m_expStr.c_str());
    m_expstrAnalyzed = false;
    return false;
  }
  m_funcName = trim_copy(upper_copy(m_expStr.substr(0, m_expStr.find("("))));
  m_expStr = m_funcName+strParams;
  strParams = trim_pair(strParams, "()");
  vector<string> vParams = split(strParams,',',"//''{}()",'\\',{'(',')'});
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
    //eParam.analyzeColumns(m_fieldnames, m_fieldtypes);
    m_params.push_back(eParam);
  }
  m_funcID = encodeFunction(m_funcName);
  switch(m_funcID){
    case UPPER:
    case LOWER:
    case SUBSTR:
    case REPLACE:
    case REGREPLACE:
    case DATEFORMAT:
    case PAD:
      m_datatype.datatype = STRING;
      break;
    case FLOOR:
    case CEIL:
    case ROUND:
    case TIMEDIFF:
    case INSTR:
    case COMPARESTR:
    case NOCASECOMPARESTR:
    case STRLEN:
    case COUNT:
    case UNIQUECOUNT:
    case ISNULL:
      m_datatype.datatype = LONG;
      break;
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
DataTypeStruct FunctionC::analyzeColumns(vector<string>* fieldnames, vector<DataTypeStruct>* fieldtypes)
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
    m_params[i].analyzeColumns(m_fieldnames, m_fieldtypes);
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

bool FunctionC::runIsnull(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 1){
    trace(ERROR, "isnull() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
  if (gotResult){
    sResult = intToStr(sResult.length()==0?1:0);
    return true;
  }else
    return false;
}

bool FunctionC::runUpper(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 1){
    trace(ERROR, "Upper() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
  if (gotResult){
    sResult = upper_copy(sResult);
    return true;
  }else
    return false;
}

bool FunctionC::runLower(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 1){
    trace(ERROR, "lower() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
  if (gotResult){
    sResult = lower_copy(sResult);
    return true;
  }else{
    trace(ERROR, "Failed to run lower(%s)\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runSubstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 2 && m_params.size() != 3){
    trace(ERROR, "substr(str, startPos, len) function accepts only two or three parameters(%d).\n",m_params.size());
    return false;
  }
  string sRaw, sPos, sLen; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sRaw, extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sPos, extrainfo) && isInt(sPos)){
    int iPos = atoi(sPos.c_str());
    if (iPos<0 || iPos>=sRaw.length()){
      trace(ERROR, "%s is out of length of %s!\n", sPos.c_str(), m_params[0].m_expStr.c_str());
      return false;
    }
    if (m_params.size() == 3){
      if (m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, sLen, extrainfo) && isInt(sLen)){
        int iLen = atoi(sLen.c_str());
        if (iLen<0 || iLen+iPos>sRaw.length()){
          trace(ERROR, "%s is out of length of %s starting from %s!\n", sLen.c_str(), m_params[0].m_expStr.c_str(), sPos.c_str());
          return false;
        }
        sResult = sRaw.substr(iPos,iLen);
        return true;
      }else{
        trace(ERROR, "Failed to run substr(%s, %s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str(), m_params[2].m_expStr.c_str());
        return false;
      }
    }else{
      sResult = sRaw.substr(iPos);
      return true;
    }      
  }else{
    trace(ERROR, "Failed to run substr(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runInstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 2){
    trace(ERROR, "instr(str, sub) function accepts only two parameters.\n");
    return false;
  }
  string sRaw, sSub; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sRaw, extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sSub, extrainfo)){
    trace(DEBUG2,"Searching '%s' in '%s'\n",sSub.c_str(),sRaw.c_str());
    sResult = intToStr(sRaw.find(sSub));
    return true;
  }else{
    trace(ERROR, "Failed to run instr(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runStrlen(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 1){
    trace(ERROR, "strlen() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
  if (gotResult){
    sResult = intToStr(sResult.length());
    return true;
  }else{
    trace(ERROR, "Failed to run strlen(%s)\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runComparestr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 2){
    trace(ERROR, "comparestr(str1, str2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, str1, extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, str2, extrainfo)){
    sResult = intToStr(str1.compare(str2));
    return true;
  }else{
    trace(ERROR, "Failed to run comparestr(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runNoCaseComparestr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 2){
    trace(ERROR, "nocasecomparestr(str1, str2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, str1, extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, str2, extrainfo)){
    str1=upper_copy(str1);
    str2=upper_copy(str2);
    sResult = intToStr(str1.compare(str2));
    return true;
  }else{
    trace(ERROR, "Failed to run nocasecomparestr(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runReplace(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 3){
    trace(ERROR, "replace(str, tobereplaced, newstr) function accepts only three parameters.\n");
    return false;
  }
  string sReplace, sNew; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sReplace, extrainfo) && m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, sNew, extrainfo)){
    replacestr(sResult, sReplace, sNew);
    return true;
  }else{
    trace(ERROR, "Failed to run replace(%s, %s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str(), m_params[2].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runRegreplace(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 3){
    trace(ERROR, "regreplace(str, regex_pattern, newstr) function accepts only three parameters.\n");
    return false;
  }
  string sReplace, sNew; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sReplace, extrainfo) && m_params[2].evalExpression(fieldvalues, varvalues, aggFuncs, sNew, extrainfo)){
    regreplacestr(sResult, sReplace, sNew);
    return true;
  }else{
    trace(ERROR, "Failed to run regreplace(%s, %s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str(), m_params[2].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runFloor(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 1){
    trace(ERROR, "floor(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sNum, extrainfo) && isDouble(sNum)){
    sResult = intToStr(floor(atof(sNum.c_str())));
    return true;
  }else{
    trace(ERROR, "Failed to run floor(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runCeil(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 1){
    trace(ERROR, "ceil(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sNum, extrainfo) && isDouble(sNum)){
    sResult = intToStr(ceil(atof(sNum.c_str())));
    return true;
  }else{
    trace(ERROR, "Failed to run ceil(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runTimediff(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 2){
    trace(ERROR, "timediff(tm1, tm2) function accepts only two parameters.\n");
    return false;
  }
  string sTm1, sTm2, sFmt1, sFmt2; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sTm1, sFmt1) && isDate(sTm1, sFmt1) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sTm2, sFmt2) && isDate(sTm2, sFmt2)){
    struct tm tm1, tm2;
    if (strToDate(sTm1, tm1, sFmt1) && strToDate(sTm2, tm2, sFmt2)){
      time_t t1 = mktime(&tm1);
      time_t t2 = mktime(&tm2);
      sResult = doubleToStr(difftime(t1, t2));
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

bool FunctionC::runRound(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 1){
    trace(ERROR, "round(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sNum, extrainfo) && isDouble(sNum)){
    sResult = intToStr(round(atof(sNum.c_str())));
    return true;
  }else{
    trace(ERROR, "Failed to run round(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runLog(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 1){
    trace(ERROR, "log(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sNum, extrainfo) && isDouble(sNum)){
    sResult = intToStr(log(atof(sNum.c_str())));
    return true;
  }else{
    trace(ERROR, "Failed to run log(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runTruncdate(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 2){
    trace(ERROR, "truncdate(date, fmt) function accepts only two parameters.\n");
    return false;
  }
  string sTm, sSeconds, tmpExtra; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sTm, extrainfo) && isDate(sTm, extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sSeconds, tmpExtra) && isInt(sSeconds)){
    struct tm tm;
    long iSeconds = atol(sSeconds.c_str());
    if (strToDate(sTm, tm, extrainfo)){
      time_t t1 = mktime(&tm);
      time_t t2 = (time_t)(trunc((long double)t1/iSeconds))*iSeconds;
      tm = *(localtime(&t2));
      sResult = dateToStr(tm, extrainfo);
      //trace(DEBUG, "Truncating date '%s'(%u) to '%s'(%u), format:%s\n", sTm.c_str(), (long)t1, sResult.c_str(), (long)t2, extrainfo.c_str());
      return !sResult.empty();
    }else{
      trace(ERROR, "Failed to run truncdate(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
      return false;
    }
  }else{
    trace(ERROR, "Failed to run truncdate(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }  
}

bool FunctionC::runDateformat(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 1 && m_params.size() != 2){
    trace(ERROR, "dateformat(date[, fmt]) function accepts only one or two parameters.\n");
    return false;
  }
  string sTm, sFmt, tmpExtra; 
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sTm, extrainfo) && isDate(sTm, extrainfo) && (m_params.size()==1 || m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sFmt, tmpExtra))){
    struct tm tm;
    if (strToDate(sTm, tm, extrainfo)){
      sResult = dateToStr(tm, m_params.size()==1?extrainfo:sFmt);
      return !sResult.empty();
    }else{
      trace(ERROR, "Failed to run dateformat(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
      return false;
    }
  }else{
    trace(ERROR, "Failed to run dateformat(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }  
}

bool FunctionC::runNow(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 0){
    trace(ERROR, "now() function does not accept any parameter.\n");
    return false;
  }
  struct tm curtime = now();
  sResult = dateToStr(curtime);
  return true;
}

// switch(input,case1,return1[,case2,result2...][,default]): if input equal to case1, then return return1, etc.. If none matched, return default or return input if no default provided.
bool FunctionC::runSwitch(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() <= 2){
    trace(ERROR, "switch() function accepts at least three parameters.\n");
    return false;
  }
  if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, extrainfo)){
    trace(ERROR, "(0)Eval expression '%s' failed.\n",m_params[0].getEntireExpstr().c_str());
    return false;
  }
  string scase,sreturn;
  for (int i=1;i<m_params.size();i++){
    if (m_params[i].getEntireExpstr().compare("2")==0)
      int breakme=1;
    if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, scase, extrainfo)){
      trace(ERROR, "(1)Eval expression '%s' failed.\n",m_params[i].getEntireExpstr().c_str());
      return false;
    }
    if (i+1<m_params.size()){
      if (!m_params[i+1].evalExpression(fieldvalues, varvalues, aggFuncs, sreturn, extrainfo)){
        trace(ERROR, "(2)Eval expression '%s' failed.\n",m_params[i+1].getEntireExpstr().c_str());
        return false;
      }
      i++;
    }else{
      //trace(DEBUG2,"Retruning default '%s'\n",scase.c_str());
      sResult = scase;
      return true;
    }
    //trace(DEBUG2,"Comparing '%s' '%s'\n",sResult.c_str(),scase.c_str());
    if (sResult.compare(scase) == 0){
      sResult = sreturn;
      return true;
    }
  }
  //trace(DEBUG2,"Retruning original '%s'\n",sResult.c_str());
  return true;
}

bool FunctionC::runPad(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() != 2){
    trace(ERROR, "pad() function accepts only two parameters.\n");
    return false;
  }
  string seed, sLen;
  if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, seed, extrainfo) && m_params[1].evalExpression(fieldvalues, varvalues, aggFuncs, sLen, extrainfo)){
    if (!isInt(sLen))
      return false;
    sResult = "";
    for (int i=0;i<atoi(sLen.c_str());i++)
      sResult.append(seed);
    return true;
  }else
    return false;
}

bool FunctionC::runGreatest(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() <= 1){
    trace(ERROR, "greatest() function accepts at least two parameters.\n");
    return false;
  }
  if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, extrainfo))
    return false;
  string scomp;
  for (int i=1;i<m_params.size();i++){
    if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, scomp, extrainfo))
      return false;
    if (scomp.compare(sResult)>0)
      sResult = scomp;
  }
  return true;
}

bool FunctionC::runLeast(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  if (m_params.size() <= 1){
    trace(ERROR, "least() function accepts at least two parameters.\n");
    return false;
  }
  if (!m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, extrainfo))
    return false;
  string scomp;
  for (int i=1;i<m_params.size();i++){
    if (!m_params[i].evalExpression(fieldvalues, varvalues, aggFuncs, scomp, extrainfo))
      return false;
    if (scomp.compare(sResult)<0)
      sResult = scomp;
  }
  return true;
}

// run function and get result
bool FunctionC::runFunction(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, string & sResult, string & extrainfo)
{
  bool getResult = false;
    switch(m_funcID){
    case UPPER:
      getResult = runUpper(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case LOWER:
      getResult = runLower(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case SUBSTR:
      getResult = runSubstr(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case FLOOR:
      getResult = runFloor(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case CEIL:
      getResult = runCeil(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case TIMEDIFF:
      getResult = runTimediff(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case ISNULL:
      getResult = runIsnull(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case INSTR:
      getResult = runInstr(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case STRLEN:
      getResult = runStrlen(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case COMPARESTR:
      getResult = runComparestr(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case NOCASECOMPARESTR:
      getResult = runNoCaseComparestr(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case REPLACE:
      getResult = runReplace(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case REGREPLACE:
      getResult = runRegreplace(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case SWITCH:
      getResult = runSwitch(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case PAD:
      getResult = runPad(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case GREATEST:
      getResult = runGreatest(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case LEAST:
      getResult = runLeast(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case ROUND:
      getResult = runRound(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case LOG:
      getResult = runLog(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case DATEFORMAT:
      getResult = runDateformat(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case TRUNCDATE:
      getResult = runTruncdate(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
      break;
    case NOW:
      getResult = runNow(fieldvalues, varvalues, aggFuncs, sResult, extrainfo);
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
      if (m_params[0].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, extrainfo))
        return true;
      else{
        trace(ERROR, "Failed to eval aggregation (%s) function parameter (%s).\n", m_funcName.c_str(),m_params[0].getEntireExpstr().c_str());
        return false;
      }
      break;
    }
    default:{
      trace(ERROR, "Function(2) '%s' is not supported yet!\n", m_funcName.c_str());
      return false;
    }
  }

  return getResult;
}
