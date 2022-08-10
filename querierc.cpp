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

  for (sregex_iterator i = words_begin; i != words_end; i++){
    string name = (*i)[1].str();
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
  m_nameline = false;
  m_line = 0;
  m_outputformat = TEXT;
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
  m_detectTypeMaxRowNum = 1;
  m_detectedTypeRows = 0;
#ifdef __DEBUG__
  m_searchtime = 0;
  m_filtertime = 0;
  m_sorttime = 0;
  m_evalGroupKeytime = 0;
  m_filtercomptime = 0;
  m_prepAggGPtime = 0;
  m_evalAggExptime = 0;
  m_evalSeltime = 0;
  m_evalSorttime = 0;
  m_updateResulttime = 0;
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
    //dumpVector(vSearchPattern);
    m_regexstr = vSearchPattern[0];
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
    m_regexstr = vSearchPattern[0];
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
    delete m_filter;
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
  for (int i=0; i<vGroups.size(); i++){
    trace(DEBUG, "Processing group (%d) '%s'!\n", i, vGroups[i].c_str());
    string sGroup = trim_copy(vGroups[i]);
    if (sGroup.empty()){
      trace(ERROR, "Empty group string!\n");
      return false;
    }
    ExpressionC eGroup(sGroup);
    if (eGroup.m_expType == FUNCTION && eGroup.m_Function && eGroup.m_Function->m_funcID==FOREACH){
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

bool QuerierC::checkSelGroupConflict(ExpressionC eSel)
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

bool QuerierC::analyzeSelString(){
  trace(DEBUG, "Analyzing selections from '%s'!\n", m_selstr.c_str());
  m_selections.clear();
  m_selnames.clear();
  vector<string> vSelections = split(m_selstr,',',"''()",'\\',{'(',')'},false,true);
  bool bGroupFunc = false, bNonGroupFuncSel = false;
  for (int i=0; i<vSelections.size(); i++){
    trace(DEBUG, "Processing selection(%d) '%s'!\n", i, vSelections[i].c_str());
    string sSel = trim_copy(vSelections[i]);
    if (sSel.empty()){
      trace(ERROR, "Empty selection string!\n");
      continue;
    }
    vector<string> vSelAlias = split(sSel," as ","''()",'\\',{'(',')'},false,true);
    string sAlias = "";
    if (vSelAlias.size()>1)
      sAlias = trim_copy(vSelAlias[1]);
    //trace(DEBUG2,"Selection expression: '%s'\n",vSelAlias[0].c_str());
    ExpressionC eSel = ExpressionC(vSelAlias[0]);
    if (eSel.m_type == LEAF && eSel.m_expType == FUNCTION && eSel.m_Function && eSel.m_Function->isAnalytic())
      trace(DEBUG,"QuerierC: The analytic function '%s' group size %d, param size %d \n", eSel.m_Function->m_expStr.c_str(),eSel.m_Function->m_anaParaNums[0],eSel.m_Function->m_params.size());
    //trace(DEBUG, "Got selection expression '%s'!\n", eSel.getEntireExpstr().c_str());
    
    // if macro function is involved, need to wait util the first data analyzed to analyze select expression
    if (eSel.m_expType == FUNCTION && eSel.m_Function && eSel.m_Function->m_funcID==FOREACH){
      if (m_fieldtypes.size()==0){
        m_bSelectContainMacro = true;
        m_bToAnalyzeSelectMacro = true;
        m_selections.clear();
        m_selnames.clear();
        trace(DEBUG2,"Skiping select FOREACH: '%s'\n",eSel.m_expStr.c_str());
        return true;
      }else{
        vector<ExpressionC> vExpandedExpr; 
        if (m_groups.size()>0)
          vExpandedExpr = eSel.m_Function->expandForeach(m_groups);
        else
          vExpandedExpr = eSel.m_Function->expandForeach(m_fieldtypes.size());
        trace(DEBUG2,"Expanding FOREACH: '%s'\n",eSel.m_expStr.c_str());
        for (int j=0; j<vExpandedExpr.size(); j++){
          trace(DEBUG2,"Expanded FOREACH expression: '%s'\n",vExpandedExpr[j].m_expStr.c_str());
          m_selnames.push_back(vExpandedExpr[j].getEntireExpstr());
          checkSelGroupConflict(vExpandedExpr[j]);
          if (vExpandedExpr[j].containGroupFunc())
            bGroupFunc = true;
          else
            bNonGroupFuncSel = true;

          vExpandedExpr[j].getAggFuncs(m_initAggProps);
          vExpandedExpr[j].getAnaFuncs(m_initAnaArray, m_anaFuncParaNums);
          m_selections.push_back(vExpandedExpr[j]);
        }
        m_bToAnalyzeSelectMacro = false;
        continue;
      }
    }
    if (sAlias.empty())
      m_selnames.push_back(eSel.getEntireExpstr());
    else
      m_selnames.push_back(sAlias);
    
    checkSelGroupConflict(eSel);
    if (eSel.containGroupFunc())
      bGroupFunc = true;
    else
      bNonGroupFuncSel = true;

    eSel.getAggFuncs(m_initAggProps);
    eSel.getAnaFuncs(m_initAnaArray, m_anaFuncParaNums);
    m_selections.push_back(eSel);
    //trace(DEBUG, "Selection: '%s'!\n", eSel.getEntireExpstr().c_str());
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

bool QuerierC::checkSortGroupConflict(ExpressionC eSort)
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
  for (int i=0; i<vSorts.size(); i++){
    trace(DEBUG, "Processing sorting keys (%d) '%s'!\n", i, vSorts[i].c_str());
    string sSort = trim_copy(vSorts[i]);
    if (sSort.empty()){
      trace(ERROR, "Empty sorting key!\n");
      return false;
    }
    SortProp keyProp;
    vector<string> vKP = split(sSort,' ',"''()",'\\',{'(',')'},false,true);
    if (vKP.size()<=1 || upper_copy(trim_copy(vKP[1])).compare("DESC")!=0)
      keyProp.direction = ASC;
    else
      keyProp.direction = DESC;
    keyProp.sortKey.setExpstr(trim_copy(vKP[0]));
    // if macro function is involved , need to wait util the first data analyzed to analyze sort expression
    if (keyProp.sortKey.m_expType == FUNCTION && keyProp.sortKey.m_Function && keyProp.sortKey.m_Function->m_funcID==FOREACH){
      if (m_fieldtypes.size()==0){
        m_bSortContainMacro = true;
        m_bToAnalyzeSortMacro = true;
        m_sorts.clear();
        trace(DEBUG2,"Skiping sort FOREACH: '%s'\n",keyProp.sortKey.m_expStr.c_str());
        return true;
      }else{
        vector<ExpressionC> vExpandedExpr;
        if (m_groups.size()>0)
          vExpandedExpr = keyProp.sortKey.m_Function->expandForeach(m_groups);
        else
          vExpandedExpr = keyProp.sortKey.m_Function->expandForeach(m_fieldtypes.size());
        for (int j=0; j<vExpandedExpr.size(); j++){
          SortProp keyPropE;
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
  vector<string> vVariables = split(variables,' ',"''()",'\\',{'(',')'},false,true);
  for (int i=0; i<vVariables.size(); i++){
    vector<string> vNameVal = split(vVariables[i],':',"''()",'\\',{'(',')'},false,true);
    if (vNameVal.size()<2){
      trace(ERROR, "Incorrect variable format!\n", vVariables[i].c_str());
      continue;
    }
    string sName=upper_copy(trim_copy(vNameVal[0])), sValue=trim_copy(vNameVal[1]);
    trace(DEBUG, "Setting variable '%s' value '%s'!\n", sName.c_str(), sValue.c_str());
    if (sName.compare("RAW")==0 || sName.compare("ROW")==0 || sName.compare("FILE")==0 || sName.compare("LINE")==0){
      trace(ERROR, "%s is a reserved word, cannot be used as a variable name!\n", sName.c_str());
      continue;
    }
    m_uservariables.insert(pair<string, string> ("@"+sName,sValue));
  }
}

void QuerierC::setFileName(string filename)
{
  m_filename = filename;
}

void QuerierC::setOutputFormat(short int format)
{
  m_outputformat = format;
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

void QuerierC::evalAggExpNode(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp > & aggGroupProp)
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
    if (ite->second.m_expType==FUNCTION && ite->second.m_Function && ite->second.m_Function->isAggFunc() && ite->second.m_Function->m_params.size()>0 && ite->second.m_Function->m_params[0].evalExpression(fieldvalues, varvalues, &aggGroupProp, &anaFuncData, sResult, dts, true)){
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
        case UNIQUECOUNT:
          it->second.uniquec.insert(sResult);
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
        case UNIQUECOUNT:
          it->second.uniquec.insert(sResult);
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

// add a data row to a result set
bool QuerierC::addResultToSet(vector<string>* fieldvalues, map<string,string>* varvalues, vector<string> rowValue, vector<ExpressionC> expressions, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs, vector<ExpressionC>* anaEvaledExp, vector< vector<string> > & resultSet)
{
    vector<string> vResults;
    //vResults.push_back(rowValue[0]);
    vResults.push_back(""); // save memory
    for (int i=0; i<expressions.size(); i++){
      string sResult;
      DataTypeStruct dts;
      if (!expressions[i].containGroupFunc()){
        ExpressionC tmpExp;// = expressions[i];
        tmpExp = expressions[i];
        //expressions[i].copyTo(&tmpExp);
        //trace(DEBUG, "addResultToSet: Before eval expression '%s':\n",tmpExp.getEntireExpstr().c_str());
        //tmpExp.dump();
        //expressions[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sResult, dts, true);
        if (expressions[i].containAnaFunc()) {// no actual result for analytic function yet. Keep the evaled expression
          tmpExp.evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sResult, dts, false);
          anaEvaledExp->push_back(tmpExp);
          trace(DEBUG, "addResultToSet: adding analytic function involved expression '%s':\n",expressions[i].getEntireExpstr().c_str());
        }else
          tmpExp.evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sResult, dts, true);
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
    return true;
}

void QuerierC::addAnaFuncData(unordered_map< string,vector<string> > anaFuncData)
{
  unordered_map< string,string > anaFuncResult;
  // convert the stored analytic data to this format: <analytic_func_str:vector of analytic data> 
  for (unordered_map< string,vector<string> >::iterator it=anaFuncData.begin(); it!=anaFuncData.end(); it++){
    vector<string> sortKey;
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
      vector< vector<string> > newData;
      vector<SortProp> sortProps;
      //Add sort props, the second part of parameters of analytic functions is the sort keys.
      for(int j=0;j<it->second.size();j++){
        if (j<iAnaGroupNum || j>=iAnaGroupNum+iAnaSortNum*2) // all parts except the second part of analytic function parameters
          sortKey.push_back(it->second[j]);
        else { // second part of analytic function parameters, which are always sort keys for all analytic functions
          if ((j-iAnaGroupNum)%2==0)
            sortKey.push_back(it->second[j]);
          else{
            SortProp sp;
            FunctionC* anaFunc = NULL;
            vector<ExpressionC> funcExps = m_anaEvaledExp[0];
            ExpressionC funExp;
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

// filt a row data by filter. no predication mean true. comparasion failed means alway false
bool QuerierC::matchFilter(vector<string> rowValue)
{
  //if (!filter){
  //  //trace(INFO, "No filter defined\n");
  //  return true;
  //}
#ifdef __DEBUG__
  long int thistime = curtime();
  long int filterbegintime = thistime;
#endif // __DEBUG__
  if (rowValue.size() != m_fieldnames.size() + 3){ // field name number + 3 variables (@raw @line @row)
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
  varValues.insert( pair<string,string>("@LINE",rowValue[m_fieldnames.size()+1]));
  varValues.insert( pair<string,string>("@ROW",rowValue[m_fieldnames.size()+2]));
  varValues.insert( pair<string,string>("@%",intToStr(m_fieldnames.size())));
  varValues.insert(m_uservariables.begin(), m_uservariables.end());
  unordered_map< string,GroupProp > aggGroupProp;
  unordered_map< string,vector<string> > anaFuncData;
  //trace(DEBUG, "Filtering '%s' ", rowValue[0].c_str());
  //dumpMap(varValues);
  bool matched = (!m_filter || m_filter->compareExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData));
#ifdef __DEBUG__
  m_filtercomptime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
  //trace(DEBUG, " selected:%d (%d)! \n", matched, m_selections.size());
  if (matched){
    if (m_selections.size()>0){
      for (unordered_map< string,vector<ExpressionC> >::iterator it=m_initAnaArray.begin(); it!=m_initAnaArray.end(); ++it){
        vector<string> vEvaledParams;
        anaFuncData.insert(pair< string,vector<string> >(it->first,vEvaledParams));
      }
      ExpressionC tmpExp;
      if (m_groups.size() == 0 && m_initAggProps.size() == 0 && !m_aggrOnly){
        vector<ExpressionC> anaEvaledExp;
        //trace(DEBUG, " No group! \n");
        if (!addResultToSet(&fieldValues, &varValues, rowValue, m_selections, &aggGroupProp, &anaFuncData, &anaEvaledExp, m_results))
          return false;
#ifdef __DEBUG__
  m_evalSeltime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__

        vector<string> vResults;
        for (int i=0; i<m_sorts.size(); i++){
          string sResult;
          DataTypeStruct dts;
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
                tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, false);
                anaEvaledExp.push_back(tmpExp);
              }
            // if the sort key is a integer, get the result from the result set at the same sequence number
            }else if (m_sorts[i].sortKey.m_type==LEAF && m_sorts[i].sortKey.m_expType==CONST && isInt(m_sorts[i].sortKey.m_expStr) && atoi(m_sorts[i].sortKey.m_expStr.c_str())<m_selections.size()){
              int iSel = atoi(m_sorts[i].sortKey.m_expStr.c_str());
              sResult = m_results[m_results.size()-1][iSel + 1];
              if (m_selections[iSel].containAnaFunc()){ // add a evaled expression from the mapped selections for analytic function involved expression
                tmpExp = m_selections[iSel];
                tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, false);
                anaEvaledExp.push_back(tmpExp);
              }
            }else{
              tmpExp = m_sorts[i].sortKey;
             // m_sorts[i].sortKey.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, true);
              if (m_sorts[i].sortKey.containAnaFunc()) {// no actual result for analytic function yet. Keep the evaled expression
                tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, false);
                anaEvaledExp.push_back(tmpExp);
              }else
                tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, true);
            }
            //trace(DEBUG, "eval '%s' => '%s'\n", m_sorts[i].sortKey.getEntireExpstr().c_str(), sResult.c_str());
          }else{
            trace(ERROR, "(3)Invalid using aggregation function in '%s', no group involved!\n", m_sorts[i].sortKey.getEntireExpstr().c_str());
            return false;
          }
          vResults.push_back(sResult);
        }
        m_anaEvaledExp.push_back(anaEvaledExp);
        //vResults.push_back(intToStr(m_sortKeys.size())); // add an index for the sort key
        m_sortKeys.push_back(vResults);
        addAnaFuncData(anaFuncData);
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
        bool dataSetExist = false;
        if (m_aggrOnly) // has aggregation function without any group, give an empty string as the key
          groupExps.push_back("");
        else // group expressions to the sorting keys
          for (int i=0; i<m_groups.size(); i++){
            m_groups[i].evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, true);
            trace(DEBUG2, "Adding '%s' for Group '%s', eval from '%s'\n", sResult.c_str(),m_groups[i].getEntireExpstr().c_str(),rowValue[0].c_str());
            groupExps.push_back(sResult);
          }
        //trace(DEBUG2, "Checking  '%s' (%d)\n", groupExps[0].c_str(), m_aggrOnly);
#ifdef __DEBUG__
  m_evalGroupKeytime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
        m_groupKeys.insert(groupExps);
        unordered_map< vector<string>, vector<string>, hash_container< vector<string> > >::iterator it1 = m_aggSelResults.find(groupExps);
        unordered_map< vector<string>, unordered_map< string,GroupProp >, hash_container< vector<string> > >::iterator it2 = m_aggGroupProp.find(groupExps);
        if (it1 != m_aggSelResults.end() && it2 != m_aggGroupProp.end()){
          // We dont need to grab the existing aggSelResult, as we only need keep one aggSelResult for each groupExps
          // aggSelResult = it1->second; 
          for (unordered_map< string,GroupProp >::iterator it=it2->second.begin(); it!=it2->second.end(); ++it)
            aggGroupProp.insert(pair< string,GroupProp >(it->first,it->second));
          dataSetExist = true;
        }else{
          for (unordered_map< string,GroupProp >::iterator it=m_initAggProps.begin(); it!=m_initAggProps.end(); ++it)
            aggGroupProp.insert(pair< string,GroupProp >(it->first,it->second));
        }
#ifdef __DEBUG__
  m_prepAggGPtime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
        // get data sets
        //if (!dataSetExist) // only need keep one aggSelResult for each groupExps
        aggSelResult.push_back(""); // save memory!! no point to store a whole raw string any more        
        evalAggExpNode(&fieldValues, &varValues, aggGroupProp);
#ifdef __DEBUG__
  m_evalAggExptime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
        vector<ExpressionC> anaEvaledExp;
        for (int i=0; i<m_selections.size(); i++){
          string sResult;
          DataTypeStruct dts;
          //trace(DEBUG1, "Selection '%s': %d \n",m_selections[i].getEntireExpstr().c_str(), m_selections[i].containGroupFunc());
          // If the expression includes aggregation function, we still eval it, but eventually, only the last value in a group will be kept.
          tmpExp = m_selections[i];
          //m_selections[i].evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, true);
          //trace(DEBUG2, "Eval selection '%s', get '%s', from '%s' \n",m_selections[i].getEntireExpstr().c_str(), sResult.c_str(), rowValue[0].c_str());
          if (m_selections[i].containAnaFunc()){ // no actual result for analytic function yet. Keep the evaled expression
            tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, false);
            anaEvaledExp.push_back(tmpExp);
          }else
            tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, true);
          aggSelResult.push_back(sResult);
        }
#ifdef __DEBUG__
  m_evalSeltime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
        for (int i=0; i<m_sorts.size(); i++){
          string sResult;
          DataTypeStruct dts;
          //if it has the exact same expression as any selection, get the result from selection
          int iSel = -1;
          for (int j=0; j<m_selections.size(); j++)
            if (m_selections[j].getEntireExpstr().compare(m_sorts[i].sortKey.getEntireExpstr())==0){
              iSel = j;
              break;
            }
          // if the sort key is a integer, get the result from the result set at the same sequence number
          if (iSel >= 0 || (m_sorts[i].sortKey.m_type==LEAF && m_sorts[i].sortKey.m_expType==CONST && isInt(m_sorts[i].sortKey.m_expStr) && atoi(m_sorts[i].sortKey.m_expStr.c_str())<m_selections.size()))
            continue;
          else{ // non aggregation function selections
            tmpExp = m_sorts[i].sortKey;
            //m_sorts[i].sortKey.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, true);
            //trace(DEBUG1, "Got non-aggr selection '%s' \n",sResult.c_str());
            if (m_sorts[i].sortKey.containAnaFunc()) { // no actual result for analytic function yet. Keep the evaled expression
              tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, false);
              anaEvaledExp.push_back(tmpExp);
            }else
              tmpExp.evalExpression(&fieldValues, &varValues, &aggGroupProp, &anaFuncData, sResult, dts, true);
            aggSelResult.push_back(sResult);
          }
        }
#ifdef __DEBUG__
  m_evalSorttime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
        // dateSet.nonAggSels.push_back(aggSelResult);
        // make sure we only keep the last value of a group.
        if (dataSetExist){
          // m_aggFuncTaget.erase(groupExps);
          m_aggGroupProp.erase(groupExps);
          m_aggSelResults.erase(groupExps);
        }else{
          m_anaEvaledExp.push_back(anaEvaledExp);
          addAnaFuncData(anaFuncData);
          m_aggGroupDataidMap.insert(pair< vector<string>,int >(groupExps,m_anaEvaledExp.size()-1));
        }
        // m_aggFuncTaget.insert( pair<vector<string>, unordered_map< string,vector<string> > >(groupExps,aggFuncTaget));
        //trace(DEBUG2, "adding result .. \n");
        //dumpVector(groupExps);
        //trace(DEBUG2, "sel: '%s' \n",aggSelResult[1].c_str());
        m_aggSelResults.insert( pair<vector<string>, vector<string> >(groupExps,aggSelResult));
        m_aggGroupProp.insert( pair<vector<string>, unordered_map< string,GroupProp > >(groupExps,aggGroupProp));
#ifdef __DEBUG__
  m_updateResulttime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
      }
    }else{
      m_results.push_back(rowValue);
      vector<ExpressionC> anaEvaledExp;
      vector<ExpressionC> vKeys;
      for (int i=0;i<m_sorts.size();i++)
        vKeys.push_back(m_sorts[i].sortKey);
      if (!addResultToSet(&fieldValues, &varValues, rowValue, vKeys, &aggGroupProp, &anaFuncData, &anaEvaledExp, m_sortKeys))
        return false;
      m_anaEvaledExp.push_back(anaEvaledExp);
      addAnaFuncData(anaFuncData);
    }
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
  trace(DEBUG2,"Detected field type %d/%d\n", m_fieldtypes.size(), matcheddata.size());
  if (m_bToAnalyzeSelectMacro || m_selections.size()==0){
    analyzeSelString();
    analyzeSortStr();
  }
  if (m_bToAnalyzeSortMacro)
    analyzeSortStr();
  if (m_filter){
    m_filter->analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype);
    //m_filter->mergeExprConstNodes();
  }
  for (int i=0; i<m_selections.size(); i++){
    //trace(DEBUG, "Analyzing selection '%s' (%d), found %d\n", m_selections[i].m_expStr.c_str(), i, found);
    m_selections[i].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype);
  }
  for (int i=0; i<m_groups.size(); i++)
    m_groups[i].analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype);
  for (int i=0; i<m_sorts.size(); i++)
    m_sorts[i].sortKey.analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype);
  for (unordered_map< string,ExpressionC >::iterator it=m_aggFuncExps.begin(); it!=m_aggFuncExps.end(); ++it)
    it->second.analyzeColumns(&m_fieldnames, &m_fieldtypes, &m_rawDatatype);
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
      vector<string> matcheddata;
      if (m_nameline && m_line==1){ // use the first line as field names
        for (int i=1; i<m_matches.size(); i++)
          m_fieldnames.push_back(m_matches[i]);
        m_nameline = false;
        m_line--;
        continue;
      }else
        for (int i=0; i<m_matches.size(); i++)
          matcheddata.push_back(m_matches[i]);
      //trace(DEBUG2,"Detected rows %d/%d\n", m_detectedRawDatatype.size(), m_detectTypeMaxRowNum);
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
    if (m_nameline && m_line==1){ // use the first line as field names
      for (int i=0; i<matcheddata.size(); i++)
        m_fieldnames.push_back(matcheddata[i]);
      m_nameline = false;
      m_line--;
      continue;
    }
    matcheddata.insert(matcheddata.begin(),sLine); // whole matched line for @raw
    //trace(DEBUG, "Matched %d\n", matcheddata.size());
    //for (int i=0; i<matcheddata.size(); i++)
    //  trace(DEBUG, "Matched %d: '%s'\n", i ,matcheddata[i].c_str());
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
    matcheddata.push_back(intToStr(m_line));
    matcheddata.push_back(intToStr(m_matchcount+1));
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
  long int thistime = curtime();
#endif // __DEBUG__
  trace(DEBUG, "Delm searching '%s'\n", m_regexstr.c_str());
  int found = 0;
  size_t pos = 0, opos;
  bool bEnded = false;
  while (!bEnded && !m_rawstr.empty() && pos<m_rawstr.length()){
    // Unlike regular matching, wildcard and delimiter only match lines.
    opos = pos;
    string sLine = m_readmode==READLINE?m_rawstr:readLine(m_rawstr, pos);
    bEnded = opos==pos; // pos will only be set back to the original pos if it reaches the end of current rawstr.
    m_rawstr = m_readmode==READLINE?"":m_rawstr.substr(pos);
    if(sLine.empty() && bEnded && m_bEof){ // read the rest of content if file reached eof, opos == pos check if it read an empty line
      sLine = m_rawstr;
      m_rawstr = "";
    }
    vector<string>  matcheddata;
    if (m_regexstr.empty()){
      if (!(sLine.empty()&&bEnded))
        matcheddata.push_back(sLine);
    }else
      matcheddata = split(sLine,m_regexstr,m_quoters,'\\',{},m_delmrepeatable, sLine.empty()&&bEnded);
    trace(DEBUG, "searchNextDelm Read '%s'(flag:%d; len:%d) vector size: %d \n", sLine.c_str(), sLine.empty()&&bEnded, sLine.length(), matcheddata.size());
    trace(DEBUG, "searchNextDelm pos/size %d/%d\n", pos, m_rawstr.length());
    pos = 0;
    //dumpVector(matcheddata);
    if (matcheddata.size()==0 && bEnded)
      continue;
    m_line++;
    if (m_nameline && m_line==1){ // use the first line as field names
      for (int i=0; i<matcheddata.size(); i++)
        m_fieldnames.push_back(matcheddata[i]);
      m_nameline = false;
      m_line--;
      continue;
    }
    matcheddata.insert(matcheddata.begin(),sLine); // whole matched line for @raw
    //trace(DEBUG, "Matched %d\n", matcheddata.size());
    //for (int i=0; i<matcheddata.size(); i++)
    //  trace(DEBUG, "Matched %d: '%s'\n", i ,matcheddata[i].c_str());
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
    matcheddata.push_back(intToStr(m_line));
    matcheddata.push_back(intToStr(m_matchcount+1));
    if (matchFilter(matcheddata))
      m_matchcount++;
    found++;
  }
  //trace(DEBUG, "(3)Found: %d in this searching\n", m_matchcount);
#ifdef __DEBUG__
  m_searchtime += (curtime()-thistime);
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
  for (unordered_map< string,GroupProp >::iterator it=m_initAggProps.begin(); it!=m_initAggProps.end(); ++it){
    ExpressionC exp(it->first);
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
#ifdef __DEBUG__
  trace(DEBUG2, "Searching time: %d, filtering time: %d, sorting time: %d, eval group key time: %d, filter compare time: %d, prepare Agg GP time: %d, eval agg expression time: %d, eval selection time: %d, eval sort time: %d, update restult time: %d, matched lines: %d, found: %d\n", m_searchtime, m_filtertime, m_sorttime, m_evalGroupKeytime, m_filtercomptime, m_prepAggGPtime, m_evalAggExptime, m_evalSeltime, m_evalSorttime, m_updateResulttime, m_line, totalfound);
#endif // __DEBUG__
  return totalfound;
}

// group result
bool QuerierC::group()
{
  if ((m_groups.size() == 0 && !m_aggrOnly) || m_groupKeys.size() == 0)
    return true;

  vector<string> vfieldvalues;
  map<string,string> mvarvalues;
  unordered_map< string,vector<string> > anaFuncData;
  //trace(DEBUG2, "m_aggSelResults size: %d, m_groupKeys size: %d\n", m_aggSelResults.size(), m_groupKeys.size());
  //for (unordered_map< vector<string>, vector<string>, hash_container< vector<string> > >::iterator it=m_aggSelResults.begin(); it!=m_aggSelResults.end(); ++it)
  //  dumpVector(it->first);
  vector< vector<ExpressionC> > tmpanaEvaledExp = m_anaEvaledExp;
  vector< unordered_map< string,string > > tmpanaFuncResult = m_anaFuncResult;
  unordered_map< string,vector< vector<string> > > tmpanaSortData = m_anaSortData;
  //int iAnaID = 0;
  m_anaEvaledExp.clear();
  m_anaFuncResult.clear();
  for (unordered_map< string,vector< vector<string> > >::iterator ita=m_anaSortData.begin(); ita!=m_anaSortData.end(); ita++)
    ita->second.clear();
  for (std::set< vector<string> >::iterator it=m_groupKeys.begin(); it!=m_groupKeys.end(); ++it){
    // filter aggregation function result
    if (m_filter && !m_filter->compareExpression(&vfieldvalues, &mvarvalues, &m_aggGroupProp[*it], &anaFuncData))
      continue;
    //dumpVector(*it);
    //trace(DEBUG2,"Sel:'%s'\n",m_aggSelResults[*it][1].c_str());
    vector<string> vResults;
    vResults.push_back(m_aggSelResults[*it][0]);
    int iAggSelID = 1;
    //trace(DEBUG1, "Selection: %d:%d\n", m_selections.size(), m_aggSelResults[*it]].size());
    for (int i=0; i<m_selections.size(); i++){
      string sResult;
      vResults.push_back(m_aggSelResults[*it][iAggSelID]);
      iAggSelID++;
    }
    m_results.push_back(vResults);
    m_anaEvaledExp.push_back(tmpanaEvaledExp[m_aggGroupDataidMap[*it]]);
    m_anaFuncResult.push_back(tmpanaFuncResult[m_aggGroupDataidMap[*it]]);
    for (unordered_map< string,vector< vector<string> > >::iterator ita=m_anaSortData.begin(); ita!=m_anaSortData.end(); ita++)
      ita->second.push_back(tmpanaSortData[ita->first][m_aggGroupDataidMap[*it]]);
    trace(DEBUG,"Group: pushing %d to %d \n",m_anaEvaledExp.size()-1,m_aggGroupDataidMap[*it]);
    //m_anaEvaledExp[m_aggGroupDataidMap[*it]] = tmpanaEvaledExp[iAnaID];
    //m_anaFuncResult[m_aggGroupDataidMap[*it]] = tmpanaFuncResult[iAnaID];
    //m_anaEvaledExp[iAnaID] = tmpanaEvaledExp[m_aggGroupDataidMap[*it]];
    //m_anaFuncResult[iAnaID] = tmpanaFuncResult[m_aggGroupDataidMap[*it]];
    //iAnaID++;
    //trace(DEBUG,"Group: pushing %d to %d \n",iAnaID,m_aggGroupDataidMap[*it]);

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
        //trace(DEBUG1, "None aggr func selection: %s\n", m_aggSelResults[*it][iAggSelID].c_str());
        vResults.push_back(m_aggSelResults[*it][iAggSelID]);
        iAggSelID++;
      }
    }
    m_sortKeys.push_back(vResults);
  }
  return true;
}

// group and sort analytic functions
bool QuerierC::analytic()
{
  if (m_initAnaArray.size() == 0)
    return true;
  if (m_anaSortData.size() != m_anaSortProps.size() || m_anaSortProps.size() != m_anaFuncParaNums.size() || m_anaFuncParaNums.size() != m_anaSortData.size()){
    trace(ERROR, "Analytic functions sorting data size %d, sorting prop size %d, group number size %d do not match!\n", m_anaSortData.size(), m_anaSortProps.size(), m_anaFuncParaNums.size());
    return false;
  }
  if (m_results.size() != m_anaFuncResult.size() || m_results.size() != m_anaEvaledExp.size() || m_anaFuncResult.size() != m_anaEvaledExp.size()){
    trace(ERROR, "Result data size %d, analytic, function result size %d, analytic function expression size %d do not match!\n", m_results.size(), m_anaFuncResult.size(), m_anaEvaledExp.size());
    return false;
  }

  vector< unordered_map< string,string > > anaFuncResult;
  for (unordered_map< string,vector< vector<string> > >::iterator it=m_anaSortData.begin(); it!=m_anaSortData.end(); it++){
    // add index for each sort key
    for (int i=0; i<it->second.size(); i++)
      it->second[i].push_back(intToStr(i));
    vector<SortProp> sortProps;
    if (it->first.find("RANK(")==0 || it->first.find("DENSERANK(")==0){ // For RANK/DENSERANK, the first part parameters are groups, which also need to be sorted (before sort keys).
      int iAnaGroupNum = 0;
      unordered_map< string,vector<int> >::iterator itn = m_anaFuncParaNums.find(it->first);
      if (itn == m_anaFuncParaNums.end())
        trace(ERROR, "(1)Failed to find analytic function parameter numbers '%s'\n", it->first.c_str());
      else{
        if (itn->second.size() < 2)
          trace(ERROR, "(1)The analytic function parameter numbers size %d is smaller than 2\n", it->first.c_str(), itn->second.size());
        iAnaGroupNum = itn->second[0];
      }
      for (int j=0;j<iAnaGroupNum;j++){
        SortProp sp;
        FunctionC* anaFunc = NULL;
        vector<ExpressionC> funcExps = m_anaEvaledExp[0];
        ExpressionC funExp;
        for (int i=0; i<funcExps.size();i++){
          //trace(DEBUG,"(2)Searching analytic function '%s' from '%s'\n",it->first.c_str(),funcExps[i].getEntireExpstr().c_str());
          //funcExps[i].dump();
          anaFunc = funcExps[i].getAnaFunc(it->first);
          if (anaFunc)
            break;
        }
        if (!anaFunc){
          trace(ERROR, "(2)Failed to find analytic function expression '%s'\n", it->first.c_str());
        }else{
          if (anaFunc->m_params.size() < j)
            trace(ERROR, "(2)Analytic function '%s' parameter size %d is smaller than expected!\n", it->first.c_str(), anaFunc->m_params.size());
          funExp = anaFunc->m_params[j];
        }
        sp.sortKey = funExp;
        sp.direction = ASC;
        sortProps.push_back(sp);
      }
    }
    sortProps.insert(sortProps.end(),m_anaSortProps[it->first].begin(),m_anaSortProps[it->first].end());
    auto sortVectorLambda = [sortProps] (vector<string> const& v1, vector<string> const& v2) -> bool
    {
      for (int i=0;i<sortProps.size();i++){
        int iCompareRslt = anyDataCompare(v1[i],v2[i],sortProps[i].sortKey.m_datatype);
        if (iCompareRslt == 0) // Compare next key only when current keys equal
          continue;
        return (sortProps[i].direction==ASC ? iCompareRslt<0 : iCompareRslt>0);
      }
      return false; // return false if all elements equal
    };
    trace(DEBUG,"sortProps size: %d; sort keys size: %d\n",sortProps.size(), it->second[0].size());
//#define DEBUG_ANALYTIC
#ifdef DEBUG_ANALYTIC
    printf("sortProps size: %d\n",sortProps.size());
    for (int i=0;i<sortProps.size();i++)
      printf("%d:%s %d\t",i,decodeDatatype(sortProps[i].sortKey.m_datatype.datatype).c_str(),sortProps[i].direction);
    printf("\n");
    printf("Before sorting analytic function data [%d][%d]\n",it->second.size(), it->second[0].size());
    for (int i=0;i<it->second.size();i++){
      for (int j=0;j<it->second[i].size();j++)
        printf("%d:%s\t",j,it->second[i][j].c_str());
      printf("\n");
    }
#endif
    std::sort(it->second.begin(), it->second.end(), sortVectorLambda);
#ifdef DEBUG_ANALYTIC
    printf("Sorted analytic function data [%d][%d]\n",it->second.size(), it->second[0].size());
    for (int i=0;i<it->second.size();i++){
      for (int j=0;j<it->second[i].size();j++)
        printf("%d:%s\t",j,it->second[i][j].c_str());
      printf("\n");
    }
#endif
    vector<string> preRow;
    int iRank = 1, iDenseRank = 1;
    for (int i=0; i<m_anaFuncResult.size(); i++){
      if (it->first.find("RANK(")==0 || it->first.find("DENSERANK(")==0){
        bool bNewGroup = false, bSortValueChanged=false;
        if (preRow.size() == 0){
          bNewGroup = true;
          for (int j=0;j<it->second[i].size()-1;j++){
            preRow.push_back(it->second[i][j]);
#ifdef DEBUG_ANALYTIC
            printf("%d:%s\t",j,it->second[i][j].c_str());
#endif
          }
        }else {
          for (int j=0;j<it->second[i].size()-1;j++){
            if (j<m_anaFuncParaNums[it->first][0] && it->second[i][j].compare(preRow[j])!=0)
              bNewGroup = true;
            if (j>=m_anaFuncParaNums[it->first][0] && it->second[i][j].compare(preRow[j])!=0)
              bSortValueChanged = true;
            preRow[j] = it->second[i][j];
#ifdef DEBUG_ANALYTIC
            printf("%d:%s\t",j,it->second[i][j].c_str());
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
#endif
        dumpVector(it->second[i]);
        trace(DEBUG,"Checking analytic function '%s' group size: %d. Check result: %d %d. Rank: %d \n",it->first.c_str(), m_anaFuncParaNums[it->first][0], bNewGroup, bSortValueChanged, iRank);
        m_anaFuncResult[atoi(it->second[i][it->second[i].size()-1].c_str())][it->first] = intToStr(it->first.find("RANK(")==0?iRank:iDenseRank);
      }
    }
  }

  trace(DEBUG, "m_anaEvaledExp size is %d, dimension is %d . \n", m_anaEvaledExp.size(),m_anaEvaledExp[0].size());
  for (int i=0; i<m_results.size();i++){
    int anaExpID = 0;
    for (int j=0; j<m_selections.size(); j++)
      if (m_selections[j].containAnaFunc()){
        string sResult;
        m_anaEvaledExp[i][anaExpID].evalAnalyticFunc(&(m_anaFuncResult[i]), sResult);
        anaExpID++;
        trace(DEBUG, "Selection '%s'(%d) contains analytic function, assign result '%s' (was '%s')\n", m_selections[j].getEntireExpstr().c_str(),j,sResult.c_str(),m_results[i][j+1].c_str());
        m_results[i][j+1] = sResult;
      }
    for (int j=0; j<m_sorts.size(); j++)
      if (m_sorts[j].sortKey.containAnaFunc()){
        string sResult;
        m_anaEvaledExp[i][anaExpID].evalAnalyticFunc(&(m_anaFuncResult[i]), sResult);
        anaExpID++;
        trace(DEBUG, "Selection '%s'(%d) contains analytic function, assign result '%s' (was '%s')\n", m_sorts[j].sortKey.getEntireExpstr().c_str(),j,sResult.c_str(),m_results[i][j+1].c_str());
        m_sortKeys[i][j] = sResult;
      }
  }

  return true;
}

void QuerierC::unique()
{
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

  for (int i=0; i<m_sorts.size(); i++)
    trace(DEBUG, "Sorting key '%s'(%d) !\n",m_sorts[i].sortKey.getEntireExpstr().c_str(),i);
  //trace(DEBUG2, "Sorting begins \n");
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
    for (int i=0;i<sortProps.size();i++){
      int iCompareRslt = anyDataCompare(v1[i],v2[i],sortProps[i].sortKey.m_datatype);
      if (iCompareRslt == 0) // Compare next key only when current keys equal
        continue;
      return (sortProps[i].direction==ASC ? iCompareRslt<0 : iCompareRslt>0);
    }
    return false; // return false if all elements equal
  };
  std::sort(m_sortKeys.begin(), m_sortKeys.end(), sortVectorLambda);
  vector< vector<string> > tmpResults = m_results;
  for (int i=0; i<m_sortKeys.size(); i++)
    m_results[i] = tmpResults[atoi(m_sortKeys[i][m_sortKeys[i].size()-1].c_str())];

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

void QuerierC::formatoutput(vector<string> datas)
{
  if (m_outputformat == JSON){
    if (m_outputrow==m_limitbottom-1){
      printf("{\n");
      printf("\t\"records\": [\n");
    }else if (m_outputrow>m_limitbottom-1){
      printf(",\n");
    }
  }
  if (m_outputrow<m_limitbottom-1 ){
    m_outputrow++;
    return;
  }else if (m_limittop>=0 && m_outputrow+1>m_limittop)
    return;
  else
    m_outputrow++;
  //printf("%d: ", m_outputrow);
  if (m_outputformat == JSON){
    printf("\t\t{\n");
    if (m_selections.size()==0){
      printf("\t\t\t\"%s\": \"%s\"\n","RAW",datas[0].c_str());
    }else{
      for (int i=1; i<datas.size(); i++){
        if (m_selections[i-1].m_datatype.datatype == INTEGER || m_selections[i-1].m_datatype.datatype == LONG || m_selections[i-1].m_datatype.datatype == DOUBLE || m_selections[i-1].m_datatype.datatype == BOOLEAN)
          printf("\t\t\t\"%s\": %s",m_selnames[i-1].c_str(),datas[i].c_str());
        else
          printf("\t\t\t\"%s\": \"%s\"",m_selnames[i-1].c_str(),datas[i].c_str());
        if (i == datas.size()-1)
          printf("\n");
        else
          printf(",\n");
      }
    }
    printf("\t\t}");
  }else{
    if (m_selections.size()==0)
      printf("%s\n", datas[0].c_str());
    else{
      for (int i=1; i<datas.size(); i++)
        printf("%s\t", datas[i].c_str());
      printf("\n");
    }
  }
}

void QuerierC::printFieldNames()
{
  //for (int i=1; i<m_fieldnames.size(); i++)
  //  printf("%s\t",m_fieldnames[i].c_str());
  if (m_bNamePrinted)
    return;
  if (m_selnames.size()>0){
    for (int i=0; i<m_selnames.size(); i++)
      printf("%s\t",m_selnames[i].c_str());
    printf("\n");
    for (int i=0; i<m_selnames.size(); i++)
      printf("%s\t",string(m_selnames[i].length(),'-').c_str());
  }else{
    printf("Row\n"); 
    printf("%s",string(58,'-').c_str());
  }
  printf("\n");
  m_bNamePrinted = true;
}

void QuerierC::output()
{
  //printf("Result Num: %d\n",m_results.size());
  for (int i=0; i<m_results.size(); i++)
    formatoutput(m_results[i]);
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
}

void QuerierC::outputExtraInfo(size_t total, short int mode, bool bPrintHeader)
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
}

void QuerierC::clear()
{
  m_groups.clear();
  m_sorts.clear();
  m_aggSelResults.clear();
  m_groupKeys.clear();
  //m_aggFuncTaget.clear();
  m_aggGroupProp.clear();
  m_initAggProps.clear();
  m_initAnaArray.clear();
  m_aggFuncExps.clear();
  m_sortKeys.clear();
  m_anaEvaledExp.clear();
  m_anaSortProps.clear();
  m_anaSortData.clear();
  m_anaFuncResult.clear();
  m_aggGroupDataidMap.clear();
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
  if (m_filter){
    m_filter->clear();
    delete m_filter;
  }
  m_filter = NULL;
  m_limitbottom = 1;
  m_limittop = -1;
  m_filename = "";
  init();
}
