/*******************************************************************************
//
//        File: querierc.cpp
// Description: Querier class defination
//       Usage: querierc.cpp
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "expression.h"
#include "function.h"
#include "querierc.h"

namesaving_smatch::namesaving_smatch()
{
}

namesaving_smatch::namesaving_smatch(const string pattern)
{
  init(pattern);
}

namesaving_smatch::~namesaving_smatch() { }

void namesaving_smatch::init(const string pattern)
{
  string pattern_str = pattern;
  sregex capture_pattern = sregex::compile("\\?P?<(\\w+)>");
  sregex_iterator words_begin = sregex_iterator(pattern_str.begin(), pattern_str.end(), capture_pattern);
  sregex_iterator words_end = sregex_iterator();

  string name;
  for (sregex_iterator i = words_begin; i != words_end; i++){
    name = (*i)[1].str();
    m_names.push_back(name);
  }
}

vector<string>::const_iterator namesaving_smatch::names_begin() const
{
    return m_names.begin();
}

vector<string>::const_iterator namesaving_smatch::names_end() const
{
    return m_names.end();
}

#ifdef __DEBUG__
long int g_tmptimer1time;
long int g_tmptimer2time;
long int g_strtodatetime;
long int g_datetostrtime;
long int g_zonetimetime;
long int g_truncdatetime;
long int g_checkisdatetime;
#endif // __DEBUG__

QuerierC::QuerierC()
{
  init();
}

QuerierC::QuerierC(const string & regexstr)
{
  init();
  setregexp(regexstr);
}

QuerierC::~QuerierC()
{

}

void QuerierC::init()
{
  m_filter = NULL;
  m_extrafilter = NULL;
  m_dupnumexp = NULL;
  m_dupfilter = NULL;
  m_rawstr = "";
  m_searchflags = regex_constants::match_default;
  m_searchMode = REGSEARCH;
  m_readmode = READLINE;
  m_bEof = false;
  m_quickwildcard = false;
  m_quickregular = false;
  m_delmrepeatable = false;
  m_delmkeepspace = false;
  m_eliminateDupField = false;
  m_delmMulitChar = false;
  m_quoters = "";
  m_filename = "";
  m_fileid = 0;
  m_nameline = false;
  m_line = 0;
  m_fileline = 0;
  m_outputformat = TEXT;
  m_outputmode = STANDARD;
  m_outputfileexp = NULL;
  m_matchcount = 0; 
  m_outputrow = 0;
  m_limitbottom = 1;
  m_limittop = -1;
  m_bNamePrinted = false;
  m_aggrOnly = false;
  m_bUniqueResult = false;
  m_bSelectContainMacro = false;
  m_bToAnalyzeSelectMacro = false;
  m_bSortContainMacro = false;
  m_bToAnalyzeSortMacro = false;
  m_bTextOnly = false;
  m_bStatOnly = false;
  m_bDetectAllOnChange = false;
  m_selstr = "";
  m_sortstr = "";
  m_treestr = "";
  m_reportstr = "";
  m_dupstr = "";
  m_extfilstr = "";
  m_fielddelim = "\t";
  m_detectTypeMaxRowNum = 1;
  m_detectedTypeRows = 0;
#ifdef __DEBUG__
  m_querystartat = curtime();
  m_totaltime = 0;
  m_searchtime = 0;
  m_runtimedatapreparetime = 0;
  m_uservarcaltime = 0;
  m_sideworktime = 0;
  m_getsidedatarowtime = 0;
  m_savetreedatatime = 0;
  m_rawprocesstrimtime = 0;
  m_rawanalyzetime = 0;
  m_readrawlinetime = 0;
  m_appendnonselresulttime = 0;
  m_addanafuncdatatime = 0;
  m_evalanaexprtime = 0;
  m_evalSeltime = 0;
  m_evalSeldeclaretime = 0;
  m_evalSelAssignExprtime = 0;
  m_evalSelRealEvaltime = 0;
  m_evalRawpreptime = 0;
  m_evalRawdupjointime = 0;
  m_appendSelToResulttime = 0;
  m_assignoutputfiletime = 0;
  m_trialanalyzetime = 0;
  m_parsepatterntime = 0;
  m_filtertime = 0;
  m_extrafiltertime = 0;
  m_sorttime = 0;
  m_treetime = 0;
  m_uniquetime = 0;
  m_grouptime = 0;
  m_analytictime = 0;
  m_evalGroupKeytime = 0;
  m_filtercomptime = 0;
  m_extrafiltercomptime = 0;
  m_prepAggGPtime = 0;
  m_evalAggExptime = 0;
  m_evalRawdatatime = 0;
  m_evalSorttime = 0;
  m_updateResulttime = 0;
  m_outputtime = 0;
  
  g_tmptimer1time = 0;
  g_tmptimer2time = 0;
  g_strtodatetime = 0;
  g_datetostrtime = 0;
  g_zonetimetime = 0;
  g_truncdatetime = 0;
  g_checkisdatetime = 0;
  g_evalexprtime = 0;
  g_evalexprconsttime = 0;
  g_evalexprfunctime = 0;
  g_evalexprvartime = 0;
  g_evalexprcoltime = 0;
  g_evalexprmacpatime = 0;
  g_evalexprcaltime = 0;
#endif // __DEBUG__
}

void QuerierC::setregexp(const string & regexstr)
{
  size_t nPos = regexstr.find_last_of('/');
  string flags = nPos == string::npos?"":upper_copy(regexstr.substr(nPos+1)), patternStr = nPos == string::npos?regexstr:regexstr.substr(0,nPos+1);
  m_delmrepeatable = flags.find("R")!=string::npos;
  m_delmkeepspace = flags.find("S")!=string::npos;
  m_delmMulitChar = flags.find("C")!=string::npos;
  m_quickwildcard = flags.find("W")!=string::npos;
  m_quickregular = m_delmrepeatable;
  if (trim_copy(patternStr).empty()){
    m_searchMode = DELMSEARCH;
    m_regexstr = trim_copy(patternStr);
  }else if ((patternStr[0] == 'l' || patternStr[0] == 'L') && patternStr.length()==1){
    m_searchMode = LINESEARCH;
  }else if ((patternStr[0] == 'q' || patternStr[0] == 'Q')&& patternStr[1] == '/'){
    m_searchMode = QUICKSEARCH;
    m_regexstr = patternStr.substr(2,patternStr.length()-3);
    replacestr(m_regexstr,{"\\\\","\\t","\\/","\\v","\\|"},{"\\","\t","/","\v","|"});
  }else if ((patternStr[0] == 'w' || patternStr[0] == 'W')&& patternStr[1] == '/'){
    m_searchMode = WILDSEARCH;
    vector<string> vSearchPattern;
    split(vSearchPattern, patternStr.substr(2,patternStr.length()-3),'/',"",'\\',{'(',')'},false,true);
    if (vSearchPattern.size() == 0){
      trace(FATAL, "(1)'%s' is an unrecognized pattern!\n", patternStr.c_str());
      return;
    }
    m_regexstr = vSearchPattern[0];
    replacestr(m_regexstr,{"\\\\","\\t","\\/","\\v","\\|"},{"\\","\t","/","\v","|"});
    if (vSearchPattern.size()>1){
      if (vSearchPattern[1].length()%2 != 0)
        trace(FATAL, "(1)Quoters must be paired. '%s' will be ignored.\n", vSearchPattern[1].c_str());
      else
        m_quoters = vSearchPattern[1];
    }
  }else if ((patternStr[0] == 'd' || patternStr[0] == 'D') && patternStr[1] == '/'){
    m_searchMode = DELMSEARCH;
    string spattern = patternStr.substr(2,patternStr.length()-3);;
    if (patternStr.length()>3){
      spattern = patternStr.substr(2,patternStr.length()-3);
    }else
      trace(FATAL, "'%s' is not a valid searching pattern\n",patternStr.c_str());
    vector<string> vSearchPattern;
    split(vSearchPattern, spattern,'/',"",'\\',{'(',')'},false,true);
    if (vSearchPattern.size() == 0){
      trace(FATAL, "(1)'%s' is an unrecognized pattern!\n", patternStr.c_str());
      return;
    }
    if (m_delmMulitChar)
      copy(vSearchPattern[0].begin(),vSearchPattern[0].end(),inserter(m_delimSet,m_delimSet.end()));
    else
      m_regexstr = vSearchPattern[0];
    replacestr(m_regexstr,{"\\\\","\\t","\\/","\\v","\\|"},{"\\","\t","/","\v","|"});
    if (m_regexstr.compare(" ")!=0)
      m_delmkeepspace = true;
    if (vSearchPattern.size()>1){
      if (vSearchPattern[1].length()%2 != 0)
        trace(ERROR, "(2)Quoters must be paired. '%s' will be ignored.\n", vSearchPattern[1].c_str());
      else
        m_quoters = vSearchPattern[1];
    }
  }else if (patternStr[0] == '/'){
    m_searchMode = REGSEARCH;
    m_regexstr = trim_one(patternStr, '/');
    try{
      m_regexp = sregex::compile(m_regexstr);
      m_matches = namesaving_smatch(m_regexstr);
    }catch (exception& e) {
      trace(FATAL, "Regular pattern compile exception: %s\n", e.what());
    }
  }else
    trace(FATAL, "Searching pattern '%s' is invalid!\n", patternStr.c_str());
  trace(DEBUG, "Parsed searching string '%s' => '%s', search mode is %d\n", patternStr.c_str(),m_regexstr.c_str(),m_searchMode);
}

void QuerierC::assignFilter(FilterC* filter)
{
  if (m_filter){
    m_filter->clear();
    SafeDelete(m_filter);
  }
  m_filter = filter;
  m_filter->getAggFuncs(m_initAggProps);
  m_filter->getAnaFuncs(m_initAnaArray, m_anaFuncParaNums);
  //m_filter->dump();
}

bool QuerierC::analyzeExtraFilterStr()
{
  FilterC* filter;
  size_t pos = findFirstSub(m_extfilstr, " TRIM ", 0,"''()",'\\',{'(',')'},false );
  if (pos != string::npos){
    filter = new FilterC(trim_copy(m_extfilstr.substr(0,pos)));
    genSelExpression(m_trimedInitSels, trim_copy(m_extfilstr.substr(pos+string(" TRIM ").length())), m_trimmedAlias);
  }else{
    filter = new FilterC(trim_copy(m_extfilstr));
  }

  if (m_extrafilter){
    m_extrafilter->clear();
    SafeDelete(m_extrafilter);
  }
  m_extrafilter = filter;
  unordered_map< string,GroupProp > initAggProps;
  m_extrafilter->getAggFuncs(initAggProps);
  if (initAggProps.size()>0){
    trace(FATAL, "Extra filter cannot have any aggregation function!\n");
    return false;
  }
  unordered_map< string,vector<ExpressionC> > initAnaArray;
  unordered_map< string,vector<int> > anaFuncParaNums;
  m_extrafilter->getAnaFuncs(initAnaArray, anaFuncParaNums);
  if (initAnaArray.size()>0){
    trace(FATAL, "Extra filter cannot have any analytic function!\n");
    return false;
  }
  //m_extrafilter->dump();
  return true;
}

void QuerierC::assignExtraFilter(const string & sFilterStr)
{
  if (trim_copy(sFilterStr).empty())
    return;
  m_extfilstr = trim_copy(sFilterStr);
  analyzeExtraFilterStr();
}

void QuerierC::appendrawstr(const string & rawstr)
{
  m_rawstr.append(rawstr);
}

void QuerierC::setrawstr(const string & rawstr)
{
  m_rawstr = rawstr;
}

void QuerierC::setReadmode(const short int & readmode)
{
  m_readmode = readmode;
}

void QuerierC::setEof(const bool & bEof)
{
  m_bEof = bEof;
}

bool QuerierC::assignGroupStr(const string & groupstr)
{
  if (groupstr.empty())
    return true;
  vector<string> vGroups;
  split(vGroups, groupstr,',',"''()",'\\',{'(',')'},false,true);
  ExpressionC eGroup;
  for (size_t i=0; i<vGroups.size(); i++){
    trace(DEBUG, "Processing group (%d) '%s'!\n", i, vGroups[i].c_str());
    string sGroup = trim_copy(vGroups[i]);
    if (sGroup.empty()){
      trace(ERROR, "Empty group string!\n");
      return false;
    }
    eGroup = ExpressionC(sGroup);
    if (eGroup.m_expType == FUNCTION && eGroup.m_Function && eGroup.m_Function->isMacro()){
      trace(FATAL,"Macro function '%s' cannot be used in GROUP!\n", eGroup.m_Function->m_funcName.c_str());
      continue;
    }
    m_groups.push_back(eGroup);
  }
  return true;
}

void QuerierC::setUniqueResult(const string & unistr)
{
  if (trim_copy(unistr).empty())
    m_bUniqueResult = true;
  else{
    vector<string> vUni;
    split(vUni, upper_copy(trim_copy(unistr)),' ',"",'\0',{},true,true);
    for (size_t i=0; i<vUni.size(); i++){
      if (trim_copy(vUni[i])[0]=='R')
        m_bUniqueResult=true;
      if (trim_copy(vUni[i])[0]=='F')
        m_eliminateDupField = true;
    }
  }
}

void QuerierC::setDetectTypeMaxRowNum(const int & detectTypeMaxRowNum)
{
  m_detectTypeMaxRowNum = max(1,detectTypeMaxRowNum);
}

void QuerierC::setNameline(const bool & nameline)
{
  m_nameline = nameline;
}

bool QuerierC::assignLimitStr(const string & limitstr)
{
  if (limitstr.empty())
    return true;
  vector<string> vLimits;
  split(vLimits, limitstr,',',"''()",'\\',{'(',')'},false,true);
  string sFirst = trim_copy(vLimits[0]);
  int iFirst = 0;
  if (isInt(sFirst))
    iFirst = atoi(sFirst.c_str());
  else{
    trace(FATAL, "%s is not a valid limit number!\n", sFirst.c_str());
    return false;
  }
  if (vLimits.size() > 1){
    string sSecond = trim_copy(vLimits[1]);
    if (isInt(sSecond)){
      m_limitbottom = iFirst;
      m_limittop = atoi(sSecond.c_str());
    }else{
      trace(FATAL, "%s is not a valid limit number!\n", sSecond.c_str());
      return false;
    }
  }else
    m_limittop = iFirst;

  trace(DEBUG1, "Limits is from %d to %d\n", m_limitbottom, m_limittop);
  return true;
}

// m_groups should always be analyzed before m_selections
bool QuerierC::assignSelString(const string & selstr)
{
  m_selstr = selstr;
  return analyzeSelString();
}

bool QuerierC::checkSelGroupConflict(ExpressionC & eSel)
{
  vector<string> allColNames;
  for (size_t i=0; i<m_groups.size(); i++)
    m_groups[i].getAllColumnNames(allColNames);
  if (m_groups.size()>0 && !eSel.groupFuncOnly() && !eSel.inColNamesRange(allColNames)){
    trace(WARNING, "Selection '%s' does not exist in Group or invalid using aggregation function \n", eSel.m_expStr.c_str());
    return false;
  }
  return true;
}

void QuerierC::genSelExpression(vector<ExpressionC> & vSelections, const string & sSel, vector<string> & vAlias)
{
  trace(DEBUG, "Analyzing selections from '%s'!\n", sSel.c_str());
  vector<string> vSelStrs;
  split(vSelStrs, sSel,',',"''()",'\\',{'(',')'},false,true);
  string sAlias, sAs=" as ";
  for (size_t i=0; i<vSelStrs.size(); i++){
    trace(DEBUG, "Processing selection(%d) '%s'!\n", i, vSelStrs[i].c_str());
    string sSel = trim_copy(vSelStrs[i]);
    if (sSel.empty()){
      trace(FATAL, "Empty selection string!\n");
      continue;
    }
    vector<string> vSelAlias;
    split(vSelAlias, sSel,sAs,"''()",'\\',{'(',')'},false,true,false);
    sAlias = "";
    if (vSelAlias.size()>1)
      sAlias = upper_copy(trim_copy(vSelAlias[1]));
    //trace(DEBUG2,"Selection expression: '%s'\n",vSelAlias[0].c_str());
    //eSel.dump();
    vSelections.push_back(ExpressionC(vSelAlias[0]));
    vAlias.push_back(sAlias);
  }
}

bool QuerierC::analyzeSelString(){
  trace(DEBUG, "Analyzing selections from '%s'!\n", m_selstr.c_str());
  m_selections.clear();
  m_selnames.clear();
  vector<ExpressionC> vExpandedExpr; 
  vector<string> vAlias;
  genSelExpression(m_selections, m_selstr, vAlias);
  for (size_t i=0; i<m_selections.size(); i++)
    if (i<vAlias.size()&& !vAlias[i].empty())
      m_selnames.push_back(vAlias[i]);
    else{
      m_selnames.push_back(m_selections[i].getEntireExpstr());
      vAlias[i] = m_selnames[i];
    }
  bool bGroupFunc = false, bNonGroupFuncSel = false;
  m_colToRows.clear();
  m_colToRowNames.clear();
  // process COLTOROW first, in case FOREACH is a parameter of COLTOROW
  for (size_t i=0; i<m_selections.size(); i++){
    // transfer parameters of COLTOROW macro function to selections
    //int iExpandedCols = 0; // number of expended fields when COLTOROW  involved. Need to be skipped in the loop
    if (m_selections[i].m_type == LEAF && m_selections[i].m_expType == FUNCTION && m_selections[i].m_Function && m_selections[i].m_Function->m_funcID==COLTOROW){
      m_colToRowNames.push_back(m_selnames[i]);
      vector<int> colToRowSels;
      m_selnames.erase(m_selnames.begin()+i);
      for (size_t j=0; j<m_selections[i].m_Function->m_params.size(); j++){
        m_selnames.insert(m_selnames.begin()+i+j,m_selections[i].m_Function->m_params[j].getEntireExpstr());
        colToRowSels.push_back(i+j);
      }
      vector<ExpressionC> params = m_selections[i].m_Function->m_params;
      m_selections.erase(m_selections.begin()+i);
      m_selections.insert(m_selections.begin()+i,params.begin(),params.end());
      m_colToRows.push_back(colToRowSels);
      i += params.size()-1;
    }
  }
  
  size_t l=0, k=0; // indexes of m_colToRows
  int extendedSelNum = 0; // extended selection number caused by foreach
  vector<ExpressionC> tmpSels=m_selections; // temp selections in case foreach expanding selections.
  // then process FOREACH
  for (size_t i=0; i<tmpSels.size(); i++){
    if (tmpSels[i].m_expType == FUNCTION && tmpSels[i].m_Function && (tmpSels[i].m_Function->m_funcID == ANYCOL || tmpSels[i].m_Function->m_funcID == ALLCOL)){
      trace(FATAL, "Marco function %s can not be used in SELECT", tmpSels[i].getEntireExpstr().c_str());
      return false;
    }
    if (tmpSels[i].m_type == LEAF && tmpSels[i].m_expType == FUNCTION && tmpSels[i].m_Function && tmpSels[i].m_Function->isAnalytic())
      trace(DEBUG,"QuerierC: The analytic function '%s' group size %d, param size %d \n", tmpSels[i].m_Function->m_expStr.c_str(),tmpSels[i].m_Function->m_anaParaNums[0],tmpSels[i].m_Function->m_params.size());
    //trace(DEBUG, "Got selection expression '%s'!\n", tmpSels[i].getEntireExpstr().c_str());

    // if macro function is involved, need to wait util the first data analyzed to analyze select expression
    if (tmpSels[i].m_expType == FUNCTION && tmpSels[i].m_Function && tmpSels[i].m_Function->m_funcID == FOREACH){
      if (m_fieldtypes.size()==0){
        m_bSelectContainMacro = true;
        m_bToAnalyzeSelectMacro = true;
        m_selections.clear();
        m_selnames.clear();
        m_colToRows.clear();
        trace(DEBUG2,"Skiping select FOREACH: '%s'\n",m_selections[i].m_expStr.c_str());
        return true;
      }else{
        switch(tmpSels[i].m_Function->m_funcID){
          case FOREACH:{
            if (m_groups.size()>0)
              tmpSels[i].m_Function->expandForeach(vExpandedExpr, m_groups);
            else
              tmpSels[i].m_Function->expandForeach(vExpandedExpr, m_fieldtypes.size());
            trace(DEBUG2,"Expanding FOREACH: '%s'\n",tmpSels[i].m_expStr.c_str());
            m_selections.erase(m_selections.begin()+i+extendedSelNum);
            m_selnames.erase(m_selnames.begin()+i+extendedSelNum);
            // go through m_colToRows to search the selection matches current foreach selection, treat l,k as a cursor.
            if (l<m_colToRows.size()&&k>=m_colToRows[l].size()){ // k could be increased by previous foreach expanding.
                k=0;
                l++;
            }
            // find the element in m_colToRows matching current foreach selections.
            while(l<m_colToRows.size()&&m_colToRows[l][k]<i){
              k++;
              if (k>=m_colToRows[l].size()){
                k=0;
                l++;
              }
            }
            bool bToConvColToRow = false;
            if (l<m_colToRows.size() && k<m_colToRows[l].size() && m_colToRows[l][k]==i){
              m_colToRows[l].erase(m_colToRows[l].begin()+k);
              bToConvColToRow = true;
            }
            // expand and insert FOREACH selections.
            for (size_t j=0; j<vExpandedExpr.size(); j++){
              trace(DEBUG2,"Expanded FOREACH expression: '%s'\n",vExpandedExpr[j].m_expStr.c_str());
              if (bToConvColToRow){
                m_colToRows[l].insert(m_colToRows[l].begin()+k,i+j+extendedSelNum);
                k++;
              }
              m_selnames.insert(m_selnames.begin()+i+j+extendedSelNum,vExpandedExpr[j].getEntireExpstr());
              //m_selnames.push_back(vExpandedExpr[j].getEntireExpstr());
              checkSelGroupConflict(vExpandedExpr[j]);
              if (vExpandedExpr[j].containGroupFunc())
                bGroupFunc = true;
              else
                bNonGroupFuncSel = true;
              vExpandedExpr[j].getAggFuncs(m_initAggProps);
              vExpandedExpr[j].getAnaFuncs(m_initAnaArray, m_anaFuncParaNums);
              m_selections.insert(m_selections.begin()+i+j+extendedSelNum,vExpandedExpr[j]);
              //i++;
            }
            extendedSelNum+=vExpandedExpr.size()-1;
            m_bToAnalyzeSelectMacro = false;
            //l++;
            break;
          }
        }
        continue;
      }
    }

    checkSelGroupConflict(tmpSels[i]);
    if (tmpSels[i].containGroupFunc())
      bGroupFunc = true;
    else
      bNonGroupFuncSel = true;

    tmpSels[i].getAggFuncs(m_initAggProps);
    tmpSels[i].getAnaFuncs(m_initAnaArray, m_anaFuncParaNums);
    //trace(DEBUG, "Selection: '%s'!\n", tmpSels[i].getEntireExpstr().c_str());
  }
  m_aggrOnly = (bGroupFunc && !bNonGroupFuncSel && m_groups.size()==0);
  if (bGroupFunc && bNonGroupFuncSel && m_groups.size() == 0){ // selection has non-aggregation function but non group field.
    trace(FATAL, "Invalid using aggregation function in selection, no group involved!\n");
    return false;
  }
  return true;
}

// m_groups, m_selections should always be analyzed before m_sorts
bool QuerierC::assignSortStr(const string & sortstr)
{
  if (sortstr.empty())
    return true;
  m_sortstr = sortstr;
  return analyzeSortStr();
}

bool QuerierC::analyzeTreeStr()
{
  clearTree();
  vector<string> vRawStrs;
  split(vRawStrs, m_treestr,';',"''()",'\\',{'(',')'},false,true);
  if (vRawStrs.size()<2){
    trace(FATAL,"You must provide tree keys and parent keys!\n");
    return false;
  }
  for (size_t i=0;i<vRawStrs.size();i++){
    vector<string> vMapStrs;
    split(vMapStrs, vRawStrs[i],':',"''()",'\\',{'(',')'},false,true);
    if (vMapStrs.size()<2){
      trace(ERROR,"The key string format should be 'k:expr1...' or 'p:expr1..., skip it'!\n");
      continue;
    }
    vector<string> vKeyStrs;
    split(vKeyStrs, vMapStrs[1],',',"''()",'\\',{'(',')'},false,true);
    if (upper_copy(trim_copy(vMapStrs[0])).compare("K")==0){
      for (size_t j=0; j<vKeyStrs.size(); j++){
        m_treeProps.push_back(ExpressionC(vKeyStrs[j]));
        if (m_treeProps[m_treeProps.size()-1].containAnaFunc() || m_treeProps[m_treeProps.size()-1].containGroupFunc()){
          trace(FATAL,"Tree key cannot contain any aggregation or analytic function!\n");
          clearTree();
          return false;
        }
      }
    }else if (upper_copy(trim_copy(vMapStrs[0])).compare("P")==0){
      for (size_t j=0; j<vKeyStrs.size(); j++){
        m_treeParentProps.push_back(ExpressionC(vKeyStrs[j]));
        if (m_treeParentProps[m_treeParentProps.size()-1].containAnaFunc() || m_treeParentProps[m_treeParentProps.size()-1].containGroupFunc()){
          trace(FATAL,"Tree parent key cannot contain any aggregation or analytic function!\n");
          clearTree();
          return false;
        }
      }
    }else{
      trace(ERROR,"Unrecognized flag '%s', it should be K or P, skip it!\n", vMapStrs[0].c_str());
      continue;
    }
  }
  if (m_treeProps.size() != m_treeParentProps.size()){
    trace(FATAL,"The size of tree key (%d) and parent key (%d) does not match!\n", m_treeProps.size(), m_treeParentProps.size());
    clearTree();
    return false;
  }

  return true;
}

// m_groups, m_selections should always be analyzed before m_sorts
bool QuerierC::assignTreeStr(const string & treestr)
{
  if (treestr.empty())
    return true;
  if (m_groups.size()>0 || m_initAnaArray.size()>0 || m_aggrOnly || m_bUniqueResult){
    trace(ERROR,"Tree cannot work with group or unique or analytic functions!\n");
    return false;
  }else{
    m_treestr = treestr;
    return analyzeTreeStr();
  }
}

bool QuerierC::analyzeReportStr()
{
  clearReport();
  vector<string> vRawStrs;
  split(vRawStrs, m_reportstr,',',"''()",'\\',{'(',')'},false,true);
  if (vRawStrs.size()<2){
    trace(FATAL,"You must provide at one pair of selection index and aggregation operation!\n");
    return false;
  }
  for (size_t i=0;i<vRawStrs.size();i++){
    vector<string> vMapStrs;
    split(vMapStrs, vRawStrs[i],':',"''()",'\\',{'(',')'},false,true);
    if (vMapStrs.size()<2){
      trace(FATAL,"The pair format should be 'selection_index:operation...'!\n");
      continue;
    }
    if (isInt(trim_copy(vMapStrs[0]))){
      int iSelIdx = atoi(trim_copy(vMapStrs[0]).c_str());
      //if (iSelIdx>=1 && iSelIdx<= m_selections.size()){ // in case FOREACH
        short int iOp=encodeFunction(upper_copy(trim_copy(vMapStrs[1])));
        if (iOp!=SUM && iOp!=COUNT && iOp!=AVERAGE && iOp!=MAX && iOp!=MIN)
          trace(WARNING,"%s is not a valid operation, it can only be SUM/COUNT/AVERAGE/MAX/MIN, will set it to SUM!\n", vMapStrs[1].c_str());
        m_reportOps.insert(pair< int,short int >(iSelIdx,iOp));
        m_reportResult.insert(pair< int,double >(iSelIdx,0));
        m_reportNames.insert(pair< int,string >(iSelIdx,upper_copy(trim_copy(vMapStrs[1]))));
      //}else{
      //  trace(FATAL,"%d is out of selection range %d!\n", iSelIdx, m_selections.size());
      //  continue;
      //}
    }else{
      trace(FATAL,"The selection index should be an integer number, starting from 1!\n");
      continue;
    }
  }

  return true;
}

bool QuerierC::assignReportStr(const string & reportstr)
{
  if (reportstr.empty())
    return true;
  m_reportstr = reportstr;
  return analyzeReportStr();
}

bool QuerierC::analyzeDupStr()
{
  clearDuplicate();
  vector<string> vAlias;
  size_t pos = findFirstSub(m_dupstr, " WHERE ", 0,"''()",'\\',{'(',')'},false );
  if (pos != string::npos){
    m_dupfilter = new FilterC(trim_copy(m_dupstr.substr(pos+string(" WHERE ").length())));
    m_dupnumexp = new ExpressionC(trim_copy(m_dupstr.substr(0,pos)));
  }else{
    m_dupnumexp = new ExpressionC(trim_copy(m_dupstr));
  }

  return true;
}

bool QuerierC::assignDupStr(const string & dupstr)
{
  if (dupstr.empty())
    return true;
  m_dupstr = dupstr;
  return analyzeDupStr();
}

bool QuerierC::checkSortGroupConflict(ExpressionC & eSort)
{
  if (m_groups.size() > 0) {// checking if compatible with GROUP
    vector<string> allColNames;
    for (size_t i=0; i<m_groups.size(); i++)
      m_groups[i].getAllColumnNames(allColNames);
    if (!eSort.groupFuncOnly() && !eSort.inColNamesRange(allColNames)){
      trace(WARNING, "Sorting key '%s' does not exist in Group or invalid using aggregation function \n", eSort.m_expStr.c_str());
      return false;
    }
  }else{
    if (eSort.containGroupFunc()){
      trace(FATAL, "Invalid using aggregation function in sorting key '%s', no group involved!\n", eSort.m_expStr.c_str());
      return false;
    }
  }

  return true;
}

bool QuerierC::analyzeSortStr(){
  // if macro function is involved in select , need to wait util the first data analyzed to analyze sort expression
  if (m_bToAnalyzeSelectMacro || m_sortstr.empty())
    return true;
  m_sorts.clear();
  trace(DEBUG, "Processing sorting string '%s'!\n", m_sortstr.c_str());
  vector<string> vSorts;
  split(vSorts, m_sortstr,',',"''()",'\\',{'(',')'},false,true);
  SortProp keyProp;
  for (size_t i=0; i<vSorts.size(); i++){
    keyProp = SortProp();
    trace(DEBUG, "Processing sorting keys (%d) '%s'!\n", i, vSorts[i].c_str());
    string sSort = trim_copy(vSorts[i]);
    if (sSort.empty()){
      trace(FATAL, "Empty sorting key!\n");
      return false;
    }
    vector<string> vKP;
    split(vKP, sSort,' ',"''()",'\\',{'(',')'},false,true);
    if (vKP.size()<=1 || upper_copy(trim_copy(vKP[1])).compare("DESC")!=0)
      keyProp.direction = ASC;
    else
      keyProp.direction = DESC;
    keyProp.sortKey.setExpstr(trim_copy(vKP[0]));
    // if macro function is involved , need to wait util the first data analyzed to analyze sort expression
    if (keyProp.sortKey.m_expType == FUNCTION && keyProp.sortKey.m_Function && keyProp.sortKey.m_Function->isMacro()){
      if (m_fieldtypes.size()==0){
        m_bSortContainMacro = true;
        m_bToAnalyzeSortMacro = true;
        m_sorts.clear();
        trace(DEBUG2,"Skiping sort due to Macro function: '%s'\n",keyProp.sortKey.m_expStr.c_str());
        return true;
      }else{
        switch (keyProp.sortKey.m_Function->m_funcID){
          case FOREACH:{
            vector<ExpressionC> vExpandedExpr;
            if (m_groups.size()>0)
              keyProp.sortKey.m_Function->expandForeach(vExpandedExpr, m_groups);
            else
              keyProp.sortKey.m_Function->expandForeach(vExpandedExpr, m_fieldtypes.size());
            SortProp keyPropE;
            for (size_t j=0; j<vExpandedExpr.size(); j++){
              keyPropE = SortProp();
              split(vKP, vExpandedExpr[j].getEntireExpstr(),' ',"''()",'\\',{'(',')'},false,true);
              trace(DEBUG, "Splited from expanded expression '%s' to '%s'(%d)\n",vExpandedExpr[j].getEntireExpstr().c_str(),vKP[0].c_str(),vKP.size());
              if (vKP.size()<=1 || upper_copy(trim_copy(vKP[1])).compare("DESC")!=0)
                keyPropE.direction = ASC;
              else
                keyPropE.direction = DESC;
              keyPropE.sortKey.setExpstr(trim_copy(vKP[0]));

              checkSortGroupConflict(keyPropE.sortKey);
              keyPropE.sortKey.getAggFuncs(m_initAggProps);
              keyPropE.sortKey.getAnaFuncs(m_initAnaArray, m_anaFuncParaNums);
              if (keyPropE.sortKey.m_type==BRANCH || keyPropE.sortKey.m_expType != CONST || (!isInt(keyPropE.sortKey.m_expStr) && !isLong(keyPropE.sortKey.m_expStr)) || atoi(keyPropE.sortKey.m_expStr.c_str())>=m_selections.size())
                m_sorts.push_back(keyPropE);
            }
            m_bToAnalyzeSortMacro = false;
          }
        }
        continue;
      }
    }
    checkSortGroupConflict(keyProp.sortKey);
    keyProp.sortKey.getAggFuncs(m_initAggProps);
    keyProp.sortKey.getAnaFuncs(m_initAnaArray, m_anaFuncParaNums);
    // discard non integer CONST
    // Any INTEGER number will be mapped to the correspond sequence of the selections.
    // Thus, m_selections should always be analyzed before m_sorts
    if (keyProp.sortKey.m_type==BRANCH || keyProp.sortKey.m_expType != CONST || isInt(keyProp.sortKey.m_expStr) || isLong(keyProp.sortKey.m_expStr) || atoi(keyProp.sortKey.m_expStr.c_str())>=m_selections.size()){
      m_sorts.push_back(keyProp);
    }
  }
  for (size_t i=0; i<m_sorts.size(); i++)
    trace(DEBUG, "Sorting key '%s'(%d) !\n",m_sorts[i].sortKey.getEntireExpstr().c_str(),i);
  //trace(DEBUG1, "Got %d sorting keys!\n",m_sorts.size());
  return true;
}

bool QuerierC::assignMeanwhileString(const string & mwstr)
{
  vector<string> vSideWorks;
  split(vSideWorks, mwstr,';',"''()",'\\',{'(',')'},false,true);
  for (size_t i=0; i<vSideWorks.size(); i++){
    size_t pos = findFirstSub(vSideWorks[i], " WHERE ", 0,"''()",'\\',{'(',')'},false );
    FilterC filter;
    vector<ExpressionC> vSelections;
    vector<string> vAlias;
    if (pos != string::npos){
      filter.setExpstr(trim_copy(vSideWorks[i].substr(pos+string(" WHERE ").length())));
      genSelExpression(vSelections, trim_copy(vSideWorks[i].substr(0,pos)), vAlias);
    }else{
      filter.setExpstr("1=1");
      genSelExpression(vSelections, trim_copy(vSideWorks[i]), vAlias);
    }
    m_sideFilters.push_back(filter);
    m_sideSelections.push_back(vSelections);
    m_sideAlias.push_back(vAlias);
  }
  return true;
}

bool QuerierC::setFieldTypeFromStr(const string & setstr)
{
  if (setstr.empty())
    return true;
  vector<string> vSetFields;
  split(vSetFields, setstr,',',"''()",'\\',{'(',')'},false,true);
  string fieldname = "";
  for (size_t i=0; i<vSetFields.size(); i++){
    vector<string> vField;
    split(vField, vSetFields[i],' ',"''()",'\\',{'(',')'},false,true);
    if (vField.size()<2)
      split(vField, vSetFields[i],'\t',"''()",'\\',{'(',')'},false,true);
    if (vField[0].empty()){
      trace(FATAL, "Field name is empty!\n");
      return false;
    }
    if (vField.size()<2){
      trace(FATAL, "SET field type failed! Correct format is SET <FIELD> <TYPE>!\n");
      return false;
    }
    int iType = encodeDatatype(vField[1]);
    fieldname = vField[0];
    if (vField[0][0]=='@' && vField[0].length()>1 && isInt(vField[0].substr(1))) // convert filed abbrevasion
      fieldname = "@FIELD"+vField[0].substr(1);
    if (iType == UNKNOWN){
      trace(FATAL, "Unknown data type %s!\n", vField[1].c_str());
      return false;
    }else if(iType == DATE && vField.size() >= 2){
      trace(DEBUG, "SET field '%s' type '%s' extrainfor '%s'!\n",fieldname.c_str(),decodeDatatype(iType).c_str(),trim_pair(vField[2],"''").c_str());
      setFieldDatatype(fieldname, iType, trim_pair(vField[2],"''"));
    }else{
      trace(DEBUG, "SET field '%s' data type '%s'!\n", fieldname.c_str(), vField[1].c_str());
      setFieldDatatype(fieldname, iType);
    }
  }
  return true;
}

void QuerierC::setUserVars(const string & variables)
{
  if (variables.empty())
    return;
  trace(DEBUG, "Setting variables from '%s' !\n", variables.c_str());
  m_uservarstr = variables;
  //m_uservariables.clear();
  //m_uservarexprs.clear();
  m_uservarnames.clear();
  int fakeId = 1;
  vector<ExpressionC> fakeSels;
  vector<string> fakeAlias;
  vector<ExpressionC> fakeExprs;
  vector<string> vVariables;
  split(vVariables, variables,';',"''()",'\\',{'(',')'},false,true);
  for (size_t i=0; i<vVariables.size(); i++){
    vector<string> vNameVal;
    split(vNameVal, trim_copy(vVariables[i]),':',"''()",'\\',{'(',')'},false,true);
    if (vNameVal.size()<2){
      trace(FATAL, "Incorrect variable format!\n", vVariables[i].c_str());
      continue;
    }
    string sName=upper_copy(trim_copy(vNameVal[0])), sValue=trim_copy(vNameVal[1]);
    trace(DEBUG, "Setting variable '%s' value '%s'!\n", sName.c_str(), sValue.c_str());
    // name cannot be a keyword used by the predefined variables
    if (sName.compare("RAW")==0 || sName.compare("ROW")==0 || sName.compare("FILE")==0 || sName.compare("LINE")==0 || sName.compare("FILEID")==0 || sName.compare("FILELINE")==0 || sName.compare("N")==0 || sName.compare("%")==0 || sName.compare("LEVEL")==0 || sName.compare("NODEID")==0 || sName.compare("ROOT")==0 || sName.compare("PATH")==0){
      trace(FATAL, "%s is a reserved word, cannot be used as a variable name!\n", sName.c_str());
      continue;
    }
    // fake R variable as a sidework dataset. R definition: R:Element_Expression[:filter], R dont need an initial value.
    string sVName = "@"+sName;
    if (sName.compare("R")==0){
      m_uservarnames.push_back(sVName);
      fakeSels.push_back(intToStr(fakeId));
      fakeAlias.push_back("");
      if (m_sideFilters.size()==0){
        FilterC filter;
        if (vNameVal.size()>2) // a filter can be defined for the fake sidework dataset
          filter.setExpstr(trim_copy(vNameVal[2]));
        else
          filter.setExpstr("1=1");
        m_sideFilters.push_back(filter);
      }else
        trace(WARNING, "The filter has been defined in the first dynamic variable @R declaration, '%s' will be skipped!\n", vNameVal[3].c_str());

      fakeId++;
      //ExpressionC tmpExp;
      //if (vNameVal.size()>1)
      //  tmpExp = ExpressionC(trim_copy(vNameVal[1]));
      //else
      //  tmpExp = ExpressionC("''");
      //fakeExprs.push_back(tmpExp);
      if (vNameVal.size()>1)
        fakeExprs.push_back(ExpressionC(trim_copy(vNameVal[1])));
      else
        fakeExprs.push_back(ExpressionC("''"));
      continue;
    }
    bool bGlobal=false;
    if (vNameVal.size()>3) // whether dynamic variable is a global variable for multiple files
      bGlobal = (upper_copy(trim_copy(vNameVal[3])).compare("G")==0);
    if (m_uservariables.find(sVName) != m_uservariables.end()){
      if (!bGlobal) // reset value for local variables
        m_uservariables[sVName] = sValue;
    }else{
      m_uservariables.insert(pair<string, string> (sVName,sValue));
      m_uservarinitval.insert(pair<string, string> (sVName,sValue));
    }
    m_uservarnames.push_back(sVName);
    if (vNameVal.size()>2 && m_uservarexprs.find(sVName) == m_uservarexprs.end()){ // dynamic variable has an expression
      //ExpressionC tmpExp = ExpressionC(trim_copy(vNameVal[2]));
      //m_uservarexprs.insert(pair<string, ExpressionC> (sVName,tmpExp));
      m_uservarexprs.insert(pair<string, ExpressionC> (sVName,ExpressionC(trim_copy(vNameVal[2]))));
    }
  }
  // set m_sideSelections to get side data types in trialAnalyze()
  if (fakeSels.size()>0 && fakeAlias.size()>0){
    if (m_sideSelections.size()>0){
      m_sideSelections[0] = fakeSels;
      m_sideAlias[0] = fakeAlias;
    }else{
      m_sideSelections.push_back(fakeSels);
      m_sideAlias.push_back(fakeAlias);
    }
  }
  m_fakeRExprs = fakeExprs;
}

std::set<string> g_userMacroFuncNames; // the global array is used for identify the user defined macro functions

void QuerierC::setUserMaroFuncs(const string & macrostr)
{
  if (macrostr.empty())
    return;
  g_userMacroFuncNames.clear();
  trace(DEBUG, "Setting macro functions from '%s' !\n", macrostr.c_str());
  m_usermacrostr = macrostr;
  vector<string> vMacroraws;
  split(vMacroraws, macrostr,';',"''()",'\\',{'(',')'},false,true);
  for (size_t i=0; i<vMacroraws.size(); i++){
    vector<string> vNameVal;
    split(vNameVal, trim_copy(vMacroraws[i]),':',"''()",'\\',{'(',')'},false,true);
    if (vNameVal.size()<2){
      trace(FATAL, "Incorrect macro function format!\n", vMacroraws[i].c_str());
      continue;
    }
    string sFuncName = upper_copy(trim_copy(vNameVal[0]));
    // check name used
    if (encodeFunction(sFuncName) != UNKNOWN){
      trace(FATAL, "You cannot use '%s' as the macro function name, it has been used by an inline function!\n", vNameVal[0].c_str());
      continue;
    }
    if (m_userMacroExprs.find(sFuncName) != m_userMacroExprs.end()){
      trace(FATAL, "You cannot use '%s' as the macro function name, it has been used by another macro function!\n", vNameVal[0].c_str());
      continue;
    }
    string expStr = trim_copy(vNameVal[1]);
    MacroFuncStruct mfs;
    mfs.sFuncName = sFuncName;
    mfs.funcExpr = new ExpressionC(expStr);
    size_t pos=0;
    while ((int)pos>=0){ // search the parameters
      string sParastr = trim_copy(readQuotedStr(expStr, pos, "~~", "''", '\0', {}));
      if (sParastr.empty() || pos<0)
        break;
      vector<string> vMacroNamVal;
      split(vMacroNamVal, sParastr,'=',"''()",'\\',{'(',')'},false,true);
      if (vMacroNamVal.size()==0 || trim_copy(vMacroNamVal[0]).empty()){
        trace(FATAL, "(1) Macro function parameter '%s' has an invalid definition! \n", sParastr.c_str());
        return;
      }
      string paraName = upper_copy(trim_copy(vMacroNamVal[0]));
      if (find(mfs.vParaNames.begin(), mfs.vParaNames.end(), paraName) == mfs.vParaNames.end())
        mfs.vParaNames.push_back(paraName);
    }
    m_userMacroExprs.insert(pair< string,MacroFuncStruct >(sFuncName,mfs));
    g_userMacroFuncNames.insert(sFuncName);
  }
}

void QuerierC::setFileName(const string & filename, const bool & bfileheaderonly)
{
  m_filename = filename;
  m_fileid++;
  m_fileline = 0;
  if (!bfileheaderonly){
    m_fieldnames.clear();
    m_fieldtypes.clear();
    m_detectedTypeRows = 0;  // force to detect data type again
  }
  setUserVars(m_uservarstr); // reset user defined variables for each file.
}

void QuerierC::setOutputFormat(const short int & format)
{
  m_outputformat = format;
}

void QuerierC::setFieldDelim(const string & delimstr)
{
  m_fielddelim = delimstr;
  replacestr(m_fielddelim,{"\\\\","\\t","\\v","\\n","\\r"},{"\\","\t","\v","\n","\r"});
}

void QuerierC::setDetectAllOnChange(const bool & bDetectAllOnChange)
{
  m_bDetectAllOnChange = bDetectAllOnChange;
}

void QuerierC::setTextonly(const bool & bTextonly)
{
  m_bTextOnly = bTextonly;
}

void QuerierC::setStatonly(const bool & bStatonly)
{
  m_bStatOnly = bStatonly;
}

void QuerierC::setOutputFiles(const string & outputfilestr, const short int & outputmode)
{
  if (m_outputfileexp)
    SafeDelete(m_outputfileexp);
  m_outputfileexp = new ExpressionC(outputfilestr);
  m_outputfileexp->getAggFuncs(m_initAggProps);
  m_outputmode = outputmode;
}

bool QuerierC::toGroupOrSort()
{
  return (m_groups.size()>0 || m_sorts.size()>0 || m_initAnaArray.size()>0 || m_aggrOnly || m_bUniqueResult || (m_treeProps.size()>0 && m_treeProps.size()==m_treeParentProps.size()));
}

void QuerierC::pairFiledNames(const namesaving_smatch & matches)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 

  for (size_t i=1; i<matches.size(); i++){
    bool foundName = false;
    for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      if (&(matches[i]) == &(matches[*it])){
        m_fieldnames.push_back(upper_copy(string(*it)));
        m_fieldInitNames.push_back(upper_copy(string(*it)));
        foundName = true;
      }
    if (!foundName){
      m_fieldnames.push_back("@FIELD"+intToStr(i));
      m_fieldInitNames.push_back("@FIELD"+intToStr(i));
    }
  }
}

void QuerierC::setFieldDatatype(const string & field, const int & datetype, const string & extrainfo)
{
  string fname = upper_copy(field);
  DataTypeStruct dts;
  dts.datatype = datetype;
  dts.extrainfo = extrainfo;
  if (m_fieldntypes.find(fname) != m_fieldntypes.end())
    m_fieldntypes[fname] = dts;
  else
    m_fieldntypes.insert( pair<string, DataTypeStruct>(fname,dts) );
  for (size_t i=0;i<m_fieldnames.size();i++){
    if (fname.compare(m_fieldnames[i]) == 0 && i<m_fieldtypes.size())
      m_fieldtypes[i] = dts;
  }
}

void QuerierC::analyzeFiledTypes(const vector<string> & matches)
{
  m_detectedTypeRows++;
  //m_fieldtypes.clear();
  DataTypeStruct dts;
  if (m_bTextOnly){
    m_rawDatatype.datatype = STRING;
  }else{
    dts.datatype = detectDataType(matches[0], dts.extrainfo);
    dts.datatype = dts.datatype==UNKNOWN?STRING:dts.datatype;
    if (m_rawDatatype.datatype == UNKNOWN)
      m_rawDatatype = dts;
    else
      m_rawDatatype = getCompatibleDataType(m_rawDatatype, dts);
    trace(DEBUG,"Detected @RAW '%s', data type '%s' extrainfo '%s'.\n",string(matches[0]).c_str(),decodeDatatype(m_rawDatatype.datatype).c_str(),m_rawDatatype.extrainfo.c_str());
    if (m_rawDatatype.datatype == UNKNOWN)
      m_rawDatatype.datatype = STRING;
  }
  for (size_t i=(m_bDetectAllOnChange||m_detectedTypeRows<=m_detectTypeMaxRowNum)?1:m_fieldtypes.size()+1; i<matches.size(); i++){
    dts = DataTypeStruct();
    if (m_detectedTypeRows>m_detectTypeMaxRowNum && m_fieldnames.size()>i-1 && m_fieldntypes.find(m_fieldnames[i-1]) != m_fieldntypes.end()){
      if (m_fieldtypes.size()>i-1)
        m_fieldtypes[i-1] = m_fieldntypes[m_fieldnames[i-1]];
      else
        m_fieldtypes.push_back(m_fieldntypes[m_fieldnames[i-1]]);
    }else if (m_fieldntypes.find("@FIELD"+intToStr(i)) != m_fieldntypes.end()){
      if (m_fieldtypes.size()>i-1)
        m_fieldtypes[i-1] = m_fieldntypes["@FIELD"+intToStr(i)];
      else
        m_fieldtypes.push_back(m_fieldntypes["@FIELD"+intToStr(i)]);
    }else{
      if (m_bTextOnly){
        if (m_fieldtypes.size()>i-1)
          m_fieldtypes[i-1].datatype = STRING;
        else{
          dts.datatype = STRING;
          m_fieldtypes.push_back(dts);
        }
        if (m_fieldnames.size()>i-1 && m_detectedTypeRows>=m_detectTypeMaxRowNum)
          m_fieldntypes.insert( pair<string, DataTypeStruct>(m_fieldnames[i-1],m_fieldtypes[i-1]) );
      }else{
        dts.datatype = detectDataType(matches[i], dts.extrainfo);
        dts.datatype = dts.datatype==UNKNOWN?STRING:dts.datatype;
        // trace(DEBUG2, "Detected column '%s' from '%s', data type '%s' extrainfo '%s'\n", m_fieldnames[i-1].c_str(), string(matches[i]).c_str(), decodeDatatype(dts.datatype).c_str(), dts.extrainfo.c_str());
        // if the date type was detected previous, get the compitable data type.
        if (m_fieldtypes.size()>i-1)
          m_fieldtypes[i-1] = getCompatibleDataType(m_fieldtypes[i-1], dts);
        else
          m_fieldtypes.push_back(dts); 
        if (m_fieldtypes[i-1].datatype==UNKNOWN)
          m_fieldtypes[i-1].datatype = STRING; // set UNKNOWN (real) data as STRING
        if (m_fieldnames.size()>i-1 && m_detectedTypeRows>=m_detectTypeMaxRowNum)
          m_fieldntypes.insert( pair<string, DataTypeStruct>(m_fieldnames[i-1],m_fieldtypes[i-1]) );
      }
    }
    trace(DEBUG, "Detected column '%s' data type '%s' extrainfo '%s'\n", m_fieldnames[i-1].c_str(), decodeDatatype(m_fieldtypes[m_fieldtypes.size()-1].datatype).c_str(),m_fieldtypes[m_fieldtypes.size()-1].extrainfo.c_str());
  }
  if (m_bDetectAllOnChange && m_fieldtypes.size()>matches.size()-1) // removed the redundant field data types in case the number of field changed when m_bDetectAllOnChange is true.
    m_fieldtypes.erase(m_fieldtypes.begin()+matches.size()-1, m_fieldtypes.end());
}

void QuerierC::getSideDatarow(vector< unordered_map< int,int > > & sideMatchedRowIDs, vector< unordered_map< string, unordered_map<string,string> > > & matchedSideDatarows)
{
  matchedSideDatarows.clear();
  if (sideMatchedRowIDs.size() == 0){
    //trace(ERROR, "Side dataset vector size %d doesnot match side matched rowID vector size %d!\n", m_sideDatasets.size(), sideMatchedRowIDs.size());
    return;
  }
  for (size_t k=0; k<sideMatchedRowIDs.size(); k++){
    unordered_map< string, unordered_map<string,string> > matchedRow;
    for (size_t i=0; i<m_sideDatasets.size(); i++){
      //if (sideMatchedRowIDs[k].find(i) == sideMatchedRowIDs[k].end())
      //  continue;
      if (sideMatchedRowIDs[k][i] >= m_sideDatasets[i].size()){
        trace(ERROR, "%d is an invalid matched ID, side work(%d) data size is %d!\n", sideMatchedRowIDs[k][i], i, m_sideDatasets[i].size());
      }
      matchedRow.insert(pair<string, unordered_map<string,string> >(intToStr(i+1), m_sideDatasets[i][sideMatchedRowIDs[k][i]]));
    }
    matchedSideDatarows.push_back(matchedRow);
  }
}

void QuerierC::evalAggExpNode(vector<string>* fieldvalues, unordered_map<string,string>* varvalues, unordered_map< string,GroupProp > & aggGroupProp, unordered_map< string, unordered_map<string,string> > & matchedSideDatarow)
{
  for (unordered_map< string,GroupProp >::iterator it=aggGroupProp.begin(); it!=aggGroupProp.end(); ++it){
    string sResult;
    unordered_map< string,ExpressionC >::iterator ite=m_aggFuncExps.find(it->first);
    if (ite == m_aggFuncExps.end()){
      trace(ERROR,"No expression found for '%s'\n", it->first.c_str());
      continue;
    }
    DataTypeStruct dts;
    unordered_map< string,vector<string> > anaFuncData;
    RuntimeDataStruct rds;
    rds.fieldvalues = fieldvalues;
    rds.varvalues = varvalues;
    rds.aggFuncs = &aggGroupProp;
    rds.anaFuncs = &anaFuncData;
    rds.sideDatasets = &m_sideDatasets;
    rds.sideDatarow = &matchedSideDatarow;
    rds.sideDatatypes = &m_sideDatatypes;
    rds.macroFuncExprs = &m_userMacroExprs;
    //trace(DEBUG2, "Eval aggregation function expression '%s'\n", ite->second.getEntireExpstr().c_str());
    if (ite->second.m_expType==FUNCTION && ite->second.m_Function && ite->second.m_Function->isAggFunc() && ite->second.m_Function->m_params.size()>0 && ite->second.m_Function->m_params[0].evalExpression(rds, sResult, dts, true)){
      it->second.funcID = ite->second.m_funcID;
      if (!it->second.inited){
        switch(ite->second.m_funcID){
        case AVERAGE:
          if (isDouble(sResult))
            it->second.sum = atof(sResult.c_str());
          it->second.count = 1;
          break;
        case SUM:
          if (isDouble(sResult))
            it->second.sum = atof(sResult.c_str());
          break;
        case COUNT:
          it->second.count = 1;
          break;
        case GROUPLIST:
        case UNIQUECOUNT:
          //if (!it->second.uniquec)
          //  it->second.uniquec = new std::set <string>;
          //it->second.uniquec->insert(sResult);
          if (!it->second.varray)
            it->second.varray = new vector <string>;
          it->second.varray->push_back(sResult);
          break;
        case MAX:
          it->second.max = sResult;
          break;
        case MIN:
          it->second.min = sResult;
          break;
        }
        it->second.inited=true;
      }else{
        switch(ite->second.m_funcID){
        case AVERAGE:
          if (isDouble(sResult))
            it->second.sum += atof(sResult.c_str());
          it->second.count += 1;
          break;
        case SUM:
          if (isDouble(sResult))
            it->second.sum += atof(sResult.c_str());
          break;
        case COUNT:
          it->second.count += 1;
          //trace(DEBUG2," count increasing 1 => %d\n",it->second.count);
          break;
        case GROUPLIST:
        case UNIQUECOUNT:
          //if (!it->second.uniquec){
          if (!it->second.varray){
            trace(ERROR,"Aggregation uniquec is not initialized! \n");
          }else
            //it->second.uniquec->insert(sResult);
            it->second.varray->push_back(sResult);
          break;
        case MAX:
          //trace(DEBUG2,"Comparing '%s' : '%s' ... ",sResult.c_str(),it->second.max.c_str());
          if (anyDataCompare(sResult,it->second.max,dts,dts)>0)
            it->second.max = sResult;
          //trace(DEBUG2," get '%s'\n",it->second.max.c_str());
          break;
        case MIN:
          if (anyDataCompare(sResult,it->second.min,dts,dts)<0)
            it->second.min = sResult;
          break;
        }
      }
    }else{
      trace(ERROR, "Failed to eval aggregation parameter!\n");
      return;
    }
  }
}

void QuerierC::addResultOutputFileMap(vector<string>* fieldvalues, unordered_map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* matchedSideDatarow)
{
  if (m_outputfileexp){
    DataTypeStruct dts;
    string sResult;
    RuntimeDataStruct rds;
    rds.fieldvalues = fieldvalues;
    rds.varvalues = varvalues;
    rds.aggFuncs = aggFuncs;
    rds.anaFuncs = anaFuncs;
    rds.sideDatasets = &m_sideDatasets;
    rds.sideDatarow = matchedSideDatarow;
    rds.sideDatatypes = &m_sideDatatypes;
    rds.macroFuncExprs = &m_userMacroExprs;
    m_outputfileexp->evalExpression(rds, sResult, dts, true);
    unordered_map< string, ofstream* >::iterator it = m_outputfiles.find(sResult);
    if (it == m_outputfiles.end()){
      ofstream* ofs=new ofstream();
      ofs->open(sResult, m_outputmode==OVERWRITE? (std::ofstream::out | ofstream::trunc) : ofstream::app);
      m_outputfiles.insert(pair< string, ofstream* >(sResult, ofs));
      m_resultfiles.push_back(ofs);
    }else
      m_resultfiles.push_back(it->second);
    trace(DEBUG2, "Result ID: %d, ResultFIle ID: %d: File %s!\n", m_results.size()-1, m_resultfiles.size()-1, sResult.c_str());
  }
}

bool QuerierC::appendResultSet(const vector<string> & vResult, const unordered_map<string, string> & varValues, const bool & bApplyExtraFilter)
{
  int iRowNumFromCols = 0;
  for (size_t i=0; i<m_colToRows.size(); i++)
    iRowNumFromCols = max(iRowNumFromCols, (int)m_colToRows[i].size());
  if (iRowNumFromCols>0){ // convert cols to rows
    size_t k=0;
    vector<string> datas;
    vector< vector<string> > tmpResult;
    for (size_t j=1; j<vResult.size(); j++){
      if(k<m_colToRows.size() && j-1>=m_colToRows[k][0] && j-1<=m_colToRows[k][m_colToRows[k].size()-1]){ // the selection is a part of a COLTOROW function
        datas.push_back(vResult[j]);
        if (j-1==m_colToRows[k][m_colToRows[k].size()-1] || j-1==vResult.size()-1){ // if current selection is the last one, complement empty strings
          datas.insert(datas.end(),iRowNumFromCols-(int)datas.size(),"");
          tmpResult.push_back(datas);
          datas.clear();
          k++;
        }
      }else{ // the selection is not a part of any COLTOROW function
        datas.push_back(vResult[j]);
        datas.insert(datas.end(),iRowNumFromCols-1,"");
        tmpResult.push_back(datas);
        datas.clear();
      }
    }
    for (size_t j=0;j<iRowNumFromCols;j++){
      datas.clear();
      datas.push_back("");
      for (size_t k=0;k<tmpResult.size();k++)
        datas.push_back(tmpResult[k][j]);
      if (bApplyExtraFilter)
        return applyExtraFilter(datas, varValues);
      else{
        m_results.push_back(datas);
        return true;
      }
    }
  }else{
    if (bApplyExtraFilter)
      return applyExtraFilter(vResult, varValues);
    else{
      m_results.push_back(vResult);
      return true;
    }
  }
  return false;
}

bool QuerierC::evalAddExprArray(vector<ExpressionC> & expressions, const string & rawval, RuntimeDataStruct & rds, vector<ExpressionC>* anaEvaledExp, vector<string> & vResults, const bool & bCheckGroupFunc)
{
  if (!anaEvaledExp)
    return false;
#ifdef __DEBUG__
long int thistime = curtime();
#endif // __DEBUG__
  static DataTypeStruct dts;
  static string sResult;
  vResults.clear();
  vResults.push_back(rawval); 
#ifdef __DEBUG__
  m_evalSeldeclaretime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
  for (size_t i=0; i<expressions.size(); i++){
    dts = DataTypeStruct();
    //tmpExp = ExpressionC();
#ifdef __DEBUG__
  m_evalSelAssignExprtime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    if (!bCheckGroupFunc || !expressions[i].containGroupFunc()){
#ifdef __DEBUG__
  m_evalSelAssignExprtime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
      //expressions[i].copyTo(&tmpExp);
      //trace(DEBUG, "evalAddExprArray: Before eval expression '%s':\n",tmpExp.getEntireExpstr().c_str());
      //tmpExp.dump();
      //expressions[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, &m_sideDatasets, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, true);
      if (expressions[i].containAnaFunc()) {// no actual result for analytic function yet. Keep the evaled expression
        //ExpressionC tmpExp;
        //tmpExp = expressions[i];
        //tmpExp.evalExpression(rds, sResult, dts, false);
        //anaEvaledExp->push_back(tmpExp);
        anaEvaledExp->push_back(expressions[i]);
        (*anaEvaledExp)[anaEvaledExp->size()-1].evalExpression(rds, sResult, dts, false);
        trace(DEBUG, "evalAddExprArray: adding analytic function involved expression '%s':\n",expressions[i].getEntireExpstr().c_str());
      }else
        expressions[i].evalExpression(rds, sResult, dts, true);
      //trace(DEBUG, "evalAddExprArray: After eval expression '%s':\n",tmpExp.getEntireExpstr().c_str());
      //tmpExp.dump();
      //trace(DEBUG, "eval '%s' => '%s'\n", expressions[i].getEntireExpstr().c_str(), sResult.c_str());
#ifdef __DEBUG__
  m_evalSelRealEvaltime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    }else{
      trace(ERROR, "(2)Invalid using aggregation function in '%s', no group involved!\n", expressions[i].getEntireExpstr().c_str());
      return false;
    }
    vResults.push_back(sResult);
  }
  return true;
}

// add sort keys. result set size increased means the current row is not filtered by extra filter.
void QuerierC::evalAddSortkeys(RuntimeDataStruct & rds, vector<ExpressionC>* anaEvaledExp, const bool & bCheckGroupFunc)
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  if (!anaEvaledExp)
    return;
  vector<string> vResults;
  static string sResult;
  static DataTypeStruct dts;
  for (size_t i=0; i<m_sorts.size(); i++){
    if (!bCheckGroupFunc || !m_sorts[i].sortKey.containGroupFunc()){
      //if it has the exact same expression as any selection, get the result from selection
      int iSel = -1;
      for (size_t j=0; j<m_selections.size(); j++)
        if (m_selections[j].getEntireExpstr().compare(m_sorts[i].sortKey.getEntireExpstr())==0){
          iSel = j;
          break;
        }
      if (iSel >= 0){
        sResult = m_results[m_results.size()-1][iSel + 1];
        if (m_selections[iSel].containAnaFunc()){ // add a evaled expression from the mapped selections for analytic function involved expression
          //ExpressionC tmpExp;
          //tmpExp = m_selections[iSel];
          //tmpExp.evalExpression(rds, sResult, dts, false);
          //anaEvaledExp->push_back(tmpExp);
          anaEvaledExp->push_back(m_selections[iSel]);
        }
      // if the sort key is a integer, get the result from the result set at the same sequence number
      //}else if (m_sorts[i].sortKey.m_type==LEAF && m_sorts[i].sortKey.m_expType==CONST && isInt(m_sorts[i].sortKey.m_expStr) && atoi(m_sorts[i].sortKey.m_expStr.c_str())<=m_selections.size()){
      }else if (m_sorts[i].iSel>=0){
        //int iSel = atoi(m_sorts[i].sortKey.m_expStr.c_str())-1;
        sResult = m_results[m_results.size()-1][m_sorts[i].iSel + 1];
        if (m_selections[m_sorts[i].iSel].containAnaFunc()){ // add a evaled expression from the mapped selections for analytic function involved expression
          //ExpressionC tmpExp;
          //tmpExp = m_selections[m_sorts[i].iSel];
          //tmpExp.evalExpression(rds, sResult, dts, false);
          //anaEvaledExp->push_back(tmpExp);
          anaEvaledExp->push_back(m_selections[m_sorts[i].iSel]);
          (*anaEvaledExp)[anaEvaledExp->size()-1].evalExpression(rds, sResult, dts, false);
        }
      }else{
       // m_sorts[i].sortKey.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, true);
        if (m_sorts[i].sortKey.containAnaFunc()) {// no actual result for analytic function yet. Keep the evaled expression
          //ExpressionC tmpExp;
          //tmpExp.evalExpression(rds, sResult, dts, false);
          //anaEvaledExp->push_back(tmpExp);
          anaEvaledExp->push_back(ExpressionC());
          (*anaEvaledExp)[anaEvaledExp->size()-1].evalExpression(rds, sResult, dts, false);
        }else
          m_sorts[i].sortKey.evalExpression(rds, sResult, dts, true);
      }
      //trace(DEBUG, "eval '%s' => '%s'\n", m_sorts[i].sortKey.getEntireExpstr().c_str(), sResult.c_str());
    }else{
      trace(ERROR, "(3)Invalid using aggregation function in '%s', no group involved!\n", m_sorts[i].sortKey.getEntireExpstr().c_str());
      return;
    }
    vResults.push_back(sResult);
  }
  //vResults.push_back(intToStr(m_sortKeys.size())); // add an index for the sort key
  m_sortKeys.push_back(vResults);
#ifdef __DEBUG__
  m_evalSorttime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
}

void QuerierC::evalDupSelAndSortkeys(const string & rawval, RuntimeDataStruct & rds, vector<string>* groupedfieldvalues, unordered_map< string,GroupProp >* calculatedAggFuncs, vector< unordered_map< string, unordered_map<string,string> > > * matchedSideDatarows, const bool & bApplyExtraFilter, const bool & bCheckGroupFunc, const int & nDupNum)
{
#ifdef __DEBUG__
long int thistime = curtime();
#endif // __DEBUG__
  static string sResult;
  static DataTypeStruct dts;
  static vector<string> vResults;
  // make duplicate records
  for (size_t i=0; i<nDupNum; i++){
    vector<ExpressionC> anaEvaledExp;
    (*rds.varvalues)["@DUPID"]=intToStr(i+1);
    rds.anaFuncs->clear();
    vector<string> vEvaledParams;
    for (unordered_map< string,vector<ExpressionC> >::iterator it=m_initAnaArray.begin(); it!=m_initAnaArray.end(); ++it){
      vEvaledParams.clear();
      rds.anaFuncs->insert(pair< string,vector<string> >(it->first,vEvaledParams));
    }
#ifdef __DEBUG__
m_evalRawpreptime += (curtime()-thistime);
thistime = curtime();
#endif // __DEBUG__
    evalAddExprArray(m_selections, rawval, rds, &anaEvaledExp, vResults, bCheckGroupFunc);
#ifdef __DEBUG__
m_evalSeltime += (curtime()-thistime);
thistime = curtime();
#endif // __DEBUG__
    if (appendResultSet(vResults, (*rds.varvalues), bApplyExtraFilter)){ // appendResultSet return false mean filtered by extra filter
#ifdef __DEBUG__
m_appendSelToResulttime += (curtime()-thistime);
thistime = curtime();
#endif // __DEBUG__
      evalAddSortkeys(rds, &anaEvaledExp, bCheckGroupFunc);
#ifdef __DEBUG__
thistime = curtime();
#endif // __DEBUG__
      // add output file mapping with result.
      if (m_outputmode != STANDARD && m_outputfileexp)
        addResultOutputFileMap(groupedfieldvalues, rds.varvalues, calculatedAggFuncs, rds.anaFuncs, rds.sideDatarow);
#ifdef __DEBUG__
m_assignoutputfiletime += (curtime()-thistime);
thistime = curtime();
#endif // __DEBUG__
      // eval filter expressions to get anaFuncData
      if (m_filter && m_filter->containAnaFunc())
        m_filter->evalAnaExprs(rds, &anaEvaledExp, sResult, dts, true);
#ifdef __DEBUG__
m_evalanaexprtime += (curtime()-thistime);
thistime = curtime();
#endif // __DEBUG__
      if (anaEvaledExp.size()>0){
        m_anaEvaledExp.push_back(anaEvaledExp);
        addAnaFuncData(*rds.anaFuncs);
      }
#ifdef __DEBUG__
m_addanafuncdatatime += (curtime()-thistime);
thistime = curtime();
#endif // __DEBUG__
    }
  }
}

void QuerierC::evalAddSelAndSortkeys(const string & rawval, RuntimeDataStruct & rds, vector<string>* groupedfieldvalues, unordered_map< string,GroupProp >* calculatedAggFuncs, vector< unordered_map< string, unordered_map<string,string> > > * matchedSideDatarows, const bool & bApplyExtraFilter, const bool & bCheckGroupFunc)
{
#ifdef __DEBUG__
long int thistime = curtime();
#endif // __DEBUG__
  static string sResult;
  static DataTypeStruct dts;
  static vector< unordered_map< int,int > > sideMatchedRowIDs;
  static int nDupNum = 1;
  if (m_dupnumexp && m_dupnumexp->evalExpression(rds, sResult, dts, true) && isInt(sResult) && (!m_dupfilter || m_dupfilter->compareExpression(rds, sideMatchedRowIDs)))
    nDupNum = atoi(sResult.c_str()); 
  if (matchedSideDatarows && matchedSideDatarows->size()>0){
    unordered_map< string, unordered_map<string,string> >* oldsideDatarow = rds.sideDatarow;
    for (size_t i=0; i<matchedSideDatarows->size(); i++){
      rds.sideDatarow = &((*matchedSideDatarows)[i]);
#ifdef __DEBUG__
m_evalRawdupjointime += (curtime()-thistime);
thistime = curtime();
#endif // __DEBUG__
      evalDupSelAndSortkeys(rawval, rds, groupedfieldvalues, calculatedAggFuncs, matchedSideDatarows, bApplyExtraFilter, bCheckGroupFunc, nDupNum);
#ifdef __DEBUG__
thistime = curtime();
#endif // __DEBUG__
    }
    rds.sideDatarow = oldsideDatarow;
  }else{
#ifdef __DEBUG__
m_evalRawdupjointime += (curtime()-thistime);
thistime = curtime();
#endif // __DEBUG__
    evalDupSelAndSortkeys(rawval, rds, groupedfieldvalues, calculatedAggFuncs, matchedSideDatarows, bApplyExtraFilter, bCheckGroupFunc, nDupNum);
  }
}

// add evaled analytic function processing data to m_anaSortProps&m_anaSortData&m_anaFuncResult
void QuerierC::addAnaFuncData(unordered_map< string,vector<string> > & anaFuncData)
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  unordered_map< string,string > anaFuncResult;
  // convert the stored analytic data to this format: <analytic_func_str:vector of analytic data> 
  vector<string> sortKey;
  vector< vector<string> > newData;
  vector<SortProp> sortProps;
  for (unordered_map< string,vector<string> >::iterator it=anaFuncData.begin(); it!=anaFuncData.end(); it++){
    sortKey.clear();
    size_t iAnaGroupNum = 0, iAnaSortNum = 0;
    unordered_map< string,vector<int> >::iterator itn = m_anaFuncParaNums.find(it->first);
    if (itn == m_anaFuncParaNums.end())
      trace(ERROR, "(1)Failed to find analytic function parameter numbers '%s'\n", it->first.c_str());
    else{
      if (itn->second.size() < 2)
        trace(ERROR, "(1)The analytic function parameter numbers size %d is smaller than 2\n", it->first.c_str(), itn->second.size());
      iAnaGroupNum = itn->second[0];
      iAnaSortNum = itn->second[1];
    }
    if (m_anaSortData.find(it->first) != m_anaSortData.end()){
      //group1,group2...,sort1[,sort_direction1(1|-1)][,sort2,[,sort_direction2(1|-1)]...], group number
      for(size_t j=0;j<it->second.size();j++)
        if (j<iAnaGroupNum || j>=iAnaGroupNum+iAnaSortNum*2) // all parts except the second part of analytic function parameters
          sortKey.push_back(it->second[j]);
        else{ // second part of analytic function parameters, which are always sort keys for all analytic functions
          if ((j-iAnaGroupNum)%2==0)
            sortKey.push_back(it->second[j]);
        }
      trace(DEBUG, "addAnaFuncData: Added %d sort keys. Raw data size: %d\n", sortKey.size(),it->second.size());
      m_anaSortData[it->first].push_back(sortKey);
    }else{
      newData.clear();
      sortProps.clear();
      FunctionC* anaFunc;
      SortProp sp;
      //Add sort props, the second part of parameters of analytic functions is the sort keys.
      for(size_t j=0;j<it->second.size();j++){
        if (j<iAnaGroupNum || j>=iAnaGroupNum+iAnaSortNum*2) // all parts except the second part of analytic function parameters
          sortKey.push_back(it->second[j]);
        else { // second part of analytic function parameters, which are always sort keys for all analytic functions
          if ((j-iAnaGroupNum)%2==0)
            sortKey.push_back(it->second[j]);
          else{
            anaFunc = NULL;
            for (size_t i=0; i<m_anaEvaledExp[0].size();i++){
              //trace(DEBUG,"Searching analytic function '%s' from '%s'\n",it->first.c_str(),m_anaEvaledExp[0][i].getEntireExpstr().c_str());
              //m_anaEvaledExp[0][i].dump();
              anaFunc = m_anaEvaledExp[0][i].getAnaFunc(it->first);
              if (anaFunc)
                break;
            }
            if (!anaFunc){
              trace(ERROR, "(1)Failed to find analytic function expression '%s'\n", it->first.c_str());
              sp.sortKey = ExpressionC();
            }else{
              if (anaFunc->m_params.size() != it->second.size())
                trace(ERROR, "(1)Analytic function '%s' sortkey size %d doesnot match the function parameter size %d!\n", it->first.c_str(), it->second.size(), anaFunc->m_params.size());
              sp.sortKey = anaFunc->m_params[j-1];
            }
            sp.direction = isInt(it->second[j])&&atoi(it->second[j].c_str())<0?DESC:ASC;
            sortProps.push_back(sp);
          }
        }
      }
      trace(DEBUG, "addAnaFuncData: Added %d sort keys. Raw data size: %d\n", sortKey.size(),it->second.size());
      trace(DEBUG, "addAnaFuncData: Added %d sort props. Raw data size: %d\n", sortProps.size(),it->second.size());
      //dumpVector(it->second);
      m_anaSortProps.insert(pair<string, vector<SortProp> >(it->first, sortProps) );
      newData.push_back(sortKey);
      m_anaSortData.insert(pair<string, vector< vector<string> > >(it->first, newData) );
    }
    anaFuncResult.insert(pair<string,string>(it->first,""));
  }
  if (anaFuncResult.size()>0)
    m_anaFuncResult.push_back(anaFuncResult);
#ifdef __DEBUG__
  m_addanafuncdatatime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
}

void QuerierC::doSideWorks(vector<string> * pfieldValues, unordered_map<string, string> * pvarValues, unordered_map< string,GroupProp > * paggGroupProp, unordered_map< string,vector<string> > * panaFuncData)
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  int nRealSWStart = m_fakeRExprs.size()>0?1:0; // if user defined R involved, the real side work data set start from 1, otherwise from 0.
  if (m_sideSelections.size()>nRealSWStart && m_sideSelections.size()==m_sideAlias.size() && m_sideSelections.size()==m_sideFilters.size()){
    vector< unordered_map< int,int > > sideMatchedRowIDs;
    vector< unordered_map< string, unordered_map<string,string> > > matchedSideDatarows;
    unordered_map< string, unordered_map<string,string> > matchedSideDatarow;
    RuntimeDataStruct rds;
    rds.fieldvalues = pfieldValues;
    rds.varvalues = pvarValues;
    rds.aggFuncs = paggGroupProp;
    rds.anaFuncs = panaFuncData;
    rds.sideDatasets = &m_sideDatasets;
    rds.sideDatarow = &matchedSideDatarow;
    rds.sideDatatypes = &m_sideDatatypes;
    rds.macroFuncExprs = &m_userMacroExprs;
    for(size_t i=nRealSWStart;i<m_sideSelections.size();i++){
      vector< unordered_map<string,string> > resultSet;
      sideMatchedRowIDs.clear();
      if (m_sideFilters[i].compareExpression(rds, sideMatchedRowIDs)){
        getSideDatarow(sideMatchedRowIDs, matchedSideDatarows);
        unordered_map<string,string> thisResult;
        for (size_t j=0; j<m_sideSelections[i].size(); j++){
          string sResult;
          DataTypeStruct dts;
          m_sideSelections[i][j].evalExpression(rds, sResult, dts, true);
          thisResult.insert(pair<string,string>(m_sideAlias[i][j].empty()?intToStr(j+1):m_sideAlias[i][j], sResult));
        }
        if (m_sideDatasets.size()<=i){ // first row of this sidework dataset
          resultSet.push_back(thisResult);
          m_sideDatasets.push_back(resultSet);
        }else
          m_sideDatasets[i].push_back(thisResult);
      }
    }
  }
#ifdef __DEBUG__
  m_sideworktime += (curtime()-thistime);
#endif // __DEBUG__
}

// filt a row data by filter. no predication mean true. comparasion failed means alway false
bool QuerierC::matchFilter(vector<string> & rowValue)
{
  //if (!filter){
  //  //trace(INFO, "No filter defined\n");
  //  return true;
  //}
#ifdef __DEBUG__
  long int thistime = curtime();
  long int filterbegintime = thistime;
#endif // __DEBUG__
  if (rowValue.size() > m_fieldnames.size() + 1){ // field name number + 1 variables (@raw )
    trace(ERROR, "Filed number %d and value number %d dont match!\n", m_fieldnames.size(), rowValue.size());
    dumpVector(m_fieldnames);
    dumpVector(rowValue);
    return false;
  }
  // add @line @row @fileline
  vector<string> fieldValues(rowValue.begin()+1,rowValue.end());
  //unordered_map<string, string> varValues;
  unordered_map<string, string> varValues{ 
    {"@RAW",rowValue[0]},
    {"@FILE",m_filename},
    {"@FILEID",intToStr(m_fileid)},
    {"@LINE",intToStr(m_line)},
    {"@ROW",intToStr(m_matchcount+1)},
    {"@FILELINE",intToStr(m_fileline)},
    {"@%",intToStr(fieldValues.size())},
    {"@DUPID","1"} 
  };
  //for (size_t i=0; i<rowValue.size()-4; i++)
  //  fieldValues.push_back(rowValue[i+1]);
    //fieldValues.insert( pair<string,string>(upper_copy(m_fieldnames[i]),rowValue[i+1]));
  //varValues.insert( {"@RAW",rowValue[0]});
  //varValues.insert( {"@FILE",m_filename});
  //varValues.insert( {"@FILEID",intToStr(m_fileid)});
  //varValues.insert( {"@LINE",rowValue[rowValue.size()-3]});
  //varValues.insert( {"@ROW",rowValue[rowValue.size()-2]});
  //varValues.insert( {"@FILELINE",rowValue[rowValue.size()-1]});
  //varValues.insert( {"@%",intToStr(rowValue.size()-4)});
  //varValues.insert( {"@DUPID","1"});
  // c++17 to improve performance
  //varValues.try_emplace( "@RAW",rowValue[0]);
  //varValues.try_emplace( "@FILE",m_filename);
  //varValues.try_emplace( "@FILEID",intToStr(m_fileid));
  //varValues.try_emplace( "@LINE",rowValue[rowValue.size()-3]);
  //varValues.try_emplace( "@ROW",rowValue[rowValue.size()-2]);
  //varValues.try_emplace( "@FILELINE",rowValue[rowValue.size()-1]);
  //varValues.try_emplace( "@%",intToStr(rowValue.size()-4));
  //varValues.try_emplace( "@DUPID","1");
  //varValues.try_emplace(m_uservariables.begin(), m_uservariables.end());
  //boost::unordered_maps
  //varValues.emplace( "@RAW",rowValue[0]);
  //varValues.emplace( "@FILE",m_filename);
  //varValues.emplace( "@FILEID",intToStr(m_fileid));
  //varValues.emplace( "@LINE",rowValue[rowValue.size()-3]);
  //varValues.emplace( "@ROW",rowValue[rowValue.size()-2]);
  //varValues.emplace( "@FILELINE",rowValue[rowValue.size()-1]);
  //varValues.emplace( "@%",intToStr(rowValue.size()-4));
  //varValues.emplace( "@DUPID","1");
  //varValues.emplace(m_uservariables.begin(), m_uservariables.end());
  varValues.insert(m_uservariables.begin(), m_uservariables.end());
  unordered_map< string,GroupProp > aggGroupProp;
  unordered_map< string,vector<string> > anaFuncData;
  vector< unordered_map< int,int > > sideMatchedRowIDs;
  vector< unordered_map< string, unordered_map<string,string> > > matchedSideDatarows;
  unordered_map< string, unordered_map<string,string> > matchedSideDatarow;
  RuntimeDataStruct rds;
  rds.fieldvalues = &fieldValues;
  rds.varvalues = &varValues;
  rds.aggFuncs = &aggGroupProp;
  rds.anaFuncs = &anaFuncData;
  rds.sideDatasets = &m_sideDatasets;
  rds.sideDatarow = &matchedSideDatarow;
  rds.sideDatatypes = &m_sideDatatypes;
  rds.macroFuncExprs = &m_userMacroExprs;

  static string sResult;
  static DataTypeStruct dts;

#ifdef __DEBUG__
  m_runtimedatapreparetime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
  // calculate user defined variables
  size_t iR = 0;
  unordered_map<string,string> fakeResult; // faked sidework dataset from variable R
  for (size_t i=0; i<m_uservarnames.size(); i++){ // make sure we calculate the dynamic and R variables in the order of input.
    if (m_uservarnames[i].compare("@R")!=0){ // eval normal dynamic variables
      m_uservarexprs[m_uservarnames[i]].evalExpression(rds, sResult, dts, true);
      m_uservariables[m_uservarnames[i]] = sResult;
      varValues[m_uservarnames[i]] = sResult;
    }else{ // eval fake sidework data
      if (iR < m_fakeRExprs.size()){
        m_fakeRExprs[iR].evalExpression(rds, sResult, dts, true);
        fakeResult.insert(pair<string,string>(intToStr(iR+1),sResult));
      }
      iR++;
    }
  }
  // add fake sidework data 
  if (fakeResult.size()>0){
    if (m_sideDatasets.size()==0){ // first row of the faked sidework dataset
      vector< unordered_map<string,string> > vFakeR; 
      vFakeR.push_back(fakeResult);
      m_sideDatasets.push_back(vFakeR);
    }else{
      m_sideDatasets[0].push_back(fakeResult);
    }
  }
#ifdef __DEBUG__
  m_uservarcaltime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__

  //trace(DEBUG, "Filtering '%s' ", rowValue[0].c_str());
  //dumpMap(varValues);
  bool matched = (!m_filter || m_filter->compareExpression(rds, sideMatchedRowIDs));

#ifdef __DEBUG__
  m_filtercomptime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
  // doing side work
  doSideWorks(&fieldValues, &varValues, &aggGroupProp, &anaFuncData);
#ifdef __DEBUG__
  thistime = curtime();
#endif // __DEBUG__
  //trace(DEBUG, " selected:%d (%d)! \n", matched, m_selections.size());
  if (matched){
    getSideDatarow(sideMatchedRowIDs, matchedSideDatarows);
#ifdef __DEBUG__
    m_getsidedatarowtime += (curtime()-thistime);
    thistime = curtime();
#endif // __DEBUG__
    vector<string> vResults;
    if (m_selections.size()>0){
      if (m_groups.size() == 0 && m_initAggProps.size() == 0 && !m_aggrOnly){
        //trace(DEBUG, " No group! \n");

        // If need to construct tree, save all fields value to m_results first, process the expressions in tree()
        if (m_treeProps.size()>0 && m_treeProps.size() == m_treeParentProps.size()){
          vector<string> extendedRowValues = rowValue;
          extendedRowValues.push_back(intToStr(m_line));
          extendedRowValues.push_back(intToStr(m_matchcount+1));
          extendedRowValues.push_back(intToStr(m_fileline));
          extendedRowValues.push_back(m_filename);
          extendedRowValues.push_back(intToStr(m_fileid));
          extendedRowValues.push_back(intToStr(m_fieldnames.size()));
          appendResultSet(extendedRowValues, varValues, false);
          ////eval selection expression to get side dataset only.
          //for (size_t i=0; i<m_selections.size()l i++)
          //  m_selections[i].evalExpression(rds, sResult, dts, true);
          for (size_t i=0; i<m_treeProps.size(); i++){
            m_treeProps[i].evalExpression(rds, sResult, dts, true);
            vResults.push_back(sResult);
          }
          m_treeKeys.push_back(vResults);
          vResults.clear();
          for (size_t i=0; i<m_treeParentProps.size(); i++){
            m_treeParentProps[i].evalExpression(rds, sResult, dts, true);
            vResults.push_back(sResult);
          }
          m_treeParentKeys.push_back(vResults);
#ifdef __DEBUG__
  m_savetreedatatime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
          return true;
        }else{
#ifdef __DEBUG__
  thistime = curtime();
#endif // __DEBUG__
          evalAddSelAndSortkeys("", rds, rds.fieldvalues, &aggGroupProp, &matchedSideDatarows, m_initAnaArray.size()==0, true);
#ifdef __DEBUG__
  m_evalRawdatatime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
        }
      }else{ // need to do group. store evaled data in a temp date set
        //trace(DEBUG2, " Grouping! \n");
        vector<string> groupExps;  // the group expressions. as the key of following hash map
        vector<string> aggSelResult;
        string sResult;
        DataTypeStruct dts;
        if (m_aggrOnly) // has aggregation function without any group, give an empty string as the key
          groupExps.push_back("");
        else // group expressions to the sorting keys
          for (size_t i=0; i<m_groups.size(); i++){
            m_groups[i].evalExpression(rds, sResult, dts, true);
            //trace(DEBUG2, "Adding '%s' for Group '%s', eval from '%s'\n", sResult.c_str(),m_groups[i].getEntireExpstr().c_str(),rowValue[0].c_str());
            groupExps.push_back(sResult);
          }
        //trace(DEBUG2, "Checking  '%s' (%d)\n", groupExps[0].c_str(), m_aggrOnly);
#ifdef __DEBUG__
  m_evalGroupKeytime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
        m_groupKeys.insert(groupExps);
        unordered_map< vector<string>, vector<string>, hash_container< vector<string> > >::iterator it1 = m_aggRowValues.find(groupExps);
        unordered_map< vector<string>, unordered_map< string,GroupProp >, hash_container< vector<string> > >::iterator it2 = m_aggGroupProp.find(groupExps);
        if (it2 == m_aggGroupProp.end()){
          //for (unordered_map< string,GroupProp >::iterator it=m_initAggProps.begin(); it!=m_initAggProps.end(); ++it)
          //  aggGroupProp.insert(pair< string,GroupProp >(it->first,it->second));
          aggGroupProp.insert(m_initAggProps.begin(), m_initAggProps.end());
          evalAggExpNode(&fieldValues, &varValues, aggGroupProp, matchedSideDatarow);
          m_aggGroupProp.insert( pair<vector<string>, unordered_map< string,GroupProp > >(groupExps,aggGroupProp));
        }else
          evalAggExpNode(&fieldValues, &varValues, it2->second, matchedSideDatarow);
        if (it1 == m_aggRowValues.end()){
          // add variables  @FILE, @FILEID
          fieldValues.push_back(m_filename);
          fieldValues.push_back(intToStr(m_fileid));
          m_aggRowValues.insert( pair<vector<string>, vector<string> >(groupExps,fieldValues));
        }
#ifdef __DEBUG__
  m_updateResulttime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
      }
    }else{ // since if no "select" provided, we will add @raw as default, this part of code will never be run.
#ifdef __DEBUG__
  thistime = curtime();
#endif // __DEBUG__
      vector<ExpressionC> anaEvaledExp;
      appendResultSet(rowValue, varValues, true);
      addResultOutputFileMap(&fieldValues, &varValues, &aggGroupProp, rds.anaFuncs, &matchedSideDatarow);
      vector<ExpressionC> vKeys;
      for (size_t i=0;i<m_sorts.size();i++)
        vKeys.push_back(m_sorts[i].sortKey);
      evalAddExprArray(vKeys, "", rds, &anaEvaledExp, vResults, true);
      m_sortKeys.push_back(vResults);
#ifdef __DEBUG__
  m_appendnonselresulttime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
      // eval filter expressions to get anaFuncData
      if (m_filter && m_filter->containAnaFunc())
        m_filter->evalAnaExprs(rds, &anaEvaledExp, sResult, dts, true);
#ifdef __DEBUG__
  m_evalanaexprtime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
      // initialize an empty anaFuncData
      vector<string> vEvaledParams;
      for (unordered_map< string,vector<ExpressionC> >::iterator it=m_initAnaArray.begin(); it!=m_initAnaArray.end(); ++it){
        vEvaledParams.clear();
        anaFuncData.insert(pair< string,vector<string> >(it->first,vEvaledParams));
      }
      if (anaEvaledExp.size()>0){
        m_anaEvaledExp.push_back(anaEvaledExp);
        addAnaFuncData(anaFuncData);
      }
#ifdef __DEBUG__
  m_addanafuncdatatime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    }
#ifdef __DEBUG__
  thistime = curtime();
#endif // __DEBUG__
  }
#ifdef __DEBUG__
  m_filtertime += (curtime()-filterbegintime);
#endif // __DEBUG__
  return matched;
}

bool QuerierC::searchStopped()
{
  //trace(DEBUG1, "Limit checking. m_groups:%d, m_sorts:%d, m_aggrOnly:%d, m_limittop:%d, m_outputrow:%d!\n",m_groups.size(),m_sorts.size(),m_aggrOnly,m_limittop,m_outputrow);
  if (m_groups.size()==0 && !m_aggrOnly && m_sorts.size()==0 && m_limittop>=0 && m_outputrow>=m_limittop)
    return true;
  else
    return false;
}

void QuerierC::trialAnalyze(const vector<string> & matcheddata)
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  trace(DEBUG, "Detecting data types from the matched line '%s'\n", matcheddata[0].c_str());
  analyzeFiledTypes(matcheddata);
  trace(DEBUG,"Detected field type %d/%d\n", m_fieldtypes.size(), matcheddata.size());
  if (m_bToAnalyzeSelectMacro || m_selections.size()==0){
    analyzeSelString();
    analyzeSortStr();
  }
  if (m_bToAnalyzeSortMacro)
    analyzeSortStr();
  for(unordered_map<string, ExpressionC>::iterator it=m_uservarexprs.begin(); it!=m_uservarexprs.end(); ++it)
    it->second.analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for(size_t i=0; i<m_fakeRExprs.size(); i++)
    m_fakeRExprs[i].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for (size_t i=0; i<m_sideSelections.size(); i++){
    unordered_map<string,DataTypeStruct> sideSelDatatypes;
    for (size_t j=0; j<m_sideSelections[i].size(); j++){
      m_sideSelections[i][j].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
      sideSelDatatypes.insert(pair<string,DataTypeStruct>(m_sideAlias[i][j].empty()?intToStr(j+1):m_sideAlias[i][j], m_sideSelections[i][j].m_datatype));
    }
    m_sideDatatypes.insert(pair< string, unordered_map<string,DataTypeStruct> >(intToStr(i+1),sideSelDatatypes));
  }
  for (size_t i=0; i<m_sideFilters.size(); i++)
    m_sideFilters[i].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  if (m_filter){
    m_filter->analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
    //m_filter->mergeExprConstNodes();
  }
  for (size_t i=0; i<m_selections.size(); i++){
    //trace(DEBUG, "Analyzing selection '%s' (%d), found %d\n", m_selections[i].m_expStr.c_str(), i, found);
    m_selections[i].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  }
  if (m_dupnumexp)
    m_dupnumexp->analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  if (m_dupfilter)
    m_dupfilter->analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for (size_t i=0; i<m_groups.size(); i++)
    m_groups[i].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for (size_t i=0; i<m_sorts.size(); i++){
    m_sorts[i].sortKey.analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
    if (m_sorts[i].sortKey.m_type==LEAF && m_sorts[i].sortKey.m_expType==CONST && isInt(m_sorts[i].sortKey.m_expStr) && atoi(m_sorts[i].sortKey.m_expStr.c_str())>0 && atoi(m_sorts[i].sortKey.m_expStr.c_str())<=m_selections.size()){
      m_sorts[i].iSel = atoi(m_sorts[i].sortKey.m_expStr.c_str()) - 1;
      m_sorts[i].sortKey.m_datatype = m_selections[m_sorts[i].iSel].m_datatype;
    }else
      m_sorts[i].iSel = -1;
  }
  for (unordered_map< string,MacroFuncStruct >::iterator it=m_userMacroExprs.begin(); it!=m_userMacroExprs.end(); it++)
    it->second.funcExpr->analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for (size_t i=0; i<m_treeProps.size(); i++)
    m_treeProps[i].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for (size_t i=0; i<m_treeParentProps.size(); i++)
    m_treeParentProps[i].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for (unordered_map< string,ExpressionC >::iterator it=m_aggFuncExps.begin(); it!=m_aggFuncExps.end(); ++it)
    it->second.analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for (size_t i=0; i<m_anaEvaledExp.size(); i++)
    for (size_t j=0; j<m_anaEvaledExp[i].size(); j++)
      m_anaEvaledExp[i][j].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  if (m_outputfileexp)
    m_outputfileexp->analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);

  // analyze extra filter
  if (m_extrafilter){
    m_trimmedFieldtypes.clear();
    for (size_t i=0;i<m_selections.size();i++)
      m_trimmedFieldtypes.push_back(m_selections[i].m_datatype);

    m_extrafilter->analyzeColumns(&m_selnames, &m_trimmedFieldtypes, &m_rawDatatype, &m_sideDatatypes);
  
    if (m_trimedInitSels.size() > 0){
      vector<ExpressionC> tmpSels = m_trimedInitSels;
      int extendedSelNum=0;
      // process foreach
      for (size_t i=0; i<m_trimedInitSels.size(); i++){
        if (m_trimedInitSels[i].m_expType == FUNCTION && m_trimedInitSels[i].m_Function && m_trimedInitSels[i].m_Function->m_funcID == FOREACH){
          vector<ExpressionC> vExpandedExpr;
          m_trimedInitSels[i].m_Function->expandForeach(vExpandedExpr, m_trimmedFieldtypes.size());
          tmpSels.erase(tmpSels.begin()+i+extendedSelNum);
          for (size_t j=0; j<vExpandedExpr.size();j++)
            tmpSels.insert(tmpSels.begin()+i+j+extendedSelNum,vExpandedExpr[j]);
          extendedSelNum+=vExpandedExpr.size()-1;
        }
      }
      m_trimedSelctions = tmpSels;
      for (size_t j=0; j<m_trimedSelctions.size(); j++)
        m_trimedSelctions[j].analyzeColumns(&m_selnames, &m_trimmedFieldtypes, &m_rawDatatype, &m_sideDatatypes);
    }
  }
#ifdef __DEBUG__
  m_trialanalyzetime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
}

int QuerierC::searchNextReg()
{
#ifdef __DEBUG__
  long int thistime = curtime();
  long int searchstarttime = thistime;
  long int rawanalyzestarttime;
#endif // __DEBUG__
  int found = 0;
  try {
    //namesaving_smatch matches(m_regexstr);
    while(!m_rawstr.empty() && regex_search(m_rawstr, m_matches, m_regexp)){
#ifdef __DEBUG__
  m_parsepatterntime += (curtime()-thistime);
  thistime = curtime();
  rawanalyzestarttime = thistime;
#endif // __DEBUG__
      m_line++;
      m_fileline++;
      vector<string> matcheddata;
      if (m_nameline && m_line==1){ // use the first line as field names
        for (size_t i=1; i<m_matches.size(); i++)
          m_fieldnames.push_back(m_matches[i]);
        m_nameline = false;
        m_line--;
        m_fileline--;
        continue;
      }else
        for (size_t i=0; i<m_matches.size(); i++)
          matcheddata.push_back(m_matches[i]);
      //trace(DEBUG2,"Detected rows %d/%d\n", m_detectedRawDatatype.size(), m_detectTypeMaxRowNum);
      // detect fileds data type. When m_bDetectAllOnChange is false, only matcheddata.size()>m_fieldnames.size()+1 triggers redetect
      if (m_detectedTypeRows < m_detectTypeMaxRowNum || (!m_bDetectAllOnChange && matcheddata.size()>m_fieldnames.size()+1) || (m_bDetectAllOnChange && matcheddata.size()!=m_fieldnames.size()+1)){
        if (matcheddata.size()!=m_fieldnames.size()+1 && m_fieldnames.size()>0){
          if (m_detectedTypeRows < m_detectTypeMaxRowNum || m_bDetectAllOnChange)
            m_fieldnames.clear();
          m_selections.clear();
          m_bToAnalyzeSelectMacro = m_bSelectContainMacro;
          m_bToAnalyzeSortMacro = m_bSortContainMacro;
        }
        if (m_fieldnames.size() == 0){
          m_fieldInitNames.clear();
          pairFiledNames(m_matches);
        }else{
          size_t initSize=min(m_fieldInitNames.size(),matcheddata.size()-1);
          for (size_t i=m_fieldnames.size(); i<initSize; i++)
            m_fieldnames.push_back(m_fieldInitNames[i]);
          for (size_t i=m_fieldnames.size();i<matcheddata.size()-1;i++)  // matcheddata[0] needs to be skipped
            m_fieldnames.push_back("@FIELD"+intToStr(i+1));
        }
        trialAnalyze(matcheddata);
      }
#ifdef __DEBUG__
  m_rawanalyzetime += (curtime()-rawanalyzestarttime);
  thistime = curtime();
#endif // __DEBUG__
      // append variables
      //matcheddata.push_back(m_filename);
      if (matchFilter(matcheddata)){
        m_matchcount++;
      }

      //trace(DEBUG, "Old m_expStr: '%s'\n", m_rawstr.c_str());
      m_rawstr = m_rawstr.substr(m_rawstr.find(matcheddata[0])+matcheddata[0].length());
      if (matcheddata[0].find("\n") == string::npos){ // if not matched a newline, skip until the next newline
        size_t newlnpos = m_rawstr.find("\n");
        if (newlnpos != string::npos)
          m_rawstr = m_rawstr.substr(newlnpos+1);
      }
      //trace(DEBUG, "Matched '%s', new m_expStr: '%s'\n", matcheddata[0].c_str(), m_rawstr.c_str());
      found++;
#ifdef __DEBUG__
  thistime = curtime();
#endif // __DEBUG__
    }
    if (found == 0){
      // if didnt match any one, discard all until the last newline
      size_t i = m_rawstr.size(), newlnpos = string::npos;
      while ((int)i>=0){
        if (m_rawstr[i] == '\n'){
          newlnpos = i;
          break;
        }
        i--;
      }
      if (newlnpos!=string::npos)
        m_rawstr = m_rawstr.substr(newlnpos+1);
    }
  }catch (exception& e) {
    trace(ERROR, "Regular search exception: %s\n", e.what());
    return found;
  }
  //trace(DEBUG, "(1)Found: %d in this searching\n", m_matchcount);
#ifdef __DEBUG__
  m_searchtime += (curtime()-searchstarttime);
#endif // __DEBUG__
  return found;
}

int QuerierC::searchNextWild()
{
#ifdef __DEBUG__
  long int thistime = curtime();
  long int searchstarttime = thistime;
  long int rawanalyzestarttime;
#endif // __DEBUG__
  trace(DEBUG, "Wild searching '%s'\n", m_regexstr.c_str());
  int found = 0;
  size_t pos = 0, opos;
  bool bEnded = false; // whether reach the end of current rawstr
  while (!bEnded && !m_rawstr.empty() && pos<m_rawstr.length()){
#ifdef __DEBUG__
  thistime = curtime();
  rawanalyzestarttime = thistime;
#endif // __DEBUG__
    // Unlike regular matching, wildcard and delimiter only match lines.
    opos = pos;
    string sLine;
    if (m_readmode==READLINE)
      sLine=m_rawstr;
    else
      readLine(m_rawstr, pos, sLine);
#ifdef __DEBUG__
  m_readrawlinetime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    bEnded = opos==pos; // pos will only be set back to the original pos if it reaches the end of current rawstr.
    m_rawstr = m_readmode==READLINE?"":m_rawstr.substr(pos);
    pos = 0;
    if(sLine.empty() && bEnded && m_bEof) {// read the rest of content if file reached eof, opos == pos check if it read an empty line
      sLine = m_rawstr;
      m_rawstr = "";
    }
    //trace(DEBUG, "Read '%s'\n", sLine.c_str());
    vector<string> matcheddata;
    matchWildcard(matcheddata,sLine,m_regexstr,m_quoters,'\\',{});
#ifdef __DEBUG__
  m_parsepatterntime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    if (matcheddata.size()==0 && bEnded)
      continue;
    m_line++;
    m_fileline++;
    if (m_nameline && m_line==1){ // use the first line as field names
      for (size_t i=0; i<matcheddata.size(); i++){
        m_fieldnames.push_back(matcheddata[i]);
        m_fieldInitNames.push_back(matcheddata[i]);
      }
      m_nameline = false;
      m_line--;
      m_fileline--;
      continue;
    }
    matcheddata.insert(matcheddata.begin(),sLine); // whole matched line for @raw
    //trace(DEBUG, "Matched %d\n", matcheddata.size());
    //for (size_t i=0; i<matcheddata.size(); i++)
    //  trace(DEBUG, "Matched %d: '%s'\n", i ,matcheddata[i].c_str());
    // detect fileds data type
    if (m_detectedTypeRows < m_detectTypeMaxRowNum || (!m_bDetectAllOnChange && matcheddata.size()>m_fieldnames.size()+1) || (m_bDetectAllOnChange && matcheddata.size()!=m_fieldnames.size()+1)){
      if (matcheddata.size()!=m_fieldnames.size()+1 && m_fieldnames.size()>0){
        if (m_detectedTypeRows < m_detectTypeMaxRowNum || m_bDetectAllOnChange)
          m_fieldnames.clear();
        m_selections.clear();
        m_bToAnalyzeSelectMacro = m_bSelectContainMacro;
        m_bToAnalyzeSortMacro = m_bSortContainMacro;
      }
      size_t initSize=min(m_fieldInitNames.size(),matcheddata.size()-1);
      for (size_t i=m_fieldnames.size(); i<initSize; i++)
        m_fieldnames.push_back(m_fieldInitNames[i]);
      for (size_t i=m_fieldnames.size();i<matcheddata.size()-1;i++) // matcheddata[0] needs to be skipped
        m_fieldnames.push_back("@FIELD"+intToStr(i+1));
      trialAnalyze(matcheddata);
    }
#ifdef __DEBUG__
  m_rawanalyzetime += (curtime()-rawanalyzestarttime);
  thistime = curtime();
#endif // __DEBUG__
    // append variables
    if (matchFilter(matcheddata))
      m_matchcount++;
    found++;
#ifdef __DEBUG__
  thistime = curtime();
#endif // __DEBUG__
  }
  //trace(DEBUG, "(2)Found: %d in this searching\n", m_matchcount);
#ifdef __DEBUG__
  m_searchtime += (curtime()-searchstarttime);
#endif // __DEBUG__
  return found;
}

int QuerierC::searchNextDelm()
{
#ifdef __DEBUG__
  long int thistime = curtime();
  long int searchstarttime = thistime;
  long int rawanalyzestarttime;
#endif // __DEBUG__
  trace(DEBUG, "Delm searching '%s'\n", m_regexstr.c_str());
  int found = 0;
  size_t pos = 0, opos;
  bool bEnded = false;
  string sLine;
  while (!bEnded && !m_rawstr.empty() && pos<m_rawstr.length()){
#ifdef __DEBUG__
  thistime = curtime();
  rawanalyzestarttime = thistime;
#endif // __DEBUG__
    // Unlike regular matching, wildcard and delimiter only match lines.
    opos = pos;
    sLine;
    if (m_readmode==READLINE)
      sLine=m_rawstr;
    else
      readLine(m_rawstr, pos,sLine);
#ifdef __DEBUG__
  m_readrawlinetime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    bEnded = opos==pos; // pos will only be set back to the original pos if it reaches the end of current rawstr.
    m_rawstr = m_readmode==READLINE?"":m_rawstr.substr(pos);
    if(sLine.empty() && bEnded && m_bEof){ // read the rest of content if file reached eof, opos == pos check if it read an empty line
      sLine = m_rawstr;
      m_rawstr = "";
    }
    string untrimmedLine = sLine; // untrimmed line is the real "raw" line
    if (!m_delmkeepspace && !trim_copy(sLine).empty())
      sLine = trim_copy(sLine);
#ifdef __DEBUG__
  m_rawprocesstrimtime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    vector<string> matcheddata;
    if ((!m_delmMulitChar && m_regexstr.empty()) || (m_delmMulitChar && m_delimSet.size()==0)){
      if (!(sLine.empty()&&bEnded))
        matcheddata.push_back(sLine);
    }else{
      if (m_delmMulitChar)
        split(matcheddata, sLine,m_delimSet,m_quoters,'\\',{},m_delmrepeatable, sLine.empty()&&bEnded);
      else
        split(matcheddata, sLine,m_regexstr,m_quoters,'\\',{},m_delmrepeatable, sLine.empty()&&bEnded, true);
        //quickersplit(matcheddata, sLine,m_regexstr[0]);
    }
#ifdef __DEBUG__
  m_parsepatterntime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    trace(DEBUG, "searchNextDelm Read '%s'(flag:%d; len:%d) vector size: %d \n", sLine.c_str(), sLine.empty()&&bEnded, sLine.length(), matcheddata.size());
    trace(DEBUG, "searchNextDelm pos/size %d/%d\n", pos, m_rawstr.length());
    pos = 0;
    //dumpVector(matcheddata);
    if (matcheddata.size()==0 && bEnded)
      continue;
    m_line++;
    m_fileline++;
    if (m_nameline && m_line==1){ // use the first line as field names
      for (size_t i=0; i<matcheddata.size(); i++){
        m_fieldnames.push_back(matcheddata[i]);
        m_fieldInitNames.push_back(matcheddata[i]);
      }
      m_nameline = false;
      m_line--;
      m_fileline--;
      continue;
    }
    matcheddata.insert(matcheddata.begin(),untrimmedLine); // whole matched line for @raw
    //trace(DEBUG, "Matched %d\n", matcheddata.size());
    //for (size_t i=0; i<matcheddata.size(); i++)
    //  trace(DEBUG, "Matched %d: '%s'\n", i ,matcheddata[i].c_str());
    // detect fileds data type
    if (m_detectedTypeRows < m_detectTypeMaxRowNum || (!m_bDetectAllOnChange && matcheddata.size()>m_fieldnames.size()+1) || (m_bDetectAllOnChange && matcheddata.size()!=m_fieldnames.size()+1)){
      if (matcheddata.size()!=m_fieldnames.size()+1 && m_fieldnames.size()>0){
        if (m_detectedTypeRows < m_detectTypeMaxRowNum || m_bDetectAllOnChange)
          m_fieldnames.clear();
        m_selections.clear();
        m_bToAnalyzeSelectMacro = m_bSelectContainMacro;
        m_bToAnalyzeSortMacro = m_bSortContainMacro;
      }
      size_t initSize=min(m_fieldInitNames.size(),matcheddata.size()-1);
      for (size_t i=m_fieldnames.size(); i<initSize; i++)
        m_fieldnames.push_back(m_fieldInitNames[i]);
      for (size_t i=m_fieldnames.size();i<matcheddata.size()-1;i++) // matcheddata[0] needs to be skipped
        m_fieldnames.push_back("@FIELD"+intToStr(i+1));
      trialAnalyze(matcheddata);
    }
#ifdef __DEBUG__
  m_rawanalyzetime += (curtime()-rawanalyzestarttime);
  thistime = curtime();
#endif // __DEBUG__
    // append variables
    if (matchFilter(matcheddata))
      m_matchcount++;
    found++;
#ifdef __DEBUG__
  thistime = curtime();
#endif // __DEBUG__
  }
  //trace(DEBUG, "(3)Found: %d in this searching\n", m_matchcount);
#ifdef __DEBUG__
  m_searchtime += (curtime()-searchstarttime);
#endif // __DEBUG__
  return found;
}

int QuerierC::searchNextLine()
{
#ifdef __DEBUG__
  long int thistime = curtime();
  long int searchstarttime = thistime;
  long int rawanalyzestarttime;
#endif // __DEBUG__
  trace(DEBUG, "Line searching '%s'\n", m_regexstr.c_str());
  int found = 0;
  size_t pos = 0, opos;
  bool bEnded = false;
  string sLine;
  while (!bEnded && !m_rawstr.empty() && pos<m_rawstr.length()){
#ifdef __DEBUG__
  thistime = curtime();
  rawanalyzestarttime = thistime;
#endif // __DEBUG__
    // Unlike regular matching, wildcard and delimiter only match lines.
    opos = pos;
    sLine;
    if (m_readmode==READLINE)
      sLine=m_rawstr;
    else
      readLine(m_rawstr, pos, sLine);
#ifdef __DEBUG__
  m_readrawlinetime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    bEnded = opos==pos; // pos will only be set back to the original pos if it reaches the end of current rawstr.
    m_rawstr = m_readmode==READLINE?"":m_rawstr.substr(pos);
    if(sLine.empty() && bEnded && m_bEof){ // read the rest of content if file reached eof, opos == pos check if it read an empty line
      sLine = m_rawstr;
      m_rawstr = "";
    }
#ifdef __DEBUG__
  m_parsepatterntime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    pos = 0;
    //dumpVector(matcheddata);
    if (sLine.empty() && bEnded)
      continue;
    vector<string> matcheddata;
    matcheddata.push_back(sLine);
    m_line++;
    m_fileline++;
    if (m_nameline && m_line==1){ // use the first line as field names
      for (size_t i=0; i<matcheddata.size(); i++){
        m_fieldnames.push_back(matcheddata[i]);
        m_fieldInitNames.push_back(matcheddata[i]);
      }
      m_nameline = false;
      m_line--;
      m_fileline--;
      continue;
    }
    matcheddata.insert(matcheddata.begin(),sLine); // whole matched line for @raw
    //trace(DEBUG, "Matched %d\n", matcheddata.size());
    //for (size_t i=0; i<matcheddata.size(); i++)
    //  trace(DEBUG, "Matched %d: '%s'\n", i ,matcheddata[i].c_str());
    // detect fileds data type
    if (m_detectedTypeRows < m_detectTypeMaxRowNum || (!m_bDetectAllOnChange && matcheddata.size()>m_fieldnames.size()+1) || (m_bDetectAllOnChange && matcheddata.size()!=m_fieldnames.size()+1)){
      if (matcheddata.size()!=m_fieldnames.size()+1 && m_fieldnames.size()>0){
        if (m_detectedTypeRows < m_detectTypeMaxRowNum || m_bDetectAllOnChange)
          m_fieldnames.clear();
        m_selections.clear();
        m_bToAnalyzeSelectMacro = m_bSelectContainMacro;
        m_bToAnalyzeSortMacro = m_bSortContainMacro;
      }
      size_t initSize=min(m_fieldInitNames.size(),matcheddata.size()-1);
      for (size_t i=m_fieldnames.size(); i<initSize; i++)
        m_fieldnames.push_back(m_fieldInitNames[i]);
      for (size_t i=m_fieldnames.size();i<matcheddata.size()-1;i++) // matcheddata[0] needs to be skipped
        m_fieldnames.push_back("@FIELD"+intToStr(i+1));
      trialAnalyze(matcheddata);
    }
#ifdef __DEBUG__
  m_rawanalyzetime += (curtime()-rawanalyzestarttime);
  thistime = curtime();
#endif // __DEBUG__
    // append variables
    if (matchFilter(matcheddata))
      m_matchcount++;
    found++;
#ifdef __DEBUG__
  thistime = curtime();
#endif // __DEBUG__
  }
  //trace(DEBUG, "(3)Found: %d in this searching\n", m_matchcount);
#ifdef __DEBUG__
  m_searchtime += (curtime()-searchstarttime);
#endif // __DEBUG__
  return found;
}

int QuerierC::searchNextQuick()
{
#ifdef __DEBUG__
  long int thistime = curtime();
  long int searchstarttime = thistime;
  long int rawanalyzestarttime;
#endif // __DEBUG__
  trace(DEBUG, "Quick searching '%s'\n", m_regexstr.c_str());
  int found = 0;
  size_t pos = 0, opos;
  bool bEnded = false;
  string sLine;
  while (!bEnded && !m_rawstr.empty() && pos<m_rawstr.length()){
#ifdef __DEBUG__
  thistime = curtime();
  rawanalyzestarttime = thistime;
#endif // __DEBUG__
    // Unlike regular matching, wildcard and delimiter only match lines.
    opos = pos;
    sLine;
    if (m_readmode==READLINE)
      sLine=m_rawstr;
    else
      readLine(m_rawstr, pos, sLine);
#ifdef __DEBUG__
  m_readrawlinetime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    bEnded = opos==pos; // pos will only be set back to the original pos if it reaches the end of current rawstr.
    m_rawstr = m_readmode==READLINE?"":m_rawstr.substr(pos);
    if(sLine.empty() && bEnded && m_bEof){ // read the rest of content if file reached eof, opos == pos check if it read an empty line
      sLine = m_rawstr;
      m_rawstr = "";
    }
    pos = 0;
    //dumpVector(matcheddata);
    if (sLine.empty() && bEnded)
      continue;
    if (m_line==0){
      if (m_nameline){ // use the first line as field names
        m_fieldnames.push_back(sLine);
        m_line++;
        continue;
      }
    }else
      m_fieldnames.push_back("@RAW");
    m_line++;
    if ((m_quickwildcard && like(sLine, m_regexstr)) || (m_quickregular && reglike(sLine, m_regexstr)) || sLine.find(m_regexstr)!=string::npos){
      vector<string> matcheddata{"",sLine};
      m_results.push_back(matcheddata);
    }
#ifdef __DEBUG__
  m_parsepatterntime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
  }
  //trace(DEBUG, "(3)Found: %d in this searching\n", m_matchcount);
#ifdef __DEBUG__
  m_searchtime += (curtime()-searchstarttime);
#endif // __DEBUG__
  return found;
}

int QuerierC::searchNext()
{
  if (searchStopped()){
    trace(DEBUG1, "stopped searching!\n");
    return 0;
  }
  switch(m_searchMode){
    case REGSEARCH:
      return searchNextReg();
    case WILDSEARCH:
      return searchNextWild();
    case DELMSEARCH:
      return searchNextDelm();
    case LINESEARCH:
      return searchNextLine();
    case QUICKSEARCH:
      return searchNextQuick();
    default:
      return 0;
  }
}

// prepare initial data before searching and matching.
void QuerierC::readyToGo()
{
  // initialize aggregation function expressions
  for (unordered_map< string,GroupProp >::iterator it=m_initAggProps.begin(); it!=m_initAggProps.end(); ++it)
    m_aggFuncExps.insert(pair<string,ExpressionC>(it->first,ExpressionC(it->first)));
  if (m_searchMode == QUICKSEARCH){ // in quick mode, no select/filter/extrafilter/sort/group/user var/tree...
    clearAllCommands();
    m_selections.clear();
    m_selections.push_back(ExpressionC("@RAW"));
  }
}

int QuerierC::searchAll()
{
  int totalfound = 0;
  //trace(DEBUG2, "Searching pattern: %s\n", m_regexstr.c_str());
  int found = searchNext();
  while (found>0 && !m_rawstr.empty()){
    //trace(DEBUG, "Searching pattern: '%s' in '%s', found %d\n", m_regexstr.c_str(), m_rawstr.c_str(), totalfound);
    found = searchNext();
    totalfound+=found;
  }
  return totalfound;
}

// group result
bool QuerierC::group()
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  if ((m_groups.size() == 0 && !m_aggrOnly) || m_groupKeys.size() == 0 || (m_treeProps.size()>0 && m_treeProps.size() == m_treeParentProps.size()))
    return true;

  unordered_map< string,vector<string> > anaFuncData;
  unordered_map<string, string> varValues;
  vector<string> vResults;
  string sResult;
  DataTypeStruct dts;
  vector< unordered_map< int,int > > sideMatchedRowIDs;
  vector< unordered_map< string, unordered_map<string,string> > > matchedSideDatarows;
  unordered_map< string, unordered_map<string,string> > matchedSideDatarow;
  int iRow = 0;
  m_anaFuncResult.clear();
  m_anaEvaledExp.clear();
  m_anaSortProps.clear();
  m_anaSortData.clear();
  RuntimeDataStruct rds;
  rds.varvalues = &varValues;
  rds.anaFuncs = &anaFuncData;
  rds.sideDatasets = &m_sideDatasets;
  rds.sideDatarow = &matchedSideDatarow;
  rds.sideDatatypes = &m_sideDatatypes;
  rds.macroFuncExprs = &m_userMacroExprs;
  for (unordered_set< vector<string>, hash_container< vector<string> > >::iterator it=m_groupKeys.begin(); it!=m_groupKeys.end(); ++it){
    rds.fieldvalues = &m_aggRowValues[*it];
    rds.aggFuncs = &m_aggGroupProp[*it];
    varValues.clear();
    varValues.insert( pair<string,string>("@RAW",m_aggRowValues[*it][0]));
    varValues.insert( pair<string,string>("@FILE",m_aggRowValues[*it][m_aggRowValues[*it].size()-2]));
    varValues.insert( pair<string,string>("@FILEID",m_aggRowValues[*it][m_aggRowValues[*it].size()-1]));
    varValues.insert( pair<string,string>("@LINE",intToStr(iRow)));
    varValues.insert( pair<string,string>("@FILELINE",intToStr(iRow)));
    varValues.insert( pair<string,string>("@ROW",intToStr(iRow)));
    varValues.insert( pair<string,string>("@%",intToStr(m_fieldnames.size())));
    varValues.insert( pair<string,string>("@DUPID","1"));
    varValues.insert(m_uservariables.begin(), m_uservariables.end());
    // removed stored variables @FILE, @FILEID.
    m_aggRowValues[*it].pop_back();
    m_aggRowValues[*it].pop_back();
    anaFuncData.clear();
    if (m_filter && !m_filter->compareExpression(rds, sideMatchedRowIDs))
      continue;
    getSideDatarow(sideMatchedRowIDs, matchedSideDatarows);
    iRow++;
    vector<string> vEvaledParams;
    for (unordered_map< string,vector<ExpressionC> >::iterator it=m_initAnaArray.begin(); it!=m_initAnaArray.end(); ++it){
      vEvaledParams.clear();
      anaFuncData.insert(pair< string,vector<string> >(it->first,vEvaledParams));
    }

    evalAddSelAndSortkeys(m_aggRowValues[*it][0], rds, &m_aggRowValues[*it], &m_aggGroupProp[*it], &matchedSideDatarows, true, false);

    //m_aggGroupDataidMap.insert(pair< vector<string>,int >(*it,m_anaEvaledExp.size()-1));
    m_aggGroupDataidMap.insert(pair< int, vector<string> >(m_anaEvaledExp.size()-1, *it));
  }

  //clearGroup();
#ifdef __DEBUG__
  m_grouptime += (curtime()-thistime);
#endif // __DEBUG__
  return true;
}

bool QuerierC::processAnalyticA(const short int & iFuncID, const string & sFuncExpStr, vector< vector<string> > & vFuncData)
{
  if (iFuncID!=SUMA && iFuncID!=COUNTA && iFuncID!=UNIQUECOUNTA && iFuncID!=AVERAGEA && iFuncID!=MAXA && iFuncID!=MINA){
    trace(ERROR,"(1)'%s' should not be processed here!\n", sFuncExpStr.c_str());
    return false;
  }
  size_t iAnaGroupNum = 0;

  // first part parameter of SUMA/COUNTA/UNIQUECOUNTA/AVERAGEA/MAXA/MINA are groups
  unordered_map< string,vector<int> >::iterator itn = m_anaFuncParaNums.find(sFuncExpStr);
  if (itn == m_anaFuncParaNums.end()){
    trace(ERROR, "(2)Failed to find analytic function parameter numbers '%s'\n", sFuncExpStr.c_str());
    return false;
  }else{
    if (itn->second.size() < 2){
      trace(ERROR, "(2)The analytic function parameter numbers size %d is smaller than 2\n", sFuncExpStr.c_str(), itn->second.size());
      return false;
    }
    iAnaGroupNum = itn->second[0];
  }
  if (vFuncData[0].size()<iAnaGroupNum+1){
    trace(ERROR, "'%s' requires two parameter parts!\n", sFuncExpStr.c_str());
    return false;
  }

  unordered_map< vector<string>, std::set<string>, hash_container< vector<string> > > tmpArray; // for calculate uniquecounta
  unordered_map< vector<string>, int, hash_container< vector<string> > > tmpCount; // for calculate averagea/counta
  unordered_map< vector<string>, float, hash_container< vector<string> > > tmpSum; // for calculate suma/averagea
  unordered_map< vector<string>, string, hash_container< vector<string> > > tmpVal; // for calculate maxa/mina
  // calculate SUMA/COUNTA/UNIQUECOUNTA/AVERAGEA/MAXA/MINA
  for (size_t i=0; i<vFuncData.size(); i++){
    vector<string> vGroups;
    for (size_t j=0;j<iAnaGroupNum;j++)
      vGroups.push_back(vFuncData[i][j]);
    switch (iFuncID){
    case SUMA:
      if (!isFloat(vFuncData[i][iAnaGroupNum])){
        trace(ERROR, " is not a valid float data!\n", vFuncData[i][iAnaGroupNum].c_str());
        continue;
      }else if (tmpSum.find(vGroups)!=tmpSum.end()){
        tmpSum[vGroups] += atof(vFuncData[i][iAnaGroupNum].c_str());
      }else
        tmpSum.insert(pair< vector<string>, float>(vGroups, atof(vFuncData[i][iAnaGroupNum].c_str())));
      break;
    case COUNTA:
      if (tmpCount.find(vGroups)!=tmpCount.end()){
        tmpCount[vGroups] += 1;
      }else
        tmpCount.insert(pair< vector<string>, int>(vGroups, 1));
      break;
    case UNIQUECOUNTA:
      if (tmpArray.find(vGroups)!=tmpArray.end()){
        tmpArray[vGroups].insert(vFuncData[i][iAnaGroupNum]);
      }else{
        std::set<string> valSet;
        valSet.insert(vFuncData[i][iAnaGroupNum]);
        tmpArray.insert(pair< vector<string>, std::set<string> >(vGroups, valSet));
      }
      break;
    case AVERAGEA:
      if (!isFloat(vFuncData[i][iAnaGroupNum])){
        trace(ERROR, " is not a valid float data!\n", vFuncData[i][iAnaGroupNum].c_str());
        continue;
      }else if (tmpSum.find(vGroups)!=tmpSum.end()){
        tmpSum[vGroups] += atof(vFuncData[i][iAnaGroupNum].c_str());
        tmpCount[vGroups] += 1;
      }else{
        tmpSum.insert(pair< vector<string>, float>(vGroups, atof(vFuncData[i][iAnaGroupNum].c_str())));
        tmpCount.insert(pair< vector<string>, int>(vGroups, 1));
      }
      break;
    case MAXA:
      if (tmpVal.find(vGroups)!=tmpVal.end()){
        if (anyDataCompare(tmpVal[vGroups], vFuncData[i][iAnaGroupNum], m_anaSortProps[sFuncExpStr][0].sortKey.m_datatype, m_anaSortProps[sFuncExpStr][0].sortKey.m_datatype) < 0)
          tmpVal[vGroups] = vFuncData[i][iAnaGroupNum];
      }else
        tmpVal.insert(pair< vector<string>, string >(vGroups, vFuncData[i][iAnaGroupNum]));
      break;
    case MINA:
      if (tmpVal.find(vGroups)!=tmpVal.end()){
        if (anyDataCompare(tmpVal[vGroups], vFuncData[i][iAnaGroupNum], m_anaSortProps[sFuncExpStr][0].sortKey.m_datatype, m_anaSortProps[sFuncExpStr][0].sortKey.m_datatype) > 0)
          tmpVal[vGroups] = vFuncData[i][iAnaGroupNum];
      }else
        tmpVal.insert(pair< vector<string>, string >(vGroups, vFuncData[i][iAnaGroupNum]));
      break;
    }
  }

  // assign calculated result of SUMA/COUNTA/UNIQUECOUNTA/AVERAGEA/MAXA/MINA then return
  for (size_t i=0; i<m_anaFuncResult.size(); i++){
    vector<string> vGroups;
    for (size_t j=0;j<iAnaGroupNum;j++)
      vGroups.push_back(vFuncData[i][j]);
    switch (iFuncID){
    case SUMA:
      m_anaFuncResult[i][sFuncExpStr] = floatToStr(tmpSum[vGroups]);
      break;
    case COUNTA:
      m_anaFuncResult[i][sFuncExpStr] = intToStr(tmpCount[vGroups]);
      break;
    case UNIQUECOUNTA:
      m_anaFuncResult[i][sFuncExpStr] = intToStr(tmpArray[vGroups].size());
      break;
    case AVERAGEA:
      m_anaFuncResult[i][sFuncExpStr] = floatToStr(tmpSum[vGroups]/(float)tmpCount[vGroups]);
      break;
    case MAXA:
      m_anaFuncResult[i][sFuncExpStr] = tmpVal[vGroups];
      break;
    case MINA:
      m_anaFuncResult[i][sFuncExpStr] = tmpVal[vGroups];
      break;
    }
  }
  return true;
}

bool QuerierC::sortAnaData(vector<SortProp> & sortProps, const short int & iFuncID, const string & sFuncExpStr, vector< vector<string> > & vFuncData)
{
  sortProps.insert(sortProps.end(),m_anaSortProps[sFuncExpStr].begin(),m_anaSortProps[sFuncExpStr].end());
  // rank/denserank sort lambda function
  auto sortVectorLambdaB = [sortProps] (vector<string> const& v1, vector<string> const& v2) -> bool
  {
    if (v1.size()!=sortProps.size()+1 || v2.size()!=sortProps.size()+1 || v2.size()!=v1.size())
      return false;
    for (size_t i=0;i<sortProps.size();i++){
      int iCompareRslt = anyDataCompare(v1[i],v2[i],sortProps[i].sortKey.m_datatype,sortProps[i].sortKey.m_datatype);
      if (iCompareRslt == 0) // Compare next key only when current keys equal
        continue;
      return (sortProps[i].direction==ASC ? iCompareRslt<0 : iCompareRslt>0);
    }
    return false; // return false if all elements equal
  };
  // nearby sort lambda function
  auto sortVectorLambdaC = [sortProps] (vector<string> const& v1, vector<string> const& v2) -> bool
  {
    if (v1.size()<2 || v2.size()<2 || v1.size()!=sortProps.size()+4 || v2.size()!=sortProps.size()+4 || v2.size()!=v1.size())
      return false;
    for (size_t i=0;i<sortProps.size();i++){
      int iCompareRslt = anyDataCompare(v1[i+1],v2[i+1],sortProps[i].sortKey.m_datatype,sortProps[i].sortKey.m_datatype);
      if (iCompareRslt == 0) // Compare next key only when current keys equal
        continue;
      return (sortProps[i].direction==ASC ? iCompareRslt<0 : iCompareRslt>0);
    }
    return false; // return false if all elements equal
  };
  trace(DEBUG,"sortProps size: %d; sort keys size: %d\n",sortProps.size(), vFuncData[0].size());
//#define DEBUG_ANALYTIC
#ifdef DEBUG_ANALYTIC
  printf("sortProps size: %d\n",sortProps.size());
  for (size_t i=0;i<sortProps.size();i++)
    printf("%d:%s %d\t",i,decodeDatatype(sortProps[i].sortKey.m_datatype.datatype).c_str(),sortProps[i].direction);
  printf("\n");
  printf("Before sorting analytic function data [%d][%d]\n",vFuncData.size(), vFuncData[0].size());
  for (size_t i=0;i<vFuncData.size();i++){
    for (size_t j=0;j<vFuncData[i].size();j++)
      printf("%d:%s\t",j,vFuncData[i][j].c_str());
    printf("\n");
  }
#endif
  if (iFuncID == RANK || iFuncID == DENSERANK)
    std::sort(vFuncData.begin(), vFuncData.end(), sortVectorLambdaB);
  if (iFuncID == NEARBY)
    std::sort(vFuncData.begin(), vFuncData.end(), sortVectorLambdaC);
#ifdef DEBUG_ANALYTIC
  printf("Sorted analytic function data [%d][%d]\n",vFuncData.size(), vFuncData[0].size());
  for (size_t i=0;i<vFuncData.size();i++){
    for (size_t j=0;j<vFuncData[i].size();j++)
      printf("%d:%s\t",j,vFuncData[i][j].c_str());
    printf("\n");
  }
#endif
  return true;
}

bool QuerierC::processAnalyticB(const short int & iFuncID, const string & sFuncExpStr, vector< vector<string> > & vFuncData)
{
  if (iFuncID!=RANK && iFuncID!=DENSERANK){
    trace(ERROR,"(2)'%s' should not be processed here!\n", sFuncExpStr.c_str());
    return false;
  }
  vector<SortProp> sortProps;
  if (iFuncID==RANK || iFuncID==DENSERANK){ // For RANK/DENSERANK, the first part parameters are groups, which also need to be sorted (before sort keys).
    size_t iAnaGroupNum = 0;
    unordered_map< string,vector<int> >::iterator itn = m_anaFuncParaNums.find(sFuncExpStr);
    if (itn == m_anaFuncParaNums.end()){
      trace(ERROR, "(3)Failed to find analytic function parameter numbers '%s'\n", sFuncExpStr.c_str());
      return false;
    }else{
      if (itn->second.size() < 2){
        trace(ERROR, "(3)The analytic function parameter numbers size %d is smaller than 2\n", sFuncExpStr.c_str(), itn->second.size());
        return false;
      }
      iAnaGroupNum = itn->second[0];
    }
    FunctionC* anaFunc;
    SortProp sp;
    for (size_t j=0;j<iAnaGroupNum;j++){
      anaFunc = NULL;
      for (size_t i=0; i<m_anaEvaledExp[0].size();i++){
        //trace(DEBUG,"(2)Searching analytic function '%s' from '%s'\n",sFuncExpStr.c_str(),m_anaEvaledExp[0][i].getEntireExpstr().c_str());
        //m_anaEvaledExp[0][i].dump();
        anaFunc = m_anaEvaledExp[0][i].getAnaFunc(sFuncExpStr);
        if (anaFunc)
          break;
      }
      if (!anaFunc){
        trace(ERROR, "(2)Failed to find analytic function expression '%s'\n", sFuncExpStr.c_str());
        sp.sortKey = ExpressionC();
      }else{
        if (anaFunc->m_params.size() < j)
          trace(ERROR, "(2)Analytic function '%s' parameter size %d is smaller than expected!\n", sFuncExpStr.c_str(), anaFunc->m_params.size());
        sp.sortKey = anaFunc->m_params[j];
      }
      sp.direction = ASC;
      sortProps.push_back(sp);
    }
  }
  
  if (!sortAnaData(sortProps, iFuncID, sFuncExpStr, vFuncData))
    return false;

  // variables for RANK/DENSERANK
  vector<string> preRow;
  size_t iRank = 1, iDenseRank = 1;
  for (size_t i=0; i<m_anaFuncResult.size(); i++){
    size_t iPosBeforeSorted = atoi(vFuncData[i][vFuncData[i].size()-1].c_str());
    bool bNewGroup = false, bSortValueChanged=false;
    if (preRow.size() == 0){
      bNewGroup = true;
      for (size_t j=0;j<vFuncData[i].size()-1;j++){
        preRow.push_back(vFuncData[i][j]);
#ifdef DEBUG_ANALYTIC
        printf("%d:%s\t",j,vFuncData[i][j].c_str());
#endif
      }
    }else {
      for (size_t j=0;j<vFuncData[i].size()-1;j++){
        if (j<m_anaFuncParaNums[sFuncExpStr][0] && vFuncData[i][j].compare(preRow[j])!=0)
          bNewGroup = true;
        if (j>=m_anaFuncParaNums[sFuncExpStr][0] && vFuncData[i][j].compare(preRow[j])!=0)
          bSortValueChanged = true;
        preRow[j] = vFuncData[i][j];
#ifdef DEBUG_ANALYTIC
        printf("%d:%s\t",j,vFuncData[i][j].c_str());
#endif
      }
    }
    if (bNewGroup){
      iRank = 1;
      iDenseRank = 1;
    }else{
      iRank += 1;
      if (bSortValueChanged)
        iDenseRank = iRank;
    }
#ifdef DEBUG_ANALYTIC
    printf(" === %d %d (%d %d)\n",iRank, iDenseRank, bNewGroup, bSortValueChanged);
    dumpVector(vFuncData[i]);
#endif
    trace(DEBUG,"Checking analytic function '%s' group size: %d. Check result: %d %d. Rank: %d \n",sFuncExpStr.c_str(), m_anaFuncParaNums[sFuncExpStr][0], bNewGroup, bSortValueChanged, iRank);
    m_anaFuncResult[iPosBeforeSorted][sFuncExpStr] = intToStr(iFuncID==RANK?iRank:iDenseRank);
  }

  return true;
}

bool QuerierC::processAnalyticC(const short int & iFuncID, const string & sFuncExpStr, vector< vector<string> > & vFuncData)
{
  if (iFuncID!=NEARBY){
    trace(ERROR,"(3)'%s' should not be processed here!\n", sFuncExpStr.c_str());
    return false;
  }
  vector<SortProp> sortProps;
  if (!sortAnaData(sortProps, iFuncID, sFuncExpStr, vFuncData))
    return false;

  // variables for NEARBY
  size_t iAnaExprNum = 0, iAnaSortNum = 0, iDistParaNum = 0, iDefaultParaNum = 0;
  unordered_map< string,vector<int> >::iterator itn = m_anaFuncParaNums.find(sFuncExpStr);
  if (itn->second.size()<4){
    trace(ERROR, "NEARBY requires 4 parts parameters, only got %d\n", itn->second.size());
    return false;
  }
  iAnaExprNum = itn->second[0], iAnaSortNum = itn->second[1], iDistParaNum = itn->second[2], iDefaultParaNum = itn->second[3];
  if (iAnaExprNum < 1){
    trace(ERROR, "NEARBY target expression is not provided!\n");
    //continue;
  }else if (iAnaExprNum > 1)
    trace(WARNING, "NEARBY target expression only need one parameter, will use the first one, the other will be discarded!\n");
  if (iDistParaNum < 1){
    trace(ERROR, "NEARBY distiance is not provided!\n");
    //continue;
  }else if (iDistParaNum > 1)
    trace(WARNING, "NEARBY distiance only need one parameter, will use the first one, the other will be discarded!\n");
  if (iDefaultParaNum < 1){
    trace(ERROR, "NEARBY default value is not provided!\n");
    //continue;
  }else if (iDefaultParaNum > 1)
    trace(WARNING, "NEARBY default value only need one parameter, will use the first one, the other will be discarded!\n");
  if (vFuncData[0].size() != iAnaExprNum+iAnaSortNum+iDistParaNum+iDefaultParaNum+1){
    trace(ERROR, "NEARBY processing data size %d doesnot match parameter group total number %d+%d+%d+%d+1 !\n",vFuncData[0].size(), iAnaExprNum, iAnaSortNum, iDistParaNum, iDefaultParaNum);
    return false;
  }

  for (size_t i=0; i<m_anaFuncResult.size(); i++){
    size_t iPosBeforeSorted = atoi(vFuncData[i][vFuncData[i].size()-1].c_str());
    int iDistance = 0;
    if (!isInt(vFuncData[i][iAnaExprNum+iAnaSortNum]))
      trace(ERROR, "NEARBY distiance '%s' is not a valid number!\n", vFuncData[i][iAnaExprNum+iAnaSortNum].c_str());
    else
      iDistance = atoi(vFuncData[i][iAnaExprNum+iAnaSortNum].c_str());
    m_anaFuncResult[iPosBeforeSorted][sFuncExpStr] = (i+iDistance>=0&&i+iDistance<vFuncData.size())?vFuncData[i+iDistance][0]:vFuncData[i][iAnaExprNum+iAnaSortNum+iDistParaNum];
  }

  return true;
}

bool QuerierC::processAnalytic(const string & sFuncExpStr, vector< vector<string> > & vFuncData)
{
  if (vFuncData.size() == 0){
    trace(ERROR,"No enough analytic processing data!\n");
    return false;
  }
  if (m_anaFuncResult.size() != vFuncData.size()){
    trace(ERROR,"Analytic processing data size %d doesnot match evolved analytic function result size %d!\n", vFuncData.size(), m_anaFuncResult.size());
    return false;
  }

  short int iFuncID = encodeFunction(sFuncExpStr.substr(0,sFuncExpStr.find("(")));
  if (iFuncID == UNKNOWN){
    trace(ERROR, "Failed to find analytic function '%s'\n", sFuncExpStr.c_str());
    return false;
  }

  if (iFuncID==SUMA || iFuncID==COUNTA || iFuncID==UNIQUECOUNTA || iFuncID==AVERAGEA || iFuncID==MAXA || iFuncID==MINA)
    return processAnalyticA(iFuncID, sFuncExpStr, vFuncData);

  // add index for each sort key for other analytic functions involved sorting
  for (size_t i=0; i<vFuncData.size(); i++)
    vFuncData[i].push_back(intToStr(i));

  if (iFuncID==RANK || iFuncID==DENSERANK)
    return processAnalyticB(iFuncID, sFuncExpStr, vFuncData);
  else if (iFuncID==NEARBY)
    return processAnalyticC(iFuncID, sFuncExpStr, vFuncData);

  return true;
}

// group and sort analytic functions
bool QuerierC::analytic()
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  if (m_initAnaArray.size() == 0 || (m_treeProps.size()>0 && m_treeProps.size() == m_treeParentProps.size()))
    return true;
  if (m_anaSortData.size() != m_anaSortProps.size() || m_anaSortProps.size() != m_anaFuncParaNums.size() || m_anaFuncParaNums.size() != m_anaSortData.size()){
    trace(ERROR, "Analytic functions sorting data size %d, sorting prop size %d, group number size %d do not match!\n", m_anaSortData.size(), m_anaSortProps.size(), m_anaFuncParaNums.size());
    return false;
  }
  if (m_results.size() != m_anaFuncResult.size() || m_results.size() != m_anaEvaledExp.size() || m_anaFuncResult.size() != m_anaEvaledExp.size()){
    trace(ERROR, "Result data size %d, analytic function result size %d, analytic function expression size %d do not match!\n", m_results.size(), m_anaFuncResult.size(), m_anaEvaledExp.size());
    return false;
  }

  vector< unordered_map< string,string > > anaFuncResult;
  for (unordered_map< string,vector< vector<string> > >::iterator it=m_anaSortData.begin(); it!=m_anaSortData.end(); it++)
    processAnalytic(it->first, it->second);
#ifdef __DEBUG__
  trace(DEBUG2,"Analytic sorting time %d\n", curtime()-thistime);
  m_analytictime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__

  unordered_map<string, string> varValues;
  varValues.insert( pair<string,string>("@FILE",m_filename));
  varValues.insert( pair<string,string>("@FILEID",intToStr(m_fileid)));
  varValues.insert( pair<string,string>("@%",intToStr(m_fieldnames.size())));
  varValues.insert( pair<string,string>("@DUPID","1"));
  varValues.insert(m_uservariables.begin(), m_uservariables.end());
  int iRow = 0;
  trace(DEBUG, "m_anaEvaledExp size is %d, dimension is %d . \n", m_anaEvaledExp.size(),m_anaEvaledExp[0].size());
  vector<unordered_map< int,int > > sideMatchedRowIDs;
  vector< unordered_map< string, unordered_map<string,string> > > matchedSideDatarows;
  unordered_map< string, unordered_map<string,string> > matchedSideDatarow;
  vector<string> vfieldvalues;
  unordered_map< string,GroupProp > dummyAggGroupProp;
  RuntimeDataStruct rds;
  rds.fieldvalues = &vfieldvalues;
  rds.varvalues = &varValues;
  rds.sideDatasets = &m_sideDatasets;
  rds.sideDatarow = &matchedSideDatarow;
  rds.sideDatatypes = &m_sideDatatypes;
  rds.macroFuncExprs = &m_userMacroExprs;
  vector< vector<string> > tmpResults = m_results;
  vector< vector<string> > tmpSortKeys = m_sortKeys;
  m_results.clear();
  m_sortKeys.clear();
  m_resultfiles.clear();
  for (int i=0; i<tmpResults.size();i++){
    varValues.insert( pair<string,string>("@RAW",tmpResults[i][0]));
    varValues.insert( pair<string,string>("@LINE",intToStr(iRow)));
    varValues.insert( pair<string,string>("@FILELINE",intToStr(iRow)));
    varValues.insert( pair<string,string>("@ROW",intToStr(iRow)));
    // create the map for the filter
    unordered_map< string,vector<string> > anaFinalResult;
    rds.anaFuncs = &anaFinalResult;
    vector<string> anaThisResult;
    for (unordered_map< string,string >::iterator it=m_anaFuncResult[i].begin(); it!=m_anaFuncResult[i].end(); it++){
      anaThisResult.clear();
      anaThisResult.push_back(it->second);
      anaFinalResult.insert(pair< string,vector<string> >(it->first,anaThisResult));
    }
    // filter by analytic function results
    rds.aggFuncs = ((m_aggGroupProp.size()>0 && m_aggGroupDataidMap.size()==tmpResults.size())?(&m_aggGroupProp[m_aggGroupDataidMap[i]]):&dummyAggGroupProp);
    if (m_filter && !m_filter->compareExpression(rds, sideMatchedRowIDs))
      continue;
    getSideDatarow(sideMatchedRowIDs, matchedSideDatarows);
    iRow++;
    int anaExpID = 0;

    for (size_t j=0; j<m_selections.size(); j++)
      if (m_selections[j].containAnaFunc()){
        string sResult;
        m_anaEvaledExp[i][anaExpID].evalAnalyticFunc(&(m_anaFuncResult[i]), sResult);
        anaExpID++;
        trace(DEBUG, "Selection '%s'(%d) contains analytic function, assign result '%s' (was '%s')\n", m_selections[j].getEntireExpstr().c_str(),j,sResult.c_str(),tmpResults[i][j+1].c_str());
        tmpResults[i][j+1] = sResult;
      }
    // if appendResultSet return false, mean filtered by extra filter
    if (appendResultSet(tmpResults[i], varValues, true)) {
      for (size_t j=0; j<m_sorts.size(); j++)
        if (m_sorts[j].sortKey.containAnaFunc()){
          string sResult;
          m_anaEvaledExp[i][anaExpID].evalAnalyticFunc(&(m_anaFuncResult[i]), sResult);
          anaExpID++;
          trace(DEBUG, "Selection '%s'(%d) contains analytic function, assign result '%s' (was '%s')\n", m_sorts[j].sortKey.getEntireExpstr().c_str(),j,sResult.c_str(),tmpResults[i][j+1].c_str());
          tmpSortKeys[i][j] = sResult;
        }
      m_sortKeys.push_back(tmpSortKeys[i]);
    }
    addResultOutputFileMap(&vfieldvalues, &varValues, &dummyAggGroupProp, &anaFinalResult, &matchedSideDatarow);
  }
  clearAnalytic();
#ifdef __DEBUG__
  trace(DEBUG2,"Analytic reorder result time %d\n", curtime()-thistime);
  m_analytictime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
  return true;
}

void QuerierC::unique()
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  trace(DEBUG, "Result number before unique: %d.\n",m_results.size());
  if (!m_bUniqueResult)
    return;
  std::set< vector<string> > uresults; // temp result set when UNIQUE involved
  vector< vector<string> > tmpResult; // we need this as SET cannot keep the same sequence with sortkeys
  size_t iSize=0, iDups=0;
  for (size_t i=0;i<m_results.size();i++){
    iSize = uresults.size();
    if (m_eliminateDupField)
      eliminateDups(m_results[i]);
    uresults.insert(m_results[i]);
    if (iSize == uresults.size()) { // duplicated row
      if (m_sortKeys.size()>i-iDups){
        m_sortKeys.erase(m_sortKeys.begin()+i-iDups);
        iDups++;
      }
    }else
      tmpResult.push_back(m_results[i]);
  }
  unordered_map<string, string> varValues;
  m_results.clear();
  for (size_t i=0;i<tmpResult.size();i++)
    appendResultSet(tmpResult[i], varValues, false);
  trace(DEBUG, "Result number after unique: %d.\n",m_results.size());
#ifdef __DEBUG__
  m_uniquetime += (curtime()-thistime);
#endif // __DEBUG__
}

// doing merging sort exchanging
void QuerierC::mergeSort(const int & iLeftB, const int & iLeftT, const int & iRightB, const int & iRightT)
{
  //trace(DEBUG2, "Mergeing %d %d %d %d\n", iLeftB, iLeftT, iRightB, iRightT);
  if (iLeftT >= iRightB || iLeftB > iLeftT || iRightB > iRightT)
    return;
  else{
    mergeSort(iLeftB, max(iLeftB,iLeftB+(int)floor(iLeftT-iLeftB)/2-1), min(iLeftT,max(iLeftB,iLeftB+(int)floor(iLeftT-iLeftB)/2-1)+1), iLeftT);
    mergeSort(iRightB, max(iRightB,iRightB+(int)floor(iRightT-iRightB)/2-1), min(iRightT,max(iRightB,iRightB+(int)floor(iRightT-iRightB)/2-1)+1), iRightT);
//#ifdef __DEBUG__
//  for (size_t i=0; i<m_sortKeys.size(); i++)
//    printf("%s(%d) ", (*(m_sortKeys.begin()+i))[0].c_str(), i);
//  printf("\n");
//#endif // __DEBUG__
    size_t iLPos = iLeftB, iRPos = iRightB, iCheckPos = iRightB;
    while (iLPos<iCheckPos && iRPos<=iRightT){
      //trace(DEBUG2, "Swaping %d %d %d %d\n", iLPos, iCheckPos, iRPos, iRightT);
      bool exchanged = false;
      for (size_t i=0; i<m_sorts.size(); i++){
        //trace(DEBUG2, "Checking '%s' : '%s'\n", (*(m_sortKeys.begin()+iLPos))[i].c_str(), (*(m_sortKeys.begin()+iRPos))[i].c_str());
//#ifdef __DEBUG__
  //trace(DEBUG2, "Checking %s(L) %s(R) (%d %d %d) (%s) (%d)\n", (*(m_sortKeys.begin()+iLPos))[i].c_str(), (*(m_sortKeys.begin()+iRPos))[i].c_str(),iLPos,iCheckPos,iRPos,decodeDatatype(m_sorts[i].sortKey.m_datatype.datatype).c_str(), m_sorts[i].direction);
//#endif // __DEBUG__
        //if (bToBeSwapped){
        int iCompareRslt = anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype,m_sorts[i].sortKey.m_datatype);
        if ((m_sorts[i].direction==ASC ? iCompareRslt>0 : iCompareRslt<0)){
//#ifdef __DEBUG__
  //trace(DEBUG2, "moving %s(R) before %s(L) (%d %d %d)\n", (*(m_sortKeys.begin()+iRPos))[i].c_str(), (*(m_sortKeys.begin()+iLPos))[i].c_str(),iLPos,iCheckPos,iRPos);
//#endif // __DEBUG__
          //trace(DEBUG2, "Before move: %s(%d) %s(%d)\n", (*(m_results.begin()+iLPos))[1].c_str(), iLPos, (*(m_results.begin()+iRPos))[1].c_str(), iRPos);
          m_results.insert(m_results.begin()+iLPos,*(m_results.begin()+iRPos));
          m_results.erase(m_results.begin()+iRPos+1);
          m_sortKeys.insert(m_sortKeys.begin()+iLPos,*(m_sortKeys.begin()+iRPos));
          m_sortKeys.erase(m_sortKeys.begin()+iRPos+1);
          //trace(DEBUG2, "After move: %s(%d) %s(%d)\n", (*(m_results.begin()+iLPos))[1].c_str(), iLPos, (*(m_results.begin()+iRPos))[1].c_str(), iRPos);
          exchanged = true;
          break;
        }
        if (iCompareRslt != 0) // Compare next key only when current keys equal
          break;
      }
      if (exchanged){
        iCheckPos++; // one element from right side insert to left side, check point needs to be moved 1 step to right.
        iLPos++; // old left element moved 1 step to right. 
        iRPos++; // compare the next right element
      }else{
        iLPos++;
        //m_sorts[0].direction==ASC?iLPos++:iRPos++; // compare to the next left element
      }
    }
//#ifdef __DEBUG__
//  trace(DEBUG1, "Completed merging %d %d %d %d\n", iLeftB, iLeftT, iRightB, iRightT);
//  for (size_t i=0; i<m_sortKeys.size(); i++)
//    printf("%s(%d) ", (*(m_sortKeys.begin()+i))[0].c_str(), i);
//  printf("\n");
//#endif // __DEBUG__
  }
}

// doing merging sort exchanging
void QuerierC::mergeSort(vector< vector<string> > *dataSet, vector<SortProp>* sortProps, vector< vector<string> >* sortKeys, const int & iLeftB, const int & iLeftT, const int & iRightB, const int & iRightT)
{
  //trace(DEBUG2, "Mergeing %d %d %d %d\n", iLeftB, iLeftT, iRightB, iRightT);
  if (iLeftT >= iRightB || iLeftB > iLeftT || iRightB > iRightT)
    return;
  else{
    mergeSort(dataSet, sortProps, sortKeys, iLeftB, max(iLeftB,iLeftB+(int)floor(iLeftT-iLeftB)/2-1), min(iLeftT,max(iLeftB,iLeftB+(int)floor(iLeftT-iLeftB)/2-1)+1), iLeftT);
    mergeSort(dataSet, sortProps, sortKeys, iRightB, max(iRightB,iRightB+(int)floor(iRightT-iRightB)/2-1), min(iRightT,max(iRightB,iRightB+(int)floor(iRightT-iRightB)/2-1)+1), iRightT);
//#ifdef __DEBUG__
//  for (size_t i=0; i<sortKeys->size(); i++)
//    printf("%s(%d) ", (*(sortKeys->begin()+i))[0].c_str(), i);
//  printf("\n");
//#endif // __DEBUG__
    size_t iLPos = iLeftB, iRPos = iRightB, iCheckPos = iRightB;
    while (iLPos<iCheckPos && iRPos<=iRightT){
      //trace(DEBUG2, "Swaping %d %d %d %d\n", iLPos, iCheckPos, iRPos, iRightT);
      bool exchanged = false;
      for (size_t i=0; i<sortProps->size(); i++){
        //trace(DEBUG2, "Checking '%s' : '%s'\n", (*(sortKeys->begin()+iLPos))[i].c_str(), (*(sortKeys->begin()+iRPos))[i].c_str());
//#ifdef __DEBUG__
  //trace(DEBUG2, "Checking %s(L) %s(R) (%d %d %d) (%s) (%d)\n", (*(sortKeys->begin()+iLPos))[i].c_str(), (*(sortKeys->begin()+iRPos))[i].c_str(),iLPos,iCheckPos,iRPos,decodeDatatype((*sortProps)[i].sortKey.m_datatype.datatype).c_str(), (*sortProps)[i].direction);
//#endif // __DEBUG__
        //if (bToBeSwapped){
        int iCompareRslt = anyDataCompare((*(sortKeys->begin()+iLPos))[i],(*(sortKeys->begin()+iRPos))[i],(*sortProps)[i].sortKey.m_datatype,(*sortProps)[i].sortKey.m_datatype);
        if (((*sortProps)[i].direction==ASC ? iCompareRslt>0 : iCompareRslt<0)){
//#ifdef __DEBUG__
  //trace(DEBUG2, "moving %s(R) before %s(L) (%d %d %d)\n", (*(sortKeys->begin()+iRPos))[i].c_str(), (*(sortKeys->begin()+iLPos))[i].c_str(),iLPos,iCheckPos,iRPos);
//#endif // __DEBUG__
          //trace(DEBUG2, "Before move: %s(%d) %s(%d)\n", (*(dataSet->begin()+iLPos))[1].c_str(), iLPos, (*(dataSet->begin()+iRPos))[1].c_str(), iRPos);
          dataSet->insert(dataSet->begin()+iLPos,*(dataSet->begin()+iRPos));
          dataSet->erase(dataSet->begin()+iRPos+1);
          if (dataSet != sortKeys){ // dataset can also be sortkeys at the mean time
            sortKeys->insert(sortKeys->begin()+iLPos,*(sortKeys->begin()+iRPos));
            sortKeys->erase(sortKeys->begin()+iRPos+1);
          }
          //trace(DEBUG2, "After move: %s(%d) %s(%d)\n", (*(dataSet->begin()+iLPos))[1].c_str(), iLPos, (*(dataSet->begin()+iRPos))[1].c_str(), iRPos);
          exchanged = true;
          break;
        }
        if (iCompareRslt != 0) // Compare next key only when current keys equal
          break;
      }
      if (exchanged){
        iCheckPos++; // one element from right side insert to left side, check point needs to be moved 1 step to right.
        iLPos++; // old left element moved 1 step to right. 
        iRPos++; // compare the next right element
      }else{
        iLPos++;
        //(*sortProps)[0].direction==ASC?iLPos++:iRPos++; // compare to the next left element
      }
    }
//#ifdef __DEBUG__
//  trace(DEBUG1, "Completed merging %d %d %d %d\n", iLeftB, iLeftT, iRightB, iRightT);
//  for (size_t i=0; i<m_sortKeys.size(); i++)
//    printf("%s(%d) ", (*(m_sortKeys.begin()+i))[0].c_str(), i);
//  printf("\n");
//#endif // __DEBUG__
  }
}

// sort result
bool QuerierC::sort()
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__

  //for (size_t i=0; i<m_sorts.size(); i++)
  //  trace(DEBUG, "Sorting key '%s'(%d) !\n",m_sorts[i].sortKey.getEntireExpstr().c_str(),i);
  trace(DEBUG2, "Sorting begins, sort key size %d, sort expr number %d!\n",m_sortKeys.size(),m_sorts.size());
  if (m_sorts.size() == 0 || m_sortKeys.size() == 0 || (m_treeProps.size()>0 && m_treeProps.size() == m_treeParentProps.size())){
    //trace(DEBUG2, "No sorting keys %d %d!\n",m_sortKeys.size(),m_sorts.size());
    return true;
  }
  if (m_sortKeys[0].size() != m_sorts.size()){
    trace(ERROR, "The sorting value number %d doesnot equal to the key number %d!\n",m_sortKeys[0].size(),m_sorts.size());
    return false;
  }
  if (m_sortKeys.size() != m_results.size()){
    trace(ERROR, "The sorting value number %d doesnot equal to the selected value number %d!\n",m_sortKeys.size(),m_results.size());
    return false;
  }

  //mergeSort(0,max(0,(int)floor(m_sortKeys.size())/2-1), min((int)(m_sortKeys.size()-1),max(0,(int)floor(m_sortKeys.size())/2-1)+1),m_sortKeys.size()-1);
  //mergeSort(&m_results,&m_sorts,&m_sortKeys,0,max(0,(int)floor(m_sortKeys.size())/2-1), min((int)(m_sortKeys.size()-1),max(0,(int)floor(m_sortKeys.size())/2-1)+1),m_sortKeys.size()-1);

  // add index for each sort key
  for (size_t i=0; i<m_sortKeys.size(); i++)
    m_sortKeys[i].push_back(intToStr(i));
  vector<SortProp> sortProps = m_sorts;
  auto sortVectorLambda = [sortProps] (vector<string> const& v1, vector<string> const& v2) -> bool
  {
    if (v1.size()!=sortProps.size()+1 || v2.size()!=sortProps.size()+1 || v2.size()!=v1.size())
      return false;
    for (size_t i=0;i<sortProps.size();i++){
      int iCompareRslt = anyDataCompare(v1[i],v2[i],sortProps[i].sortKey.m_datatype,sortProps[i].sortKey.m_datatype);
      if (iCompareRslt == 0) // Compare next key only when current keys equal
        continue;
      return (sortProps[i].direction==ASC ? iCompareRslt<0 : iCompareRslt>0);
    }
    return false; // return false if all elements equal
  };
  std::sort(m_sortKeys.begin(), m_sortKeys.end(), sortVectorLambda);
  //std::sort(&m_sortKeys[0], &m_sortKeys[m_sortKeys.size()-1], sortVectorLambda);
  //vector< vector<string> >::iterator it = m_sortKeys.begin();
  //std::sort(it, it+m_sortKeys.size()-1, sortVectorLambda);
  vector< vector<string> > tmpResults = m_results;
  vector< ofstream* > tmpRsltFiles = m_resultfiles;
  for (size_t i=0; i<m_sortKeys.size(); i++){
    m_results[i] = tmpResults[atoi(m_sortKeys[i][m_sortKeys[i].size()-1].c_str())];
    if (i<m_resultfiles.size())
      m_resultfiles[i] = tmpRsltFiles[atoi(m_sortKeys[i][m_sortKeys[i].size()-1].c_str())];
  }
  clearSort();
  //trace(DEBUG1, "Sorting completed \n");
#ifdef __DEBUG__
  m_sorttime += (curtime()-thistime);
#endif // __DEBUG__
  return true;
}

void QuerierC::releaseTree(TreeNode* tNode)
{
  if (tNode->firstChild)
    releaseTree(tNode->firstChild);
  if (tNode->nextSibling)
    releaseTree(tNode->nextSibling);
  SafeDelete(tNode);
}

void QuerierC::SetTree(const vector< vector<string> > & tmpResults, TreeNode* tNode, const short int & level, int & nodeid, unordered_map< string,vector<ExpressionC> > & treeFuncs)
{
  if (tNode->rowid>=tmpResults.size()){
    trace(ERROR, "Node rowid %d is out of result set range (%d)!\n", tNode->rowid, tmpResults.size());
    return;
  }
  nodeid++;
  vector<string> fieldValues;
  unordered_map<string, string> varValues;
  for (size_t i=0; i<tmpResults[tNode->rowid].size()-6; i++)
    fieldValues.push_back(tmpResults[tNode->rowid][i+1]);
  varValues.insert( pair<string,string>("@RAW",tmpResults[tNode->rowid][0]));
  varValues.insert( pair<string,string>("@LINE",tmpResults[tNode->rowid][tmpResults[tNode->rowid].size()-6]));
  varValues.insert( pair<string,string>("@ROW",tmpResults[tNode->rowid][tmpResults[tNode->rowid].size()-5]));
  varValues.insert( pair<string,string>("@FILELINE",tmpResults[tNode->rowid][tmpResults[tNode->rowid].size()-4]));
  varValues.insert( pair<string,string>("@FILE",tmpResults[tNode->rowid][tmpResults[tNode->rowid].size()-3]));
  varValues.insert( pair<string,string>("@FILEID",tmpResults[tNode->rowid][tmpResults[tNode->rowid].size()-2]));
  varValues.insert( pair<string,string>("@%",tmpResults[tNode->rowid][tmpResults[tNode->rowid].size()-1]));
  varValues.insert( pair<string,string>("@DUPID","1"));
  varValues.insert( pair<string,string>("@LEVEL",intToStr(level)));
  varValues.insert( pair<string,string>("@NODEID",intToStr(nodeid)));
  varValues.insert( pair<string,string>("@ISLEAF",intToStr(tNode->firstChild?0:1)));
  unordered_map< string,GroupProp > aggGroupProp;
  unordered_map< string,vector<string> > anaFuncData;
  vector< unordered_map< int,int > > sideMatchedRowIDs;
  vector< unordered_map< string, unordered_map<string,string> > > matchedSideDatarows;
  unordered_map< string, unordered_map<string,string> > matchedSideDatarow;
  RuntimeDataStruct rds;
  rds.fieldvalues = &fieldValues;
  rds.varvalues = &varValues;
  rds.aggFuncs = &aggGroupProp;
  rds.anaFuncs = &anaFuncData;
  rds.sideDatasets = &m_sideDatasets;
  rds.sideDatarow = &matchedSideDatarow;
  rds.sideDatatypes = &m_sideDatatypes;
  rds.macroFuncExprs = &m_userMacroExprs;

  string sResult;
  DataTypeStruct dts;
  for (unordered_map<string, ExpressionC>::iterator it=m_uservarexprs.begin(); it!=m_uservarexprs.end(); ++it){
    it->second.evalExpression(rds, sResult, dts, true);
    m_uservariables[it->first] = sResult;
    varValues[it->first] = sResult;
  }
  varValues.insert(m_uservariables.begin(), m_uservariables.end());
  
  vector<string> vResults;
  string sConn="/";
  for (unordered_map< string,vector<ExpressionC> >::iterator it=treeFuncs.begin(); it!=treeFuncs.end(); it++){
    vResults.clear();
    for (size_t i=0; i<it->second.size(); i++){
      it->second[i].evalExpression(rds, sResult, dts, true);
      vResults.push_back(sResult);
    }
    if (vResults.size()>0 && upper_copy(trim_copy(it->first)).find("ROOT(")==0){
      if (tNode->parentNode && tNode->parentNode->mFuncVals.find(it->first) != tNode->parentNode->mFuncVals.end())
        tNode->mFuncVals.insert(pair< string,string >(it->first,tNode->parentNode->mFuncVals[it->first]));
      else
        tNode->mFuncVals.insert(pair< string,string >(it->first,vResults[0]));
    }else if (vResults.size()>0 && upper_copy(trim_copy(it->first)).find("PARENT(")==0){
      if (tNode->parentNode && tNode->parentNode->mParentVals.find(it->first) != tNode->parentNode->mParentVals.end())
        tNode->mFuncVals.insert(pair< string,string >(it->first,tNode->parentNode->mParentVals[it->first]));
      else
        tNode->mFuncVals.insert(pair< string,string >(it->first,""));
      tNode->mParentVals.insert(pair< string,string >(it->first,vResults[0]));
    }else if (vResults.size()>0 && upper_copy(trim_copy(it->first)).find("PATH(")==0){
      sConn=vResults.size()>1?vResults[1]:"/"; 
      if (tNode->parentNode && tNode->parentNode->mFuncVals.find(it->first) != tNode->parentNode->mFuncVals.end())
        tNode->mFuncVals.insert(pair< string,string >(it->first,tNode->parentNode->mFuncVals[it->first]+sConn+vResults[0]));
      else
        tNode->mFuncVals.insert(pair< string,string >(it->first,vResults[0]));
    }
  }

  evalAddSelAndSortkeys("", rds, rds.fieldvalues, rds.aggFuncs, &matchedSideDatarows, true, true);

  if (tNode->firstChild)
    SetTree(tmpResults, tNode->firstChild, level+1, nodeid, treeFuncs);
  if (tNode->nextSibling)
    SetTree(tmpResults, tNode->nextSibling, level, nodeid, treeFuncs);
}

// construct tree
bool QuerierC::tree()
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  if (m_treeProps.size()==0 || m_treeProps.size()!=m_treeParentProps.size() || m_treeKeys.size()==0 || m_treeKeys.size()!=m_treeParentKeys.size() || m_treeKeys.size()!=m_results.size())
    return true;

  std::set < TreeNode* > roots;
  unordered_map< vector<string>, TreeNode*, hash_container< vector<string> > > treeNodes;
  for (size_t i=0; i<m_treeKeys.size();i++){
    if (treeNodes.find(m_treeKeys[i]) != treeNodes.end()){
      trace(WARNING, "Duplicated tree keys detected, skip one row!\n");
      continue;
    }
    TreeNode* tNode=new TreeNode();
    tNode->rowid = i;
    tNode->rowValues = m_results[i];
    tNode->treeKeys = m_treeKeys[i];
    tNode->parentKeys = m_treeParentKeys[i];
    tNode->parentNode=NULL;
    tNode->firstChild=NULL;
    tNode->nextSibling=NULL;
    tNode->bInTheTree=false;
    treeNodes.insert( pair< vector<string>, TreeNode* >(m_treeKeys[i],tNode)); 
  }
  
  for (unordered_map< vector<string>, TreeNode*, hash_container< vector<string> > >::iterator it=treeNodes.begin(); it!=treeNodes.end(); it++){
    if (it->second->bInTheTree)
      continue;
    TreeNode* tNode = it->second;
    while (tNode && treeNodes.find(tNode->parentKeys) != treeNodes.end()){ // if find this node's parent in the processed nodes, set it as the first child.
      tNode->bInTheTree=true; // set it to true in case a loop detected, the node will not be added twice.
      if (treeNodes[tNode->parentKeys] == it->second){ // detect a loop
        trace(WARNING, "Loop detected, break the loop!\n");
        //treeNodes[tNode->parentKeys]->parentNode=NULL;
        break;
      }
      if (treeNodes[tNode->parentKeys]->firstChild)
        tNode->nextSibling=treeNodes[tNode->parentKeys]->firstChild;
      treeNodes[tNode->parentKeys]->firstChild=tNode;
      tNode->parentNode=treeNodes[tNode->parentKeys];
      if (tNode->parentNode && tNode->parentNode->bInTheTree) // check if parent has already been built
        break;
      tNode=tNode->parentNode;
    }
    if (tNode && !tNode->parentNode)
      roots.insert(tNode);
  }

  unordered_map< string,vector<ExpressionC> > treeFuncs;
  for (size_t i=0; i<m_selections.size(); i++)
    m_selections[i].getTreeFuncs(treeFuncs);

  vector< vector<string> > tmpResults = m_results;
  m_results.clear();
  int nodeid=0;
  for (std::set < TreeNode* >::iterator it = roots.begin(); it!=roots.end(); it++)
    SetTree(tmpResults, *it, 0, nodeid, treeFuncs);
  for (std::set < TreeNode* >::iterator it = roots.begin(); it!=roots.end(); it++)
    releaseTree(*it);

  clearTree();
#ifdef __DEBUG__
  m_treetime += (curtime()-thistime);
#endif // __DEBUG__
  return true;
}

//void QuerierC::formatoutput(namesaving_smatch matches)
//{
//  //for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
//  //  printf("%s\t", matches[*it].str().c_str());
//  for (size_t i=1; i<matches.size(); i++)
//    printf("%s\t",matches[i].str().c_str());
//  printf("\n");
//}

void QuerierC::outputstream(const int & resultid, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  if (m_outputmode == STANDARD)
    vprintf(fmt, args);
  else{
    if (resultid<m_resultfiles.size() && m_resultfiles[resultid]){
      char buf[8192];
      int bsize = 0;
      bsize = vsprintf(buf, fmt, args);
      m_resultfiles[resultid]->write(buf, bsize);
    }else
      trace(ERROR, "Failed to find output file!\n");
  }
  va_end(args);
}

void QuerierC::formatoutput(vector<string> & datas, const int & resultid)
{
  if (m_eliminateDupField)
    eliminateDups(datas);
  if (m_bStatOnly)
    return;
  if (m_outputformat == JSON){
    if (m_outputrow==m_limitbottom-1){
      outputstream(resultid, "{\n");
      outputstream(resultid, "\t\"records\": [\n");
    }else if (m_outputrow>m_limitbottom-1){
      outputstream(resultid, ",\n");
    }
  }
  if (m_outputrow<m_limitbottom-1 ){
    m_outputrow++;
    return;
  }else if (m_limittop>=0 && m_outputrow+1>m_limittop)
    return;
  else
    m_outputrow++;
  //outputstream(resultid, "%d: ", m_outputrow);
  if (m_outputformat == JSON){
    outputstream(resultid, "\t\t{\n");
    if (m_selections.size()==0){
      outputstream(resultid, "\t\t\t\"%s\": \"%s\"\n","RAW",datas[0].c_str());
    }else{
      for (size_t i=1; i<datas.size(); i++){
        if (m_selections[i-1].m_datatype.datatype == INTEGER || m_selections[i-1].m_datatype.datatype == LONG || m_selections[i-1].m_datatype.datatype == DOUBLE || m_selections[i-1].m_datatype.datatype == BOOLEAN)
          outputstream(resultid, "\t\t\t\"%s\": %s",m_selnames[i-1].c_str(),datas[i].c_str());
        else
          outputstream(resultid, "\t\t\t\"%s\": \"%s\"",m_selnames[i-1].c_str(),datas[i].c_str());
        if (i == datas.size()-1)
          outputstream(resultid, "\n");
        else
          outputstream(resultid, ",\n");
      }
    }
    outputstream(resultid, "\t\t}");
  }else{
    if (m_selections.size()==0)
      outputstream(resultid, "%s\n", datas[0].c_str());
    else{
      for (size_t i=1; i<datas.size(); i++)
        outputstream(resultid, "%s%s", datas[i].c_str(),i<datas.size()-1?m_fielddelim.c_str():"");
      outputstream(resultid, "\n");
    }
  }
}

void QuerierC::printFieldNames()
{
  // output field names only in STANDARD(screen) output mode
  if (m_outputmode != STANDARD)
    return;
  if (m_bNamePrinted)
    return;
  // re-analyze extra filter string to get the trimmed selection alias
  if (m_trimmedAlias.size()>0){
    m_selnames.clear();
    for (size_t i=0; i<m_trimedInitSels.size(); i++)
      if (i<m_trimmedAlias.size()&& !m_trimmedAlias[i].empty())
        m_selnames.push_back(m_trimmedAlias[i]);
      else
        m_selnames.push_back(m_trimedInitSels[i].getEntireExpstr());
  }

  if (m_colToRows.size() != m_colToRowNames.size()){
    trace(ERROR,"COLTOROW marco function size %d doesnot match filed names of COLTOROW marco function %d",m_colToRows.size(),m_colToRowNames.size());
  }else{
    for (size_t i=m_colToRows.size()-1;(int)i>=0;i--){
      m_selnames.insert(m_selnames.begin()+m_colToRows[i][0],m_colToRowNames[i]);
      for (size_t j=m_colToRows[i].size()-1;(int)j>=0;j--)
        m_selnames.erase(m_selnames.begin()+1+m_colToRows[i][j]);
    }
  }
  if (m_selnames.size()>0){
    for (size_t i=0; i<m_selnames.size(); i++){
      printf("%s%s",m_selnames[i].c_str(),i<m_selnames.size()-1?m_fielddelim.c_str():"");
    }
    printf("\n");
    for (size_t i=0; i<m_selnames.size(); i++)
      printf("%s%s",string(m_selnames[i].length(),'-').c_str(),i<m_selnames.size()-1?m_fielddelim.c_str():"");
  }else{
    printf("Row\n"); 
    printf("%s",string(58,'-').c_str());
  }
  printf("\n");
  m_bNamePrinted = true;
}

// return false if filtered by extra filter
bool QuerierC::applyExtraFilter(const vector<string> & aRow, const unordered_map<string, string> & varValues)
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  if (!m_extrafilter){
    m_results.push_back(aRow);
    return true;
  }

  vector<string> fieldValues;
  unordered_map< string,GroupProp > aggGroupProp;
  unordered_map< string,vector<string> > anaFuncData;
  vector< vector< unordered_map<string,string> > > sideDatasets;
  vector< unordered_map< string, unordered_map<string,string> > > matchedSideDatarows;
  unordered_map< string, unordered_map<string,string> > matchedSideDatarow;
  unordered_map< string, unordered_map<string,DataTypeStruct> > sideDatatypes;
  vector< unordered_map< int,int > > sideMatchedRowIDs;
  unordered_map<string, string> newVarVals = varValues;
  RuntimeDataStruct rds;
  rds.fieldvalues = &fieldValues;
  rds.varvalues = &newVarVals;
  rds.aggFuncs = &aggGroupProp;
  rds.anaFuncs = &anaFuncData;
  rds.sideDatasets = &sideDatasets;
  rds.sideDatarow = &matchedSideDatarow;
  rds.sideDatatypes = &sideDatatypes;
  rds.macroFuncExprs = &m_userMacroExprs;

  fieldValues.clear();
  for (size_t j=1; j<aRow.size(); j++)
    fieldValues.push_back(aRow[j]);
  newVarVals["@RAW"]=concatArray(aRow,m_fielddelim);
  newVarVals["@%"]=intToStr(aRow.size()-1);
  //fieldValues.insert( pair<string,string>(upper_copy(m_fieldnames[i]),rowValue[i+1]));
  if (!m_extrafilter || m_extrafilter->compareExpression(rds, sideMatchedRowIDs)){
    if (m_trimedSelctions.size()>0){
      vector<string> trimmedResult;
      trimmedResult.push_back("");
      string sResult;
      DataTypeStruct dts;
      for (size_t j=0; j<m_trimedSelctions.size(); j++){
        m_trimedSelctions[j].evalExpression(rds, sResult, dts, true);
        trimmedResult.push_back(sResult);
      }
      //appendResultSet(trimmedResult, newVarVals, false); // to avoid recursive call
      m_results.push_back(trimmedResult);
    }else
      //appendResultSet(aRow, newVarVals, false); // to avoid recursive call
      m_results.push_back(aRow);
    return true;
  }
  
#ifdef __DEBUG__
  m_extrafiltertime += (curtime()-thistime);
#endif // __DEBUG__
  return false;
}

void QuerierC::applyExtraFilter()
{
  return;
  if (!m_extrafilter)
    return;

  unordered_map<string, string> varValues{
    {"@RAW",""},
    {"@LINE","0"},
    {"@ROW","0"},
    {"@FILELINE","0"},
    {"@%","0"},
    {"@FILE",m_filename},
    {"@FILEID",intToStr(m_fileid)},
    {"@DUPID","1"}
  };
  //varValues.insert( pair<string,string>("@RAW",""));
  //varValues.insert( pair<string,string>("@LINE","0"));
  //varValues.insert( pair<string,string>("@ROW","0"));
  //varValues.insert( pair<string,string>("@FILELINE","0"));
  //varValues.insert( pair<string,string>("@%","0"));
  //varValues.insert( pair<string,string>("@FILE",m_filename));
  //varValues.insert( pair<string,string>("@FILEID",intToStr(m_fileid)));
  //varValues.insert( pair<string,string>("@DUPID","1"));
  varValues.insert(m_uservariables.begin(), m_uservariables.end());

  m_colToRows.clear();
  vector< vector<string> > tmpResults = m_results;
  m_results.clear();
  for (size_t i=0; i<tmpResults.size(); i++)
    applyExtraFilter(tmpResults[i], varValues);
}

void QuerierC::genReport(const vector<string> & datas)
{
  if (m_outputrow<m_limitbottom-1 || (m_limittop>=0 && m_outputrow+1>m_limittop)){
    return;
  }

  if (m_reportResult.size()>0){
    for (size_t i=1; i<datas.size(); i++){
      if (i<datas.size() && isDouble(datas[i]) && m_reportOps.find(i)!=m_reportOps.end() && m_reportResult.find(i)!=m_reportResult.end()){
        switch (m_reportOps[i]){
          case COUNT:
            m_reportResult[i]++;
            break;
          case MAX:
            m_reportResult[i] = m_outputrow<=m_limitbottom-1?atof(datas[i].c_str()):max(m_reportResult[i],atof(datas[i].c_str()));
            break;
          case MIN:
            m_reportResult[i] = m_outputrow<=m_limitbottom-1?atof(datas[i].c_str()):min(m_reportResult[i],atof(datas[i].c_str()));
            break;
          case AVERAGE:
          case SUM:
          default:
            m_reportResult[i]+=atof(datas[i].c_str());
            break;
        }
      }
    }
  }
}

void QuerierC::output()
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  //printf("Result Num: %d\n",m_results.size());
  //size_t iRowNumFromCols = 0;
  //for (size_t i=0; i<m_colToRows.size(); i++)
  //  iRowNumFromCols = max(iRowNumFromCols, (int)m_colToRows[i].size());
  for (size_t i=0; i<m_results.size(); i++){
    genReport(m_results[i]);
    formatoutput(m_results[i], i);
  }
#ifdef __DEBUG__
  m_outputtime += (curtime()-thistime);
#endif // __DEBUG__
}

long QuerierC::getMatchedCount()
{
  return m_matchcount;
}

long QuerierC::getOutputCount()
{
  return m_outputrow;
}

long QuerierC::getLines()
{
  return m_line;
}

size_t QuerierC::getRawStrSize()
{
  return m_rawstr.size();
}

void QuerierC::outputAndClean()
{
  applyExtraFilter();
  output();
  m_results.clear();
  m_resultfiles.clear();
}

void QuerierC::outputExtraInfo(const size_t & total, const bool & bPrintHeader)
{
  if (m_outputformat == JSON){
    if (m_outputrow>0)
      printf("\n");
    printf("\t],\n");
    if (m_reportResult.size()>0){
      outputstream(-1, "\t\"reports\": [\n");
      outputstream(-1, "\t\t{\n");
      for (size_t i=1; i<=m_selections.size(); i++){
        if (m_reportOps.find(i)!=m_reportOps.end() && m_reportResult.find(i)!=m_reportResult.end() && m_reportNames.find(i)!=m_reportNames.end()){
          if (m_reportOps[i] == AVERAGE)
            m_reportResult[i]=m_reportResult[i]/(m_outputrow-m_limitbottom+1);
          outputstream(-1, "\t\t\t\"%s\": %.2f",m_reportNames[i].c_str(),m_reportResult[i]);
        }else
          outputstream(-1, "\t\t\t\"\": \"\"");
        if (i == m_selections.size())
          outputstream(-1, "\n");
        else
          outputstream(-1, ",\n");
      }
      outputstream(-1, "\t\t}");
      outputstream(-1, "\t],\n");
    }
    //if (mode == READLINE)
    //  printf("\"ReadLines\": %d,\n", total);
    //else
    //  printf("\"ReadBytes\": %d,\n", total);
    printf("\t\"MatchedLines\": %ld,\n", m_line);
    printf("\t\"SelectedRows\": %ld\n", m_outputrow-m_limitbottom+1);
    printf("}\n");
  }else{
    if (m_reportResult.size()>0){
      for (size_t i=1; i<=m_selections.size(); i++){
        if (m_reportOps.find(i)!=m_reportOps.end() && m_reportResult.find(i)!=m_reportResult.end()){
          if (m_reportOps[i] == AVERAGE)
            m_reportResult[i]=m_reportResult[i]/(m_outputrow-m_limitbottom+1);
          outputstream(-1, "%.2f%s", m_reportResult[i],i<m_selections.size()?m_fielddelim.c_str():"");
        }else
          outputstream(-1, " %s", i<m_selections.size()?m_fielddelim.c_str():"");
      }
      outputstream(-1, "\n");
    }
    if (bPrintHeader){
      //if (mode == READLINE)
      //  printf("Read %d lines(s).\n", total);
      //else
      //  printf("Read %d byte(s).\n", total);
      printf("Pattern matched %ld line(s).\n", m_line);
      printf("Selected %ld row(s).\n", m_outputrow-m_limitbottom+1);
    }
  }
#ifdef __DEBUG__
  m_totaltime = curtime() - m_querystartat;
  trace(PERFM, "Total time: %d. Break down:\n", m_totaltime);
  trace(PERFM, ", Trial Analyze Columns time (detected rows: %d): %d\n", m_detectedTypeRows, m_trialanalyzetime);
  trace(PERFM, ", Searching (matched rows %d) time: %d. Break down:\n", m_line, m_searchtime);
  trace(PERFM, ",   Analyze raw content time: %d\n", m_rawanalyzetime);
  trace(PERFM, ",     Read raw line time: %d\n", m_readrawlinetime);
  trace(PERFM, ",     Read line process trim time: %d\n", m_rawprocesstrimtime);
  trace(PERFM, ",     Raw content pattern parsing time: %d\n", m_parsepatterntime);

  trace(PERFM, ",   eval expression and filtering time: %d. Break down:\n", m_filtertime);
  trace(PERFM, ",     Run time data for evaling prepare time: %d\n", m_runtimedatapreparetime);
  trace(PERFM, ",     Dynamic user variable calculation time: %d\n", m_uservarcaltime);
  trace(PERFM, ",     filter compare time: %d\n", m_filtercomptime);
  trace(PERFM, ",     Sidework processing time: %d\n", m_sideworktime);
  trace(PERFM, ",     Get sidework row time: %d\n", m_getsidedatarowtime);
  trace(PERFM, ",     Save tree data time: %d\n", m_savetreedatatime);
  trace(PERFM, ",     eval matched data to selection/sortkey/analytic data time: %d\n", m_evalRawdatatime);
  trace(PERFM, ",       eval matched data prepare evaling time: %d\n", m_evalRawpreptime);
  trace(PERFM, ",       eval matched data duplicates & 1-N join time: %d\n", m_evalRawdupjointime);
  trace(PERFM, ",       eval selection time: %d\n", m_evalSeltime);
  trace(PERFM, ",         eval selection declare variable time: %d\n", m_evalSeldeclaretime);
  trace(PERFM, ",         eval selection assign expression time: %d\n", m_evalSelAssignExprtime);
  trace(PERFM, ",         eval selection call eval time: %d\n", m_evalSelRealEvaltime);
  trace(PERFM, ",       append selection to result set time: %d\n", m_appendSelToResulttime);
  trace(PERFM, ",       assign output file time: %d\n", m_assignoutputfiletime);
  trace(PERFM, ",       eval sort time: %d\n", m_evalSorttime);
  trace(PERFM, ",       eval analytic expression time: %d\n", m_evalanaexprtime);
  trace(PERFM, ",       add analytic data time: %d\n", m_addanafuncdatatime);
  trace(PERFM, ",     eval group key time: %d\n", m_evalGroupKeytime);
  trace(PERFM, ",     update restult time: %d\n", m_updateResulttime);
  trace(PERFM, ",     append non-selection result time: %d\n", m_appendnonselresulttime);

  trace(PERFM, ", extra filtering time: %d\n", m_extrafiltertime);
  trace(PERFM, ", sorting time: %d\n", m_sorttime);
  trace(PERFM, ", constructing tree time: %d\n", m_treetime);
  trace(PERFM, ", unique time: %d\n",  m_uniquetime);
  trace(PERFM, ", aggregation time: %d\n", m_grouptime);
  trace(PERFM, ", analytic time: %d\n", m_analytictime);
  trace(PERFM, ", extra filter compare time: %d\n", m_extrafiltercomptime);
  trace(PERFM, ", prepare Agg GP time: %d\n", m_prepAggGPtime);
  trace(PERFM, ", eval agg expression time: %d\n", m_evalAggExptime);
  trace(PERFM, ", Output time: %d\n", m_outputtime);
  trace(PERFM, ", Check is date time: %d\n", g_checkisdatetime);
  trace(PERFM, ", Truncate date time: %d\n", g_truncdatetime);
  trace(PERFM, ", Convert string to date time: %d\n", g_strtodatetime);
  trace(PERFM, ", Convert date to string time: %d\n", g_datetostrtime);
  trace(PERFM, ", Zonetime time: %d\n", g_zonetimetime);
  trace(PERFM, ", Temp timer 1 time: %d\n", g_tmptimer1time);
  trace(PERFM, ", Temp timer 2 time: %d\n", g_tmptimer2time);
  trace(PERFM, ", Eval expression time: %d. Break down:\n", g_evalexprtime);
  trace(PERFM, ",   Eval const expression time: %d\n", g_evalexprconsttime);
  trace(PERFM, ",   Eval function expression time: %d.\n", g_evalexprfunctime);
  trace(PERFM, ",   Eval variable expression time: %d\n", g_evalexprvartime);
  trace(PERFM, ",   Eval column expression time: %d\n", g_evalexprcoltime);
  trace(PERFM, ",   Eval macro parameter expression time: %d\n", g_evalexprmacpatime);
  trace(PERFM, ",   Eval expression calculation time: %d\n", g_evalexprcaltime);
  trace(PERFM, ", matched lines: %d\n", m_line);
  //trace(PERFM, "Total time: %d. Break down:\n, Searching time: %d\n, Read raw content time: %d\n, Split raw content time: %d\n, Analyze raw content(analyzed rows %d) time: %d\n, Trial Analyze Columns time: %d\n, filtering time: %d\n, extra filtering time: %d\n, sorting time: %d\n, constructing tree time: %d\n, unique time: %d\n, aggregation time: %d\n, analytic time: %d\n, eval group key time: %d\n, filter compare time: %d\n, extra filter compare time: %d\n, prepare Agg GP time: %d\n, eval agg expression time: %d\n, eval selection time: %d\n, eval sort time: %d\n, update restult time: %d\n, Output time: %d\n, matched lines: %d\n", m_totaltime, m_searchtime, m_rawprocesstrimtime, m_parsepatterntime, m_line,m_rawanalyzetime, m_trialanalyzetime, m_filtertime, m_extrafiltertime, m_sorttime, m_treetime,  m_uniquetime, m_grouptime, m_analytictime, m_evalGroupKeytime, m_filtercomptime, m_extrafiltercomptime, m_prepAggGPtime, m_evalAggExptime, m_evalRawdatatime, m_evalSorttime, m_updateResulttime, m_outputtime, m_line);
#endif // __DEBUG__
}

void QuerierC::clearGroup()
{
  // need to manually free the memory of GroupProp, as it's not freed in the destructor to improve performance.
  for (unordered_map< vector<string>, unordered_map< string,GroupProp >, hash_container< vector<string> > >::iterator it=m_aggGroupProp.begin(); it!=m_aggGroupProp.end(); ++it){
    clearGroupPropMap(it->second);
  }
  clearGroupPropMap(m_initAggProps);

  m_groups.clear();
  m_aggRowValues.clear();
  m_groupKeys.clear();
  m_aggGroupProp.clear();
  m_initAggProps.clear();
  m_aggFuncExps.clear();
  //m_aggFuncTaget.clear();
  m_aggGroupDataidMap.clear();
}

void QuerierC::clearSort()
{
  m_sorts.clear();
  m_sortKeys.clear();
}

void QuerierC::clearTree()
{
  m_treeProps.clear();
  m_treeParentProps.clear();
  m_treeKeys.clear();
  m_treeParentKeys.clear();
}

void QuerierC::clearReport()
{
  m_reportOps.clear();
  m_reportResult.clear();
  m_reportNames.clear();
}

void QuerierC::clearAnalytic()
{
  m_initAnaArray.clear();
  m_anaEvaledExp.clear();
  m_anaSortProps.clear();
  m_anaSortData.clear();
  m_anaFuncResult.clear();
  //m_analyticresults.clear();
}

void QuerierC::clearFilter()
{
  if (m_filter){
    m_filter->clear();
    SafeDelete(m_filter);
  }
  if (m_extrafilter){
    m_extrafilter->clear();
    SafeDelete(m_extrafilter);
  }
}

void QuerierC::clearDuplicate()
{
  if (m_dupfilter){
    m_dupfilter->clear();
    SafeDelete(m_dupfilter);
  }
  if (m_dupnumexp){
    m_dupnumexp->clear();
    SafeDelete(m_dupnumexp);
  }
}

void QuerierC::clearOutputfile()
{
  for (unordered_map< string, ofstream* >::iterator it=m_outputfiles.begin();it!=m_outputfiles.end();it++){
    if (it->second && it->second->is_open()){
      it->second->close();
      SafeDelete(it->second);
    }
  }
  m_outputfiles.clear();
  if (m_outputfileexp){
    m_outputfileexp->clear();
    SafeDelete(m_outputfileexp);
  }
  m_resultfiles.clear();
}

void QuerierC::clearUservars()
{
  m_uservarnames.clear();
  m_uservariables.clear();
  m_uservarinitval.clear();
  m_uservarexprs.clear();
  m_fakeRExprs.clear();
  for (unordered_map< string,MacroFuncStruct >::iterator it=m_userMacroExprs.begin(); it!=m_userMacroExprs.end(); it++){
    it->second.funcExpr->clear();
    SafeDelete(it->second.funcExpr);
  }
  m_userMacroExprs.clear();
}

void QuerierC::clearAllCommands()
{
  clearGroup();
  clearAnalytic();
  clearSort();
  clearTree();
  clearReport();
  clearFilter();
  clearDuplicate();
  clearOutputfile();
  clearUservars();
  m_results.clear();
  m_fieldnames.clear();
  m_fieldInitNames.clear();
  m_fieldtypes.clear();
  m_trimmedFieldtypes.clear();
  m_fieldntypes.clear();
  //m_userMacroParas.clear();
  m_selnames.clear();
  m_sideSelections.clear();
  m_sideAlias.clear();
  m_sideFilters.clear();
  m_sideDatasets.clear();
  m_sideDatatypes.clear();
  m_trimedSelctions.clear();
  m_trimedInitSels.clear();
  m_trimmedAlias.clear();
  m_colToRows.clear();
  m_colToRowNames.clear();
  m_delimSet.clear();
}

void QuerierC::clear()
{
  clearAllCommands();
  m_selections.clear();
  m_searchMode = REGSEARCH;
  m_readmode = READLINE;
  m_uservarstr = "";
  m_quoters = "";
  m_nameline = false;
  m_bEof = false;
  m_quickwildcard = false;
  m_quickregular = false;
  m_delmrepeatable = false;
  m_delmkeepspace = false;
  m_eliminateDupField = false;
  m_delmMulitChar = false;
  m_bNamePrinted = false;
  m_aggrOnly = false;
  m_bUniqueResult = false;
  m_bSelectContainMacro = false;
  m_bToAnalyzeSelectMacro = false;
  m_bSortContainMacro = false;
  m_bToAnalyzeSortMacro = false;
  m_bTextOnly = false;
  m_bStatOnly = false;
  m_bDetectAllOnChange = false;
  m_selstr = "";
  m_sortstr = "";
  m_detectTypeMaxRowNum = 1;
  m_detectedTypeRows = 0;
  m_outputformat = TEXT;
  m_filter = NULL;
  m_extrafilter = NULL;
  m_dupnumexp = NULL;
  m_dupfilter = NULL;
  m_limitbottom = 1;
  m_limittop = -1;
  m_filename = "";
  m_fileid = 0;
  m_fileline = 0;
  init();
}
