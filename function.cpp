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
  m_expStr = expStr;
  checkDataType();
  if (!analyzeExpStr()){
    m_funcName = "";
    m_params.clear();
  }
}

// detect function return type
void FunctionC::checkDataType()
{
  m_datatype = UNKNOWN;
}

// analyze expression string to get the function name (upper case) and parameter expression (classes)
bool FunctionC::analyzeExpStr()
{
  m_expStr = boost::algorithm::trim_copy<string>(m_expStr);
  if (m_expStr.empty()){
    trace(ERROR, "Empty function expression string!\n");
    return false;
  }
  int iPos = -1;
  string strParams = readQuotedStr(m_expStr, iPos, "()", '\0');
  if (iPos<0 || strParams.empty()){
    trace(ERROR, "No quoted parameters found!\n");
    return false;
  }
  m_funcName = boost::to_upper_copy<string>(m_expStr.substr(0, iPos));
  strParams = trim_pair(strParams, "()");
  vector<string> vParams = split(strParams,',',"//''{}",'\\');
  for (int i=0; i<vParams.size(); i++){
    string sParam = boost::algorithm::trim_copy<string>(vParams[i]);
    if (sParam.empty()){
      trace(ERROR, "Empty parameter string!\n");
      return false;
    }
    ExpressionC eParam(sParam);
    m_params.push_back(eParam);
  }
  switch(m_funcName){
  case "UPPER":
  case "LOWER":
  case "SUBSTR":
    m_datatype = STRING;
    break;
  case "FLOOR":
  case "CEIL":
  case "TIMEDIFF":
  case "INSTR":
  case "COMPARESTR":
  case "NOCASECOMPARESTR":
    m_datatype = LONG;
    break;
  case "ROUND":
    m_datatype = DOUBLE;
    break;
  case "NOW":
    m_datatype = DATE;
    break;
  }
  return true;
}

void FunctionC::dump(){
  trace(INFO,"%s\n", m_expStr.c_str());
}

// analyze column ID & name from metadata, return data type of current node
// decide current node data type by checking children's data type
int FunctionC::analyzeColumns(vector<string> fieldnames, vector<int> fieldtypes)
{
  m_metaDataAnzlyzed = true;
  return m_datatype;
}

bool FunctionC::columnsAnalyzed(){
    return m_metaDataAnzlyzed;
}

FunctionC* FunctionC::cloneMe(){
  FunctionC* node = new FunctionC();
  node->m_metaDataAnzlyzed = m_metaDataAnzlyzed;
  node->m_datatype = m_datatype;
  node->m_expStr = m_expStr;

  return node;
}

void FunctionC::copyTo(FunctionC* node){
  if (!node)
    return;
  else{
    node->m_metaDataAnzlyzed = m_metaDataAnzlyzed;
    node->m_datatype = m_datatype;
    node->m_expStr = m_expStr;
  }
}

// clear expression
void FunctionC::clear(){
  m_datatype = UNKNOWN;
  m_expStr = "";
  m_funcName = "";
  m_params.clear();
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

bool FunctionC::runUpper(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
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

bool FunctionC::runLower(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  if (m_params.size() != 1){
    trace(ERROR, "Lower() function accepts only one parameter.\n");
    return false;
  }
  bool gotResult = m_params[0].evalExpression(fieldnames, fieldvalues, varvalues, sResult);
  if (gotResult){
    sResult = boost::to_lower_copy<string>(sResult);
    return true;
  }else
    return false;
}

bool FunctionC::runSubstr(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  return false;
}

bool FunctionC::runInstr(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  return false;
}

bool FunctionC::runComparestr(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  return false;
}

bool FunctionC::runNoCaseComparestr(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  return false;
}

bool FunctionC::runFloor(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  return false;
}

bool FunctionC::runCeil(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  return false;
}

bool FunctionC::runTimediff(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  return false;
}

bool FunctionC::runRound(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  return false;
}

bool FunctionC::runNow(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  return false;
}


// run function and get result
bool FunctionC::runFunction(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult)
{
  bool getResult = false;
  switch(m_funcName){
  case "UPPER":
    getResult = runUpper(fieldnames, fieldvalues, varvalues, sResult);
    break;
  case "LOWER":
    getResult = runLower(fieldnames, fieldvalues, varvalues, sResult);
    break;
  case "SUBSTR":
    getResult = runSubstr(fieldnames, fieldvalues, varvalues, sResult);
    break;
  case "FLOOR":
    getResult = runFloor(fieldnames, fieldvalues, varvalues, sResult);
    break;
  case "CEIL":
    getResult = runCeil(fieldnames, fieldvalues, varvalues, sResult);
    break;
  case "TIMEDIFF":
    getResult = runTimediff(fieldnames, fieldvalues, varvalues, sResult);
    break;
  case "INSTR":
    getResult = runInstr(fieldnames, fieldvalues, varvalues, sResult);
    break;
  case "COMPARESTR":
    getResult = runComparestr(fieldnames, fieldvalues, varvalues, sResult);
    break;
  case "NOCASECOMPARESTR":
    getResult = runNoCaseComparestr(fieldnames, fieldvalues, varvalues, sResult);
    break;
  case "ROUND":
    getResult = runRound(fieldnames, fieldvalues, varvalues, sResult);
    break;
  case "NOW":
    getResult = runNow(fieldnames, fieldvalues, varvalues, sResult);
    break;
  }

  return getResult;
}
