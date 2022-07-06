/*******************************************************************************
//
//        File: querierc.h
// Description: Querier class header
//       Usage: querierc.h
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/

#ifndef __QUERIERC_H
#define __QUERIERC_H

#include <regex.h>
#include <vector>
#include <string>
#include "commfuncs.h"
#include "filter.h"
//#include <boost/regex.hpp>

struct GroupDataSet{
  vector<string> nonAggSels;  // all non-aggr selections values
  map<string,vector<string>> aggFuncTaget; // evaled func parameter expression. mapping func_expr:evaled parameter expressions
};

class ExpressionC;

class QuerierC
{
  public:

    QuerierC();
    QuerierC(string regexstr);
    ~QuerierC();

    void setregexp(string regexstr);
    void setrawstr(string rawstr);
    void appendrawstr(string rawstr);
    void assignFilter(FilterC* filter);
    bool assignSelString(string selstr);
    bool assignGroupStr(string groupstr);
    bool setFieldTypeFromStr(string setstr);
    int searchNext();
    int searchAll();
    bool group();
    void output();
    void outputAndClean();
    int boostmatch( vector<string> *result = NULL);
    int boostmatch( map<string,string> & result);
    void printFieldNames();
    void setFieldDatatype(string field, int datetype);
    long getMatchedCount();

  private:
    string m_regexstr;
    sregex m_regexp;
    string m_rawstr;
    regex_constants::match_flag_type m_searchflags;
    
    string m_filename;  // Data source file name
    long m_line;        // data line number matched searching pattern in the file or in a stream input
    long m_matchcount;  // number of matched rows. Can be used to match @row
    long m_outputrow;   // number of outputed rows. m_matchcount doent not always equal to m_outputrow. When sorting is required, outputed rows could be a part of sorted matched rows. Can be used to match @rowsorted.
    
    //vector<namesaving_smatch> m_results;
    vector<string> m_fieldnames;    // field names
    vector<int> m_fieldtypes;       // field datatype in sequence
    map<string, int> m_fieldntypes; // field datatype by names, set by setFieldDatatype
    vector< vector<string> > m_results; // First element is the matched raw string, followed by each filed value, then line number, matched row sequence number
    FilterC* m_filter;
    vector<ExpressionC> m_selections;    // selected expressions
    vector<ExpressionC> m_groups;    // group expressions
    //vector< GroupDataSet > m_tmpResults;  // temp results for calculating aggregation functions. 
    map<vector<string>, GroupDataSet> m_tmpResults;  // temp results for calculating aggregation functions. 

    bool matchFilter(vector<string> rowValue, FilterC* filter); // filt a row data by filter. no predication mean true. comparasion failed means alway false
    void evalAggExpNode(ExpressionC* node, vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, GroupDataSet & dateSet); // eval expression in aggregation paramter and store in a data set
    void runAggFuncExp(ExpressionC* node, map<string,vector<string>>* dateSet, string & sResult); // run aggregation function

  protected:
    void init();
    //void formatoutput(namesaving_smatch matches);
    void formatoutput(vector<string> datas);
    void pairFiledNames(namesaving_smatch matches);
    void analyzeFiledTypes(namesaving_smatch matches);
};

#endif // __QUERIERC_H

