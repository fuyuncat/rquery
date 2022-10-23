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
#include <unordered_map>
#include "commfuncs.h"

class ExpressionC;

class FunctionC
{
  public:

    FunctionC();
    FunctionC(string expStr);
    FunctionC(const FunctionC& other);

    ~FunctionC();
    FunctionC& operator=(const FunctionC& other);

    void copyTo(FunctionC* node) const;

    DataTypeStruct m_datatype;   // 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
    string m_expStr;  // it's the full function string, including function name and parameters
    string m_funcName; // analyzed function name, upper case
    short int m_funcID; // function ID
    vector<ExpressionC> m_params; // parameter expressions.
    vector<int> m_anaParaNums; // The number of parameter (splitted by ;) in each part (splitted by ,) of analytic function
    bool m_bDistinct;  // distinct flag for aggregation function
    //int m_anaFirstParamNum; // The number of first part  parameters of analytic function . 

    bool runFunction(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool isConst() const;   // if all parameters are const
    void setExpStr(string expStr);
    DataTypeStruct analyzeColumns(vector<string>* fieldnames, vector<DataTypeStruct>* fieldtypes, DataTypeStruct* rawDatatype, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes); 
    bool columnsAnalyzed();
    bool expstrAnalyzed();
    bool isAggFunc() const;
    bool isMacro() const;
    bool isAnalytic() const;
    bool containRefVar() const; // check if the function (parameters) contains reference variable @R[][]
    void dump();
    FunctionC* cloneMe();
    void clear(); // clear predictin
    bool remove(FunctionC* node); // remove a node from prediction. Note: the input node is the address of the node contains in current prediction
    vector<ExpressionC> expandForeach(int maxFieldNum); // expand foreach to a vector of expression
    vector<ExpressionC> expandForeach(vector<ExpressionC> vExps); // expand foreach to a vector of expression

  private:
    bool m_metaDataAnzlyzed; // analyze column name to column id.
    bool m_expstrAnalyzed; // if expression string analyzed
    vector<string>* m_fieldnames;  // all nodes (parent & children) point to the same address!!!
    vector<DataTypeStruct>* m_fieldtypes;     // all nodes (parent & children) point to the same address!!!

    bool analyzeExpStr();  // analyze expression string to get the function name (upper case) and parameter expression (classes)
    
    bool runIsnull(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runUpper(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runLower(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runSubstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runInstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runFindnth(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runStrlen(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runComparestr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runComparenum(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runComparedate(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runNoCaseComparestr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runReplace(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runRegreplace(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runRegmatch(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runCountword(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runGetword(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runCountstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runFieldname(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runTrimleft(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runTrimright(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runTrim(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runToint(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runTolong(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runTofloat(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runTostr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runTodate(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runDectohex(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runHextodec(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runDectobin(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runBintodec(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runDatatype(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runSwitch(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runPad(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runGreatest(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runLeast(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runFloor(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runCeil(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runTimediff(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runAddtime(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runRound(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runLog(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runRandom(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runRandstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runCamelstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runSnakestr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runRevertstr(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runAscii(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runChar(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runMod(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runAbs(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runDateformat(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runTruncdate(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);
    bool runNow(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* sideDatarow, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes, string & sResult, DataTypeStruct & dts);

  protected:
    void init();
};

#endif // __FUNCTIONC_H

