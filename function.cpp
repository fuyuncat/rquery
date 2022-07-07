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
  m_datatype = UNKNOWN;   // 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
  m_expStr = "";          // it's the full function string, including function name and parameters
  m_funcName = "";        // analyzed function name, upper case
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
    m_params.clear();
  }
}

bool FunctionC::isConst()
{
  if (m_expstrAnalyzed){
    for (int i=0; i<m_params.size(); i++){
      if (m_params[i].m_expType != CONST)
        return false;
    }
    return true;
  }
  return false;
}

// analyze expression string to get the function name (upper case) and parameter expression (classes)
bool FunctionC::analyzeExpStr()
{
  trace(DEBUG, "Analyzing function from '%s'\n", m_expStr.c_str());
  m_expStr = boost::algorithm::trim_copy<string>(m_expStr);
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
  m_funcName = boost::trim_copy<string>(boost::to_upper_copy<string>(m_expStr.substr(0, m_expStr.find("("))));
  m_expStr = m_funcName+strParams;
  strParams = trim_pair(strParams, "()");
  vector<string> vParams = split(strParams,',',"//''{}",'\\');
  for (int i=0; i<vParams.size(); i++){
    trace(DEBUG, "Processing parameter(%d) '%s'!\n", i, vParams[i].c_str());
    string sParam = boost::algorithm::trim_copy<string>(vParams[i]);
    if (sParam.empty()){
      trace(ERROR, "Empty parameter string!\n");
      m_expstrAnalyzed = false;
      return false;
    }
    ExpressionC eParam(sParam);
    //eParam.analyzeColumns(m_fieldnames, m_fieldtypes);
    m_params.push_back(eParam);
  }
  if(m_funcName.compare("UPPER")==0 || m_funcName.compare("LOWER")==0 || m_funcName.compare("SUBSTR")==0 || m_funcName.compare("REPLACE")==0 || m_funcName.compare("DATEFORMAT")==0)
    m_datatype = STRING;
  else if(m_funcName.compare("FLOOR")==0 || m_funcName.compare("CEIL")==0 || m_funcName.compare("ROUND")==0 || m_funcName.compare("TIMEDIFF")==0 || m_funcName.compare("INSTR")==0 || m_funcName.compare("COMPARESTR")==0 || m_funcName.compare("NOCASECOMPARESTR")==0 || m_funcName.compare("COUNT")==0 || m_funcName.compare("UNIQUECOUNT")==0)
    m_datatype = LONG;
  else if(m_funcName.compare("LOG")==0 || m_funcName.compare("AVERAGE")==0 || m_funcName.compare("SUM")==0)
    m_datatype = DOUBLE;
  else if(m_funcName.compare("NOW")==0 || m_funcName.compare("TRUNCDATE")==0)
    m_datatype = DATE;
  else if(m_funcName.compare("MAX")==0 || m_funcName.compare("MIN")==0) // MAX and MIN could be any data type
    m_datatype = UNKNOWN;
  else{
    trace(ERROR, "Function(1) '%s' is not supported yet!\n", m_funcName.c_str());
    m_datatype = UNKNOWN;
  }
  m_expstrAnalyzed = true;
  return true;
}

void FunctionC::dump(){
  trace(DUMP,"%s\n", m_expStr.c_str());
}

// analyze column ID & name from metadata, return data type of current node
// decide current node data type by checking children's data type
int FunctionC::analyzeColumns(vector<string>* fieldnames, vector<int>* fieldtypes)
{
  trace(DEBUG, "Analyzing columns in function '%s'\n", m_expStr.c_str());
  if (!fieldnames || !fieldtypes){
    trace(ERROR, "(Function)fieldnames or fieldtypes is NULL!\n");
    return UNKNOWN;
  }
  m_metaDataAnzlyzed = true;
  m_fieldnames = fieldnames;
  m_fieldtypes = fieldtypes;
  for (int i=0; i<m_params.size(); i++)
    m_params[i].analyzeColumns(m_fieldnames, m_fieldtypes);
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
    node->m_params = m_params;
    node->m_fieldnames = m_fieldnames;
    node->m_fieldtypes = m_fieldtypes;
  }
}

// clear expression
void FunctionC::clear(){
  m_datatype = UNKNOWN;
  m_expStr = "";
  m_funcName = "";
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

bool FunctionC::runUpper(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 1){
    trace(ERROR, "Upper() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sResult);
  if (gotResult){
    sResult = boost::to_upper_copy<string>(sResult);
    return true;
  }else
    return false;
}

bool FunctionC::runLower(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 1){
    trace(ERROR, "lower() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sResult);
  if (gotResult){
    sResult = boost::to_lower_copy<string>(sResult);
    return true;
  }else{
    trace(ERROR, "Failed to run lower(%s)\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runSubstr(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 2 || m_params.size() != 3){
    trace(ERROR, "substr(str, startPos, len) function accepts only two or three parameters.\n");
    return false;
  }
  string sRaw, sPos, sLen; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sRaw) && m_params[1].evalExpression(fieldnames, fieldvalues, varvalues, sPos) && isInt(sPos)){
    int iPos = atoi(sPos.c_str());
    if (iPos<0 || iPos>=sRaw.length()){
      trace(ERROR, "%s is out of length of %s!\n", sPos.c_str(), m_params[0].m_expStr.c_str());
      return false;
    }
    if (m_params.size() == 3){
      if (m_params[2].evalExpression(fieldnames, fieldvalues, varvalues, sLen) && isInt(sLen)){
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

bool FunctionC::runInstr(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 2){
    trace(ERROR, "instr(str, sub) function accepts only two parameters.\n");
    return false;
  }
  string sRaw, sSub; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sRaw) && m_params[1].evalExpression(fieldnames, fieldvalues, varvalues, sSub)){
    sResult = intToStr(sRaw.find(sSub));
    return true;
  }else{
    trace(ERROR, "Failed to run instr(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runComparestr(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 2){
    trace(ERROR, "comparestr(str1, str2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, str1) && m_params[1].evalExpression(fieldnames, fieldvalues, varvalues, str2)){
    sResult = intToStr(str1.compare(str2));
    return true;
  }else{
    trace(ERROR, "Failed to run comparestr(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runNoCaseComparestr(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 2){
    trace(ERROR, "nocasecomparestr(str1, str2) function accepts only two parameters.\n");
    return false;
  }
  string str1, str2; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, str1) && m_params[1].evalExpression(fieldnames, fieldvalues, varvalues, str2)){
    str1=boost::to_upper_copy<string>(str1);
    str2=boost::to_upper_copy<string>(str2);
    sResult = intToStr(str1.compare(str2));
    return true;
  }else{
    trace(ERROR, "Failed to run nocasecomparestr(%s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runReplace(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 3){
    trace(ERROR, "replace(str, tobereplaced, newstr) function accepts only two parameters.\n");
    return false;
  }
  string sRaw, sReplace, sNew; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sRaw) && m_params[1].evalExpression(fieldnames, fieldvalues, varvalues, sReplace) && m_params[1].evalExpression(fieldnames, fieldvalues, varvalues, sNew)){
    replacestr(sRaw, sReplace, sNew);
    sResult=sRaw;
    return true;
  }else{
    trace(ERROR, "Failed to run replace(%s, %s, %s)!\n", m_params[0].m_expStr.c_str(), m_params[1].m_expStr.c_str(), m_params[2].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runFloor(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 1){
    trace(ERROR, "floor(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sNum) && isDouble(sNum)){
    sResult = intToStr(floor(atof(sNum.c_str())));
    return true;
  }else{
    trace(ERROR, "Failed to run floor(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runCeil(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 1){
    trace(ERROR, "ceil(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sNum) && isDouble(sNum)){
    sResult = intToStr(ceil(atof(sNum.c_str())));
    return true;
  }else{
    trace(ERROR, "Failed to run ceil(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runTimediff(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 2){
    trace(ERROR, "timediff(tm1, tm2) function accepts only two parameters.\n");
    return false;
  }
  string sTm1, sTm2, sFmt1, sFmt2; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sTm1) && isDate(sTm1, sFmt1) && m_params[1].evalExpression(fieldnames, fieldvalues, varvalues, sTm2) && isDate(sTm2, sFmt2)){
    struct tm tm1, tm2;
    if (strptime(sTm1.c_str(), sFmt1.c_str(), &tm1) && strptime(sTm2.c_str(), sFmt2.c_str(), &tm2)){
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

bool FunctionC::runRound(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 1){
    trace(ERROR, "round(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sNum) && isDouble(sNum)){
    sResult = intToStr(round(atof(sNum.c_str())));
    return true;
  }else{
    trace(ERROR, "Failed to run round(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runLog(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 1){
    trace(ERROR, "log(num) function accepts only one parameter.\n");
    return false;
  }
  string sNum; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sNum) && isDouble(sNum)){
    sResult = intToStr(log(atof(sNum.c_str())));
    return true;
  }else{
    trace(ERROR, "Failed to run log(%s)!\n", m_params[0].m_expStr.c_str());
    return false;
  }
}

bool FunctionC::runTruncdate(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 2){
    trace(ERROR, "truncdate(date, fmt) function accepts only two parameters.\n");
    return false;
  }
  string sTm, sFmt, sSeconds; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sTm) && isDate(sTm, sFmt) && m_params[1].evalExpression(fieldnames, fieldvalues, varvalues, sSeconds) && isInt(sSeconds)){
    struct tm tm;
    long iSeconds = atol(sSeconds.c_str());
    if (strptime(sTm.c_str(), sFmt.c_str(), &tm)){
      time_t t1 = ((long)mktime(&tm)/iSeconds)*iSeconds;
      tm = *(localtime(&t1));
      sResult = dateToStr(tm, sFmt);
      trace(DEBUG, "Truncating date '%s'(%d) to '%s'(%d), format:%s", sTm.c_str(), (long)mktime(&tm), sResult.c_str(), (long)t1, sFmt.c_str());
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

bool FunctionC::runDateformat(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 2){
    trace(ERROR, "dateformat(date, fmt) function accepts only two parameters.\n");
    return false;
  }
  string sTm, sOrigFmt, sFmt; 
  if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sTm) && isDate(sTm, sOrigFmt) && m_params[1].evalExpression(fieldnames, fieldvalues, varvalues, sFmt)){
    struct tm tm;
    if (strptime(sTm.c_str(), sOrigFmt.c_str(), &tm)){
      sResult = dateToStr(tm, sFmt);
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

bool FunctionC::runNow(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 0){
    trace(ERROR, "now() function does not accept any parameter.\n");
    return false;
  }
  struct tm curtime = now();
  sResult = dateToStr(curtime);
  return true;
}

// run function and get result
bool FunctionC::runFunction(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  bool getResult = false;
  if(m_funcName.compare("UPPER")==0)
    getResult = runUpper(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("LOWER")==0)
    getResult = runLower(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("SUBSTR")==0)
    getResult = runSubstr(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("FLOOR")==0)
    getResult = runFloor(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("CEIL")==0)
    getResult = runCeil(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("TIMEDIFF")==0)
    getResult = runTimediff(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("INSTR")==0)
    getResult = runInstr(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("COMPARESTR")==0)
    getResult = runComparestr(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("NOCASECOMPARESTR")==0)
    getResult = runNoCaseComparestr(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("REPLACE")==0)
    getResult = runReplace(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("ROUND")==0)
    getResult = runRound(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("LOG")==0)
    getResult = runLog(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("DATEFORMAT")==0)
    getResult = runDateformat(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("TRUNCDATE")==0)
    getResult = runTruncdate(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("NOW")==0)
    getResult = runNow(fieldnames, fieldvalues, varvalues, sResult);
  else if(m_funcName.compare("SUM")==0 || m_funcName.compare("COUNT")==0 || m_funcName.compare("UNIQUECOUNT")==0 || m_funcName.compare("MAX")==0 || m_funcName.compare("MIN")==0 || m_funcName.compare("AVERAGE")==0) {// aggregation function eval parameter expression only!
    if (m_params.size() != 1){
      trace(ERROR, "Aggregation (%s) function accepts only one parameter.\n", m_funcName.c_str());
      return false;
    }
    if (m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sResult))
      return true;
    else{
      trace(ERROR, "Failed to eval aggregation (%s) function parameter (%s).\n", m_funcName.c_str(),m_params[0].getEntireExpstr().c_str());
      return false;
    }
  }else{
    trace(ERROR, "Function(2) '%s' is not supported yet!\n", m_funcName.c_str());
    return false;
  }

  return getResult;
}
