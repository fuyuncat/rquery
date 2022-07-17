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
  m_limitbottom = 0;
  m_limittop = -1;
  m_bNamePrinted = false;
  m_aggrOnly = false;
#ifdef __DEBUG__
  m_searchtime = 0;
  m_filtertime = 0;
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
      return false;
    }
    m_selnames.push_back(sSel);
    ExpressionC eSel(sSel);
    vector<string> allColNames;
    for (int i=0; i<m_groups.size(); i++)
      m_groups[i].getAllColumnNames(allColNames);
    if (!eSel.groupFuncOnly() && !eSel.inColNamesRange(allColNames)){
      trace(ERROR, "Selection '%s' does not exist in Group or invalid using aggregation function \n", sSel.c_str());
      //continue;
      //return false;
    }

    if (eSel.containGroupFunc())
      bGroupFunc = true;
    else
      bNonGroupFuncSel = true;

    m_selections.push_back(eSel);
    eSel.dump();
    //trace(DEBUG, "Selection: '%s'!\n", eSel.getEntireExpstr().c_str());
  }
  m_aggrOnly = (bGroupFunc && !bNonGroupFuncSel);
  if (!m_aggrOnly && m_groups.size() == 0){ // selection has non-aggregation function but non group field.
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
    if (m_groups.size() > 0) {// checking if compatible with GROUP
      vector<string> allColNames;
      for (int i=0; i<m_groups.size(); i++)
        m_groups[i].getAllColumnNames(allColNames);
      if (!keyProp.sortKey.groupFuncOnly() && !keyProp.sortKey.inColNamesRange(allColNames)){
        trace(ERROR, "Sorting key '%s' does not exist in Group or invalid using aggregation function \n", sSort.c_str());
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
      setFieldDatatype(vField[0], iType, vField[2]);
    }else
      setFieldDatatype(vField[0], iType);
  }
  return true;
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
  return (m_groups.size()>0 || m_sorts.size()>0 || m_aggrOnly);
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
      dts.datatype = UNKNOWN?STRING:dts.datatype;
      m_fieldtypes.push_back(dts); // set UNKNOWN (real) data as STRING
    }
    trace(DEBUG, "Detected column '%s' data type '%s'\n", m_fieldnames[i-1].c_str(), decodeDatatype(m_fieldtypes[m_fieldtypes.size()-1].datatype).c_str());
  }
}

void QuerierC::evalAggExpNode(ExpressionC* node, vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,vector<string> > & aggFuncTaget)
{
  if (node->m_type == LEAF){ // eval leaf and store
    if (node->m_expType == FUNCTION && node->groupFuncOnly()){
      string sResult;
      FunctionC* func = new FunctionC(node->m_expStr);
      func->analyzeColumns(&m_fieldnames, &m_fieldtypes);
      bool gotResult = func->runFunction(fieldnames, fieldvalues, varvalues, sResult);
      func->clear();
      delete func;
      if (gotResult){
        string sFuncStr = node->getEntireExpstr();
        //trace(DEBUG, "Eval: '%s'...\n", sFuncStr.c_str());
        vector<string> newdata = aggFuncTaget[sFuncStr];
        if (newdata.size() == 0)
          aggFuncTaget[sFuncStr].push_back(sResult);
        else{
          newdata.push_back(sResult);
          aggFuncTaget[sFuncStr] = newdata;
          //aggFuncTaget.erase(sFuncStr);
          //aggFuncTaget.insert( pair< string,vector<string> >(sFuncStr,newdata));
        }
        //if (aggFuncTaget.find(sFuncStr) != aggFuncTaget.end())
        //  aggFuncTaget[sFuncStr].push_back(sResult);
        //else{
        //  vector<string> newdata;
        //  newdata.push_back(sResult);
        //  aggFuncTaget.insert( pair< string,vector<string> >(sFuncStr,newdata));
        //}
        //trace(DEBUG, "aggFuncTaget: '%s'(%d)!\n", sFuncStr.c_str(), aggFuncTaget[sFuncStr].size());
      }else{
        trace(ERROR, "Failed to eval aggregation parameter!\n");
        return;
      }
    }
  }else{ // eval branch and store
    if (node->m_leftNode)
      evalAggExpNode(node->m_leftNode, fieldnames, fieldvalues, varvalues, aggFuncTaget);
    if (node->m_rightNode)
      evalAggExpNode(node->m_rightNode, fieldnames, fieldvalues, varvalues, aggFuncTaget);
  }
}

void QuerierC::evalAggExpNode(ExpressionC* node, vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp > & aggGroupProp)
{
  if (node->m_type == LEAF){ // eval leaf and store
    if (node->m_expType == FUNCTION && node->groupFuncOnly()){
      string sResult;
      FunctionC* func = new FunctionC(node->m_expStr);
      func->analyzeColumns(&m_fieldnames, &m_fieldtypes);
      bool gotResult = func->runFunction(fieldnames, fieldvalues, varvalues, sResult);
      func->clear();
      delete func;
      if (gotResult){
        string sFuncStr = node->getEntireExpstr();
        //trace(DEBUG, "Eval: '%s'...\n", sFuncStr.c_str());
        unordered_map< string,GroupProp >::iterator it = aggGroupProp.find(sFuncStr);
        if (it == aggGroupProp.end()){
          GroupProp gp;
          switch(node->m_funcID){
          case AVERAGE:
            if (isDouble(sResult))
              gp.sum = atof(sResult.c_str());
            gp.count = 1;
            break;
          case SUM:
            if (isDouble(sResult))
              gp.sum = atof(sResult.c_str());
            break;
          case COUNT:
            gp.count = 1;
            break;
          case UNIQUECOUNT:
            gp.uniquec.insert(sResult);
            break;
          case MAX:
            gp.max = sResult;
            break;
          case MIN:
            gp.min = sResult;
            break;
          }
          aggGroupProp.insert(pair<string,GroupProp>(sFuncStr,gp));
        }else{
          switch(node->m_funcID){
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
            break;
          case UNIQUECOUNT:
            it->second.uniquec.insert(sResult);
            break;
          case MAX:
            if (sResult.compare(it->second.max)>0)
              it->second.max = sResult;
            break;
          case MIN:
            if (sResult.compare(it->second.max)<0)
              it->second.min = sResult;
            break;
          }
        }
      }else{
        trace(ERROR, "Failed to eval aggregation parameter!\n");
        return;
      }
    }
  }else{ // eval branch and store
    if (node->m_leftNode)
      evalAggExpNode(node->m_leftNode, fieldnames, fieldvalues, varvalues, aggGroupProp);
    if (node->m_rightNode)
      evalAggExpNode(node->m_rightNode, fieldnames, fieldvalues, varvalues, aggGroupProp);
  }
}

// add a data row to a result set
bool QuerierC::addResultToSet(vector<string>* fieldvalues, map<string,string>* varvalues, vector<string> rowValue, vector<ExpressionC> expressions, vector< vector<string> > & resultSet)
{
    vector<string> vResults;
    //vResults.push_back(rowValue[0]);
    vResults.push_back(""); // save memory
    for (int i=0; i<expressions.size(); i++){
      string sResult;
      if (!expressions[i].containGroupFunc()){
        expressions[i].evalExpression(&m_fieldnames, fieldvalues, varvalues, sResult);
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
bool QuerierC::matchFilter(vector<string> rowValue, FilterC* filter)
{
  //if (!filter){
  //  //trace(INFO, "No filter defined\n");
  //  return true;
  //}
#ifdef __DEBUG__
  long int thistime = curtime();
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
  //trace(DEBUG, "Filtering '%s' ", rowValue[0].c_str());
  //dumpMap(varValues);
  bool matched = (!filter || filter->compareExpression(&m_fieldnames, &fieldValues, &varValues));
  //trace(DEBUG, " selected:%d (%d)! \n", matched, m_selections.size());
  if (matched){
    if (m_selections.size()>0){
      if (m_groups.size() == 0 && !m_aggrOnly){
        //trace(DEBUG, " No group! \n");
        if (!addResultToSet(&fieldValues, &varValues, rowValue, m_selections, m_results))
          return false;
        
        vector<string> vResults;
        for (int i=0; i<m_sorts.size(); i++){
          string sResult;
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
              m_sorts[i].sortKey.evalExpression(&m_fieldnames, &fieldValues, &varValues, sResult);
            //trace(DEBUG, "eval '%s' => '%s'\n", m_sorts[i].sortKey.getEntireExpstr().c_str(), sResult.c_str());
          }else{
            trace(ERROR, "(3)Invalid using aggregation function in '%s', no group involved!\n", m_sorts[i].sortKey.getEntireExpstr().c_str());
            return false;
          }
          vResults.push_back(sResult);
        }
        m_sortKeys.push_back(vResults);
      }else{ // need to do group. store evaled data in a temp date set
        //trace(DEBUG, " Grouping! \n");
        vector<string> groupExps;  // the group expressions. as the key of following hash map
        vector<string> nonAggVals;
        //unordered_map< string,vector<string> > aggFuncTaget;
        unordered_map< string,GroupProp > aggGroupProp;
        //GroupDataSet dateSet;
        string sResult;
        bool dataSetExist = false;
        if (m_aggrOnly) // has aggregation function without any group, give an empty string as the key
          groupExps.push_back("");
        else // group expressions to the sorting keys
          for (int i=0; i<m_groups.size(); i++){
            m_groups[i].evalExpression(&m_fieldnames, &fieldValues, &varValues, sResult);
            groupExps.push_back(sResult);
          }
        unordered_map< vector<string>, vector<string>, hash_container< vector<string> > >::iterator it1 = m_nonAggSels.find(groupExps);
        unordered_map< vector<string>, unordered_map< string,GroupProp >, hash_container< vector<string> > >::iterator it2 = m_aggGroupProp.find(groupExps);
        //unordered_map< vector<string>, unordered_map< string,vector<string> >, hash_container< vector<string> > >::iterator it2 = m_aggFuncTaget.find(groupExps);
        if (it1 != m_nonAggSels.end() && it2 != m_aggGroupProp.end()){
        //if (it1 != m_nonAggSels.end() && it2 != m_aggFuncTaget.end()){
          nonAggVals = it1->second; // only need keep one nonAggVals for each groupExps
          //for (unordered_map< string,vector<string> >::iterator it=it2->second.begin(); it!=it2->second.end(); ++it)
          //  aggFuncTaget.insert(pair< string,vector<string> >(it->first,it->second));
          for (unordered_map< string,GroupProp >::iterator it=it2->second.begin(); it!=it2->second.end(); ++it)
            aggGroupProp.insert(pair< string,GroupProp >(it->first,it->second));
          dataSetExist = true;
        }

        // get data sets
        if (!dataSetExist) // only need keep one nonAggVals for each groupExps
          //nonAggVals.push_back(rowValue[0]); // nonAggSels store raw string in the first member. This is kinda meaningless.
          nonAggVals.push_back(""); // save memory!! no point to store a whole raw string any more
        for (int i=0; i<m_selections.size(); i++){
          string sResult;
          //trace(DEBUG1, "Selection '%s': %d \n",m_selections[i].getEntireExpstr().c_str(), m_selections[i].containGroupFunc());
          if (!m_selections[i].containGroupFunc() && !dataSetExist){ // non aggregation function selections
            m_selections[i].evalExpression(&m_fieldnames, &fieldValues, &varValues, sResult);
            nonAggVals.push_back(sResult);
            //trace(DEBUG1, "Got non-aggr selection '%s' \n",sResult.c_str());
          }else{
            // eval agg function parameter expression and store in the temp data set
            evalAggExpNode(&m_selections[i], &m_fieldnames, &fieldValues, &varValues, aggGroupProp);
            //evalAggExpNode(&m_selections[i], &m_fieldnames, &fieldValues, &varValues, aggFuncTaget);
          }
        }
        for (int i=0; i<m_sorts.size(); i++){
          string sResult;
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
          else if (!m_sorts[i].sortKey.containGroupFunc() && !dataSetExist){ // non aggregation function selections
            m_sorts[i].sortKey.evalExpression(&m_fieldnames, &fieldValues, &varValues, sResult);
            nonAggVals.push_back(sResult);
            //trace(DEBUG1, "Got non-aggr selection '%s' \n",sResult.c_str());
          }else{
            // eval agg function parameter expression and store in the temp data set
            evalAggExpNode(&m_selections[i], &m_fieldnames, &fieldValues, &varValues, aggGroupProp);
            //evalAggExpNode(&(m_sorts[i].sortKey), &m_fieldnames, &fieldValues, &varValues, aggFuncTaget);
          }
        }
        // dateSet.nonAggSels.push_back(nonAggVals);
        if (dataSetExist){
          // m_aggFuncTaget.erase(groupExps);
          m_aggGroupProp.erase(groupExps);
        }else{
          m_nonAggSels.insert( pair<vector<string>, vector<string> >(groupExps,nonAggVals));
        }
        // m_aggFuncTaget.insert( pair<vector<string>, unordered_map< string,vector<string> > >(groupExps,aggFuncTaget));
        m_aggGroupProp.insert( pair<vector<string>, unordered_map< string,GroupProp > >(groupExps,aggGroupProp));
      }
    }else{
      m_results.push_back(rowValue);
      vector<ExpressionC> vKeys;
      for (int i=0;i<m_sorts.size();i++)
        vKeys.push_back(m_sorts[i].sortKey);
      if (!addResultToSet(&fieldValues, &varValues, rowValue, vKeys, m_sortKeys))
        return false;
    }
  }
#ifdef __DEBUG__
  m_filtertime += (curtime()-thistime);
#endif // __DEBUG__
  return matched;
}

bool QuerierC::searchStopped()
{
  //trace(DEBUG1, "Limit checking. m_groups:%d, m_sorts:%d, m_limittop:%d, m_outputrow:%d!\n",m_groups.size(),m_sorts.size(),m_limittop,m_outputrow);
  if (m_groups.size()==0 && !m_aggrOnly && m_sorts.size()==0 && m_limittop>=0 && m_outputrow>m_limittop)
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
      }
      // append variables
      //matcheddata.push_back(m_filename);
      matcheddata.push_back(intToStr(m_line));
      matcheddata.push_back(intToStr(m_matchcount+1));
      if (matchFilter(matcheddata, m_filter)){
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
  //trace(DEBUG2, "\rSearching time: %d, agg size: %d, aggFuncTarge size: %d, filtering time: %d, matched lines: %d, found: %d", m_searchtime, m_aggFuncTaget.size(), m_aggFuncTaget.size()==0?0:m_aggFuncTaget.begin()->second.begin()->second.size(), m_filtertime, m_line, totalfound);
#endif // __DEBUG__
  return totalfound;
}

// run the aggregation functions in an expression 
void QuerierC::runAggFuncExp(ExpressionC* node, unordered_map< string,vector<string> >* dateSet, string & sResult)
{
  if (node->m_type == LEAF){ // eval leaf and store
    if (node->m_expType == FUNCTION && node->containGroupFunc()){
      string sFuncStr = node->getEntireExpstr();
      //trace(DEBUG,"Processing aggregation Function %s\n",sFuncStr.c_str());
      if (dateSet->find(sFuncStr) != dateSet->end()){
        //trace(DEBUG,"Data size:%d\n", (*dateSet)[sFuncStr].size());
        if (sFuncStr.find("SUM(")!=string::npos){
          double dSum=0;
          for (int i=0; i<(*dateSet)[sFuncStr].size(); i++){
            if (isDouble((*dateSet)[sFuncStr][i]))
              dSum+=atof((*dateSet)[sFuncStr][i].c_str());
            else
              trace(ERROR, "Invalid number '%s' to be SUM up!\n", (*dateSet)[sFuncStr][i].c_str());
          }
          sResult = doubleToStr(dSum);
        }else if (sFuncStr.find("AVERAGE(")!=string::npos){
          double dSum=0;
          for (int i=0; i<(*dateSet)[sFuncStr].size(); i++){
            if (isDouble((*dateSet)[sFuncStr][i]))
              dSum+=atof((*dateSet)[sFuncStr][i].c_str());
            else
              trace(ERROR, "Invalid number '%s' to be SUM up!\n", (*dateSet)[sFuncStr][i].c_str());
          }
          sResult = doubleToStr(dSum/(*dateSet)[sFuncStr].size());
        }else if (sFuncStr.find("COUNT(")!=string::npos){
          sResult = intToStr((*dateSet)[sFuncStr].size());
        }else if (sFuncStr.find("UNIQUECOUNT(")!=string::npos){
          std::set<string> uniqueSet;
          for (int i=0; i<(*dateSet)[sFuncStr].size(); i++)
            uniqueSet.insert((*dateSet)[sFuncStr][i]);
          sResult = intToStr(uniqueSet.size());
        }else if (sFuncStr.find("MAX(")!=string::npos){
          sResult=(*dateSet)[sFuncStr][0];
          for (int i=1; i<(*dateSet)[sFuncStr].size(); i++)
            if (sResult.compare((*dateSet)[sFuncStr][i])<0)
              sResult = (*dateSet)[sFuncStr][i];
        }else if (sFuncStr.find("MIN(")!=string::npos){
          sResult=(*dateSet)[sFuncStr][0];
          for (int i=1; i<(*dateSet)[sFuncStr].size(); i++)
            if (sResult.compare((*dateSet)[sFuncStr][i])>0)
              sResult = (*dateSet)[sFuncStr][i];
        }else{
          trace(ERROR, "Invalid aggregation function '%s'!\n", node->getEntireExpstr().c_str());
          return;
        }
      }else{
        trace(ERROR, "No data found for aggregation function '%s'!\n", node->getEntireExpstr().c_str());
        return;
      }
    }else{
      trace(ERROR, "No aggregation function found!\n");
      return;
    }
  }else{ // eval branch and store
    string sLeftRslt, sRightRslt;
    if (node->m_leftNode)
      runAggFuncExp(node->m_leftNode, dateSet, sLeftRslt);
    if (node->m_rightNode)
      runAggFuncExp(node->m_rightNode, dateSet, sRightRslt);
    anyDataOperate(sLeftRslt, node->m_operate, sRightRslt, node->m_datatype, sResult);
  }
}

// run the aggregation functions in an expression 
void QuerierC::runAggFuncExp(ExpressionC* node, unordered_map< string,GroupProp > * dateSet, string & sResult)
{
  if (node->m_type == LEAF){ // eval leaf and store
    if (node->m_expType == FUNCTION && node->containGroupFunc()){
      string sFuncStr = node->getEntireExpstr();
      //trace(DEBUG,"Processing aggregation Function %s\n",sFuncStr.c_str());
      unordered_map< string,GroupProp >::iterator it = dateSet->find(sFuncStr);
      if (it != dateSet->end()){
        switch(node->m_funcID){
        case AVERAGE:
          sResult = doubleToStr(it->second.sum/(long double)(it->second.count));
          break;
        case SUM:
          sResult = doubleToStr(it->second.sum);
          break;
        case COUNT:
          sResult = intToStr(it->second.count);
          break;
        case UNIQUECOUNT:
          sResult = intToStr(it->second.uniquec.size());
          break;
        case MAX:
          sResult = it->second.max;
          break;
        case MIN:
          sResult = it->second.min;
          break;
        default:
          trace(ERROR, "Invalid aggregation function '%s'!\n", node->getEntireExpstr().c_str());
          return;
        }
      }else{
        trace(ERROR, "No data found for aggregation function '%s'!\n", sFuncStr.c_str());
        return;
      }
    }else{
      trace(ERROR, "No aggregation function found!\n");
      return;
    }
  }else{ // eval branch and store
    string sLeftRslt, sRightRslt;
    if (node->m_leftNode)
      runAggFuncExp(node->m_leftNode, dateSet, sLeftRslt);
    if (node->m_rightNode)
      runAggFuncExp(node->m_rightNode, dateSet, sRightRslt);
    anyDataOperate(sLeftRslt, node->m_operate, sRightRslt, node->m_datatype, sResult);
  }
}

// group result
bool QuerierC::group()
{
  if ((m_groups.size() == 0 && !m_aggrOnly) || m_nonAggSels.size() == 0)
    return true;

  for (unordered_map< vector<string>, vector<string>, hash_container< vector<string> > >::iterator it=m_nonAggSels.begin(); it!=m_nonAggSels.end(); ++it){
    vector<string> vResults;
    vResults.push_back(it->second[0]);
    int iNonAggSelID = 1;
    //trace(DEBUG1, "Selection: %d:%d\n", m_selections.size(), it->second.size());
    for (int i=0; i<m_selections.size(); i++){
      string sResult;
      if (!m_selections[i].containGroupFunc()){ // non aggregation function selections
        //trace(DEBUG1, "None aggr func selection: %s\n", it->second[iNonAggSelID].c_str());
        vResults.push_back(it->second[iNonAggSelID]);
        iNonAggSelID++;
      }else{
        // eval agg function parameter expression and store in the temp data set
        string sResult;
        //runAggFuncExp(&m_selections[i], &(m_aggFuncTaget[it->first]), sResult);
        runAggFuncExp(&m_selections[i], &(m_aggGroupProp[it->first]), sResult);
        vResults.push_back(sResult);
      }
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
      }else if (!m_sorts[i].sortKey.containGroupFunc()){ // non aggregation function selections
        //trace(DEBUG1, "None aggr func selection: %s\n", it->second[iNonAggSelID].c_str());
        vResults.push_back(it->second[iNonAggSelID]);
        iNonAggSelID++;
      }else{
        // eval agg function parameter expression and store in the temp data set
        string sResult;
        //runAggFuncExp(&(m_sorts[i].sortKey), &(m_aggFuncTaget[it->first]), sResult);
        runAggFuncExp(&(m_sorts[i].sortKey), &(m_aggGroupProp[it->first]), sResult);
        vResults.push_back(sResult);
      }
    }
    m_sortKeys.push_back(vResults);
  }
}

// doing merging sort exchanging
void QuerierC::mergeSort(int iLeftB, int iLeftT, int iRightB, int iRightT)
{
  //trace(DEBUG1, "Mergeing %d %d %d %d\n", iLeftB, iLeftT, iRightB, iRightT);
  if (iLeftT >= iRightB || iLeftB > iLeftT || iRightB > iRightT)
    return;
  else{
    mergeSort(iLeftB, iLeftB+(int)floor(iLeftT-iLeftB)/2, iLeftB+(int)floor(iLeftT-iLeftB)/2+1, iLeftT);
    mergeSort(iRightB, iRightB+(int)floor(iRightT-iRightB)/2, iRightB+(int)floor(iRightT-iRightB)/2+1, iRightT);
//#ifdef __DEBUG__
//  for (int i=0; i<m_sortKeys.size(); i++)
//    printf("%s(%d) ", (*(m_sortKeys.begin()+i))[0].c_str(), i);
//  printf("\n");
//#endif // __DEBUG__
    int iLPos = iLeftB, iRPos = iRightB, iCheckPos = iRightB;
    while (iLPos<iCheckPos && iRPos<=iRightT){
      //trace(DEBUG1, "Swaping %d %d %d %d\n", iLPos, iCheckPos, iRPos, iRight);
      bool exchanged = false;
      for (int i=0; i<m_sorts.size(); i++){
        bool bToBeSwapped = false;
        if (m_sorts[i].direction==ASC)
          bToBeSwapped = (anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype)>0);
        else
          bToBeSwapped = (anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype)<0);
//#ifdef __DEBUG__
//  trace(DEBUG1, "Checking %s(L) %s(R) (%d %d %d) (%s) (%d %d)\n", (*(m_sortKeys.begin()+iLPos))[i].c_str(), (*(m_sortKeys.begin()+iRPos))[i].c_str(),iLPos,iCheckPos,iRPos,decodeDatatype(m_sorts[i].sortKey.m_datatype.datatype).c_str(), m_sorts[i].direction, bToBeSwapped);
//#endif // __DEBUG__
        //if (bToBeSwapped){
        if ((m_sorts[i].direction==ASC ? anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype)>0 : anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype)<0)){
        //if (anyDataCompare((*(m_sortKeys.begin()+iLPos))[i],(*(m_sortKeys.begin()+iRPos))[i],m_sorts[i].sortKey.m_datatype)>0){
          //vector<string> tmp;
          //tmp.insert(tmp.begin(), (*(m_results.begin()+iRPos)).begin(), (*(m_results.begin()+iRPos)).end());
//#ifdef __DEBUG__
//  trace(DEBUG1, "moving %s(R) before %s(L) (%d %d %d)\n", (*(m_sortKeys.begin()+iRPos))[i].c_str(), (*(m_sortKeys.begin()+iLPos))[i].c_str(),iLPos,iCheckPos,iRPos);
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
  //trace(DEBUG1, "Sorting begins \n");
  if (m_sorts.size() == 0 || m_sortKeys.size() == 0){
    //trace(DEBUG1, "No sorting keys %d %d!\n",m_sortKeys.size(),m_sorts.size());
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
  mergeSort(0,(int)floor(m_sortKeys.size())/2, (int)floor(m_sortKeys.size())/2+1,m_sortKeys.size()-1);
  //trace(DEBUG1, "Sorting completed \n");
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
    if (m_outputrow==0){
      printf("{\n");
      printf("\t\"records\": [\n");
    }else{
      printf(",\n");
    }
  }
  m_outputrow++;
  if (m_outputrow<m_limitbottom || (m_limittop>=0 && m_outputrow>m_limittop))
    return;
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
    printf("\t\"MatchedLines\": %d,\n", m_line);
    printf("\t\"SelectedRows\": %d\n", m_outputrow);
    printf("}\n");
  }else if (bPrintHeader){
    //if (mode == READLINE)
    //  printf("Read %d lines(s).\n", total);
    //else
    //  printf("Read %d byte(s).\n", total);
    printf("Pattern matched %d line(s).\n", m_line);
    printf("Selected %d row(s).\n", m_outputrow);
  }
}

void QuerierC::clear()
{
  m_groups.clear();
  m_sorts.clear();
  m_nonAggSels.clear();
  //m_aggFuncTaget.clear();
  m_aggGroupProp.clear();
  m_sortKeys.clear();
  m_bNamePrinted = false;
  m_outputformat = TEXT;
  if (m_filter){
    m_filter->clear();
    delete m_filter;
  }
  m_filter = NULL;
  m_limitbottom = 0;
  m_limittop = -1;
  m_filename = "";
  init();
}
