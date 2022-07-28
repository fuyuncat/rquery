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
  m_filename = "";
  m_line = 0;
  m_outputformat = TEXT;
  m_matchcount = 0; 
  m_outputrow = 0;
  m_limitbottom = 1;
  m_limittop = -1;
  m_bNamePrinted = false;
  m_aggrOnly = false;
  m_bUniqueResult = false;
#ifdef __DEBUG__
  m_searchtime = 0;
  m_filtertime = 0;
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
  m_regexstr = regexstr;
  try{
    m_regexp = sregex::compile(m_regexstr);
  }catch (exception& e) {
    trace(FATAL, "Regular pattern compile exception: %s\n", e.what());
  }
}

void QuerierC::assignFilter(FilterC* filter)
{
  if (m_filter){
    m_filter->clear();
    delete m_filter;
  }
  m_filter = filter;
  m_filter->getAggFuncs(m_initAggProps);
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

bool QuerierC::assignGroupStr(string groupstr)
{
  vector<string> vGroups = split(groupstr,',',"''{}()",'\\',{'(',')'});
  for (int i=0; i<vGroups.size(); i++){
    trace(DEBUG, "Processing group (%d) '%s'!\n", i, vGroups[i].c_str());
    string sGroup = trim_copy(vGroups[i]);
    if (sGroup.empty()){
      trace(ERROR, "Empty group string!\n");
      return false;
    }
    ExpressionC eGroup(sGroup);
    m_groups.push_back(eGroup);
  }
  return true;
}

void QuerierC::setUniqueResult(bool bUnique)
{
  m_bUniqueResult = bUnique;
}

bool QuerierC::assignLimitStr(string limitstr)
{
  vector<string> vLimits = split(limitstr,',',"''{}()",'\\',{'(',')'});
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
  vector<string> vSelections = split(selstr,',',"''{}()",'\\',{'(',')'});
  bool bGroupFunc = false, bNonGroupFuncSel = false;
  for (int i=0; i<vSelections.size(); i++){
    trace(DEBUG, "Processing selection(%d) '%s'!\n", i, vSelections[i].c_str());
    string sSel = trim_copy(vSelections[i]);
    if (sSel.empty()){
      trace(ERROR, "Empty selection string!\n");
      continue;
    }
    vector<string> vSelAlias = split(sSel," as ","''{}()",'\\',{'(',')'});
    if (vSelAlias.size()>1)
      m_selnames.push_back(trim_copy(vSelAlias[1]));
    else
      m_selnames.push_back(sSel);
    //trace(DEBUG2,"Selection expression: '%s'\n",vSelAlias[0].c_str());
    ExpressionC eSel(vSelAlias[0]);
    //trace(DEBUG2,"'%s' merged const to '%s'.\n",vSelAlias[0].c_str(),eSel.getEntireExpstr().c_str());
    //eSel.dump();
    vector<string> allColNames;
    for (int i=0; i<m_groups.size(); i++)
      m_groups[i].getAllColumnNames(allColNames);
    if (m_groups.size()>0 && !eSel.groupFuncOnly() && !eSel.inColNamesRange(allColNames)){
      trace(WARNING, "Selection '%s' does not exist in Group or invalid using aggregation function \n", vSelAlias[0].c_str());
      //continue;
      //return false;
    }

    if (eSel.containGroupFunc())
      bGroupFunc = true;
    else
      bNonGroupFuncSel = true;

    eSel.getAggFuncs(m_initAggProps);
    m_selections.push_back(eSel);
    //eSel.dump();
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
  vector<string> vSorts = split(sortstr,',',"''{}()",'\\',{'(',')'});
  for (int i=0; i<vSorts.size(); i++){
    trace(DEBUG, "Processing sorting keys (%d) '%s'!\n", i, vSorts[i].c_str());
    string sSort = trim_copy(vSorts[i]);
    if (sSort.empty()){
      trace(ERROR, "Empty sorting key!\n");
      return false;
    }
    SortProp keyProp;
    vector<string> vKP = split(sSort,' ',"''{}()",'\\',{'(',')'});
    if (vKP.size()<=1 || upper_copy(trim_copy(vKP[1])).compare("DESC")!=0)
      keyProp.direction = ASC;
    else
      keyProp.direction = DESC;
    keyProp.sortKey.setExpstr(trim_copy(vKP[0]));
    keyProp.sortKey.getAggFuncs(m_initAggProps);
    if (m_groups.size() > 0) {// checking if compatible with GROUP
      vector<string> allColNames;
      for (int i=0; i<m_groups.size(); i++)
        m_groups[i].getAllColumnNames(allColNames);
      if (!keyProp.sortKey.groupFuncOnly() && !keyProp.sortKey.inColNamesRange(allColNames)){
        trace(WARNING, "Sorting key '%s' does not exist in Group or invalid using aggregation function \n", sSort.c_str());
        //continue;
        //return false;
      }
    }else{
      if (keyProp.sortKey.containGroupFunc()){
        trace(FATAL, "Invalid using aggregation function in sorting key '%s', no group involved!\n", sSort.c_str());
        return false;
      }
    }

    // discard non integer CONST
    // Any INTEGER number will be mapped to the correspond sequence of the selections.
    // Thus, m_selections should always be analyzed before m_sorts
    if (keyProp.sortKey.m_type==BRANCH || keyProp.sortKey.m_expType != CONST || (!isInt(keyProp.sortKey.m_expStr) && !isLong(keyProp.sortKey.m_expStr)) || atoi(keyProp.sortKey.m_expStr.c_str())>=m_selections.size()){
      m_sorts.push_back(keyProp);
    }
  }
  //trace(DEBUG1, "Got %d sorting keys!\n",m_sorts.size());
  return true;
}

bool QuerierC::setFieldTypeFromStr(string setstr)
{
  vector<string> vSetFields = split(setstr,',',"//''{}",'\\',{'(',')'});
  for (int i=0; i<vSetFields.size(); i++){
    vector<string> vField = split(vSetFields[i],' ',"//''{}",'\\',{'(',')'});
    vField = vField.size()>=2?vField:split(vSetFields[i],'\t',"//''{}",'\\',{'(',')'});
    if (vField.size()<2){
      trace(ERROR, "SET field type failed! Correct format is SET <FIELD> <TYPE>!\n");
      return false;
    }
    int iType = encodeDatatype(vField[1]);
    if (iType == UNKNOWN){
      trace(ERROR, "Unknown data type %s!\n", vField[1].c_str());
      return false;
    }else if(iType == DATE && vField.size() >= 2){
      //trace(DEBUG2, "SET field '%s' type '%s' extrainfor '%s'!\n",vField[0].c_str(),decodeDatatype(iType).c_str(),trim_pair(vField[2],"''").c_str());
      setFieldDatatype(vField[0], iType, trim_pair(vField[2],"''"));
    }else
      setFieldDatatype(vField[0], iType);
  }
  return true;
}

void QuerierC::setUserVars(string variables)
{
  vector<string> vVariables = split(variables,' ',"//''{}",'\\',{'(',')'});
  for (int i=0; i<vVariables.size(); i++){
    vector<string> vNameVal = split(vVariables[i],':',"//''{}",'\\',{'(',')'});
    if (vNameVal.size()<2){
      trace(ERROR, "Incorrect variable format!\n", vVariables[i].c_str());
      continue;
    }
    string sName=upper_copy(trim_copy(vNameVal[0])), sValue=trim_copy(vNameVal[1]);
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
  return (m_groups.size()>0 || m_sorts.size()>0 || m_aggrOnly || m_bUniqueResult);
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
      try{
        m_fieldnames.push_back("@FIELD"+boost::lexical_cast<std::string>(i));
      }catch (bad_lexical_cast &){
        m_fieldnames.push_back("?");
      }
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

void QuerierC::analyzeFiledTypes(namesaving_smatch matches)
{
  m_fieldtypes.clear();
  for (int i=1; i<matches.size(); i++){
    if (m_fieldnames.size()>i-1 && m_fieldntypes.find(m_fieldnames[i-1]) != m_fieldntypes.end()) 
      m_fieldtypes.push_back(m_fieldntypes[m_fieldnames[i-1]]);
    else if (m_fieldntypes.find("@FIELD"+intToStr(i)) != m_fieldntypes.end())
      m_fieldtypes.push_back(m_fieldntypes["@FIELD"+intToStr(i)]);
    else{
      DataTypeStruct dts;
      dts.datatype = detectDataType(matches[i], dts.extrainfo);
      // trace(DEBUG2, "Detected column '%s' from '%s', data type '%s' extrainfo '%s'\n", m_fieldnames[i-1].c_str(), string(matches[i]).c_str(), decodeDatatype(dts.datatype).c_str(), dts.extrainfo.c_str());
      if (dts.datatype==UNKNOWN)
        dts.datatype = STRING;
      m_fieldtypes.push_back(dts); // set UNKNOWN (real) data as STRING
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
    //trace(DEBUG2, "Eval aggregation function expression '%s'\n", ite->second.getEntireExpstr().c_str());
    if (ite->second.m_expType==FUNCTION && ite->second.m_Function && ite->second.m_Function->isAggFunc() && ite->second.m_Function->m_params.size()>0 && ite->second.m_Function->m_params[0].evalExpression(fieldvalues, varvalues, &aggGroupProp, sResult, dts)){
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
bool QuerierC::addResultToSet(vector<string>* fieldvalues, map<string,string>* varvalues, vector<string> rowValue, vector<ExpressionC> expressions, unordered_map< string,GroupProp >* aggFuncs, vector< vector<string> > & resultSet)
{
    vector<string> vResults;
    //vResults.push_back(rowValue[0]);
    vResults.push_back(""); // save memory
    for (int i=0; i<expressions.size(); i++){
      string sResult;
      DataTypeStruct dts;
      if (!expressions[i].containGroupFunc()){
        expressions[i].evalExpression(fieldvalues, varvalues, aggFuncs, sResult, dts);
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
  varValues.insert(m_uservariables.begin(), m_uservariables.end());
  unordered_map< string,GroupProp > aggGroupProp;
  //trace(DEBUG, "Filtering '%s' ", rowValue[0].c_str());
  //dumpMap(varValues);
  bool matched = (!m_filter || m_filter->compareExpression(&fieldValues, &varValues, &aggGroupProp));
#ifdef __DEBUG__
  m_filtercomptime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
  //trace(DEBUG, " selected:%d (%d)! \n", matched, m_selections.size());
  if (matched){
    if (m_selections.size()>0){
      if (m_groups.size() == 0 && m_initAggProps.size() == 0 && !m_aggrOnly){
        //trace(DEBUG, " No group! \n");
        if (!addResultToSet(&fieldValues, &varValues, rowValue, m_selections, &aggGroupProp, m_results))
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
            if (iSel >= 0)
              sResult = m_results[m_results.size()-1][iSel + 1];
            // if the sort key is a integer, get the result from the result set at the same sequence number
            else if (m_sorts[i].sortKey.m_type==LEAF && m_sorts[i].sortKey.m_expType==CONST && isInt(m_sorts[i].sortKey.m_expStr) && atoi(m_sorts[i].sortKey.m_expStr.c_str())<m_selections.size())
              sResult = m_results[m_results.size()-1][atoi(m_sorts[i].sortKey.m_expStr.c_str()) + 1];
            else
              m_sorts[i].sortKey.evalExpression(&fieldValues, &varValues, &aggGroupProp, sResult, dts);
            //trace(DEBUG, "eval '%s' => '%s'\n", m_sorts[i].sortKey.getEntireExpstr().c_str(), sResult.c_str());
          }else{
            trace(ERROR, "(3)Invalid using aggregation function in '%s', no group involved!\n", m_sorts[i].sortKey.getEntireExpstr().c_str());
            return false;
          }
          vResults.push_back(sResult);
        }
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
        bool dataSetExist = false;
        if (m_aggrOnly) // has aggregation function without any group, give an empty string as the key
          groupExps.push_back("");
        else // group expressions to the sorting keys
          for (int i=0; i<m_groups.size(); i++){
            m_groups[i].evalExpression(&fieldValues, &varValues, &aggGroupProp, sResult, dts);
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
        for (int i=0; i<m_selections.size(); i++){
          string sResult;
          DataTypeStruct dts;
          //trace(DEBUG1, "Selection '%s': %d \n",m_selections[i].getEntireExpstr().c_str(), m_selections[i].containGroupFunc());
          m_selections[i].evalExpression(&fieldValues, &varValues, &aggGroupProp, sResult, dts);
          //trace(DEBUG2, "Eval selection '%s', get '%s', from '%s' \n",m_selections[i].getEntireExpstr().c_str(), sResult.c_str(), rowValue[0].c_str());
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
            m_sorts[i].sortKey.evalExpression(&fieldValues, &varValues, &aggGroupProp, sResult, dts);
            aggSelResult.push_back(sResult);
            //trace(DEBUG1, "Got non-aggr selection '%s' \n",sResult.c_str());
          }
        }
#ifdef __DEBUG__
  m_evalSorttime += (curtime()-thistime);
  thistime = curtime();
#endif // __DEBUG__
        // dateSet.nonAggSels.push_back(aggSelResult);
        if (dataSetExist){
          // m_aggFuncTaget.erase(groupExps);
          m_aggGroupProp.erase(groupExps);
          m_aggSelResults.erase(groupExps);
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
      vector<ExpressionC> vKeys;
      for (int i=0;i<m_sorts.size();i++)
        vKeys.push_back(m_sorts[i].sortKey);
      if (!addResultToSet(&fieldValues, &varValues, rowValue, vKeys, &aggGroupProp, m_sortKeys))
        return false;
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

int QuerierC::searchNext(namesaving_smatch & matches)
{
  if (searchStopped()){
    //trace(DEBUG1, "stopped searching!\n");
    return 0;
  }
#ifdef __DEBUG__
  long int thistime = curtime();
#endif // __DEBUG__
  //string::const_iterator start = m_rawstr.begin(), end = m_rawstr.end();
  //smatch matches;
  //m_line++;
  int found = 0;
  try {
    while(!m_rawstr.empty() && regex_search(m_rawstr, matches, m_regexp)){
      m_line++;
      vector<string> matcheddata;
      for (int i=0; i<matches.size(); i++)
        matcheddata.push_back(matches[i]);
      if (m_fieldnames.size() == 0){
        pairFiledNames(matches);
        analyzeFiledTypes(matches);
        if (m_filter){
          m_filter->analyzeColumns(&m_fieldnames, &m_fieldtypes);
          //m_filter->mergeExprConstNodes();
        }
        for (int i=0; i<m_selections.size(); i++){
          //trace(DEBUG, "Analyzing selection '%s' (%d), found %d\n", m_selections[i].m_expStr.c_str(), i, found);
          m_selections[i].analyzeColumns(&m_fieldnames, &m_fieldtypes);
        }
        for (int i=0; i<m_groups.size(); i++)
          m_groups[i].analyzeColumns(&m_fieldnames, &m_fieldtypes);
        for (int i=0; i<m_sorts.size(); i++)
          m_sorts[i].sortKey.analyzeColumns(&m_fieldnames, &m_fieldtypes);
        for (unordered_map< string,ExpressionC >::iterator it=m_aggFuncExps.begin(); it!=m_aggFuncExps.end(); ++it)
          it->second.analyzeColumns(&m_fieldnames, &m_fieldtypes);
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
  //trace(DEBUG, "So far found: %d\n", m_matchcount);
#ifdef __DEBUG__
  m_searchtime += (curtime()-thistime);
#endif // __DEBUG__
  return found;
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
  namesaving_smatch matches(m_regexstr);
  int found = searchNext(matches);
  while (found>0 && !m_rawstr.empty()){
    //trace(DEBUG, "Searching pattern: '%s' in '%s', found %d\n", m_regexstr.c_str(), m_rawstr.c_str(), totalfound);
    found = searchNext(matches);
    totalfound+=found;
  }
#ifdef __DEBUG__
  trace(DEBUG2, "Searching time: %d, filtering time: %d, eval group key time: %d, filter compare time: %d, prepare Agg GP time: %d, eval agg expression time: %d, eval selection time: %d, eval sort time: %d, update restult time: %d, matched lines: %d, found: %d\n", m_searchtime, m_filtertime, m_evalGroupKeytime, m_filtercomptime, m_prepAggGPtime, m_evalAggExptime, m_evalSeltime, m_evalSorttime, m_updateResulttime, m_line, totalfound);
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
  //trace(DEBUG2, "m_aggSelResults size: %d, m_groupKeys size: %d\n", m_aggSelResults.size(), m_groupKeys.size());
  //for (unordered_map< vector<string>, vector<string>, hash_container< vector<string> > >::iterator it=m_aggSelResults.begin(); it!=m_aggSelResults.end(); ++it)
  //  dumpVector(it->first);
  for (std::set< vector<string> >::iterator it=m_groupKeys.begin(); it!=m_groupKeys.end(); ++it){
    if (m_filter && !m_filter->compareExpression(&vfieldvalues, &mvarvalues, &m_aggGroupProp[*it]))
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
        bool bToBeSwapped = false;
        if (m_sorts[i].direction==ASC)
          bToBeSwapped = (anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype)>0);
        else
          bToBeSwapped = (anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype)<0);
//#ifdef __DEBUG__
  //trace(DEBUG2, "Checking %s(L) %s(R) (%d %d %d) (%s) (%d %d)\n", (*(m_sortKeys.begin()+iLPos))[i].c_str(), (*(m_sortKeys.begin()+iRPos))[i].c_str(),iLPos,iCheckPos,iRPos,decodeDatatype(m_sorts[i].sortKey.m_datatype.datatype).c_str(), m_sorts[i].direction, bToBeSwapped);
//#endif // __DEBUG__
        //if (bToBeSwapped){
        if ((m_sorts[i].direction==ASC ? anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype)>0 : anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype)<0)){
        //if (anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype)>0){
          //vector<string> tmp;
          //tmp.insert(tmp.begin(), (*(m_results.begin()+iRPos)).begin(), (*(m_results.begin()+iRPos)).end());
//#ifdef __DEBUG__
  //trace(DEBUG2, "moving %s(R) before %s(L) (%d %d %d)\n", (*(m_sortKeys.begin()+iRPos))[i].c_str(), (*(m_sortKeys.begin()+iLPos))[i].c_str(),iLPos,iCheckPos,iRPos);
//#endif // __DEBUG__
          //trace(DEBUG1, "Before move: %s(%d) %s(%d)\n", (*(m_results.begin()+iLPos))[2].c_str(), iLPos, (*(m_results.begin()+iRPos))[2].c_str(), iRPos);
          m_results.insert(m_results.begin()+iLPos,*(m_results.begin()+iRPos));
          m_results.erase(m_results.begin()+iRPos+1);
          m_sortKeys.insert(m_sortKeys.begin()+iLPos,*(m_sortKeys.begin()+iRPos));
          m_sortKeys.erase(m_sortKeys.begin()+iRPos+1);
          //trace(DEBUG1, "After move: %s(%d) %s(%d)\n", (*(m_results.begin()+iLPos))[2].c_str(), iLPos, (*(m_results.begin()+iRPos))[2].c_str(), iRPos);
          exchanged = true;
          break;
        }
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

// sort result
bool QuerierC::sort()
{
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
  mergeSort(0,max(0,(int)floor(m_sortKeys.size())/2-1), min((int)(m_sortKeys.size()-1),max(0,(int)floor(m_sortKeys.size())/2-1)+1),m_sortKeys.size()-1);
  //trace(DEBUG1, "Sorting completed \n");
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
  m_aggFuncExps.clear();
  m_sortKeys.clear();
  m_bNamePrinted = false;
  m_aggrOnly = false;
  m_bUniqueResult = false;
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
