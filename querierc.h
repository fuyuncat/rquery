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
#include <unordered_map>
#include "commfuncs.h"
#include "filter.h"
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
//#include <boost/regex.hpp>

using namespace boost::xpressive;

class namesaving_smatch : public smatch
{
public:
  namesaving_smatch();

  namesaving_smatch(const string pattern);

  ~namesaving_smatch();
  
  void init(const string pattern);

  vector<string>::const_iterator names_begin() const;

  vector<string>::const_iterator names_end() const;

private:
  vector<string> m_names;
};

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
    void setFileName(string filename);
    void setUserVars(string variables);
    void setOutputFormat(short int format=TEXT);
    int searchNext(namesaving_smatch & matches);
    int searchAll();
    bool toGroupOrSort();
    bool group();
    bool sort();
    bool searchStopped();
    void output();
    void clear();
    void outputExtraInfo(size_t total, short int mode, bool bPrintHeader);
    void outputAndClean();
    int boostmatch( vector<string> *result = NULL);
    int boostmatch( map<string,string> & result);
    void printFieldNames();
    void setFieldDatatype(string field, int datetype, string extrainfo="");
    long getMatchedCount();
    long getOutputCount();
    long getLines();
    size_t getRawStrSize();

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
    short int m_outputformat; // TEXT or JSON
    
    //vector<namesaving_smatch> m_results;
    FilterC* m_filter;
    vector<string> m_fieldnames;    // field names
    //vector<int> m_fieldtypes;       // field datatype in sequence
    //map<string, int> m_fieldntypes; // field datatype by names, set by setFieldDatatype
    vector<DataTypeStruct> m_fieldtypes;       // field datatype in sequence
    map<string, DataTypeStruct> m_fieldntypes; // field datatype by names, set by setFieldDatatype
    map<string, string> m_uservariables; // User defined variables
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
    std::set< vector<string> > m_groupKeys;  // group keys
    unordered_map< vector<string>, vector<string>, hash_container< vector<string> > > m_aggSelResults;  // temp results for all selections if aggregation functions or group involed. mapping group keys:selections, it should be 1:1
    unordered_map< string,GroupProp > m_initAggProps; // an initial aggregation function properties map retrieved from the selection&sort expressions
    unordered_map< string,ExpressionC > m_aggFuncExps; // aggregation function expressions map retrieved from the selection&sort expressions. mapping funcExpStr:ExpressionC
    unordered_map< vector<string>, unordered_map< string,GroupProp >, hash_container< vector<string> > > m_aggGroupProp; // mapping group keys: group properties
    bool m_aggrOnly; // the selection has aggregation functions only, no matter if there is any group field.

    bool matchFilter(vector<string> rowValue); // filt a row data by filter. no predication mean true. comparasion failed means alway false
    void evalAggExpNode(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp > & aggGroupProp);  // eval expression in aggregation paramter and store in a data set
    bool addResultToSet(vector<string>* fieldvalues, map<string,string>* varvalues, vector<string> rowValue, vector<ExpressionC> expressions, unordered_map< string,GroupProp >* aggFuncs, vector< vector<string> > & resultSet); // add a data row to a result set
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

