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
class FilterC;

class FunctionC
{
  public:

    FunctionC();
    FunctionC(const string & expStr);
    FunctionC(const FunctionC& other);

    ~FunctionC();
    FunctionC& operator=(const FunctionC& other);

    void copyTo(FunctionC* node) const;

    DataTypeStruct m_datatype;   // 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
    string m_expStr;  // it's the full function string, including function name and parameters
    string m_funcName; // analyzed function name, upper case
    short int m_funcID; // function ID
    vector<ExpressionC> m_params; // parameter expressions.
    vector<FilterC> m_filters; // filter as parameter for some functions, e.g WHEN.
    vector<int> m_anaParaNums; // The number of parameter (splitted by ;) in each part (splitted by ,) of analytic function
    bool m_bDistinct;  // distinct flag for aggregation function
    //int m_anaFirstParamNum; // The number of first part  parameters of analytic function . 

    bool runFunction(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool isConst();   // if all parameters are const
    void setExpStr(const string & expStr);
    DataTypeStruct analyzeColumns(vector<string>* fieldnames, vector<DataTypeStruct>* fieldtypes, DataTypeStruct* rawDatatype, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes); 
    bool columnsAnalyzed();
    bool expstrAnalyzed();
    bool isAggFunc();
    bool isMacro();
    bool isAnalytic();
    bool isTree();
    bool containRefVar(); // check if the function (parameters) contains reference variable @R[][]
    void dump();
    FunctionC* cloneMe();
    void clear(); // clear predictin
    bool remove(FunctionC* node); // remove a node from prediction. Note: the input node is the address of the node contains in current prediction
    void expandForeach(vector<ExpressionC> & vExpr, const int & maxFieldNum); // expand foreach to a vector of expression
    void expandForeach(vector<ExpressionC> & vExpr, const vector<ExpressionC> & vExps); // expand foreach to a vector of expression

  private:
    bool m_metaDataAnzlyzed; // analyze column name to column id.
    bool m_expstrAnalyzed; // if expression string analyzed
    vector<string>* m_fieldnames;  // all nodes (parent & children) point to the same address!!!
    vector<DataTypeStruct>* m_fieldtypes;     // all nodes (parent & children) point to the same address!!!
    DataTypeStruct* m_rawDatatype;

    bool analyzeExpStr();  // analyze expression string to get the function name (upper case) and parameter expression (classes)
    
    bool runIsnull(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runUpper(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runLower(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runSubstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runInstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runFindnth(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runStrlen(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runComparestr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runComparenum(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runComparedate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runNoCaseComparestr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runReplace(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRegreplace(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRegmatch(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRegcount(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRegget(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runCountword(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runGetword(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runGetpart(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runGetparts(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runCountpart(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runCalparts(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runReggetpart(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runReggetparts(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRegcountpart(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRegcalparts(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runCountstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runFieldname(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runConcat(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runConcatcol(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTrimleft(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTrimright(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTrim(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runPad(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runGreatest(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runLeast(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runSumall(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runEval(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRandstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runCamelstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runSnakestr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRevertstr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runAscii(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runChar(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runUrlencode(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runUrldecode(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runBase64encode(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runBase64decode(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runDuplicate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);

    bool runFloor(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runCeil(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRound(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runLog(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRandom(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runMod(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runAbs(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runAcos(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runAcosh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runAsin(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runAsinh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runAtan(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runAtan2(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runAtanh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runCbrt(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runCopysign(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runCos(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runCosh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runErf(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runExp(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runExp2(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runFma(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runFmod(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runFpclassify(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runHypot(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIlogb(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsfinite(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsinf(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsnormal(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runLgamma(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runLog10(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runLog2(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runPow(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRemainder(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runScalbln(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runScalbn(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runSin(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runSinh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runSqrt(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTan(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTanh(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTgamma(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runPi(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);

    bool runDateformat(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTruncdate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTruncdateu(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTimediff(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runAddtime(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runNow(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsleap(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runWeekday(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runMonthfirstday(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runMonthfirstmonday(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runYearweek(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runYearday(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runDatetolong(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runLongtodate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runLocaltime(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runGmtime(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);

    bool runRcount(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRmember(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRmemberid(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRmembers(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);

    bool runDetectdt(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIslong(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsdouble(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsdate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsstring(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runToint(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTolong(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTofloat(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTostr(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runTodate(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runDectohex(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runHextodec(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runDectobin(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runBintodec(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runDatatype(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runSwitch(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runWhen(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runCalcol(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);

    bool runMd5(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runHash(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsip(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsipv6(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsmac(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runMyips(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runHostname(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runExec(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);

    bool runAppendfile(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsfile(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsfolder(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runFileexist(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRmfile(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runRenamefile(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runFilesize(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runFileattrs(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIsexecutable(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runIssymblink(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);
    bool runGetsymblink(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);

    bool runUsermacro(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts);

  protected:
    void init();
};

#endif // __FUNCTIONC_H

