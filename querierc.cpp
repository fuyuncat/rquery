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

QuerierC::QuerierC()
{
  init();
}

QuerierC::QuerierC(string regexstr)
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
  m_rawstr = "";
  m_searchflags = regex_constants::match_default;
  m_searchMode = REGSEARCH;
  m_readmode = READBUFF;
  m_bEof = false;
  m_delmrepeatable = false;
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
  m_selstr = "";
  m_sortstr = "";
  m_fielddelim = "\t";
  m_detectTypeMaxRowNum = 1;
  m_detectedTypeRows = 0;
#ifdef __DEBUG__
  m_querystartat = curtime();
  m_totaltime = 0;
  m_searchtime = 0;
  m_rawreadtime = 0;
  m_rawsplittime = 0;
  m_rawanalyzetime = 0;
  m_filtertime = 0;
  m_sorttime = 0;
  m_uniquetime = 0;
  m_grouptime = 0;
  m_analytictime = 0;
  m_evalGroupKeytime = 0;
  m_filtercomptime = 0;
  m_prepAggGPtime = 0;
  m_evalAggExptime = 0;
  m_evalSeltime = 0;
  m_evalSorttime = 0;
  m_updateResulttime = 0;
  m_outputtime = 0;
#endif // __DEBUG__
}

void QuerierC::setregexp(string regexstr)
{
  if (trim_copy(regexstr).empty()){
    m_searchMode = DELMSEARCH;
    m_delmrepeatable = false;
    m_regexstr = trim_copy(regexstr);
  }else if ((regexstr[0] == 'w' || regexstr[0] == 'W')&& regexstr[1] == '/' && regexstr[regexstr.length()-1] == '/'){
    m_searchMode = WILDSEARCH;
    vector<string> vSearchPattern = split(regexstr.substr(2,regexstr.length()-3),'/',"",'\\',{'(',')'},false,true);
    if (vSearchPattern.size() == 0){
      trace(ERROR, "(1)'%s' is an unrecognized pattern!\n", regexstr.c_str());
      return;
    }
    m_regexstr = vSearchPattern[0];
    replacestr(m_regexstr,{"\\t","\\/","\\v","\\|"},{"\t","/","\v","|"});
    if (vSearchPattern.size()>1){
      if (vSearchPattern[1].length()%2 != 0)
        trace(ERROR, "(1)Quoters must be paired. '%s' will be ignored.\n", vSearchPattern[1].c_str());
      else
        m_quoters = vSearchPattern[1];
    }
  }else if ((regexstr[0] == 'd' || regexstr[0] == 'D') && regexstr[1] == '/' && (regexstr[regexstr.length()-1] == '/' || (regexstr.length()>3 && regexstr[regexstr.length()-2] == '/'))){
    m_searchMode = DELMSEARCH;
    m_delmrepeatable = false;
    string spattern = "";
    if (regexstr[regexstr.length()-1] == '/')
      spattern = regexstr.substr(2,regexstr.length()-3);
    else{
      if (regexstr.length()>3 && regexstr[regexstr.length()-2] == '/'){
        if (regexstr[regexstr.length()-1] == 'r' || regexstr[regexstr.length()-1] == 'R')
          m_delmrepeatable = true;
      }else
        trace(FATAL, "'%s' is not a valid searching pattern\n",regexstr.c_str());
      spattern = regexstr.substr(2,regexstr.length()-4);
    }
    vector<string> vSearchPattern = split(spattern,'/',"",'\\',{'(',')'},false,true);
    if (vSearchPattern.size() == 0){
      trace(ERROR, "(1)'%s' is an unrecognized pattern!\n", regexstr.c_str());
      return;
    }
    m_regexstr = vSearchPattern[0];
    replacestr(m_regexstr,{"\\t","\\/","\\v","\\|"},{"\t","/","\v","|"});
    if (vSearchPattern.size()>1){
      if (vSearchPattern[1].length()%2 != 0)
        trace(ERROR, "(2)Quoters must be paired. '%s' will be ignored.\n", vSearchPattern[1].c_str());
      else
        m_quoters = vSearchPattern[1];
    }
  }else if (regexstr[0] == '/' && regexstr[regexstr.length()-1] == '/'){
    m_searchMode = REGSEARCH;
    m_regexstr = trim_one(regexstr, '/');
    try{
      m_regexp = sregex::compile(m_regexstr);
      m_matches = namesaving_smatch(m_regexstr);
    }catch (exception& e) {
      trace(FATAL, "Regular pattern compile exception: %s\n", e.what());
    }
  }else
    trace(FATAL, "Searching pattern '%s' is invalid!\n", regexstr.c_str());
  trace(DEBUG, "Parsed searching string '%s' => '%s', search mode is %d\n", regexstr.c_str(),m_regexstr.c_str(),m_searchMode);
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

void QuerierC::appendrawstr(string rawstr)
{
  m_rawstr.append(rawstr);
}

void QuerierC::setrawstr(string rawstr)
{
  m_rawstr = rawstr;
}

void QuerierC::setReadmode(short int readmode)
{
  m_readmode = readmode;
}

void QuerierC::setEof(bool bEof)
{
  m_bEof = bEof;
}

bool QuerierC::assignGroupStr(string groupstr)
{
  vector<string> vGroups = split(groupstr,',',"''()",'\\',{'(',')'},false,true);
  ExpressionC eGroup;
  for (int i=0; i<vGroups.size(); i++){
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

void QuerierC::setUniqueResult(bool bUnique)
{
  m_bUniqueResult = bUnique;
}

void QuerierC::setDetectTypeMaxRowNum(int detectTypeMaxRowNum)
{
  m_detectTypeMaxRowNum = max(1,detectTypeMaxRowNum);
}

void QuerierC::setNameline(bool nameline)
{
  m_nameline = nameline;
}

bool QuerierC::assignLimitStr(string limitstr)
{
  vector<string> vLimits = split(limitstr,',',"''()",'\\',{'(',')'},false,true);
  string sFirst = trim_copy(vLimits[0]);
  int iFirst = 0;
  if (isInt(sFirst))
    iFirst = atoi(sFirst.c_str());
  else{
    trace(ERROR, "%s is not a valid limit number!\n", sFirst.c_str());
    return false;
  }
  if (vLimits.size() > 1){
    string sSecond = trim_copy(vLimits[1]);
    if (isInt(sSecond)){
      m_limitbottom = iFirst;
      m_limittop = atoi(sSecond.c_str());
    }else{
      trace(ERROR, "%s is not a valid limit number!\n", sSecond.c_str());
      return false;
    }
  }else
    m_limittop = iFirst;

  trace(DEBUG1, "Limits is from %d to %d\n", m_limitbottom, m_limittop);
  return true;
}

// m_groups should always be analyzed before m_selections
bool QuerierC::assignSelString(string selstr)
{
  m_selstr = selstr;
  return analyzeSelString();
}

bool QuerierC::checkSelGroupConflict(const ExpressionC & eSel)
{
  vector<string> allColNames;
  for (int i=0; i<m_groups.size(); i++)
    m_groups[i].getAllColumnNames(allColNames);
  if (m_groups.size()>0 && !eSel.groupFuncOnly() && !eSel.inColNamesRange(allColNames)){
    trace(WARNING, "Selection '%s' does not exist in Group or invalid using aggregation function \n", eSel.m_expStr.c_str());
    return false;
  }
  return true;
}

vector<ExpressionC> QuerierC::genSelExpression(string sSel, vector<string> & vAlias)
{
  vector<ExpressionC> vSelections;
  trace(DEBUG, "Analyzing selections from '%s'!\n", sSel.c_str());
  vector<string> vSelStrs = split(sSel,',',"''()",'\\',{'(',')'},false,true);
  string sAlias;
  ExpressionC eSel;
  for (int i=0; i<vSelStrs.size(); i++){
    trace(DEBUG, "Processing selection(%d) '%s'!\n", i, vSelStrs[i].c_str());
    string sSel = trim_copy(vSelStrs[i]);
    if (sSel.empty()){
      trace(ERROR, "Empty selection string!\n");
      continue;
    }
    vector<string> vSelAlias = split(sSel," as ","''()",'\\',{'(',')'},false,true);
    sAlias = "";
    if (vSelAlias.size()>1)
      sAlias = upper_copy(trim_copy(vSelAlias[1]));
    eSel = ExpressionC(vSelAlias[0]);
    //trace(DEBUG2,"Selection expression: '%s'\n",vSelAlias[0].c_str());
    //eSel.dump();
    vSelections.push_back(eSel);
    vAlias.push_back(sAlias);
  }
  return vSelections;
}

bool QuerierC::analyzeSelString(){
  trace(DEBUG, "Analyzing selections from '%s'!\n", m_selstr.c_str());
  m_selections.clear();
  m_selnames.clear();
  vector<ExpressionC> vExpandedExpr; 
  vector<string> vAlias;
  m_selections = genSelExpression(m_selstr, vAlias);
  for (int i=0; i<m_selections.size(); i++)
    if (i<vAlias.size()&& !vAlias[i].empty())
      m_selnames.push_back(vAlias[i]);
    else
      m_selnames.push_back(m_selections[i].getEntireExpstr());
  bool bGroupFunc = false, bNonGroupFuncSel = false;
  for (int i=0; i<m_selections.size(); i++){
    // transfer parameters of COLTOROW macro function to selections
    if (m_selections[i].m_type == LEAF && m_selections[i].m_expType == FUNCTION && m_selections[i].m_Function && m_selections[i].m_Function->m_funcID==COLTOROW){
      m_colToRowNames.push_back(m_selnames[i]);
      vector<int> colToRowSels;
      m_selnames.erase(m_selnames.begin()+i);
      for (int j=0; j<m_selections[i].m_Function->m_params.size(); j++){
        m_selnames.insert(m_selnames.begin()+i+j,m_selections[i].m_Function->m_params[j].getEntireExpstr());
        colToRowSels.push_back(i+j);
      }
      vector<ExpressionC> params = m_selections[i].m_Function->m_params;
      m_selections.erase(m_selections.begin()+i);
      m_selections.insert(m_selections.begin()+i,params.begin(),params.end());
      m_colToRows.push_back(colToRowSels);
    }
    if (m_selections[i].m_type == LEAF && m_selections[i].m_expType == FUNCTION && m_selections[i].m_Function && m_selections[i].m_Function->isAnalytic())
      trace(DEBUG,"QuerierC: The analytic function '%s' group size %d, param size %d \n", m_selections[i].m_Function->m_expStr.c_str(),m_selections[i].m_Function->m_anaParaNums[0],m_selections[i].m_Function->m_params.size());
    //trace(DEBUG, "Got selection expression '%s'!\n", m_selections[i].getEntireExpstr().c_str());

    // if macro function is involved, need to wait util the first data analyzed to analyze select expression
    if (m_selections[i].m_expType == FUNCTION && m_selections[i].m_Function && m_selections[i].m_Function->m_funcID==FOREACH){
      if (m_fieldtypes.size()==0){
        m_bSelectContainMacro = true;
        m_bToAnalyzeSelectMacro = true;
        m_selections.clear();
        m_selnames.clear();
        m_colToRows.clear();
        trace(DEBUG2,"Skiping select FOREACH: '%s'\n",m_selections[i].m_expStr.c_str());
        return true;
      }else{
        switch(m_selections[i].m_Function->m_funcID){
          case FOREACH:{
            if (m_groups.size()>0)
              vExpandedExpr = m_selections[i].m_Function->expandForeach(m_groups);
            else
              vExpandedExpr = m_selections[i].m_Function->expandForeach(m_fieldtypes.size());
            trace(DEBUG2,"Expanding FOREACH: '%s'\n",m_selections[i].m_expStr.c_str());
            m_selections.erase(m_selections.begin()+i);
            m_selnames.erase(m_selnames.begin()+i);
            int l=0, k=0;
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
            for (int j=0; j<vExpandedExpr.size(); j++){
              trace(DEBUG2,"Expanded FOREACH expression: '%s'\n",vExpandedExpr[j].m_expStr.c_str());
              if (bToConvColToRow)
                m_colToRows[l].insert(m_colToRows[l].begin()+k+j,i);
              m_selnames.insert(m_selnames.begin()+i,vExpandedExpr[j].getEntireExpstr());
              //m_selnames.push_back(vExpandedExpr[j].getEntireExpstr());
              checkSelGroupConflict(vExpandedExpr[j]);
              if (vExpandedExpr[j].containGroupFunc())
                bGroupFunc = true;
              else
                bNonGroupFuncSel = true;
              vExpandedExpr[j].getAggFuncs(m_initAggProps);
              vExpandedExpr[j].getAnaFuncs(m_initAnaArray, m_anaFuncParaNums);
              m_selections.insert(m_selections.begin()+i,vExpandedExpr[j]);
              i++;
            }
            m_bToAnalyzeSelectMacro = false;
            break;
          }
        }
        continue;
      }
    }

    checkSelGroupConflict(m_selections[i]);
    if (m_selections[i].containGroupFunc())
      bGroupFunc = true;
    else
      bNonGroupFuncSel = true;

    m_selections[i].getAggFuncs(m_initAggProps);
    m_selections[i].getAnaFuncs(m_initAnaArray, m_anaFuncParaNums);
    //trace(DEBUG, "Selection: '%s'!\n", m_selections[i].getEntireExpstr().c_str());
  }
  m_aggrOnly = (bGroupFunc && !bNonGroupFuncSel && m_groups.size()==0);
  if (bGroupFunc && bNonGroupFuncSel && m_groups.size() == 0){ // selection has non-aggregation function but non group field.
    trace(FATAL, "Invalid using aggregation function in selection, no group involved!\n");
    return false;
  }
  return true;
}

// m_groups, m_selections should always be analyzed before m_sorts
bool QuerierC::assignSortStr(string sortstr)
{
  m_sortstr = sortstr;
  return analyzeSortStr();
}

bool QuerierC::checkSortGroupConflict(const ExpressionC & eSort)
{
  if (m_groups.size() > 0) {// checking if compatible with GROUP
    vector<string> allColNames;
    for (int i=0; i<m_groups.size(); i++)
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
  if (m_bToAnalyzeSelectMacro)
    return true;
  m_sorts.clear();
  trace(DEBUG, "Processing sorting string '%s'!\n", m_sortstr.c_str());
  vector<string> vSorts = split(m_sortstr,',',"''()",'\\',{'(',')'},false,true);
  SortProp keyProp;
  for (int i=0; i<vSorts.size(); i++){
    keyProp = SortProp();
    trace(DEBUG, "Processing sorting keys (%d) '%s'!\n", i, vSorts[i].c_str());
    string sSort = trim_copy(vSorts[i]);
    if (sSort.empty()){
      trace(ERROR, "Empty sorting key!\n");
      return false;
    }
    vector<string> vKP = split(sSort,' ',"''()",'\\',{'(',')'},false,true);
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
              vExpandedExpr = keyProp.sortKey.m_Function->expandForeach(m_groups);
            else
              vExpandedExpr = keyProp.sortKey.m_Function->expandForeach(m_fieldtypes.size());
            SortProp keyPropE;
            for (int j=0; j<vExpandedExpr.size(); j++){
              keyPropE = SortProp();
              vKP = split(vExpandedExpr[j].getEntireExpstr(),' ',"''()",'\\',{'(',')'},false,true);
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
    if (keyProp.sortKey.m_type==BRANCH || keyProp.sortKey.m_expType != CONST || (!isInt(keyProp.sortKey.m_expStr) && !isLong(keyProp.sortKey.m_expStr)) || atoi(keyProp.sortKey.m_expStr.c_str())>=m_selections.size()){
      m_sorts.push_back(keyProp);
    }
  }
  for (int i=0; i<m_sorts.size(); i++)
    trace(DEBUG, "Sorting key '%s'(%d) !\n",m_sorts[i].sortKey.getEntireExpstr().c_str(),i);
  //trace(DEBUG1, "Got %d sorting keys!\n",m_sorts.size());
  return true;
}

bool QuerierC::assignMeanwhileString(string mwstr)
{
  vector<string> vSideWorks = split(mwstr,';',"''()",'\\',{'(',')'},false,true);
  for (int i=0; i<vSideWorks.size(); i++){
    size_t pos = findFirstSub(vSideWorks[i], " WHERE ", 0,"''()",'\\',{'(',')'},false );
    FilterC filter;
    vector<ExpressionC> vSelections;
    vector<string> vAlias;
    if (pos != string::npos){
      filter.setExpstr(trim_copy(vSideWorks[i].substr(pos+string(" WHERE ").length())));
      vSelections = genSelExpression(trim_copy(vSideWorks[i].substr(0,pos)), vAlias);
    }else{
      filter.setExpstr("1=1");
      vSelections = genSelExpression(trim_copy(vSideWorks[i]), vAlias);
    }
    m_sideFilters.push_back(filter);
    m_sideSelections.push_back(vSelections);
    m_sideAlias.push_back(vAlias);
  }
  return true;
}

bool QuerierC::setFieldTypeFromStr(string setstr)
{
  vector<string> vSetFields = split(setstr,',',"''()",'\\',{'(',')'},false,true);
  string fieldname = "";
  for (int i=0; i<vSetFields.size(); i++){
    vector<string> vField = split(vSetFields[i],' ',"''()",'\\',{'(',')'},false,true);
    vField = vField.size()>=2?vField:split(vSetFields[i],'\t',"''()",'\\',{'(',')'},false,true);
    if (vField[0].empty()){
      trace(ERROR, "Field name is empty!\n");
      return false;
    }
    if (vField.size()<2){
      trace(ERROR, "SET field type failed! Correct format is SET <FIELD> <TYPE>!\n");
      return false;
    }
    int iType = encodeDatatype(vField[1]);
    fieldname = vField[0];
    if (vField[0][0]='@' && vField[0].length()>1 && isInt(vField[0].substr(1))) // convert filed abbrevasion
      fieldname = "@FIELD"+vField[0].substr(1);
    if (iType == UNKNOWN){
      trace(ERROR, "Unknown data type %s!\n", vField[1].c_str());
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

void QuerierC::setUserVars(string variables)
{
  trace(DEBUG, "Setting variables from '%s' !\n", variables.c_str());
  vector<string> vVariables = split(variables,';',"''()",'\\',{'(',')'},false,true);
  for (int i=0; i<vVariables.size(); i++){
    vector<string> vNameVal = split(trim_copy(vVariables[i]),':',"''()",'\\',{'(',')'},false,true);
    if (vNameVal.size()<2){
      trace(ERROR, "Incorrect variable format!\n", vVariables[i].c_str());
      continue;
    }
    string sName=upper_copy(trim_copy(vNameVal[0])), sValue=trim_copy(vNameVal[1]);
    trace(DEBUG, "Setting variable '%s' value '%s'!\n", sName.c_str(), sValue.c_str());
    if (sName.compare("RAW")==0 || sName.compare("ROW")==0 || sName.compare("FILE")==0 || sName.compare("LINE")==0 || sName.compare("FILEID")==0 || sName.compare("FILELINE")==0 || sName.compare("N")==0 || sName.compare("%")==0 || sName.compare("R")==0){
      trace(ERROR, "%s is a reserved word, cannot be used as a variable name!\n", sName.c_str());
      continue;
    }
    m_uservariables.insert(pair<string, string> ("@"+sName,sValue));
    if (vNameVal.size()>2){ // dynamic variable has an expression
      ExpressionC tmpExp = ExpressionC(trim_copy(vNameVal[2]));
      m_uservarexprs.insert(pair<string, ExpressionC> ("@"+sName,tmpExp));
    }
  }
}

void QuerierC::setFileName(string filename)
{
  m_filename = filename;
  m_fileid++;
  m_fileline = 0;
  m_detectedTypeRows = 0;  // force to detect data type again
}

void QuerierC::setOutputFormat(short int format)
{
  m_outputformat = format;
}

void QuerierC::setFieldDelim(string delimstr)
{
  m_fielddelim = delimstr;
  replacestr(m_fielddelim,{"\\t","\\v","\\n","\\r"},{"\t","\v","\n","\r"});
}

void QuerierC::setOutputFiles(string outputfilestr, short int outputmode)
{
  if (m_outputfileexp)
    SafeDelete(m_outputfileexp);
  m_outputfileexp = new ExpressionC(outputfilestr);
  m_outputmode = outputmode;
}

bool QuerierC::toGroupOrSort()
{
  return (m_groups.size()>0 || m_sorts.size()>0 || m_initAnaArray.size()>0 || m_aggrOnly || m_bUniqueResult);
}

void QuerierC::pairFiledNames(namesaving_smatch matches)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 

  for (int i=1; i<matches.size(); i++){
    bool foundName = false;
    for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      if (&(matches[i]) == &(matches[*it])){
        m_fieldnames.push_back(upper_copy(string(*it)));
        foundName = true;
      }
    if (!foundName){
      m_fieldnames.push_back("@FIELD"+intToStr(i));
    }
  }
}

void QuerierC::setFieldDatatype(string field, int datetype, string extrainfo)
{
  string fname = upper_copy(field);
  DataTypeStruct dts;
  dts.datatype = datetype;
  dts.extrainfo = extrainfo;
  if (m_fieldntypes.find(fname) != m_fieldntypes.end())
    m_fieldntypes[fname] = dts;
  else
    m_fieldntypes.insert( pair<string, DataTypeStruct>(fname,dts) );
  for (int i=0;i<m_fieldnames.size();i++){
    if (fname.compare(m_fieldnames[i]) == 0 && i<m_fieldtypes.size())
      m_fieldtypes[i] = dts;
  }
}

void QuerierC::analyzeFiledTypes(vector<string> matches)
{
  m_detectedTypeRows++;
  m_fieldtypes.clear();
  DataTypeStruct dts;
  dts.datatype = detectDataType(matches[0], dts.extrainfo);
  dts.datatype = dts.datatype==UNKNOWN?STRING:dts.datatype;
  if (m_rawDatatype.datatype == UNKNOWN)
    m_rawDatatype = dts;
  else
    m_rawDatatype = getCompatibleDataType(m_rawDatatype, dts);
  trace(DEBUG,"Detected @RAW '%s', data type '%s' extrainfo '%s'.\n",string(matches[0]).c_str(),decodeDatatype(m_rawDatatype.datatype).c_str(),m_rawDatatype.extrainfo.c_str());
  if (m_rawDatatype.datatype == UNKNOWN)
    m_rawDatatype.datatype = STRING;
  for (int i=1; i<matches.size(); i++){
    dts = DataTypeStruct();
    if (m_fieldnames.size()>i-1 && m_fieldntypes.find(m_fieldnames[i-1]) != m_fieldntypes.end()) 
      m_fieldtypes.push_back(m_fieldntypes[m_fieldnames[i-1]]);
    else if (m_fieldntypes.find("@FIELD"+intToStr(i)) != m_fieldntypes.end())
      m_fieldtypes.push_back(m_fieldntypes["@FIELD"+intToStr(i)]);
    else{
      dts.datatype = detectDataType(matches[i], dts.extrainfo);
      dts.datatype = dts.datatype==UNKNOWN?STRING:dts.datatype;
      // trace(DEBUG2, "Detected column '%s' from '%s', data type '%s' extrainfo '%s'\n", m_fieldnames[i-1].c_str(), string(matches[i]).c_str(), decodeDatatype(dts.datatype).c_str(), dts.extrainfo.c_str());
      if (m_fieldtypes.size()>i-1)
        m_fieldtypes[i-1] = getCompatibleDataType(m_fieldtypes[i-1], dts);
      else
        m_fieldtypes.push_back(dts); 
      if (m_fieldtypes[i-1].datatype==UNKNOWN)
        m_fieldtypes[i-1].datatype = STRING; // set UNKNOWN (real) data as STRING
      if (m_fieldnames.size()>i-1)
        m_fieldntypes.insert( pair<string, DataTypeStruct>(m_fieldnames[i-1],dts) );
    }
    trace(DEBUG, "Detected column '%s' data type '%s' extrainfo '%s'\n", m_fieldnames[i-1].c_str(), decodeDatatype(m_fieldtypes[m_fieldtypes.size()-1].datatype).c_str(),m_fieldtypes[m_fieldtypes.size()-1].extrainfo.c_str());
  }
}

void QuerierC::getSideDatarow(unordered_map< int,int > & sideMatchedRowIDs, unordered_map< string, unordered_map<string,string> > & matchedSideDatarow)
{
  matchedSideDatarow.clear();
  if (sideMatchedRowIDs.size() == 0){
    //trace(ERROR, "Side dataset vector size %d doesnot match side matched rowID vector size %d!\n", m_sideDatasets.size(), sideMatchedRowIDs.size());
    return;
  }
  for (int i=0; i<m_sideDatasets.size(); i++){
    //if (sideMatchedRowIDs.find(i) == sideMatchedRowIDs.end())
    //  continue;
    if (sideMatchedRowIDs[i] >= m_sideDatasets[i].size()){
      trace(ERROR, "%d is an invalid matched ID, side work(%d) data size is %d!\n", sideMatchedRowIDs[i], i, m_sideDatasets[i].size());
    }
    matchedSideDatarow.insert(pair<string, unordered_map<string,string> >(intToStr(i+1), m_sideDatasets[i][sideMatchedRowIDs[i]]));
  }
}

void QuerierC::evalAggExpNode(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp > & aggGroupProp, unordered_map< string, unordered_map<string,string> > & matchedSideDatarow)
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
    //trace(DEBUG2, "Eval aggregation function expression '%s'\n", ite->second.getEntireExpstr().c_str());
    if (ite->second.m_expType==FUNCTION && ite->second.m_Function && ite->second.m_Function->isAggFunc() && ite->second.m_Function->m_params.size()>0 && ite->second.m_Function->m_params[0].evalExpression(fieldvalues, varvalues, &aggGroupProp, &anaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, true)){
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
          if (anyDataCompare(sResult,it->second.max,dts)>0)
            it->second.max = sResult;
          //trace(DEBUG2," get '%s'\n",it->second.max.c_str());
          break;
        case MIN:
          if (anyDataCompare(sResult,it->second.min,dts)<0)
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

void QuerierC::addResultOutputFileMap(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, unordered_map< string, unordered_map<string,string> >* matchedSideDatarow)
{
  if (m_outputfileexp){
    DataTypeStruct dts;
    string sResult;
    m_outputfileexp->evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, matchedSideDatarow, &m_sideDatatypes, sResult, dts, true);
    unordered_map< string, ofstream* >::iterator it = m_outputfiles.find(sResult);
    if (it == m_outputfiles.end()){
      ofstream* ofs=new ofstream();
      ofs->open(sResult, std::ofstream::out | m_outputmode==OVERWRITE? ofstream::trunc : ofstream::app);
      m_outputfiles.insert(pair< string, ofstream* >(sResult, ofs));
      m_resultfiles.push_back(ofs);
    }else
      m_resultfiles.push_back(it->second);
  }
}

// add a data row to a result set
bool QuerierC::addResultToSet(vector<string>* fieldvalues, map<string,string>* varvalues, vector<string> rowValue, vector<ExpressionC> expressions, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, vector<ExpressionC>* anaEvaledExp, unordered_map< string, unordered_map<string,string> > & matchedSideDatarow, vector< vector<string> > & resultSet)
{
  vector<string> vResults;
  ExpressionC tmpExp;
  DataTypeStruct dts;
  string sResult;
  //vResults.push_back(rowValue[0]);
  vResults.push_back(""); // save memory
  for (int i=0; i<expressions.size(); i++){
    dts = DataTypeStruct();
    tmpExp = ExpressionC();
    if (!expressions[i].containGroupFunc()){
      tmpExp = expressions[i];
      //expressions[i].copyTo(&tmpExp);
      //trace(DEBUG, "addResultToSet: Before eval expression '%s':\n",tmpExp.getEntireExpstr().c_str());
      //tmpExp.dump();
      //expressions[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, true);
      if (expressions[i].containAnaFunc()) {// no actual result for analytic function yet. Keep the evaled expression
        tmpExp.evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, false);
        anaEvaledExp->push_back(tmpExp);
        trace(DEBUG, "addResultToSet: adding analytic function involved expression '%s':\n",expressions[i].getEntireExpstr().c_str());
      }else
        tmpExp.evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, true);
      //trace(DEBUG, "addResultToSet: After eval expression '%s':\n",tmpExp.getEntireExpstr().c_str());
      //tmpExp.dump();
      //trace(DEBUG, "eval '%s' => '%s'\n", expressions[i].getEntireExpstr().c_str(), sResult.c_str());
    }else{
      trace(ERROR, "(2)Invalid using aggregation function in '%s', no group involved!\n", expressions[i].getEntireExpstr().c_str());
      return false;
    }
    vResults.push_back(sResult);
  }
  resultSet.push_back(vResults);
  // add output file mapping with result.
  if (m_outputmode != STANDARD && &m_results == &resultSet && m_outputfileexp){
    addResultOutputFileMap(fieldvalues, varvalues, aggFuncs, anaFuncs, &matchedSideDatarow);
  }
  return true;
}

// add evaled analytic function processing data to m_anaSortProps&m_anaSortData&m_anaFuncResult
void QuerierC::addAnaFuncData(unordered_map< string,vector<string> > anaFuncData)
{
  unordered_map< string,string > anaFuncResult;
  // convert the stored analytic data to this format: <analytic_func_str:vector of analytic data> 
  vector<string> sortKey;
  vector< vector<string> > newData;
  vector<SortProp> sortProps;
  for (unordered_map< string,vector<string> >::iterator it=anaFuncData.begin(); it!=anaFuncData.end(); it++){
    sortKey.clear();
    int iAnaGroupNum = 0, iAnaSortNum = 0;
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
      for(int j=0;j<it->second.size();j++)
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
      vector<ExpressionC> funcExps;
      ExpressionC funExp;
      SortProp sp;
      //Add sort props, the second part of parameters of analytic functions is the sort keys.
      for(int j=0;j<it->second.size();j++){
        if (j<iAnaGroupNum || j>=iAnaGroupNum+iAnaSortNum*2) // all parts except the second part of analytic function parameters
          sortKey.push_back(it->second[j]);
        else { // second part of analytic function parameters, which are always sort keys for all analytic functions
          if ((j-iAnaGroupNum)%2==0)
            sortKey.push_back(it->second[j]);
          else{
            anaFunc = NULL;
            funcExps = m_anaEvaledExp[0];
            funExp = ExpressionC();
            for (int i=0; i<funcExps.size();i++){
              //trace(DEBUG,"Searching analytic function '%s' from '%s'\n",it->first.c_str(),funcExps[i].getEntireExpstr().c_str());
              //funcExps[i].dump();
              anaFunc = funcExps[i].getAnaFunc(it->first);
              if (anaFunc)
                break;
            }
            if (!anaFunc){
              trace(ERROR, "(1)Failed to find analytic function expression '%s'\n", it->first.c_str());
            }else{
              if (anaFunc->m_params.size() != it->second.size())
                trace(ERROR, "(1)Analytic function '%s' sortkey size %d doesnot match the function parameter size %d!\n", it->first.c_str(), it->second.size(), anaFunc->m_params.size());
              funExp = anaFunc->m_params[j-1];
            }
            sp.sortKey = funExp;
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
}

void QuerierC::doSideWorks(vector<string> * pfieldValues, map<string, string> * pvarValues, unordered_map< string,GroupProp > * paggGroupProp, unordered_map< string,vector<string> > * panaFuncData)
{
  if (m_sideSelections.size()>0 && m_sideSelections.size()==m_sideAlias.size() && m_sideSelections.size()==m_sideFilters.size()){
    unordered_map< int,int > sideMatchedRowIDs;
    unordered_map< string, unordered_map<string,string> > matchedSideDatarow;
    for(int i=0;i<m_sideSelections.size();i++){
      vector< unordered_map<string,string> > resultSet;
      sideMatchedRowIDs.clear();
      if (m_sideFilters[i].compareExpression(pfieldValues, pvarValues, paggGroupProp, panaFuncData, &m_sideDatasets, &m_sideDatatypes, sideMatchedRowIDs)){
        getSideDatarow(sideMatchedRowIDs, matchedSideDatarow);
        unordered_map<string,string> thisResult;
        for (int j=0; j<m_sideSelections[i].size(); j++){
          string sResult;
          DataTypeStruct dts;
          m_sideSelections[i][j].evalExpression(pfieldValues, pvarValues, paggGroupProp, panaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, true);
          thisResult.insert(pair<string,string>(m_sideAlias[i][j].empty()?intToStr(j+1):m_sideAlias[i][j], sResult));
        }
        if (m_sideDatasets.size()<=i){
          resultSet.push_back(thisResult);
          m_sideDatasets.push_back(resultSet);
        }else
          m_sideDatasets[i].push_back(thisResult);
      }
    }
  }
}

// filt a row data by filter. no predication mean true. comparasion failed means alway false
bool QuerierC::matchFilter(const vector<string> & rowValue)
{
  //if (!filter){
  //  //trace(INFO, "No filter defined\n");
  //  return true;
  //}
#ifdef __DEBUG__
  long int thistime = curtime();
  long int filterbegintime = thistime;
#endif // __DEBUG__
  if (rowValue.size() != m_fieldnames.size() + 4){ // field name number + 4 variables (@raw @line @row @fileline)
    trace(ERROR, "Filed number %d and value number %d dont match!\n", m_fieldnames.size(), rowValue.size());
    dumpVector(m_fieldnames);
    dumpVector(rowValue);
    return false;
  }  
  vector<string> fieldValues;
  map<string, string> varValues;
  for (int i=0; i<m_fieldnames.size(); i++)
    fieldValues.push_back(rowValue[i+1]);
    //fieldValues.insert( pair<string,string>(upper_copy(m_fieldnames[i]),rowValue[i+1]));
  varValues.insert( pair<string,string>("@RAW",rowValue[0]));
  varValues.insert( pair<string,string>("@FILE",m_filename));
  varValues.insert( pair<string,string>("@FILEID",intToStr(m_fileid)));
  varValues.insert( pair<string,string>("@LINE",rowValue[m_fieldnames.size()+1]));
  varValues.insert( pair<string,string>("@ROW",rowValue[m_fieldnames.size()+2]));
  varValues.insert( pair<string,string>("@FILELINE",rowValue[m_fieldnames.size()+3]));
  varValues.insert( pair<string,string>("@%",intToStr(m_fieldnames.size())));
  varValues.insert(m_uservariables.begin(), m_uservariables.end());
  unordered_map< string,GroupProp > aggGroupProp;
  unordered_map< string,vector<string> > anaFuncData;
  unordered_map< int,int > sideMatchedRowIDs;
  unordered_map< string, unordered_map<string,string> > matchedSideDatarow;

  //trace(DEBUG, "Filtering '%s' ", rowValue[0].c_str());
  //dumpMap(varValues);
  bool matched = (!m_filter || m_filter->compareExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, &m_sideDatasets, &m_sideDatatypes, sideMatchedRowIDs));
  if (matched) {// calculate dynamic variables
    string sResult;
    DataTypeStruct dts;
    for(map<string, ExpressionC>::iterator it=m_uservarexprs.begin(); it!=m_uservarexprs.end(); ++it){
      it->second.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, true);
      m_uservariables[it->first] = sResult;
    }
  }
  // doing side work
  doSideWorks(&fieldValues, &varValues, &aggGroupProp, &anaFuncData);
#ifdef __DEBUG__
  m_filtercomptime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
  //trace(DEBUG, " selected:%d (%d)! \n", matched, m_selections.size());
  if (matched){
    getSideDatarow(sideMatchedRowIDs, matchedSideDatarow);
    string sResult;
    DataTypeStruct dts;
    vector<ExpressionC> anaEvaledExp;
    if (m_selections.size()>0){
      // initialize an empty anaFuncData
      vector<string> vEvaledParams;
      for (unordered_map< string,vector<ExpressionC> >::iterator it=m_initAnaArray.begin(); it!=m_initAnaArray.end(); ++it){
        vEvaledParams.clear();
        anaFuncData.insert(pair< string,vector<string> >(it->first,vEvaledParams));
      }
      ExpressionC tmpExp;
      if (m_groups.size() == 0 && m_initAggProps.size() == 0 && !m_aggrOnly){
        //trace(DEBUG, " No group! \n");
        if (!addResultToSet(&fieldValues, &varValues, rowValue, m_selections, &aggGroupProp, &anaFuncData, &anaEvaledExp, matchedSideDatarow, m_results))
          return false;
#ifdef __DEBUG__
  m_evalSeltime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__

        vector<string> vResults;
        for (int i=0; i<m_sorts.size(); i++){
          if (!m_sorts[i].sortKey.containGroupFunc()){
            //if it has the exact same expression as any selection, get the result from selection
            int iSel = -1;
            for (int j=0; j<m_selections.size(); j++)
              if (m_selections[j].getEntireExpstr().compare(m_sorts[i].sortKey.getEntireExpstr())==0){
                iSel = j;
                break;
              }
            if (iSel >= 0){
              sResult = m_results[m_results.size()-1][iSel + 1];
              if (m_selections[iSel].containAnaFunc()){ // add a evaled expression from the mapped selections for analytic function involved expression
                tmpExp = m_selections[iSel];
                tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, false);
                anaEvaledExp.push_back(tmpExp);
              }
            // if the sort key is a integer, get the result from the result set at the same sequence number
            }else if (m_sorts[i].sortKey.m_type==LEAF && m_sorts[i].sortKey.m_expType==CONST && isInt(m_sorts[i].sortKey.m_expStr) && atoi(m_sorts[i].sortKey.m_expStr.c_str())<m_selections.size()){
              int iSel = atoi(m_sorts[i].sortKey.m_expStr.c_str());
              sResult = m_results[m_results.size()-1][iSel + 1];
              if (m_selections[iSel].containAnaFunc()){ // add a evaled expression from the mapped selections for analytic function involved expression
                tmpExp = m_selections[iSel];
                tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, false);
                anaEvaledExp.push_back(tmpExp);
              }
            }else{
              tmpExp = m_sorts[i].sortKey;
             // m_sorts[i].sortKey.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, true);
              if (m_sorts[i].sortKey.containAnaFunc()) {// no actual result for analytic function yet. Keep the evaled expression
                tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, false);
                anaEvaledExp.push_back(tmpExp);
              }else
                tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, true);
            }
            //trace(DEBUG, "eval '%s' => '%s'\n", m_sorts[i].sortKey.getEntireExpstr().c_str(), sResult.c_str());
          }else{
            trace(ERROR, "(3)Invalid using aggregation function in '%s', no group involved!\n", m_sorts[i].sortKey.getEntireExpstr().c_str());
            return false;
          }
          vResults.push_back(sResult);
        }
        //vResults.push_back(intToStr(m_sortKeys.size())); // add an index for the sort key
        m_sortKeys.push_back(vResults);
#ifdef __DEBUG__
  m_evalSorttime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
      }else{ // need to do group. store evaled data in a temp date set
        //trace(DEBUG2, " Grouping! \n");
        vector<string> groupExps;  // the group expressions. as the key of following hash map
        vector<string> aggSelResult;
        string sResult;
        DataTypeStruct dts;
        if (m_aggrOnly) // has aggregation function without any group, give an empty string as the key
          groupExps.push_back("");
        else // group expressions to the sorting keys
          for (int i=0; i<m_groups.size(); i++){
            m_groups[i].evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, true);
            trace(DEBUG2, "Adding '%s' for Group '%s', eval from '%s'\n", sResult.c_str(),m_groups[i].getEntireExpstr().c_str(),rowValue[0].c_str());
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
          for (unordered_map< string,GroupProp >::iterator it=m_initAggProps.begin(); it!=m_initAggProps.end(); ++it)
            aggGroupProp.insert(pair< string,GroupProp >(it->first,it->second));
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
    }else{
      m_results.push_back(rowValue);
      addResultOutputFileMap(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, &matchedSideDatarow);
      vector<ExpressionC> vKeys;
      for (int i=0;i<m_sorts.size();i++)
        vKeys.push_back(m_sorts[i].sortKey);
      if (!addResultToSet(&fieldValues, &varValues, rowValue, vKeys, &aggGroupProp, &anaFuncData, &anaEvaledExp, matchedSideDatarow, m_sortKeys))
        return false;
    }
    // eval filter expressions to get anaFuncData
    if (m_filter && m_filter->containAnaFunc())
      m_filter->evalAnaExprs(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, &anaEvaledExp, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, true);
    m_anaEvaledExp.push_back(anaEvaledExp);
    addAnaFuncData(anaFuncData);
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

void QuerierC::trialAnalyze(vector<string> matcheddata)
{
  trace(DEBUG, "Detecting data types from the matched line '%s'\n", matcheddata[0].c_str());
  analyzeFiledTypes(matcheddata);
  trace(DEBUG,"Detected field type %d/%d\n", m_fieldtypes.size(), matcheddata.size());
  if (m_bToAnalyzeSelectMacro || m_selections.size()==0){
    analyzeSelString();
    analyzeSortStr();
  }
  if (m_bToAnalyzeSortMacro)
    analyzeSortStr();
  for(map<string, ExpressionC>::iterator it=m_uservarexprs.begin(); it!=m_uservarexprs.end(); ++it)
    it->second.analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for (int i=0; i<m_sideSelections.size(); i++){
    unordered_map<string,DataTypeStruct> sideSelDatatypes;
    for (int j=0; j<m_sideSelections[i].size(); j++){
      m_sideSelections[i][j].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
      sideSelDatatypes.insert(pair<string,DataTypeStruct>(m_sideAlias[i][j].empty()?intToStr(j+1):m_sideAlias[i][j], m_sideSelections[i][j].m_datatype));
    }
    m_sideDatatypes.insert(pair< string, unordered_map<string,DataTypeStruct> >(intToStr(i+1),sideSelDatatypes));
  }
  for (int i=0; i<m_sideFilters.size(); i++)
    m_sideFilters[i].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  if (m_filter){
    m_filter->analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
    //m_filter->mergeExprConstNodes();
  }
  for (int i=0; i<m_selections.size(); i++){
    //trace(DEBUG, "Analyzing selection '%s' (%d), found %d\n", m_selections[i].m_expStr.c_str(), i, found);
    m_selections[i].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  }
  for (int i=0; i<m_groups.size(); i++)
    m_groups[i].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for (int i=0; i<m_sorts.size(); i++)
    m_sorts[i].sortKey.analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for (unordered_map< string,ExpressionC >::iterator it=m_aggFuncExps.begin(); it!=m_aggFuncExps.end(); ++it)
    it->second.analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  for (int i=0; i<m_anaEvaledExp.size(); i++)
    for (int j=0; j<m_anaEvaledExp[i].size(); j++)
      m_anaEvaledExp[i][j].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
  if (m_outputfileexp)
    m_outputfileexp->analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype, &m_sideDatatypes);
}

int QuerierC::searchNextReg()
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  int found = 0;
  try {
    //namesaving_smatch matches(m_regexstr);
    while(!m_rawstr.empty() && regex_search(m_rawstr, m_matches, m_regexp)){
      m_line++;
      m_fileline++;
      vector<string> matcheddata;
      if (m_nameline && m_line==1){ // use the first line as field names
        for (int i=1; i<m_matches.size(); i++)
          m_fieldnames.push_back(m_matches[i]);
        m_nameline = false;
        m_line--;
        m_fileline--;
        continue;
      }else
        for (int i=0; i<m_matches.size(); i++)
          matcheddata.push_back(m_matches[i]);
      //trace(DEBUG2,"Detected rows %d/%d\n", m_detectedRawDatatype.size(), m_detectTypeMaxRowNum);
      // detect fileds data type
      if (m_detectedTypeRows < m_detectTypeMaxRowNum || matcheddata.size()!=m_fieldnames.size()+1){
        if (matcheddata.size()!=m_fieldnames.size()+1 && m_fieldnames.size()>0){
          m_fieldnames.clear();
          m_selections.clear();
          m_bToAnalyzeSelectMacro = m_bSelectContainMacro;
          m_bToAnalyzeSortMacro = m_bSortContainMacro;
        }
        if (m_fieldnames.size() == 0)
          pairFiledNames(m_matches);
        trialAnalyze(matcheddata);
      }
      // append variables
      //matcheddata.push_back(m_filename);
      matcheddata.push_back(intToStr(m_line));
      matcheddata.push_back(intToStr(m_matchcount+1));
      matcheddata.push_back(intToStr(m_fileline));
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
    }
    if (found == 0){
      // if didnt match any one, discard all until the last newline
      int i = m_rawstr.size(), newlnpos = -1;
      while (i>=0){
        if (m_rawstr[i] == '\n'){
          newlnpos = i;
          break;
        }
        i--;
      }
      if (newlnpos>=0)
        m_rawstr = m_rawstr.substr(newlnpos+1);
    }
  }catch (exception& e) {
    trace(ERROR, "Regular search exception: %s\n", e.what());
    return found;
  }
  //trace(DEBUG, "(1)Found: %d in this searching\n", m_matchcount);
#ifdef __DEBUG__
  m_searchtime += (curtime()-thistime);
#endif // __DEBUG__
  return found;
}

int QuerierC::searchNextWild()
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  trace(DEBUG, "Wild searching '%s'\n", m_regexstr.c_str());
  int found = 0;
  size_t pos = 0, opos;
  bool bEnded = false;
  while (!bEnded && !m_rawstr.empty() && pos<m_rawstr.length()){
    // Unlike regular matching, wildcard and delimiter only match lines.
    opos = pos;
    string sLine = m_readmode==READLINE?m_rawstr:readLine(m_rawstr, pos);
    bEnded = opos==pos; // pos will only be set back to the original pos if it reaches the end of current rawstr.
    m_rawstr = m_readmode==READLINE?"":m_rawstr.substr(pos);
    pos = 0;
    if(sLine.empty() && bEnded && m_bEof) {// read the rest of content if file reached eof, opos == pos check if it read an empty line
      sLine = m_rawstr;
      m_rawstr = "";
    }
    //trace(DEBUG, "Read '%s'\n", sLine.c_str());
    vector<string>  matcheddata = matchWildcard(sLine,m_regexstr,m_quoters,'\\',{});
    if (matcheddata.size()==0 && bEnded)
      continue;
    m_line++;
    m_fileline++;
    if (m_nameline && m_line==1){ // use the first line as field names
      for (int i=0; i<matcheddata.size(); i++)
        m_fieldnames.push_back(matcheddata[i]);
      m_nameline = false;
      m_line--;
      m_fileline--;
      continue;
    }
    matcheddata.insert(matcheddata.begin(),sLine); // whole matched line for @raw
    //trace(DEBUG, "Matched %d\n", matcheddata.size());
    //for (int i=0; i<matcheddata.size(); i++)
    //  trace(DEBUG, "Matched %d: '%s'\n", i ,matcheddata[i].c_str());
    // detect fileds data type
    if (m_detectedTypeRows < m_detectTypeMaxRowNum || matcheddata.size()!=m_fieldnames.size()+1){
      if (matcheddata.size()!=m_fieldnames.size()+1 && m_fieldnames.size()>0){
        m_fieldnames.clear();
        m_selections.clear();
        m_bToAnalyzeSelectMacro = m_bSelectContainMacro;
        m_bToAnalyzeSortMacro = m_bSortContainMacro;
      }
      if (m_fieldnames.size() == 0)
        for (int i=1;i<matcheddata.size();i++)
          m_fieldnames.push_back("@FIELD"+intToStr(i));
      trialAnalyze(matcheddata);
    }
    // append variables
    matcheddata.push_back(intToStr(m_line));
    matcheddata.push_back(intToStr(m_matchcount+1));
    matcheddata.push_back(intToStr(m_fileline));
    if (matchFilter(matcheddata))
      m_matchcount++;
    found++;
  }
  //trace(DEBUG, "(2)Found: %d in this searching\n", m_matchcount);
#ifdef __DEBUG__
  m_searchtime += (curtime()-thistime);
#endif // __DEBUG__
  return found;
}

int QuerierC::searchNextDelm()
{
#ifdef __DEBUG__
  long int searchstarttime = curtime();
  long int thistime = curtime();
#endif // __DEBUG__
  trace(DEBUG, "Delm searching '%s'\n", m_regexstr.c_str());
  int found = 0;
  size_t pos = 0, opos;
  bool bEnded = false;
  string sLine;
  while (!bEnded && !m_rawstr.empty() && pos<m_rawstr.length()){
    // Unlike regular matching, wildcard and delimiter only match lines.
    opos = pos;
    sLine = m_readmode==READLINE?m_rawstr:readLine(m_rawstr, pos);
    bEnded = opos==pos; // pos will only be set back to the original pos if it reaches the end of current rawstr.
    m_rawstr = m_readmode==READLINE?"":m_rawstr.substr(pos);
    if(sLine.empty() && bEnded && m_bEof){ // read the rest of content if file reached eof, opos == pos check if it read an empty line
      sLine = m_rawstr;
      m_rawstr = "";
    }
#ifdef __DEBUG__
  m_rawreadtime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    vector<string>  matcheddata;
    if (m_regexstr.empty()){
      if (!(sLine.empty()&&bEnded))
        matcheddata.push_back(sLine);
    }else
      matcheddata = split(sLine,m_regexstr,m_quoters,'\\',{},m_delmrepeatable, sLine.empty()&&bEnded);
#ifdef __DEBUG__
  m_rawsplittime += (curtime()-thistime);
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
      for (int i=0; i<matcheddata.size(); i++)
        m_fieldnames.push_back(matcheddata[i]);
      m_nameline = false;
      m_line--;
      m_fileline--;
      continue;
    }
    matcheddata.insert(matcheddata.begin(),sLine); // whole matched line for @raw
    //trace(DEBUG, "Matched %d\n", matcheddata.size());
    //for (int i=0; i<matcheddata.size(); i++)
    //  trace(DEBUG, "Matched %d: '%s'\n", i ,matcheddata[i].c_str());
    // detect fileds data type
    if (m_detectedTypeRows < m_detectTypeMaxRowNum || matcheddata.size()!=m_fieldnames.size()+1){
      if (matcheddata.size()!=m_fieldnames.size()+1 && m_fieldnames.size()>0){
        m_fieldnames.clear();
        m_selections.clear();
        m_bToAnalyzeSelectMacro = m_bSelectContainMacro;
        m_bToAnalyzeSortMacro = m_bSortContainMacro;
      }
      if (m_fieldnames.size() == 0)
        for (int i=1;i<matcheddata.size();i++)
          m_fieldnames.push_back("@FIELD"+intToStr(i));
      trialAnalyze(matcheddata);
    }
#ifdef __DEBUG__
  m_rawanalyzetime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
    // append variables
    matcheddata.push_back(intToStr(m_line));
    matcheddata.push_back(intToStr(m_matchcount+1));
    matcheddata.push_back(intToStr(m_fileline));
    if (matchFilter(matcheddata))
      m_matchcount++;
    found++;
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
    default:
      return 0;
  }
}

int QuerierC::searchAll()
{
  // initialize aggregation function expressions
  ExpressionC exp;
  for (unordered_map< string,GroupProp >::iterator it=m_initAggProps.begin(); it!=m_initAggProps.end(); ++it){
    exp = ExpressionC(it->first);
    m_aggFuncExps.insert(pair<string,ExpressionC>(it->first,exp));
  }

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
  if ((m_groups.size() == 0 && !m_aggrOnly) || m_groupKeys.size() == 0)
    return true;

  unordered_map< string,vector<string> > anaFuncData;
  map<string, string> varValues;
  vector<string> vResults;
  string sResult;
  DataTypeStruct dts;
  ExpressionC tmpExp;
  unordered_map< int,int > sideMatchedRowIDs;
  unordered_map< string, unordered_map<string,string> > matchedSideDatarow;
  int iRow = 0;
  m_anaFuncResult.clear();
  m_anaEvaledExp.clear();
  m_anaSortProps.clear();
  m_anaSortData.clear();
  for (std::set< vector<string> >::iterator it=m_groupKeys.begin(); it!=m_groupKeys.end(); ++it){
    varValues.clear();
    varValues.insert( pair<string,string>("@RAW",m_aggRowValues[*it][0]));
    varValues.insert( pair<string,string>("@FILE",m_aggRowValues[*it][m_aggRowValues[*it].size()-2]));
    varValues.insert( pair<string,string>("@FILEID",m_aggRowValues[*it][m_aggRowValues[*it].size()-1]));
    varValues.insert( pair<string,string>("@LINE",intToStr(iRow)));
    varValues.insert( pair<string,string>("@FILELINE",intToStr(iRow)));
    varValues.insert( pair<string,string>("@ROW",intToStr(iRow)));
    varValues.insert( pair<string,string>("@%",intToStr(m_fieldnames.size())));
    varValues.insert(m_uservariables.begin(), m_uservariables.end());
    // removed stored variables @FILE, @FILEID.
    m_aggRowValues[*it].pop_back();
    m_aggRowValues[*it].pop_back();
    anaFuncData.clear();
    if (m_filter && !m_filter->compareExpression(&m_aggRowValues[*it], &varValues, &m_aggGroupProp[*it], &anaFuncData, &m_sideDatasets, &m_sideDatatypes, sideMatchedRowIDs))
      continue;
    getSideDatarow(sideMatchedRowIDs, matchedSideDatarow);
    iRow++;
    vector<string> vEvaledParams;
    for (unordered_map< string,vector<ExpressionC> >::iterator it=m_initAnaArray.begin(); it!=m_initAnaArray.end(); ++it){
      vEvaledParams.clear();
      anaFuncData.insert(pair< string,vector<string> >(it->first,vEvaledParams));
    }
    // filter aggregation function result
    vResults.clear();
    vResults.push_back(m_aggRowValues[*it][0]);
    vector<ExpressionC> anaEvaledExp;
    for (int i=0; i<m_selections.size(); i++){
      if (m_selections[i].containAnaFunc()){ // eval analytic function processing data anaFuncData
        tmpExp = m_selections[i];
        tmpExp.evalExpression(&m_aggRowValues[*it], &varValues, &m_aggGroupProp[*it], &anaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, false);
        anaEvaledExp.push_back(tmpExp);
      }else
        m_selections[i].evalExpression(&m_aggRowValues[*it], &varValues, &m_aggGroupProp[*it], &anaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, true);
      vResults.push_back(sResult);
    }
    m_results.push_back(vResults);
    addResultOutputFileMap(&m_aggRowValues[*it], &varValues, &m_aggGroupProp[*it], &anaFuncData, &matchedSideDatarow);

    vResults.clear();
    for (int i=0; i<m_sorts.size(); i++){
      string sResult;
      //if it has the exact same expression as any selection, get the result from selection
      int iSel = -1;
      for (int j=0; j<m_selections.size(); j++)
        if (m_selections[j].getEntireExpstr().compare(m_sorts[i].sortKey.getEntireExpstr())==0){
          iSel = j;
          break;
        }
      if (iSel >= 0){
        vResults.push_back(m_results[m_results.size()-1][iSel + 1]);
      // if the sort key is a integer, get the result from the result set at the same sequence number
      }else if ((m_sorts[i].sortKey.m_type==LEAF && m_sorts[i].sortKey.m_expType==CONST && isInt(m_sorts[i].sortKey.m_expStr) && atoi(m_sorts[i].sortKey.m_expStr.c_str())<m_selections.size())){
        vResults.push_back(m_results[m_results.size()-1][atoi(m_sorts[i].sortKey.m_expStr.c_str()) + 1]);
      }else{ // sort expressions
        if (m_sorts[i].sortKey.containAnaFunc()){ // eval analytic function processing data anaFuncData
          tmpExp = m_sorts[i].sortKey;
          tmpExp.evalExpression(&m_aggRowValues[*it], &varValues, &m_aggGroupProp[*it], &anaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, false);
          anaEvaledExp.push_back(tmpExp);
        }else
          m_sorts[i].sortKey.evalExpression(&m_aggRowValues[*it], &varValues, &m_aggGroupProp[*it], &anaFuncData, &matchedSideDatarow, &m_sideDatatypes, sResult, dts, true);
        vResults.push_back(sResult);
      }
    }
    m_sortKeys.push_back(vResults);

    m_anaEvaledExp.push_back(anaEvaledExp);
    addAnaFuncData(anaFuncData);
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
  int iAnaGroupNum = 0;

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
  for (int i=0; i<vFuncData.size(); i++){
    vector<string> vGroups;
    for (int j=0;j<iAnaGroupNum;j++)
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
        if (anyDataCompare(tmpVal[vGroups], vFuncData[i][iAnaGroupNum], m_anaSortProps[sFuncExpStr][0].sortKey.m_datatype) < 0)
          tmpVal[vGroups] = vFuncData[i][iAnaGroupNum];
      }else
        tmpVal.insert(pair< vector<string>, string >(vGroups, vFuncData[i][iAnaGroupNum]));
      break;
    case MINA:
      if (tmpVal.find(vGroups)!=tmpVal.end()){
        if (anyDataCompare(tmpVal[vGroups], vFuncData[i][iAnaGroupNum], m_anaSortProps[sFuncExpStr][0].sortKey.m_datatype) > 0)
          tmpVal[vGroups] = vFuncData[i][iAnaGroupNum];
      }else
        tmpVal.insert(pair< vector<string>, string >(vGroups, vFuncData[i][iAnaGroupNum]));
      break;
    }
  }

  // assign calculated result of SUMA/COUNTA/UNIQUECOUNTA/AVERAGEA/MAXA/MINA then return
  for (int i=0; i<m_anaFuncResult.size(); i++){
    vector<string> vGroups;
    for (int j=0;j<iAnaGroupNum;j++)
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
    for (int i=0;i<sortProps.size();i++){
      int iCompareRslt = anyDataCompare(v1[i],v2[i],sortProps[i].sortKey.m_datatype);
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
    for (int i=0;i<sortProps.size();i++){
      int iCompareRslt = anyDataCompare(v1[i+1],v2[i+1],sortProps[i].sortKey.m_datatype);
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
  for (int i=0;i<sortProps.size();i++)
    printf("%d:%s %d\t",i,decodeDatatype(sortProps[i].sortKey.m_datatype.datatype).c_str(),sortProps[i].direction);
  printf("\n");
  printf("Before sorting analytic function data [%d][%d]\n",vFuncData.size(), vFuncData[0].size());
  for (int i=0;i<vFuncData.size();i++){
    for (int j=0;j<vFuncData[i].size();j++)
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
  for (int i=0;i<vFuncData.size();i++){
    for (int j=0;j<vFuncData[i].size();j++)
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
    int iAnaGroupNum = 0;
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
    vector<ExpressionC> funcExps;
    ExpressionC funExp;
    for (int j=0;j<iAnaGroupNum;j++){
      anaFunc = NULL;
      funcExps = m_anaEvaledExp[0];
      for (int i=0; i<funcExps.size();i++){
        //trace(DEBUG,"(2)Searching analytic function '%s' from '%s'\n",sFuncExpStr.c_str(),funcExps[i].getEntireExpstr().c_str());
        //funcExps[i].dump();
        anaFunc = funcExps[i].getAnaFunc(sFuncExpStr);
        if (anaFunc)
          break;
      }
      if (!anaFunc){
        trace(ERROR, "(2)Failed to find analytic function expression '%s'\n", sFuncExpStr.c_str());
      }else{
        if (anaFunc->m_params.size() < j)
          trace(ERROR, "(2)Analytic function '%s' parameter size %d is smaller than expected!\n", sFuncExpStr.c_str(), anaFunc->m_params.size());
        funExp = anaFunc->m_params[j];
      }
      sp.sortKey = funExp;
      sp.direction = ASC;
      sortProps.push_back(sp);
    }
  }
  
  if (!sortAnaData(sortProps, iFuncID, sFuncExpStr, vFuncData))
    return false;

  // variables for RANK/DENSERANK
  vector<string> preRow;
  int iRank = 1, iDenseRank = 1;
  for (int i=0; i<m_anaFuncResult.size(); i++){
    int iPosBeforeSorted = atoi(vFuncData[i][vFuncData[i].size()-1].c_str());
    bool bNewGroup = false, bSortValueChanged=false;
    if (preRow.size() == 0){
      bNewGroup = true;
      for (int j=0;j<vFuncData[i].size()-1;j++){
        preRow.push_back(vFuncData[i][j]);
#ifdef DEBUG_ANALYTIC
        printf("%d:%s\t",j,vFuncData[i][j].c_str());
#endif
      }
    }else {
      for (int j=0;j<vFuncData[i].size()-1;j++){
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
  int iAnaExprNum = 0, iAnaSortNum = 0, iDistParaNum = 0, iDefaultParaNum = 0;
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

  for (int i=0; i<m_anaFuncResult.size(); i++){
    int iPosBeforeSorted = atoi(vFuncData[i][vFuncData[i].size()-1].c_str());
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
  for (int i=0; i<vFuncData.size(); i++)
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
  if (m_initAnaArray.size() == 0)
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

  map<string, string> varValues;
  varValues.insert( pair<string,string>("@FILE",m_filename));
  varValues.insert( pair<string,string>("@FILEID",intToStr(m_fileid)));
  varValues.insert( pair<string,string>("@%",intToStr(m_fieldnames.size())));
  varValues.insert(m_uservariables.begin(), m_uservariables.end());
  int iRow = 0;
  trace(DEBUG, "m_anaEvaledExp size is %d, dimension is %d . \n", m_anaEvaledExp.size(),m_anaEvaledExp[0].size());
  unordered_map< int,int > sideMatchedRowIDs;
  unordered_map< string, unordered_map<string,string> > matchedSideDatarow;
  vector<string> vfieldvalues;
  unordered_map< string,GroupProp > dummyAggGroupProp;
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
    vector<string> anaThisResult;
    for (unordered_map< string,string >::iterator it=m_anaFuncResult[i].begin(); it!=m_anaFuncResult[i].end(); it++){
      anaThisResult.clear();
      anaThisResult.push_back(it->second);
      anaFinalResult.insert(pair< string,vector<string> >(it->first,anaThisResult));
    }
    // filter by analytic function results
    if (m_filter && !m_filter->compareExpression(&vfieldvalues, &varValues, ((m_aggGroupProp.size()>0 && m_aggGroupDataidMap.size()==tmpResults.size())?(&m_aggGroupProp[m_aggGroupDataidMap[i]]):&dummyAggGroupProp), &anaFinalResult, &m_sideDatasets, &m_sideDatatypes, sideMatchedRowIDs))
      continue;
    getSideDatarow(sideMatchedRowIDs, matchedSideDatarow);
    iRow++;
    int anaExpID = 0;
    for (int j=0; j<m_selections.size(); j++)
      if (m_selections[j].containAnaFunc()){
        string sResult;
        m_anaEvaledExp[i][anaExpID].evalAnalyticFunc(&(m_anaFuncResult[i]), sResult);
        anaExpID++;
        trace(DEBUG, "Selection '%s'(%d) contains analytic function, assign result '%s' (was '%s')\n", m_selections[j].getEntireExpstr().c_str(),j,sResult.c_str(),tmpResults[i][j+1].c_str());
        tmpResults[i][j+1] = sResult;
      }
    for (int j=0; j<m_sorts.size(); j++)
      if (m_sorts[j].sortKey.containAnaFunc()){
        string sResult;
        m_anaEvaledExp[i][anaExpID].evalAnalyticFunc(&(m_anaFuncResult[i]), sResult);
        anaExpID++;
        trace(DEBUG, "Selection '%s'(%d) contains analytic function, assign result '%s' (was '%s')\n", m_sorts[j].sortKey.getEntireExpstr().c_str(),j,sResult.c_str(),tmpResults[i][j+1].c_str());
        tmpSortKeys[i][j] = sResult;
      }
    m_results.push_back(tmpResults[i]); 
    m_sortKeys.push_back(tmpSortKeys[i]);
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
  int iSize=0, iDups=0;
  for (int i=0;i<m_results.size();i++){
    iSize = uresults.size();
    uresults.insert(m_results[i]);
    if (iSize == uresults.size()) { // duplicated row
      if (m_sortKeys.size()>i-iDups){
        m_sortKeys.erase(m_sortKeys.begin()+i-iDups);
        iDups++;
      }
    }else
      tmpResult.push_back(m_results[i]);
  }
  m_results.clear();
  for (int i=0;i<tmpResult.size();i++)
    m_results.push_back(tmpResult[i]);
  trace(DEBUG, "Result number after unique: %d.\n",m_results.size());
#ifdef __DEBUG__
  m_uniquetime += (curtime()-thistime);
#endif // __DEBUG__
}

// doing merging sort exchanging
void QuerierC::mergeSort(int iLeftB, int iLeftT, int iRightB, int iRightT)
{
  //trace(DEBUG2, "Mergeing %d %d %d %d\n", iLeftB, iLeftT, iRightB, iRightT);
  if (iLeftT >= iRightB || iLeftB > iLeftT || iRightB > iRightT)
    return;
  else{
    mergeSort(iLeftB, max(iLeftB,iLeftB+(int)floor(iLeftT-iLeftB)/2-1), min(iLeftT,max(iLeftB,iLeftB+(int)floor(iLeftT-iLeftB)/2-1)+1), iLeftT);
    mergeSort(iRightB, max(iRightB,iRightB+(int)floor(iRightT-iRightB)/2-1), min(iRightT,max(iRightB,iRightB+(int)floor(iRightT-iRightB)/2-1)+1), iRightT);
//#ifdef __DEBUG__
//  for (int i=0; i<m_sortKeys.size(); i++)
//    printf("%s(%d) ", (*(m_sortKeys.begin()+i))[0].c_str(), i);
//  printf("\n");
//#endif // __DEBUG__
    int iLPos = iLeftB, iRPos = iRightB, iCheckPos = iRightB;
    while (iLPos<iCheckPos && iRPos<=iRightT){
      //trace(DEBUG2, "Swaping %d %d %d %d\n", iLPos, iCheckPos, iRPos, iRightT);
      bool exchanged = false;
      for (int i=0; i<m_sorts.size(); i++){
        //trace(DEBUG2, "Checking '%s' : '%s'\n", (*(m_sortKeys.begin()+iLPos))[i].c_str(), (*(m_sortKeys.begin()+iRPos))[i].c_str());
//#ifdef __DEBUG__
  //trace(DEBUG2, "Checking %s(L) %s(R) (%d %d %d) (%s) (%d)\n", (*(m_sortKeys.begin()+iLPos))[i].c_str(), (*(m_sortKeys.begin()+iRPos))[i].c_str(),iLPos,iCheckPos,iRPos,decodeDatatype(m_sorts[i].sortKey.m_datatype.datatype).c_str(), m_sorts[i].direction);
//#endif // __DEBUG__
        //if (bToBeSwapped){
        int iCompareRslt = anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype);
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
//  for (int i=0; i<m_sortKeys.size(); i++)
//    printf("%s(%d) ", (*(m_sortKeys.begin()+i))[0].c_str(), i);
//  printf("\n");
//#endif // __DEBUG__
  }
}

// doing merging sort exchanging
void QuerierC::mergeSort(vector< vector<string> > *dataSet, vector<SortProp>* sortProps, vector< vector<string> >* sortKeys, int iLeftB, int iLeftT, int iRightB, int iRightT)
{
  //trace(DEBUG2, "Mergeing %d %d %d %d\n", iLeftB, iLeftT, iRightB, iRightT);
  if (iLeftT >= iRightB || iLeftB > iLeftT || iRightB > iRightT)
    return;
  else{
    mergeSort(dataSet, sortProps, sortKeys, iLeftB, max(iLeftB,iLeftB+(int)floor(iLeftT-iLeftB)/2-1), min(iLeftT,max(iLeftB,iLeftB+(int)floor(iLeftT-iLeftB)/2-1)+1), iLeftT);
    mergeSort(dataSet, sortProps, sortKeys, iRightB, max(iRightB,iRightB+(int)floor(iRightT-iRightB)/2-1), min(iRightT,max(iRightB,iRightB+(int)floor(iRightT-iRightB)/2-1)+1), iRightT);
//#ifdef __DEBUG__
//  for (int i=0; i<sortKeys->size(); i++)
//    printf("%s(%d) ", (*(sortKeys->begin()+i))[0].c_str(), i);
//  printf("\n");
//#endif // __DEBUG__
    int iLPos = iLeftB, iRPos = iRightB, iCheckPos = iRightB;
    while (iLPos<iCheckPos && iRPos<=iRightT){
      //trace(DEBUG2, "Swaping %d %d %d %d\n", iLPos, iCheckPos, iRPos, iRightT);
      bool exchanged = false;
      for (int i=0; i<sortProps->size(); i++){
        //trace(DEBUG2, "Checking '%s' : '%s'\n", (*(sortKeys->begin()+iLPos))[i].c_str(), (*(sortKeys->begin()+iRPos))[i].c_str());
//#ifdef __DEBUG__
  //trace(DEBUG2, "Checking %s(L) %s(R) (%d %d %d) (%s) (%d)\n", (*(sortKeys->begin()+iLPos))[i].c_str(), (*(sortKeys->begin()+iRPos))[i].c_str(),iLPos,iCheckPos,iRPos,decodeDatatype((*sortProps)[i].sortKey.m_datatype.datatype).c_str(), (*sortProps)[i].direction);
//#endif // __DEBUG__
        //if (bToBeSwapped){
        int iCompareRslt = anyDataCompare((*(sortKeys->begin()+iLPos))[i],(*(sortKeys->begin()+iRPos))[i],(*sortProps)[i].sortKey.m_datatype);
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
//  for (int i=0; i<m_sortKeys.size(); i++)
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

  //for (int i=0; i<m_sorts.size(); i++)
  //  trace(DEBUG, "Sorting key '%s'(%d) !\n",m_sorts[i].sortKey.getEntireExpstr().c_str(),i);
  trace(DEBUG2, "Sorting begins, sort key size %d, sort expr number %d!\n",m_sortKeys.size(),m_sorts.size());
  if (m_sorts.size() == 0 || m_sortKeys.size() == 0){
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
  for (int i=0; i<m_sortKeys.size(); i++)
    m_sortKeys[i].push_back(intToStr(i));
  vector<SortProp> sortProps = m_sorts;
  auto sortVectorLambda = [sortProps] (vector<string> const& v1, vector<string> const& v2) -> bool
  {
    if (v1.size()!=sortProps.size()+1 || v2.size()!=sortProps.size()+1 || v2.size()!=v1.size())
      return false;
    for (int i=0;i<sortProps.size();i++){
      int iCompareRslt = anyDataCompare(v1[i],v2[i],sortProps[i].sortKey.m_datatype);
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
  for (int i=0; i<m_sortKeys.size(); i++)
    m_results[i] = tmpResults[atoi(m_sortKeys[i][m_sortKeys[i].size()-1].c_str())];
  clearSort();
  //trace(DEBUG1, "Sorting completed \n");
#ifdef __DEBUG__
  m_sorttime += (curtime()-thistime);
#endif // __DEBUG__
  return true;
}

//void QuerierC::formatoutput(namesaving_smatch matches)
//{
//  //for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
//  //  printf("%s\t", matches[*it].str().c_str());
//  for (int i=1; i<matches.size(); i++)
//    printf("%s\t",matches[i].str().c_str());
//  printf("\n");
//}

void QuerierC::outputstream(int resultid, const char *fmt, ...)
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

void QuerierC::formatoutput(vector<string> datas, int resultid)
{
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
      for (int i=1; i<datas.size(); i++){
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
      for (int i=1; i<datas.size(); i++)
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
  if (m_colToRows.size() != m_colToRowNames.size()){
    trace(ERROR,"COLTOROW marco function size %d doesnot match filed names of COLTOROW marco function %d",m_colToRows.size(),m_colToRowNames.size());
  }else{
    for (int i=m_colToRows.size()-1;i>=0;i--){
      m_selnames.insert(m_selnames.begin()+m_colToRows[i][0],m_colToRowNames[i]);
      for (int j=m_colToRows[i].size()-1;j>=0;j--)
        m_selnames.erase(m_selnames.begin()+1+m_colToRows[i][j]);
    }
  }
  if (m_selnames.size()>0){
    for (int i=0; i<m_selnames.size(); i++){
      printf("%s%s",m_selnames[i].c_str(),i<m_selnames.size()-1?m_fielddelim.c_str():"");
    }
    printf("\n");
    for (int i=0; i<m_selnames.size(); i++)
      printf("%s%s",string(m_selnames[i].length(),'-').c_str(),i<m_selnames.size()-1?m_fielddelim.c_str():"");
  }else{
    printf("Row\n"); 
    printf("%s",string(58,'-').c_str());
  }
  printf("\n");
  m_bNamePrinted = true;
}

void QuerierC::output()
{
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  //printf("Result Num: %d\n",m_results.size());
  int iRowNumFromCols = 0;
  for (int i=0; i<m_colToRows.size(); i++)
    iRowNumFromCols = max(iRowNumFromCols, (int)m_colToRows[i].size());
  for (int i=0; i<m_results.size(); i++){
    if (iRowNumFromCols>0){ // convert cols to rows
      int k=0;
      vector<string> datas;
      vector< vector<string> > tmpResult;
      for (int j=1; j<m_results[i].size(); j++){
        if(k<m_colToRows.size() && j-1>=m_colToRows[k][0] && j-1<=m_colToRows[k][m_colToRows[k].size()-1]){ // the selection is a part of a COLTOROW function
          datas.push_back(m_results[i][j]);
          if (j-1==m_colToRows[k][m_colToRows[k].size()-1] || j-1==m_results[i].size()-1){ // if current selection is the last one, complement empty strings
            datas.insert(datas.end(),iRowNumFromCols-(int)datas.size(),"");
            tmpResult.push_back(datas);
            datas.clear();
            k++;
          }
        }else{ // the selection is not a part of any COLTOROW function
          datas.push_back(m_results[i][j]);
          datas.insert(datas.end(),iRowNumFromCols-1,"");
          tmpResult.push_back(datas);
          datas.clear();
        }
      }
      for (int j=0;j<iRowNumFromCols;j++){
        datas.clear();
        datas.push_back("");
        for (int k=0;k<tmpResult.size();k++)
          datas.push_back(tmpResult[k][j]);
        formatoutput(datas, i);
      }
    }else
      formatoutput(m_results[i], i);
  }
#ifdef __DEBUG__
  m_outputtime += (curtime()-thistime);
#endif // __DEBUG__
}

int QuerierC::boostmatch(vector<string> *result)
{
  //namesaving_smatch matches(m_regexstr);
  smatch matches;
  //boost::match_results<string::const_iterator>  matches;
  if ( result != NULL ) {
    //printf("Matching %s => %s\n",m_rawstr.c_str(), m_regexstr.c_str());
    result->clear();
    try{
      //if (boost::regex_match(m_rawstr, matches, m_regexp, boost::match_perl|boost::match_extra)) {
      if (regex_match(m_rawstr, matches, m_regexp)) {
        //printf("Matched %d!\n", matches.size());
        //for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
        //  printf("%s: %s\n",string(*it).c_str(),matches[*it].str().c_str());
        for (int i=1; i<matches.size(); i++){
          //result->push_back(matches[i].str());
          //printf("Matching %s => %s\n",matches[i].first, matches[i].second);
          result->push_back(matches[i]);
        }
      }
    }catch (exception& e) {
      trace(ERROR, "Regular search exception: %s\n", e.what());
      return -1;
    }
    return result->size();
  }else
    return -1;
}

int QuerierC::boostmatch(map<string,string> & result)
{
  namesaving_smatch matches(m_regexstr);
  //printf("Matching %s => %s\n",m_rawstr.c_str(), m_regexstr.c_str());
  try{
    if (regex_match(m_rawstr, matches, m_regexp)) {
      for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
        result[string(*it)] = matches[*it].str();
    }
  }catch (exception& e) {
    trace(ERROR, "Regular match exception: %s\n", e.what());
    return -1;
  }
  return result.size();
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
  output();
  m_results.clear();
  m_resultfiles.clear();
}

void QuerierC::outputExtraInfo(size_t total, bool bPrintHeader)
{
  if (m_outputformat == JSON){
    if (m_outputrow>0)
      printf("\n");
    printf("\t],\n");
    //if (mode == READLINE)
    //  printf("\"ReadLines\": %d,\n", total);
    //else
    //  printf("\"ReadBytes\": %d,\n", total);
    printf("\t\"MatchedLines\": %ld,\n", m_line);
    printf("\t\"SelectedRows\": %ld\n", m_outputrow-m_limitbottom+1);
    printf("}\n");
  }else if (bPrintHeader){
    //if (mode == READLINE)
    //  printf("Read %d lines(s).\n", total);
    //else
    //  printf("Read %d byte(s).\n", total);
    printf("Pattern matched %ld line(s).\n", m_line);
    printf("Selected %ld row(s).\n", m_outputrow-m_limitbottom+1);
  }
#ifdef __DEBUG__
  m_totaltime = curtime() - m_querystartat;
  trace(DEBUG2, "Total time: %d. Break down:\n, Searching time: %d\n, Read raw content time: %d\n, Split raw content time: %d\n, Analyze raw content(analyzed rows %d) time: %d\n, filtering time: %d\n, sorting time: %d\n, unique time: %d\n, aggregation time: %d\n, analytic time: %d\n, eval group key time: %d\n, filter compare time: %d\n, prepare Agg GP time: %d\n, eval agg expression time: %d\n, eval selection time: %d\n, eval sort time: %d\n, update restult time: %d\n, Output time: %d\n, matched lines: %d\n", m_totaltime, m_searchtime, m_rawreadtime, m_rawsplittime, m_rawanalyzetime, m_filtertime, m_sorttime, m_uniquetime, m_grouptime, m_detectedTypeRows, m_analytictime, m_evalGroupKeytime, m_filtercomptime, m_prepAggGPtime, m_evalAggExptime, m_evalSeltime, m_evalSorttime, m_updateResulttime, m_outputtime, m_line);
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
}

void QuerierC::clear()
{
  clearGroup();
  clearAnalytic();
  clearSort();
  clearFilter();
  m_results.clear();
  m_fieldnames.clear();
  m_fieldtypes.clear();
  m_fieldntypes.clear();
  m_uservariables.clear();
  m_selnames.clear();
  m_selections.clear();
  m_sideSelections.clear();
  m_sideAlias.clear();
  m_sideFilters.clear();
  m_sideDatasets.clear();
  m_sideDatatypes.clear();
  for (unordered_map< string, ofstream* >::iterator it=m_outputfiles.begin();it!=m_outputfiles.end();it++){
    if (it->second && it->second->is_open()){
      it->second->close();
      SafeDelete(it->second);
    }
  }
  m_outputfiles.clear();
  if (m_outputfileexp)
    SafeDelete(m_outputfileexp);
  m_resultfiles.clear();
  m_colToRows.clear();
  m_colToRowNames.clear();
  m_searchMode = REGSEARCH;
  m_readmode = READBUFF;
  m_quoters = "";
  m_nameline = false;
  m_bEof = false;
  m_delmrepeatable = false;
  m_bNamePrinted = false;
  m_aggrOnly = false;
  m_bUniqueResult = false;
  m_bSelectContainMacro = false;
  m_bToAnalyzeSelectMacro = false;
  m_bSortContainMacro = false;
  m_bToAnalyzeSortMacro = false;
  m_selstr = "";
  m_sortstr = "";
  m_detectTypeMaxRowNum = 1;
  m_detectedTypeRows = 0;
  m_outputformat = TEXT;
  m_filter = NULL;
  m_limitbottom = 1;
  m_limittop = -1;
  m_filename = "";
  m_fileid = 0;
  m_fileline = 0;
  init();
}
