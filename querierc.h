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

//struct GroupDataSet{
//  vector< vector<string> > nonAggSels;  // all non-aggr selections values
//  map< string,vector<string> > aggFuncTaget; // evaled func parameter expression. mapping func_expr:evaled parameter expressions
//};

struct SortProp{
  ExpressionC sortKey;
  short int direction; //1 ASC; 2 DESC
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
    bool assignSortStr(string sortstr);
    bool assignLimitStr(string limitstr);
    bool setFieldTypeFromStr(string setstr);
    int searchNext();
    int searchAll();
    bool group();
    bool sort();
    bool searchStopped();
    void output();
    void clear();
    void outputAndClean();
    int boostmatch( vector<string> *result = NULL);
    int boostmatch( map<string,string> & result);
    void printFieldNames();
    void setFieldDatatype(string field, int datetype, string extrainfo="");
    long getMatchedCount();
    long getOutputCount();
    long getLines();

  private:
    string m_regexstr;
    sregex m_regexp;
    string m_rawstr;
    regex_constants::match_flag_type m_searchflags;
    
    string m_filename;  // Data source file name
    bool m_bNamePrinted;// a flag for checking if field names are printed.
    long m_line;        // data line number matched searching pattern in the file or in a stream input
    long m_matchcount;  // number of matched rows. Can be used to match @row
    long m_outputrow;   // number of outputed rows. m_matchcount doent not always equal to m_outputrow. When sorting is required, outputed rows could be a part of sorted matched rows. Can be used to match @rowsorted.
    
    //vector<namesaving_smatch> m_results;
    FilterC* m_filter;
    vector<string> m_fieldnames;    // field names
    //vector<int> m_fieldtypes;       // field datatype in sequence
    //map<string, int> m_fieldntypes; // field datatype by names, set by setFieldDatatype
    vector<DataTypeStruct> m_fieldtypes;       // field datatype in sequence
    map<string, DataTypeStruct> m_fieldntypes; // field datatype by names, set by setFieldDatatype
    vector<string>  m_selnames; // selection names
    vector<ExpressionC> m_selections;    // selected expressions
    vector<ExpressionC> m_groups;    // group expressions
    vector<SortProp> m_sorts;     // sorting expressions. Any INTEGER number will be mapped to the correspond sequence of the selections.
    int m_limitbottom;  // output limit start
    int m_limittop;     // output limit top, -1 means no limit
    vector< vector<string> > m_sortKeys;  // extra sorting keys. The sorting keys that are not a parts of selections, it could be aggregation functions
    vector< vector<string> > m_results; // First element is the matched raw string, followed by each filed value, then line number, matched row sequence number
    //vector< GroupDataSet > m_tmpResults;  // temp results for calculating aggregation functions. 
    //map<vector<string>, GroupDataSet> m_tmpResults;  // temp results for calculating aggregation functions. 
    map< vector<string>, vector<string> > m_nonAggSels;  // temp results for non aggregation functions selections. mapping func_expr:selections 
    map< vector<string>, map< string,vector<string> > > m_aggFuncTaget;  // temp results for calculating aggregation functions. mapping func_expr:evaled parameter expressions

    bool matchFilter(vector<string> rowValue, FilterC* filter); // filt a row data by filter. no predication mean true. comparasion failed means alway false
    void evalAggExpNode(ExpressionC* node, vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, map< string,vector<string> > & aggFuncTaget); // eval expression in aggregation paramter and store in a data set
    void runAggFuncExp(ExpressionC* node, map< string,vector<string> >* dateSet, string & sResult); // run aggregation function
    bool addResultToSet(vector<string>* fieldvalues, map<string,string>* varvalues, vector<string> rowValue, vector<ExpressionC> expressions, vector< vector<string> > & resultSet); // add a data row to a result set
    //void mergeSort(int iLeft, int iMid, int iRight);
    void mergeSort(int iLeftB, int iLeftT, int iRightB, int iRightT);

#ifdef __DEBUG__
    long int m_searchtime;
    long int m_filtertime;
#endif // __DEBUG__

  protected:
    void init();
    //void formatoutput(namesaving_smatch matches);
    void formatoutput(vector<string> datas);
    void pairFiledNames(namesaving_smatch matches);
    void analyzeFiledTypes(namesaving_smatch matches);
};

#endif // __QUERIERC_H

