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

  SortProp& operator=(SortProp other)
  {
    if (this != &other){
      other.sortKey.copyTo(&(sortKey));
      direction=other.direction;
    }
    return *this;
  }
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
    void setReadmode(short int readmode);
    void setEof(bool bEof);
    void assignFilter(FilterC* filter);
    void assignExtraFilter(string sFilterStr);
    bool assignGroupStr(string groupstr);
    bool assignSelString(string selstr);
    bool assignSortStr(string sortstr);
    bool assignLimitStr(string limitstr);
    bool assignMeanwhileString(string mwstr);
    bool assignTreeStr(string treestr);
    bool assignReportStr(string reportstr);
    bool setFieldTypeFromStr(string setstr);
    void setFileName(string filename);
    void setNameline(bool nameline);
    void setUserVars(string variables);
    void setUniqueResult(bool bUnique);
    void setOutputFiles(string outputfilestr, short int outputmode=OVERWRITE);
    void setDetectTypeMaxRowNum(int detectTypeMaxRowNum);
    void setOutputFormat(short int format=TEXT);
    void setFieldDelim(string delimstr);
    int searchNext();
    int searchAll();
    bool toGroupOrSort();
    bool group();
    bool sort();
    bool tree();
    bool analytic();
    void unique();
    bool searchStopped();
    void applyExtraFilter();
    void output();
    void clear();
    void outputExtraInfo(size_t total, bool bPrintHeader);
    void outputAndClean();
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
    string m_quoters;
    namesaving_smatch m_matches;
    regex_constants::match_flag_type m_searchflags;
    short int m_searchMode;
    short int m_readmode;
    bool m_bEof;
    bool m_delmrepeatable;
    bool m_delmkeepspace; // whether to trim the space for delimeter search
    string m_fielddelim;
    
    short int m_outputformat; // TEXT or JSON
    short int m_outputmode; // file write mode: OVERWRITE or APPEND
    ExpressionC* m_outputfileexp; // expression of output files
    unordered_map< string, ofstream* > m_outputfiles; // output files. mapping: filename:outstream
    vector< ofstream* > m_resultfiles; // output file of each result row
    
    string m_filename;  // Data source file name
    long m_fileid;        // File sequence number, starting from 1
    bool m_bNamePrinted;// a flag for checking if field names are printed.
    long m_line;        // data line number matched searching pattern in the file or in a stream input
    long m_fileline;    // data line number matched searching pattern in the current file
    long m_matchcount;  // number of matched rows. Can be used to match @row
    long m_outputrow;   // number of outputed rows. m_matchcount doent not always equal to m_outputrow. When sorting is required, outputed rows could be a part of sorted matched rows. Can be used to match @rowsorted.
    bool m_nameline; // The first matched line be used for field name.
    bool m_bUniqueResult; // flag for the returned result is unique or not
    int m_detectTypeMaxRowNum; // How many rows to be used to detect the data types
    int m_detectedTypeRows; // How many rows have been used to detect the data types so far
    string m_selstr; // select raw string
    string m_sortstr; // sort raw string
    string m_treestr; // tree raw string
    string m_reportstr; // report raw string
    bool m_bSelectContainMacro; // flag indicating if macro function exists in select expressions
    bool m_bToAnalyzeSelectMacro; // whether need to analyze marco in selections
    bool m_bSortContainMacro; // flag indicating if macro function exists in sort expressions
    bool m_bToAnalyzeSortMacro; // whether need to analyze marco in sort
    
    vector<vector<int>> m_colToRows; // the selection list of each COLTOROW marco function
    vector<string> m_colToRowNames; // field names of COLTOROW marco function

    vector< vector<ExpressionC> > m_sideSelections; // side query selections
    vector< vector<string> > m_sideAlias; // side query selections
    vector< FilterC > m_sideFilters; // side query filters
    vector< vector< unordered_map<string,string> > > m_sideDatasets; // map sideworkID(starting from 1):vector of result set (selExp/alias:result)
    unordered_map< string, unordered_map<string,DataTypeStruct> > m_sideDatatypes; // map sideworkID(starting from 1):vector of data types (selExp/alias:datatype)
    //unordered_map< int,int > m_sideMatchedRowIDs; // the matched side work data set row IDs when the dataset involved in the main query filter.
    //unordered_map< string, unordered_map<string,string> > m_matchedSideDatarow; // the matched side work data set rows

    //vector<namesaving_smatch> m_results;
    FilterC* m_filter;
    FilterC* m_extrafilter; // an extra filter to filter the result set.
    vector<ExpressionC> m_trimedSelctions; // trimmed selections in the extra filter.
    vector<string> m_fieldInitNames;    // field names, in case field size changes, we need to keep the initial field names.
    vector<string> m_fieldnames;    // field names
    //vector<int> m_fieldtypes;       // field datatype in sequence
    //map<string, int> m_fieldntypes; // field datatype by names, set by setFieldDatatype
    vector<DataTypeStruct> m_fieldtypes;       // field datatype in sequence
    DataTypeStruct m_rawDatatype;  // @raw data type for special cases, e.g. each line contains only one data
    map<string, DataTypeStruct> m_fieldntypes; // field datatype by names, set by setFieldDatatype
    string m_uservarstr;
    map<string, string> m_uservariables; // User defined variables, map to initial/calculated values
    map<string, string> m_uservarinitval; // User defined variables initial values
    map<string, ExpressionC> m_uservarexprs; // User defined dynamic variables expressions
    vector<string>  m_selnames; // selection names
    vector<ExpressionC> m_selections;    // selected expressions
    vector<ExpressionC> m_groups;    // group expressions
    vector<SortProp> m_sorts;     // sorting expressions. Any INTEGER number will be mapped to the correspond sequence of the selections.
    int m_limitbottom;  // output limit start
    int m_limittop;     // output limit top, -1 means no limit
    vector< vector<string> > m_sortKeys;  // extra sorting keys. The sorting keys that are not a parts of selections, it could be aggregation functions
    vector< vector<string> > m_results; // Result set. First element is the matched raw string, followed by each filed value, then line number, matched row sequence number
    vector<ExpressionC> m_treeProps; // Key Properties (fields) of hierarchy structure (tree)
    vector<ExpressionC> m_treeParentProps; // Parent Key Properties (fields) of hierarchy structure (tree)
    vector< vector<string> > m_treeKeys;  // Keys of hierarchy structure (tree)
    vector< vector<string> > m_treeParentKeys;  // Parent keys of hierarchy structure (tree)
    //vector< vector< vector<string> > > m_analyticresults; // vector to store result set of agg functions, like rank(group, sort, equalincrease)
    //vector< GroupDataSet > m_tmpResults;  // temp results for calculating aggregation functions. 
    //map<vector<string>, GroupDataSet> m_tmpResults;  // temp results for calculating aggregation functions. 
    std::set< vector<string> > m_groupKeys;  // group keys
    unordered_map< vector<string>, vector<string>, hash_container< vector<string> > > m_aggRowValues;  // Matched row values for each group. mapping group keys:selections, it should be 1:1
    unordered_map< string,GroupProp > m_initAggProps; // an initial aggregation function properties map retrieved from the selection&sort expressions
    unordered_map< string,ExpressionC > m_aggFuncExps; // aggregation function expressions map retrieved from the selection&sort expressions. mapping funcExpStr:ExpressionC
    unordered_map< vector<string>, unordered_map< string,GroupProp >, hash_container< vector<string> > > m_aggGroupProp; // mapping group keys: group properties
    bool m_aggrOnly; // the selection has aggregation functions only, no matter if there is any group field.
    unordered_map< string,vector<ExpressionC> > m_initAnaArray; // an initial analytic function vector retrieved from the selection&sort expressions. Map analytic_func_str(including params):vector of param expressions
    unordered_map< int, vector<string> > m_aggGroupDataidMap; // map the aggregation function groups to the id of the first matched data row. mapping id:groups. Need to eval the filter in analytic final calculating function
    vector< vector<ExpressionC> > m_anaEvaledExp; // Evaled selection/sort expression including analytic function grouping&sorting.
    vector< unordered_map< string,string > > m_anaFuncResult; // the result of each evolved analytic function. The value will be empty during searching&matching, it will be assigned actually in the final analytic() function
    unordered_map< string,vector< vector<string> > > m_anaSortData; // data to be sorted for analytic function
    unordered_map< string,vector<SortProp> > m_anaSortProps; // sort props of sorting data for analytic function
    unordered_map< string,vector<int> > m_anaFuncParaNums; // The number of parameter (splitted by ;) in each part (splitted by ,) of analytic function
    unordered_map< int,string > m_reportNames; // report aggregation operations. mapping <selectionIndex>:<aggregation operation>
    unordered_map< int,short int > m_reportOps; // report aggregation operations. mapping <selectionIndex>:<aggregation operation>
    unordered_map< int,double > m_reportResult; // report aggregation results. mapping <selectionIndex>:<result>

    bool analyzeSelString();
    vector<ExpressionC> genSelExpression(string sSel, vector<string> & vAlias);
    bool analyzeSortStr();
    bool analyzeTreeStr();
    bool analyzeReportStr();
    bool checkSelGroupConflict(const ExpressionC & eSel);
    bool checkSortGroupConflict(const ExpressionC & eSort);
    void addResultOutputFileMap(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* matchedSideDatarow);
    void doSideWorks(vector<string> * pfieldValues, map<string, string> * pvarValues, unordered_map< string,GroupProp > * paggGroupProp, unordered_map< string,vector<string> > * panaFuncData); // do side queries
    void getSideDatarow(unordered_map< int,int > & sideMatchedRowIDs, unordered_map< string, unordered_map<string,string> > & matchedSideDatarow);
    bool matchFilter(const vector<string> & rowValue); // filt a row data by filter. no predication mean true. comparasion failed means alway false
    void evalAggExpNode(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp > & aggGroupProp, unordered_map< string, unordered_map<string,string> > & matchedSideDatarow);  // eval expression in aggregation paramter and store in a data set
    void appendResultSet(vector<string> vResult); // add a row to result set, doing columns to rows if coltorow involved
    bool addResultToSet(vector<string>* fieldvalues, map<string,string>* varvalues, vector<string> rowValue, vector<ExpressionC> expressions, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, vector<ExpressionC>* anaEvaledExp, unordered_map< string, unordered_map<string,string> > & matchedSideDatarow, vector< vector<string> > & resultSet); // add a data row to a result set
    //void mergeSort(int iLeft, int iMid, int iRight);
    void mergeSort(int iLeftB, int iLeftT, int iRightB, int iRightT);
    void mergeSort(vector< vector<string> > *dataSet, vector<SortProp>* sortProps, vector< vector<string> >* sortKeys, int iLeftB, int iLeftT, int iRightB, int iRightT);
    void addAnaFuncData(unordered_map< string,vector<string> > anaFuncData);
    bool sortAnaData(vector<SortProp> & sortProps, const short int & iFuncID, const string & sFuncExpStr, vector< vector<string> > & vFuncData);
    bool processAnalytic(const string & sFuncExpStr, vector< vector<string> > & vFuncData);
    bool processAnalyticA(const short int & iFuncID, const string & sFuncExpStr, vector< vector<string> > & vFuncData);
    bool processAnalyticB(const short int & iFuncID, const string & sFuncExpStr, vector< vector<string> > & vFuncData);
    bool processAnalyticC(const short int & iFuncID, const string & sFuncExpStr, vector< vector<string> > & vFuncData);
    int searchNextReg();
    int searchNextWild();
    int searchNextDelm();
    void genReport(vector<string> datas);
    void SetTree(const vector< vector<string> > & tmpResults, TreeNode* tNode, short int level, int & nodeid, unordered_map< string,vector<ExpressionC> > & treeFuncs);
    void releaseTree(TreeNode* tNode);
    void clearGroup();
    void clearSort();
    void clearTree();
    void clearAnalytic();
    void clearReport();
    void clearFilter();

#ifdef __DEBUG__
    long int m_querystartat;
    long int m_totaltime;
    long int m_searchtime;
    long int m_rawreadtime;
    long int m_rawsplittime;
    long int m_rawanalyzetime;
    long int m_filtertime;
    long int m_extrafiltertime;
    long int m_sorttime;
    long int m_treetime;
    long int m_uniquetime;
    long int m_grouptime;
    long int m_analytictime;
    long int m_filtercomptime;
    long int m_extrafiltercomptime;
    long int m_evalGroupKeytime;
    long int m_prepAggGPtime;
    long int m_evalAggExptime;
    long int m_evalSeltime;
    long int m_evalSorttime;
    long int m_updateResulttime;
    long int m_outputtime;
#endif // __DEBUG__

  protected:
    void init();
    //void formatoutput(namesaving_smatch matches);
    void outputstream(int resultid, const char *fmt, ...);
    void formatoutput(vector<string> datas, int resultid);
    void pairFiledNames(namesaving_smatch matches);
    void analyzeFiledTypes(vector<string> matches);
    void trialAnalyze(vector<string> matcheddata);
};

#endif // __QUERIERC_H

