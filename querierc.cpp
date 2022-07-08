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
#include <boost/algorithm/string.hpp>
#include "function.h"
#include "querierc.h"

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
  m_matchcount = 0; 
  m_outputrow = 0;
  m_bNamePrinted = false;
}

void QuerierC::setregexp(string regexstr)
{
  m_regexstr = regexstr;
  m_regexp = sregex::compile(m_regexstr);
}

void QuerierC::assignFilter(FilterC* filter)
{
  m_filter = filter;
  m_filter->dump();
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
  vector<string> vGroups = split(groupstr,',',"''{}()",'\\');
  for (int i=0; i<vGroups.size(); i++){
    trace(DEBUG, "Processing group (%d) '%s'!\n", i, vGroups[i].c_str());
    string sGroup = boost::algorithm::trim_copy<string>(vGroups[i]);
    if (sGroup.empty()){
      trace(ERROR, "Empty group string!\n");
      return false;
    }
    ExpressionC eGroup(sGroup);
    m_groups.push_back(eGroup);
  }
  return true;
}

// m_groups should always be analyzed before m_selections
bool QuerierC::assignSelString(string selstr)
{
  vector<string> vSelections = split(selstr,',',"''{}()",'\\');
  for (int i=0; i<vSelections.size(); i++){
    trace(DEBUG, "Processing selection(%d) '%s'!\n", i, vSelections[i].c_str());
    string sSel = boost::algorithm::trim_copy<string>(vSelections[i]);
    if (sSel.empty()){
      trace(ERROR, "Empty selection string!\n");
      return false;
    }
    m_selnames.push_back(sSel);
    ExpressionC eSel(sSel);
    if (m_groups.size() > 0) {// checking if compatible with GROUP
      vector<string> allColNames;
      for (int i=0; i<m_groups.size(); i++)
        m_groups[i].getAllColumnNames(allColNames);
      if (!eSel.groupFuncOnly() && !eSel.inColNamesRange(allColNames)){
        trace(ERROR, "Selection '%s' does not exist in Group or invalid using aggregation function \n", sSel.c_str());
        //continue;
        //return false;
      }
    }else{
      if (eSel.containGroupFunc()){
        trace(FATAL, "Invalid using aggregation function in selection '%s', no group involved!\n", sSel.c_str());
        return false;
      }
    }
    m_selections.push_back(eSel);
  }
  return true;
}

// m_groups, m_selections should always be analyzed before m_sorts
bool QuerierC::assignSortStr(string sortstr)
{
  vector<string> vSorts = split(sortstr,',',"''{}()",'\\');
  for (int i=0; i<vSorts.size(); i++){
    trace(DEBUG, "Processing sorting keys (%d) '%s'!\n", i, vSorts[i].c_str());
    string sSort = boost::algorithm::trim_copy<string>(vSorts[i]);
    if (sSort.empty()){
      trace(ERROR, "Empty sorting key!\n");
      return false;
    }
    ExpressionC eSort(sSort);
    if (m_groups.size() > 0) {// checking if compatible with GROUP
      vector<string> allColNames;
      for (int i=0; i<m_groups.size(); i++)
        m_groups[i].getAllColumnNames(allColNames);
      if (!eSort.groupFuncOnly() && !eSort.inColNamesRange(allColNames)){
        trace(ERROR, "Sorting key '%s' does not exist in Group or invalid using aggregation function \n", sSort.c_str());
        //continue;
        //return false;
      }
    }else{
      if (eSort.containGroupFunc()){
        trace(FATAL, "Invalid using aggregation function in sorting key '%s', no group involved!\n", sSort.c_str());
        return false;
      }
    }
    
    // discard non integer CONST
    // Any INTEGER number will be mapped to the correspond sequence of the selections.
    // Thus, m_selections should always be analyzed before m_sorts
    if (eSort.m_type==BRANCH || eSort.m_expType != CONST || (!isInt(eSort.m_expStr) && !isLong(eSort.m_expStr)) || atoi(eSort.m_expStr.c_str())>=m_selections.size())
      m_sorts.push_back(eSort);
  }
  //trace(DEBUG1, "Got %d sorting keys!\n",m_sorts.size());
  return true;
}

bool QuerierC::setFieldTypeFromStr(string setstr)
{
  vector<string> vSetFields = split(setstr,',',"//''{}",'\\');
  for (int i=0; i<vSetFields.size(); i++){
    vector<string> vField = split(vSetFields[i],' ',"//''{}",'\\');
    vField = vField.size()>=2?vField:split(vSetFields[i],'\t',"//''{}",'\\');
    if (vField.size()<2){
      trace(ERROR, "SET field type failed! Correct format is SET <FIELD> <TYPE>!\n");
      return false;
    }
    int iType = encodeDatatype(vField[1]);
    if (iType == UNKNOWN){
      trace(ERROR, "Unknown data type %s!\n", vField[1].c_str());
      return false;
    }else
      setFieldDatatype(vField[0], encodeDatatype(vField[1]));
  }
  return true;
}

void QuerierC::pairFiledNames(namesaving_smatch matches)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast; 

  for (int i=1; i<matches.size(); i++){
    bool foundName = false;
    for (vector<string>::const_iterator it = matches.names_begin(); it != matches.names_end(); ++it)
      if (&(matches[i]) == &(matches[*it])){
        m_fieldnames.push_back(boost::to_upper_copy<string>(string(*it)));
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

void QuerierC::setFieldDatatype(string field, int datetype)
{
  string fname = boost::to_upper_copy<string>(field);
  if (m_fieldntypes.find(fname) != m_fieldntypes.end())
    m_fieldntypes[fname] = datetype;
  else
    m_fieldntypes.insert( pair<string, int>(fname,datetype) );
  for (int i=0;i<m_fieldnames.size();i++){
    if (fname.compare(m_fieldnames[i]) == 0 && i<m_fieldtypes.size())
      m_fieldtypes[i] = datetype;
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
      int iType = detectDataType(matches[i]);
      m_fieldtypes.push_back(iType==UNKNOWN?STRING:iType); // set UNKNOWN (real) data as STRING
    }
    trace(DEBUG, "Detected column '%s' data type '%s'\n", m_fieldnames[i-1].c_str(), decodeDatatype(m_fieldtypes[m_fieldtypes.size()-1]).c_str());
  }
}

void QuerierC::evalAggExpNode(ExpressionC* node, vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, map< string,vector<string> > & aggFuncTaget)
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
        if (aggFuncTaget.find(sFuncStr) != aggFuncTaget.end())
          aggFuncTaget[sFuncStr].push_back(sResult);
        else{
          vector<string> newdata;
          newdata.push_back(sResult);
          aggFuncTaget.insert( pair< string,vector<string> >(sFuncStr,newdata));
        }
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

// add a data row to a result set
bool QuerierC::addResultToSet(vector<string>* fieldvalues, map<string,string>* varvalues, vector<string> rowValue, vector<ExpressionC> expressions, vector< vector<string> > & resultSet)
{
    vector<string> vResults;
    vResults.push_back(rowValue[0]);
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
  if (rowValue.size() != m_fieldnames.size() + 3){ // field name number + 3 variables (@raw @line @row)
    trace(ERROR, "Filed number %d and value number %d dont match!\n", m_fieldnames.size(), rowValue.size());
    dumpVector(m_fieldnames);
    dumpVector(rowValue);
    return false;
  }
  //trace(DEBUG, "Filtering '%s' ", rowValue[0].c_str());
  bool matched = false; 
  vector<string> fieldValues;
  map<string, string> varValues;
  for (int i=0; i<m_fieldnames.size(); i++)
    fieldValues.push_back(rowValue[i+1]);
    //fieldValues.insert( pair<string,string>(boost::algorithm::to_upper_copy<string>(m_fieldnames[i]),rowValue[i+1]));
  varValues.insert( pair<string,string>("@RAW",rowValue[0]));
  varValues.insert( pair<string,string>("@FILE",m_filename));
  varValues.insert( pair<string,string>("@LINE",rowValue[m_fieldnames.size()+1]));
  varValues.insert( pair<string,string>("@ROW",rowValue[m_fieldnames.size()+2]));
  bool bMatchedbMatched = (!filter || filter->compareExpression(&m_fieldnames, &fieldValues, &varValues));
  //trace(DEBUG, " selected:%d (%d)! \n", bMatchedbMatched, m_selections.size());
  if (bMatchedbMatched){
    if (m_selections.size()>0){
      if (m_groups.size() == 0){
        //trace(DEBUG, " No group! \n");
        if (!addResultToSet(&fieldValues, &varValues, rowValue, m_selections, m_results))
          return false;
        
        vector<string> vResults;
        for (int i=0; i<m_sorts.size(); i++){
          string sResult;
          if (!m_sorts[i].containGroupFunc()){
            //if it has the exact same expression as any selection, get the result from selection
            int iSel = -1;
            for (int j=0; j<m_selections.size(); j++)
              if (m_selections[j].getEntireExpstr().compare(m_sorts[i].getEntireExpstr())==0){
                iSel = j;
                break;
              }
            if (iSel >= 0)
              sResult = m_results[m_results.size()-1][iSel + 1];
            // if the sort key is a integer, get the result from the result set at the same sequence number
            else if (m_sorts[i].m_type==LEAF && m_sorts[i].m_expType==CONST && isInt(m_sorts[i].m_expStr) && atoi(m_sorts[i].m_expStr.c_str())<m_selections.size())
              sResult = m_results[m_results.size()-1][atoi(m_sorts[i].m_expStr.c_str()) + 1];
            else
              m_sorts[i].evalExpression(&m_fieldnames, &fieldValues, &varValues, sResult);
            //trace(DEBUG, "eval '%s' => '%s'\n", m_sorts[i].getEntireExpstr().c_str(), sResult.c_str());
          }else{
            trace(ERROR, "(3)Invalid using aggregation function in '%s', no group involved!\n", m_sorts[i].getEntireExpstr().c_str());
            return false;
          }
          vResults.push_back(sResult);
        }
        m_sortKeys.push_back(vResults);
      }else{ // need to do group. store evaled data in a temp date set
        //trace(DEBUG, " Grouping! \n");
        vector<string> groupExps;  // the group expressions. as the key of following hash map
        vector<string> nonAggVals;
        map< string,vector<string> > aggFuncTaget;
        //GroupDataSet dateSet;
        string sResult;
        bool dataSetExist = false;
        // group expressions to the sorting keys
        for (int i=0; i<m_groups.size(); i++){
          m_groups[i].evalExpression(&m_fieldnames, &fieldValues, &varValues, sResult);
          groupExps.push_back(sResult);
        }
        if (m_aggFuncTaget.find(groupExps) != m_aggFuncTaget.end()){
          nonAggVals = m_nonAggSels[groupExps]; // only need keep one nonAggVals for each groupExps
          aggFuncTaget = m_aggFuncTaget[groupExps];
          dataSetExist = true;
        }

        // get data sets
        if (!dataSetExist) // only need keep one nonAggVals for each groupExps
          nonAggVals.push_back(rowValue[0]); // nonAggSels store raw string in the first member. This is kinda meaningless.
        for (int i=0; i<m_selections.size(); i++){
          string sResult;
          //trace(DEBUG1, "Selection '%s': %d \n",m_selections[i].getEntireExpstr().c_str(), m_selections[i].containGroupFunc());
          if (!m_selections[i].containGroupFunc() && !dataSetExist){ // non aggregation function selections
            m_selections[i].evalExpression(&m_fieldnames, &fieldValues, &varValues, sResult);
            nonAggVals.push_back(sResult);
            //trace(DEBUG1, "Got non-aggr selection '%s' \n",sResult.c_str());
          }else{
            // eval agg function parameter expression and store in the temp data set
            evalAggExpNode(&m_selections[i], &m_fieldnames, &fieldValues, &varValues, aggFuncTaget);
          }
        }
        for (int i=0; i<m_sorts.size(); i++){
          string sResult;
          //if it has the exact same expression as any selection, get the result from selection
          int iSel = -1;
          for (int j=0; j<m_selections.size(); j++)
            if (m_selections[j].getEntireExpstr().compare(m_sorts[i].getEntireExpstr())==0){
              iSel = j;
              break;
            }
          // if the sort key is a integer, get the result from the result set at the same sequence number
          if (iSel >= 0 || (m_sorts[i].m_type==LEAF && m_sorts[i].m_expType==CONST && isInt(m_sorts[i].m_expStr) && atoi(m_sorts[i].m_expStr.c_str())<m_selections.size()))
            continue;
          else if (!m_sorts[i].containGroupFunc() && !dataSetExist){ // non aggregation function selections
            m_sorts[i].evalExpression(&m_fieldnames, &fieldValues, &varValues, sResult);
            nonAggVals.push_back(sResult);
            //trace(DEBUG1, "Got non-aggr selection '%s' \n",sResult.c_str());
          }else{
            // eval agg function parameter expression and store in the temp data set
            evalAggExpNode(&m_sorts[i], &m_fieldnames, &fieldValues, &varValues, aggFuncTaget);
          }
        }
        // dateSet.nonAggSels.push_back(nonAggVals);
        if (dataSetExist){
          // m_nonAggSels[groupExps] = nonAggVals; // only need keep one nonAggVals for each groupExps
          m_aggFuncTaget[groupExps] = aggFuncTaget;
        }else{
          m_nonAggSels.insert( pair<vector<string>, vector<string> >(groupExps,nonAggVals));
          m_aggFuncTaget.insert( pair<vector<string>, map< string,vector<string> > >(groupExps,aggFuncTaget));
        }
      }
    }else{
      m_results.push_back(rowValue);
      if (!addResultToSet(&fieldValues, &varValues, rowValue, m_sorts, m_sortKeys))
        return false;
    }
  }
  return bMatchedbMatched;

  //if (filter->m_type == BRANCH){
  //  if (!filter->m_leftNode || !filter->m_rightNode){
  //    return false;
  //  }
  //  if (filter->m_junction == AND)  // and
  //    matched = matchFilter(rowValue, filter->m_leftNode) && matchFilter(rowValue, filter->m_rightNode);
  //  else // or
  //    matched = matchFilter(rowValue, filter->m_leftNode) || matchFilter(rowValue, filter->m_rightNode);
  //}else if (filter->m_type == LEAF){
  //  if (filter->m_leftColId < 0) // filter is expression
  //    return !filter->m_leftExpStr.empty()?!filter->m_rightExpStr.empty():filter->m_leftExpStr.compare(filter->m_rightExpStr)==0;
  //  if (rowValue.size() == 0 || filter->m_leftColId > rowValue.size()-1)
  //    return false;
  //  //printf("left:%s %s right:%s (%s)\n",rowValue[filter->m_leftColId].c_str(),decodeComparator(filter->m_comparator).c_str(),filter->m_rightExpStr.c_str(),decodeDatatype(filter->m_datatype).c_str());
  //  return anyDataCompare(rowValue[filter->m_leftColId], filter->m_comparator, filter->m_rightExpStr, filter->m_datatype) == 1;
  //}else{ // no predication means alway true
  //  return false;
  //}
  return matched;
}

//#include <iostream>
//#include <fstream>

int QuerierC::searchNext()
{
  //string::const_iterator start = m_rawstr.begin(), end = m_rawstr.end();
  //smatch matches;
  //m_line++;
  int found = 0;
  try {
    //static int suffix = 0;
    //suffix++;
    //ofstream myfile;
    //myfile.open(("example."+intToStr(suffix)).c_str());
    //myfile << m_rawstr;
    //myfile.close();

    namesaving_smatch matches(m_regexstr);
    while(regex_search(m_rawstr, matches, m_regexp)){
      //if (string(matches[0]).empty()){ // found an empty string means no more searching!
      //  m_rawstr = "";
      //  return found;
      //}
      m_line++;
      //if(m_results.size()>0)
      //  formatoutput(m_results[0]);
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
        for (int i=0; i<m_selections.size(); i++)
          m_selections[i].analyzeColumns(&m_fieldnames, &m_fieldtypes);
        for (int i=0; i<m_groups.size(); i++)
          m_groups[i].analyzeColumns(&m_fieldnames, &m_fieldtypes);
        for (int i=0; i<m_sorts.size(); i++)
          m_sorts[i].analyzeColumns(&m_fieldnames, &m_fieldtypes);
      }
      // append variables
      //matcheddata.push_back(m_filename);
      matcheddata.push_back(intToStr(m_line));
      matcheddata.push_back(intToStr(m_matchcount+1));
      if (matchFilter(matcheddata, m_filter)){
        m_matchcount++;
      }
      //m_results.push_back(matches);
      //vector<namesaving_smatch>::iterator p = m_results.end();
      //m_results.insert(p, matches);
      //if(m_results.size()>0)
      //  formatoutput(m_results[0]);
      //formatoutput(m_results[m_results.size()-1]);
      //printf("matched: %s; start pos: %d; len: %d\n",string(matches[0]).c_str(),m_rawstr.find(string(matches[0])),string(matches[0]).length());
      //printf("orig: %s\n",m_rawstr.c_str());
      //formatoutput(matches);

      //smatch m;
      //regex_search(m_rawstr, m, m_regexp);
      //for (int i=1; i<m.size(); i++)
      //  printf("%s\t",m[i].str().c_str());
      //printf("\n");

      m_rawstr = m_rawstr.substr(m_rawstr.find(matcheddata[0])+matcheddata[0].length());
      if (matcheddata[0].find("\n") == string::npos){ // if not matched a newline, skip until the next newline
        size_t newlnpos = m_rawstr.find("\n");
        if (newlnpos != string::npos)
          m_rawstr = m_rawstr.substr(newlnpos+1);
      }
      //printf("new: %s\n",m_rawstr.c_str());
      //m_rawstr.emplace_back( start, matches[0].first );
      //auto start = distance(m_rawstr.begin(),start);
      //auto len   = distance(start, matches[0].first);
      //auto sub_str = m_rawstr.substr(start,len);
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
  return found;
}

int QuerierC::searchAll()
{
  int totalfound = 0;
  int found = searchNext();
  while (found>0 && !m_rawstr.empty()){
    found = searchNext();
    totalfound+=found;
  }
  return totalfound;
}

// run the aggregation functions in an expression 
void QuerierC::runAggFuncExp(ExpressionC* node, map< string,vector<string> >* dateSet, string & sResult)
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
        }else if (sFuncStr.find("MAX(")!=string::npos){
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

// group result
bool QuerierC::group()
{
  if (m_groups.size() == 0 || m_nonAggSels.size() == 0)
    return true;

  for (map< vector<string>, vector<string> >::iterator it=m_nonAggSels.begin(); it!=m_nonAggSels.end(); ++it){
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
        runAggFuncExp(&m_selections[i], &(m_aggFuncTaget[it->first]), sResult);
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
        if (m_selections[j].getEntireExpstr().compare(m_sorts[i].getEntireExpstr())==0){
          iSel = j;
          break;
        }
      if (iSel >= 0){
        vResults.push_back(m_results[m_results.size()-1][iSel + 1]);
      // if the sort key is a integer, get the result from the result set at the same sequence number
      }else if ((m_sorts[i].m_type==LEAF && m_sorts[i].m_expType==CONST && isInt(m_sorts[i].m_expStr) && atoi(m_sorts[i].m_expStr.c_str())<m_selections.size())){
        vResults.push_back(m_results[m_results.size()-1][atoi(m_sorts[i].m_expStr.c_str()) + 1]);
      }else if (!m_sorts[i].containGroupFunc()){ // non aggregation function selections
        //trace(DEBUG1, "None aggr func selection: %s\n", it->second[iNonAggSelID].c_str());
        vResults.push_back(it->second[iNonAggSelID]);
        iNonAggSelID++;
      }else{
        // eval agg function parameter expression and store in the temp data set
        string sResult;
        runAggFuncExp(&m_sorts[i], &(m_aggFuncTaget[it->first]), sResult);
        vResults.push_back(sResult);
      }
    }
    m_sortKeys.push_back(vResults);
  }
}

// doing merging sort exchanging
void QuerierC::mergeSort(int iLeft, int iMid, int iRight)
{
  //trace(DEBUG1, "Mergeing %d %d %d\n", iLeft, iMid, iRight);
  if (iLeft >= iRight)
    return;
  else{
    mergeSort(iLeft, (int)floor(iMid)/2, iMid);
    mergeSort(iMid+1, iMid+1+(int)floor(iRight-iMid-1)/2, iRight);
    int iLPos = iLeft, iRPos = iMid, iCheckPos = iMid;
    while (iLPos<iCheckPos && iRPos<=iRight){
      //trace(DEBUG1, "Swaping %d %d %d %d\n", iLPos, iCheckPos, iRPos, iRight);
      bool exchanged = false;
      for (int i=0; i<m_sorts.size(); i++){
        if ((*(m_sortKeys.begin()+iLPos))[i]>(*(m_sortKeys.begin()+iRPos))[i]){
          //vector<string> tmp;
          //tmp.insert(tmp.begin(), (*(m_results.begin()+iRPos)).begin(), (*(m_results.begin()+iRPos)).end());
          trace(DEBUG1, "moving %s before %s\n", (*(m_sortKeys.begin()+iRPos))[i].c_str(), (*(m_sortKeys.begin()+iLPos))[i].c_str());
          trace(DEBUG1, "Before move: %s(%d) before %s(%d)\n", (*(m_results.begin()+iLPos))[2].c_str(), iLPos, (*(m_results.begin()+iRPos))[2].c_str(), iLPos);
          m_results.insert(m_results.begin()+iLPos,*(m_results.begin()+iRPos));
          m_results.erase(m_results.begin()+iRPos+1);
          trace(DEBUG1, "After move: %s(%d) before %s(%d)\n", (*(m_results.begin()+iLPos))[2].c_str(), iLPos, (*(m_results.begin()+iRPos))[2].c_str(, iLPos));
          exchanged = true;
          break;
        }
      }
      if (exchanged){
        iCheckPos++; // one element from right side insert to left side, check point needs to be moved 1 step to right.
        iLPos++; // old left element moved 1 step to right. 
        iRPos++; // compare the next right element
      }else{
        iLPos++; // compare to the next left element
      }
    }
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
  mergeSort(0,(int)floor(m_sortKeys.size())/2,m_sortKeys.size()-1);
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
  m_outputrow++;
  //printf("%d: ", m_outputrow);
  if (m_selections.size()==0)
    printf("%s\n", datas[0].c_str());
  else{
    for (int i=1; i<datas.size(); i++)
      printf("%s\t", datas[i].c_str());
    printf("\n");
  }
}

void QuerierC::printFieldNames()
{
  //for (int i=1; i<m_fieldnames.size(); i++)
  //  printf("%s\t",m_fieldnames[i].c_str());
  if (!gv.g_printheader || m_bNamePrinted)
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

void QuerierC::outputAndClean()
{
  output();
  m_results.clear();
  m_groups.clear();
  m_sorts.clear();
  m_nonAggSels.clear();
  m_aggFuncTaget.clear();
  m_sortKeys.clear();
  m_bNamePrinted = false;
}
