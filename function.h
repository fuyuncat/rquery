/*******************************************************************************
//
//        File: function.h
// Description: Function class header
//       Usage: function.h
//     Created: 28/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  28/06/2022: Created
//
*******************************************************************************/

#ifndef __FUNCTIONC_H
#define __FUNCTIONC_H

#include "function.h"
#include "commfuncs.h"

class ExpressionC;

class FunctionC
{
  public:

    FunctionC();
    FunctionC(string expStr);

    ~FunctionC();

    DataTypeStruct m_datatype;   // 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
    string m_expStr;  // it's the full function string, including function name and parameters
    string m_funcName; // analyzed function name, upper case
    vector<ExpressionC> m_params; // parameter expressions.
    
    bool runFunction(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool isConst();   // if all parameters are const
    void setExpStr(string expStr);
    DataTypeStruct analyzeColumns(vector<string>* fieldnames, vector<DataTypeStruct>* fieldtypes); 
    bool columnsAnalyzed();
    bool expstrAnalyzed();
    void dump();
    FunctionC* cloneMe();
    void copyTo(FunctionC* node);
    void clear(); // clear predictin
    bool remove(FunctionC* node); // remove a node from prediction. Note: the input node is the address of the node contains in current prediction

  private:
    bool m_metaDataAnzlyzed; // analyze column name to column id.
    bool m_expstrAnalyzed; // if expression string analyzed
    vector<string>* m_fieldnames;  // all nodes (parent & children) point to the same address!!!
    vector<DataTypeStruct>* m_fieldtypes;     // all nodes (parent & children) point to the same address!!!

    bool analyzeExpStr();  // analyze expression string to get the function name (upper case) and parameter expression (classes)
    
    bool runUpper(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runLower(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runSubstr(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runInstr(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runStrlen(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runComparestr(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runNoCaseComparestr(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runReplace(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runFloor(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runCeil(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runTimediff(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runRound(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runLog(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runDateformat(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runTruncdate(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);
    bool runNow(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult);

  protected:
    void init();
};

#endif // __FUNCTIONC_H

