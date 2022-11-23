/*******************************************************************************
//
//        File: filter.cpp
// Description: Filter class defination
//       Usage: filter.cpp
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "expression.h"
#include "function.h"
#include "filter.h"

vector<string> FilterC::m_comparators;

void FilterC::init()
{
  m_type = UNKNOWN;       // 1: branch; 2: leaf
  m_junction = UNKNOWN;   // if type is BRANCH, 1: and; 2: or. Otherwise, it's meaningless
  m_comparator = UNKNOWN; // if type is LEAF, 1: ==; 2: >; 3: <; 4: !=; 5: >=; 6: <=. Otherwise, it's meaningless
  m_datatype.datatype = UNKNOWN;   // if type is LEAF, 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
  m_datatype.extrainfo = "";
  m_leftColId = -1;              // if type is LEAF, it's id of column on the left to be predicted. Otherwise, it's meaningless
  m_rightColId = -1;             // if type is LEAF, it's id of column on the right to be predicted. Otherwise, it's meaningless
  m_expStr = "";
  m_leftExpStr = "";    // if type is LEAF, it's id of column to be predicted. Otherwise, it's meaningless
  m_rightExpStr = "";   // if type is LEAF, it's data to be predicted. Otherwise, it's meaningless
  m_leftExpression = NULL; // meaningful only if type is LEAF
  m_rightExpression = NULL; // meaningful only if type is LEAF
  m_leftNode = NULL;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
  m_rightNode = NULL;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
  m_parentNode = NULL;    // for all types except the root, it links to parent node. Otherwise, it's meaningless
  m_fieldnames = NULL;
  m_fieldtypes = NULL;
  m_rawDatatype = NULL;
  m_sideDatatypes = NULL;
  
  m_metaDataAnzlyzed = false; // analyze column name to column id.
  m_expstrAnalyzed = false;

  m_inExpressions.clear();
  m_comparators.clear();
  m_comparators.push_back("!=");m_comparators.push_back(">=");m_comparators.push_back("<=");m_comparators.push_back("=");m_comparators.push_back(">");m_comparators.push_back("<");m_comparators.push_back(" LIKE ");m_comparators.push_back(" REGLIKE ");m_comparators.push_back(" NOLIKE ");m_comparators.push_back(" NOREGLIKE ");m_comparators.push_back(" IN ");m_comparators.push_back(" NOIN "); // "=", "<", ">" should be put after "<=" ">=" "!="
}

FilterC::FilterC()
{
  init();
}

// Rule one Copy Constructor
FilterC::FilterC(const FilterC& other)
{
  if (this != &other){
    init();
    other.copyTo(this);
  }
}

// Rule two Destructor
FilterC::~FilterC()
{
  clear();
}

// Rule one Copy Assignment Operator
FilterC& FilterC::operator=(const FilterC& other)
{
  if (this != &other){
    clear();
    other.copyTo(this);
  }
  return *this;
}

void FilterC::copyTo(FilterC* node) const
{
  if (!node || this==node)
    return;
  else{
    node->clear();
    node->m_metaDataAnzlyzed = m_metaDataAnzlyzed;
    node->m_expstrAnalyzed = m_expstrAnalyzed;
    //node->predStr = predStr;
    node->m_type = m_type;
    node->m_datatype = m_datatype;
    node->m_junction = m_junction;
    node->m_comparator = m_comparator;
    node->m_leftColId = m_leftColId;
    node->m_rightColId = m_rightColId;
    node->m_rightExpStr = m_rightExpStr;
    node->m_leftExpStr = m_leftExpStr;
    node->m_expStr = m_expStr;
    node->m_fieldnames = m_fieldnames;
    node->m_fieldtypes = m_fieldtypes;
    node->m_rawDatatype = m_rawDatatype;
    node->m_sideDatatypes = m_sideDatatypes;
    if (m_type == BRANCH){
      if (m_leftNode){
        node->m_leftNode = new FilterC();
        m_leftNode->copyTo(node->m_leftNode);
        node->m_leftNode->m_parentNode = node;
      }else
        node->m_leftNode = NULL;

      if (m_rightNode){
        node->m_rightNode = new FilterC();
        m_rightNode->copyTo(node->m_rightNode);
        node->m_rightNode->m_parentNode = node;
      }else
        node->m_rightNode = NULL;
      node->m_leftExpression = NULL;
      node->m_rightExpression = NULL;
    }else{
      if (m_leftExpression){
        node->m_leftExpression = new ExpressionC(m_leftExpStr);
        m_leftExpression->copyTo(node->m_leftExpression);
      }
      if (m_rightExpression){
        node->m_rightExpression = new ExpressionC(m_rightExpStr);
        m_rightExpression->copyTo(node->m_rightExpression);
      }
    }
  }
}

FilterC::FilterC(string expStr)
{
  init();
  setExpstr(expStr);
}

FilterC::FilterC(FilterC* node)
{
  if (node && node!=this){
    init();
    node->copyTo(this);
  }
}

FilterC::FilterC(int junction, FilterC* leftNode, FilterC* rightNode)
{
  init();
  m_type = BRANCH;
  m_junction = junction;
  m_leftNode = leftNode;
  m_rightNode = rightNode;
}

FilterC::FilterC(int comparator, int colId, string data)
{
  init();
  m_type = LEAF;
  m_comparator = comparator;
  m_leftColId = colId;
  m_rightExpStr = data;
}

void FilterC::setExpstr(string expStr)
{
  m_expStr = expStr;
  buildFilter();
}

// clear predictin
void FilterC::clear(){
  if (m_leftNode){
    m_leftNode->clear();
    SafeDelete(m_leftNode);
  }
  if (m_rightNode){
    m_rightNode->clear();
    SafeDelete(m_rightNode);
  }
  if (m_leftExpression){
    m_leftExpression->clear();
    SafeDelete(m_leftExpression);
  }
  if (m_rightExpression){
    m_rightExpression->clear();
    SafeDelete(m_rightExpression);
  }
  m_comparators.clear();
  m_inExpressions.clear();
  init();
}

// build a leaf node
void FilterC::buildLeafNodeFromStr(FilterC* node, string str)
{
  bool quoteStarted = false;
  int lastPos = 0;
  for (int i=0;i<str.length();i++){
    if (str[i] == '"'){
      quoteStarted = !quoteStarted;
      str = str.substr(0, i)+(i<str.length()-1?str.substr(i+1):"");
      i--;
    }else if(str[i] == '\\' && i<str.length()-1 && str[i+1] == '"'){
      i++; // skip escaped " :\"
      str = str.substr(0, i-1)+str.substr(i);
    }else if(!quoteStarted && startsWithWords(str.substr(i), m_comparators) >= 0){ // splitor that not between quato are the real splitor
      string compStr = m_comparators[startsWithWords(str.substr(i), m_comparators)];
      node->m_comparator = encodeComparator(trim_copy(compStr));
      node->m_type = LEAF;
      node->m_leftExpStr =  trim_copy(str.substr(0,i));
      node->m_rightExpStr = trim_one( trim_copy(str.substr(i+compStr.length())),'"');
      //trace(DEBUG, "Converted '%s' to '%s'. \n",str.substr(i+compStr.length()).c_str(),node->m_rightExpStr.c_str());
      trace(DEBUG, "Found comparator '%s' in '%s'. '%s' %s '%s'\n",compStr.c_str(),str.c_str(),node->m_leftExpStr.c_str(),compStr.c_str(),node->m_rightExpStr.c_str());
      node->m_leftExpression = new ExpressionC(node->m_leftExpStr);
      node->m_rightExpression = new ExpressionC(node->m_rightExpStr);
      if (node->m_comparator == IN || node->m_comparator == NOIN){ // hard code for IN/NOIN,m_rightExpression is NULL, m_inExpressions contains IN expressions
        if (!node->m_rightExpression->containRefVar()){ // check if it's a side work IN or an iterating element IN
          if (node->m_rightExpression){
            node->m_rightExpression->clear();
            SafeDelete(node->m_rightExpression);
          }
          if (node->m_rightExpStr.length()<2 || node->m_rightExpStr[0]!='(' || node->m_rightExpStr[node->m_rightExpStr.length()-1]!=')'){
            trace(ERROR, "Invalid IN string '%s'\n", node->m_rightExpStr.c_str());
            return;
          }
          string sElements = node->m_rightExpStr.substr(1,node->m_rightExpStr.length()-2);
          vector<string> vElements = split(sElements,',',"''()",'\\',{'(',')'},false,true);
          ExpressionC eElement;
          for (int i=0;i<vElements.size();i++){
            string sResult, sElement = trim_copy(vElements[i]);
            if (sElement.empty()){
              trace(ERROR, "Empty IN element string!\n");
              return;
            }
            eElement = ExpressionC(sElement);
            if (!eElement.expstrAnalyzed()){
              trace(ERROR, "Failed to analyze the expression of %s!\n", sElement.c_str());
              return;
            }
            m_inExpressions.push_back(eElement);
          }
        }
      }
      i+=(compStr.length()-1); // skip comparator
    }
  }
}

// build current filter class from the expression string
bool FilterC::buildFilter(string splitor, string quoters)
{
  //System.out.println(String.format("%d",deep) +":"+m_expStr);
  if (m_expStr.empty()){
    trace(ERROR,"buildFilter: No statement found!\n");
    return false;
  }else
    m_expStr = trim_copy(m_expStr);
  //m_expStr = m_expStr.trim();
  char stringQuoter = '\'';
  if (quoters.empty() || quoters.length() != 2)
      quoters = "()";
  //trace(DEBUG, "Building: %s; splitor: %s; quoters: %s\n", m_expStr.c_str(), splitor.c_str(), quoters.c_str());
  int quoteDeep = 0;
  int quoteStart = -1;  // top quoter start position
  int quoteEnd = -1;;   // top quoter end position
  bool stringStart = false;
  for (int i=0;i<m_expStr.length();i++){
    if (m_expStr[i] == stringQuoter &&(i==0 || (i>0 && m_expStr[i-1]!='\\'))){ // \ is ignore character
      stringStart = !stringStart;
      continue;
    }
    if (stringStart) // ignore all character being a string
      continue;
    if (m_expStr[i] == quoters[0]){
      quoteDeep++;
      if (quoteDeep == 1 && quoteStart < 0)
        quoteStart = i;
    }else if (m_expStr[i] == quoters[1]){
      quoteDeep--;
      if (quoteDeep == 0 && quoteEnd < 0)
        quoteEnd = i;
      if  (quoteDeep < 0){
        trace(ERROR, "buildFilter: Left quoter missed!\n");
      }
    }else if(m_expStr[i] == '\\' && i<m_expStr.length()-1 && 
            ((m_expStr[i+1] == quoters[0] || m_expStr[i+1] == quoters[1] || m_expStr[i+1] == ' '))){
      i++; // skip escaped " \"
      m_expStr = m_expStr.substr(0, i-1)+m_expStr.substr(i);
    }else if(quoteDeep == 0 && m_expStr[i] == ' '){ // splitor that not between quato are the real splitor
      if ( upper_copy(m_expStr.substr(i)).find(splitor) == 0){
        m_type = BRANCH;
        m_junction = encodeJunction(trim_copy(splitor));
        m_leftNode = new FilterC(m_expStr.substr(0, i));
        m_leftNode->m_parentNode = this;
        //trace(DEBUG,"Building leftNode\n");
        if (!m_leftNode->buildFilter(" OR ",quoters)) { // OR priority higher than AND
          m_leftNode->clear();
          SafeDelete(m_leftNode);
          return false;
        }
        m_rightNode = new FilterC(m_expStr.substr(i+splitor.length()));
        m_rightNode->m_parentNode = this;
        //trace(DEBUG,"Building rightNode\n");
        if (!m_rightNode->buildFilter(" OR ",quoters)){
          m_rightNode->clear();
          SafeDelete(m_rightNode);
          return false;
        }
        //trace(DEBUG2, "(1)Filter expression '%s' \n",m_expStr.c_str());
        return true;
      }
    }
  }

  if (quoteDeep != 0){
    trace(ERROR, "Right quoter missed!\n");
    return false;
  }

  if (quoteStart == 0 && quoteEnd == m_expStr.length()-1){ // sub expression quoted 
    m_expStr = m_expStr.substr(1,m_expStr.length()-2);  // trim the quoters
    return buildFilter(" OR ",quoters);
  }else if (upper_copy(splitor).compare(" OR ") == 0){
    return buildFilter(" AND ",quoters);
  }else
    buildLeafNodeFromStr(this, m_expStr);
  //trace(DEBUG2, "(2)Filter expression '%s' \n",m_expStr.c_str());
  return true;
}

void FilterC::buildFilter()
{
  if (!buildFilter(" OR ", "()")){
    clear();
    m_metaDataAnzlyzed = false;
  }else{
    m_metaDataAnzlyzed = true;
    //mergeExprConstNodes();
  }
}

// merge const in the expressions
void FilterC::mergeExprConstNodes()
{
  trace(DEBUG, "Merging consts in filter '%s'!\n", m_expStr.c_str());
  if (m_leftNode)
    m_leftNode->mergeExprConstNodes();
  if (m_rightNode)
    m_rightNode->mergeExprConstNodes();
  if (m_leftExpression){
    string sResult;
    if (m_leftExpression->mergeConstNodes(sResult)){
      m_leftExpStr = sResult;
    }      
  }
  if (m_rightExpression){
    string sResult;
    if (m_rightExpression->mergeConstNodes(sResult)){
      m_rightExpStr = sResult;
    }
  }
}

// get left tree Height
int FilterC::getLeftHeight(){
  int height = 1;
  if (m_type == BRANCH && m_leftNode)
    height += m_leftNode->getLeftHeight();

  return height;
}

// get left tree Height
int FilterC::getRightHeight(){
  int height = 1;
  if (m_type == BRANCH && m_rightNode)
    height += m_rightNode->getRightHeight();
  
  return height;
}

// add a NEW filter into tree
void FilterC::add(FilterC* node, int junction, bool leafGrowth, bool addOnTop){
  // not add any null or UNKNOWN node
  if (node || node->m_type ==  UNKNOWN) 
      return;
  if (m_type ==  UNKNOWN){ // not assinged
      node->copyTo(this);
  }else if (m_type == LEAF){
    FilterC* existingNode = new FilterC();
    copyTo(existingNode);
    m_type = BRANCH;
    m_junction = junction;
    m_comparator = UNKNOWN;   
    m_leftColId = -1;  
    m_rightColId = -1;
    m_expStr = "";
    m_leftExpStr = "";
    m_rightExpStr = "";
    m_leftExpression = NULL;
    m_rightExpression = NULL;
    m_fieldnames = NULL;
    m_fieldtypes = NULL;
    m_rawDatatype = NULL;
    m_sideDatatypes = NULL;
    if (leafGrowth){
      m_rightNode = existingNode;
      m_rightNode->m_parentNode = this;
      m_leftNode = node;
      m_leftNode->m_parentNode = this;
    }else{
      m_leftNode = existingNode;
      m_leftNode->m_parentNode = this;
      m_rightNode = node;
      m_rightNode->m_parentNode = this;
    }
  }else{
    if (addOnTop){
      FilterC* existingNode = new FilterC();
      copyTo(existingNode);
      m_junction = junction;
      if (leafGrowth){
        m_leftNode = node;
        m_leftNode->m_parentNode = this;
        m_rightNode = existingNode;
        m_rightNode->m_parentNode = this;
      }else{
        m_leftNode = existingNode;
        m_leftNode->m_parentNode = this;
        m_rightNode = node;
        m_rightNode->m_parentNode = this;
      }
    }else{
      if (leafGrowth){
        if (m_leftNode)
          m_leftNode->add(node, junction, leafGrowth, addOnTop);
        else 
          m_rightNode->add(node, junction, leafGrowth, addOnTop);
      }else{
        if (m_rightNode)
          m_rightNode->add(node, junction, leafGrowth, addOnTop);
        else 
          m_leftNode->add(node, junction, leafGrowth, addOnTop);
      }
    }
  }
}

void FilterC::dump(int deep){
  if (m_type == BRANCH){
    trace(DUMP,"(%d)%s\n",deep,decodeJunction(m_junction).c_str());
    trace(DUMP,"L-");
    m_leftNode->dump(deep+1);
    trace(DUMP,"R-");
    m_rightNode->dump(deep+1);
  }else{
    trace(DUMP,"(%d)%s %s %s\n",deep,m_leftExpStr.c_str(),decodeComparator(m_comparator).c_str(),m_rightExpStr.c_str());
  }
}

void FilterC::dump(){
  trace(DEBUG,"Dumping filter construct...\n");;
  dump(0);
}

// detect if predication contains special colId    
bool FilterC::containsColId(int colId){
  bool contain = false;
  if (m_type == BRANCH){
    contain = contain || m_leftNode->containsColId(colId);
    contain = contain || m_rightNode->containsColId(colId);
  }else
    contain = (m_leftColId == colId);

  return contain;
}

// detect if predication contains special colId    
FilterC* FilterC::getFirstPredByColId(int colId, bool leftFirst){
  FilterC* node;
  if (m_type == BRANCH){
    if (leftFirst){
      if (m_leftNode)
        node = m_leftNode->getFirstPredByColId(colId, leftFirst);
      if (!node)
        node = m_rightNode->getFirstPredByColId(colId, leftFirst);
    }else{
      if (m_rightNode)
        node = m_rightNode->getFirstPredByColId(colId, leftFirst);
      if (!node)
        node = m_leftNode->getFirstPredByColId(colId, leftFirst);
    }
  }else if (m_type == LEAF)
    if (m_leftColId == colId)
      node = this;

  return node;
}

// analyze column ID & name from metadata
bool FilterC::analyzeColumns(vector<string>* fieldnames, vector<DataTypeStruct>* fieldtypes, DataTypeStruct* rawDatatype, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes){
  trace(DEBUG, "Analyzing columns in filter '%s'\n", m_expStr.c_str());
  if (!fieldnames || !fieldtypes){
    trace(ERROR, "(Filter)fieldnames or fieldtypes is NULL!\n");
    return UNKNOWN;
  }
  m_fieldnames = fieldnames;
  m_fieldtypes = fieldtypes;
  m_rawDatatype = rawDatatype;
  m_sideDatatypes = sideDatatypes;
  if (m_type == BRANCH){
    m_metaDataAnzlyzed = true;
    if (m_leftNode)
        m_metaDataAnzlyzed = m_leftNode->analyzeColumns(fieldnames, fieldtypes, rawDatatype, sideDatatypes) && m_metaDataAnzlyzed ;
    if (m_rightNode)
        m_metaDataAnzlyzed = m_rightNode->analyzeColumns(fieldnames, fieldtypes, rawDatatype, sideDatatypes) && m_metaDataAnzlyzed;
    if (!m_metaDataAnzlyzed)
        return m_metaDataAnzlyzed;
    //if (m_leftExpression)
    //  m_leftExpression->analyzeColumns(fieldnames, fieldtypes, rawDatatype, sideDatatypes);
    //if (m_rightExpression)
    //  m_rightExpression->analyzeColumns(fieldnames, fieldtypes, rawDatatype, sideDatatypes);
  }else if (m_type == LEAF){
    if (m_leftExpression){
      if (!m_leftExpression->expstrAnalyzed()){
        m_leftExpression->clear();
        SafeDelete(m_leftExpression);
        m_leftExpression = new ExpressionC(m_leftExpStr);
      }
    }else
      m_leftExpression = new ExpressionC(m_leftExpStr);
    if (!m_leftExpression->expstrAnalyzed()){
      trace(ERROR, "Failed to analyze m_leftExpression of filter '%s'!\n", m_leftExpression->getEntireExpstr().c_str());
      m_leftExpression->clear();
      SafeDelete(m_leftExpression);
      return false;
    }
    m_leftExpression->analyzeColumns(fieldnames, fieldtypes, rawDatatype, sideDatatypes);

    if (m_rightExpression){
      if (!m_rightExpression->expstrAnalyzed()){
        m_rightExpression->clear();
        SafeDelete(m_rightExpression);
        m_rightExpression = new ExpressionC(m_leftExpStr);
      }
    }else if(m_comparator != IN && m_comparator != NOIN)
      m_rightExpression = new ExpressionC(m_leftExpStr);
    if (m_rightExpression){ // m_rightExpression exist means it either a Non-IN comparasion or side work IN comparasion
      if (!m_rightExpression->expstrAnalyzed()){
        trace(ERROR, "Failed to analyze m_rightExpression of filter '%s'!\n", m_rightExpression->getEntireExpstr().c_str());
        m_rightExpression->clear();
        SafeDelete(m_rightExpression);
        return false;
      }
      m_rightExpression->analyzeColumns(fieldnames, fieldtypes, rawDatatype, sideDatatypes);
      m_datatype = getCompatibleDataType(m_leftExpression->m_datatype, m_rightExpression->m_datatype);
    }else{
      if (m_comparator == IN || m_comparator == NOIN){ // hard code for element iterating IN/NOIN,m_rightExpression is NULL, m_inExpressions contains IN expressions
        DataTypeStruct dts;
        dts.datatype = -99;
        for (int i=0; i<m_inExpressions.size(); i++){
          if (m_inExpressions[i].expstrAnalyzed()){
            m_inExpressions[i].analyzeColumns(fieldnames, fieldtypes, rawDatatype, sideDatatypes);
            if (dts.datatype == -99)
              dts = m_inExpressions[i].m_datatype;
            else
              dts = getCompatibleDataType(dts, m_inExpressions[i].m_datatype);
          }else
            return false;
        }
        m_datatype = getCompatibleDataType(m_leftExpression->m_datatype, dts);
      }
    }
  }
  return m_metaDataAnzlyzed;
}

bool FilterC::columnsAnalyzed(){
    return m_metaDataAnzlyzed;
}

bool FilterC::expstrAnalyzed()
{
  return m_expstrAnalyzed;
}

FilterC* FilterC::cloneMe(){
  FilterC* node = new FilterC();
  node->m_metaDataAnzlyzed = m_metaDataAnzlyzed;
  node->m_expstrAnalyzed = m_expstrAnalyzed;
  //node->predStr = predStr;
  node->m_type = m_type;
  node->m_datatype = m_datatype;
  node->m_junction = m_junction;
  node->m_comparator = m_comparator;
  node->m_leftColId = m_leftColId;
  node->m_rightColId = m_rightColId;
  node->m_expStr = m_expStr;
  node->m_rightExpStr = m_rightExpStr;
  node->m_leftExpStr = m_leftExpStr;
  node->m_fieldnames = m_fieldnames;
  node->m_fieldtypes = m_fieldtypes;
  node->m_rawDatatype = m_rawDatatype;
  node->m_sideDatatypes = m_sideDatatypes;
  if (m_type == BRANCH){
    node->m_leftExpression = new ExpressionC(node->m_leftExpStr);
    node->m_leftExpression = m_leftExpression->cloneMe();
    node->m_rightExpression = new ExpressionC(node->m_rightExpStr);
    node->m_rightExpression = m_rightExpression->cloneMe();
    node->m_leftNode = new FilterC();
    node->m_leftNode = m_leftNode->cloneMe();
    node->m_rightNode = new FilterC();
    node->m_rightNode = m_rightNode->cloneMe();
    node->m_leftNode->m_parentNode = node;
    node->m_rightNode->m_parentNode = node;
  }else{
    node->m_leftExpression = NULL;
    node->m_rightExpression = NULL;
    node->m_leftNode = NULL;
    node->m_rightNode = NULL;
  }
  return node;
}

// get all involved colIDs in this prediction
std::set<int> FilterC::getAllColIDs(int side){
  std::set<int> colIDs;
  if (m_type == BRANCH){
    if (m_leftNode){
      std::set<int> foo = m_leftNode->getAllColIDs(side);
      colIDs.insert(foo.begin(), foo.end());
    }
    if (m_rightNode){
      std::set<int> foo = m_rightNode->getAllColIDs(side);
      colIDs.insert(foo.begin(), foo.end());
    }
  }else if(m_type == LEAF){
    if (side == LEFT && m_leftColId>=0)
      colIDs.insert(m_leftColId);
    else if (side == RIGHT && m_rightColId>=0)
      colIDs.insert(m_rightColId);
  }
  return colIDs;
}

// build the prediction as a HashMap
map<int,string> FilterC::buildMap(){
  map<int,string> datas;
  if (m_type == BRANCH){
    if (m_leftNode){
      map<int,string> foo = m_leftNode->buildMap();
      datas.insert(foo.begin(), foo.end());
    }
    if (m_rightNode){
      map<int,string> foo = m_rightNode->buildMap();
      datas.insert(foo.begin(), foo.end());
    }
  }else if(m_type == LEAF){
    if (m_leftColId>=0)
      datas.insert( pair<int,string>(m_leftColId,m_rightExpStr) );
  }
  return datas;
}

// get all involved colIDs in this prediction
int FilterC::size(){
  int size = 0;
  if (m_type == BRANCH){
    if (m_leftNode)
      size += m_leftNode->size();
    if (m_rightNode)
      size += m_rightNode->size();
  }else if (m_type == LEAF)
    size = 1;
  else 
    size = 0;
  return size;
}

// remove a node from prediction. Note: the input node is the address of the node contains in current prediction
//   0                      0                  2                 1
//  1  2  (remove 3) =>   4   2 (remove 1) =>      (remove 2)  3   4
//3  4
bool FilterC::remove(FilterC* node){
  bool removed = false;
  if (m_leftNode){
    if (m_leftNode == node){
      m_leftNode->clear();
      SafeDelete(m_leftNode);
      return true;
    }else{
      removed = removed || m_leftNode->remove(node);
      if (removed)
        return removed;
    }
  }
  if (m_rightNode){
    if (m_rightNode == node){
      m_rightNode->clear();
      SafeDelete(m_rightNode);
      return true;
    }else{
      removed = removed || m_rightNode->remove(node);
      if (removed)
        return removed;
    }
  }
  if (this == node){
    clear();
    return true;
  }else
    return removed;
}

bool FilterC::getAggFuncs(unordered_map< string,GroupProp > & aggFuncs)
{
  if (m_type == LEAF){
    bool bGotAggFunc = (m_leftExpression && m_leftExpression->getAggFuncs(aggFuncs));
    bGotAggFunc = (m_rightExpression && m_rightExpression->getAggFuncs(aggFuncs));
    return bGotAggFunc;
  }else{
    bool bGotAggFunc = (m_leftNode && m_leftNode->getAggFuncs(aggFuncs));
    bGotAggFunc = (m_rightNode && m_rightNode->getAggFuncs(aggFuncs));
    return bGotAggFunc;
  }
}

bool FilterC::getAnaFuncs(unordered_map< string,vector<ExpressionC> > & anaFuncs, unordered_map< string, vector<int> > & anaGroupNums)
{
  if (m_type == LEAF){
    bool bGotAnnFunc = (m_leftExpression && m_leftExpression->getAnaFuncs(anaFuncs, anaGroupNums));
    bGotAnnFunc = (m_rightExpression && m_rightExpression->getAnaFuncs(anaFuncs, anaGroupNums));
    return bGotAnnFunc;
  }else{
    bool bGotAnnFunc = (m_leftNode && m_leftNode->getAnaFuncs(anaFuncs, anaGroupNums));
    bGotAnnFunc = (m_rightNode && m_rightNode->getAnaFuncs(anaFuncs, anaGroupNums));
    return bGotAnnFunc;
  }
}
// build a data list for a set of column, keeping same sequence, fill the absent column with NULL
void FilterC::fillDataForColumns(map <string, string> & dataList, vector <string> columns){
  if (columns.size() == 0)
    return;
  if (m_type == BRANCH){
    if (m_leftNode)
      m_leftNode->fillDataForColumns(dataList, columns);
    if (m_rightNode)
      m_rightNode->fillDataForColumns(dataList, columns);
  }else if (m_type == LEAF && m_leftColId >= 0)
    dataList.insert( pair<string,string>(columns[m_leftColId],m_rightExpStr) );
}

bool FilterC::containAnaFunc() const
{
  if (m_type == LEAF){
    if ((m_leftExpression && m_leftExpression->containAnaFunc()) || (m_rightExpression && m_rightExpression->containAnaFunc()))
      return true;
  }else{
    if ((m_leftNode && m_leftNode->containAnaFunc()) || (m_rightNode && m_rightNode->containAnaFunc()))
      return true;
  }
  return false;
}

bool FilterC::isConst() const
{
  if (m_type == LEAF){
    if ((m_leftExpression && m_leftExpression->m_expType==CONST) && (m_rightExpression && m_rightExpression->m_expType==CONST))
      return true;
  }else{
    if ((m_leftNode && m_leftNode->isConst()) && (m_rightNode && m_rightNode->isConst()))
      return true;
  }
  return false;
}

// calculate this expression. fieldnames: column names; fieldvalues: column values; varvalues: variable values; sResult: return result. column names are upper case; skipRow: wheather skip @row or not. extrainfo so far for date format only
bool FilterC::evalAnaExprs(RuntimeDataStruct & rds, vector<ExpressionC>* anaEvaledExp, string & sResult, DataTypeStruct & dts, bool getresultonly)
{
  if (m_type == LEAF){
    if (m_leftExpression && m_leftExpression->containAnaFunc()){
      ExpressionC tmpExp;
      tmpExp = *m_leftExpression;
      tmpExp.evalExpression(rds, sResult, dts, false);
      anaEvaledExp->push_back(tmpExp);
    }
    if (m_rightExpression && m_rightExpression->containAnaFunc()){
      ExpressionC tmpExp;
      tmpExp = *m_rightExpression;
      tmpExp.evalExpression(rds, sResult, dts, false);
      anaEvaledExp->push_back(tmpExp);
    }
    return true;
  }else{
    if ((m_leftNode && m_leftNode->evalAnaExprs(rds, anaEvaledExp, sResult, dts, getresultonly)) && (m_rightNode && m_rightNode->evalAnaExprs(rds, anaEvaledExp, sResult, dts, getresultonly)))
      return true;
  }
  return false;
}

bool FilterC::compareIn(RuntimeDataStruct & rds)
{
  string leftRst, sResult;
  DataTypeStruct dts1, dts2;
  unordered_map< string, unordered_map<string,string> > sideDatarow;
  bool bDoAggFilter = (rds.aggFuncs && rds.aggFuncs->size()>0);
  // dont do filte in two scenarios: 1: rds.aggFuncs prvoided, but no aggregation function involved in the expression (except CONST); 2: no rds.aggFuncs provided, but aggregation function involved in the expression;
  if ((!bDoAggFilter && m_leftExpression && m_leftExpression->containGroupFunc()) || (bDoAggFilter && !m_leftExpression->containGroupFunc() && !(m_leftExpression->m_type==LEAF&&m_leftExpression->m_expType==CONST)))
    return true;
  if (!m_leftExpression || !m_leftExpression->evalExpression(rds, leftRst, dts1, true)){
    trace(ERROR, "Failed to get the value to be compared in IN!\n"); 
    return false;
  }
  if (m_rightExpression && m_rightExpression->containRefVar()){
    int sidWorkID = m_rightExpression->getSideWorkID()-1;
    if (sidWorkID>=rds.sideDatasets->size()){
      //trace(ERROR, "%d is an invalide side work ID!\n", sidWorkID);
      return false;
    }
    bool bResult = false;
    unordered_map< string, unordered_map<string,string> >* oldSideDatarow = rds.sideDatarow;
    rds.sideDatarow = &sideDatarow;
    for (int i=0;i<(*rds.sideDatasets)[sidWorkID].size();i++){
      sideDatarow.clear();
      sideDatarow.insert(pair<string, unordered_map<string,string> >(intToStr(sidWorkID+1), (*rds.sideDatasets)[sidWorkID][i]));
      if (!m_rightExpression->evalExpression(rds, sResult, dts2, true)){
        trace(ERROR, "Failed to get result of IN element %s!\n", m_rightExpression->getEntireExpstr().c_str());
        bResult = false;
        break;
      }
      if (anyDataCompare(leftRst, EQ, sResult, m_datatype) == 1){
        bResult = true;
        break;
      }
    }
    rds.sideDatarow = oldSideDatarow;
    return bResult;
  }else{
    if (m_inExpressions.size() == 0){
      trace(ERROR, "Failed to get the IN elements!\n");
      return false;
    }
    // trace(DEBUG, "Comparing '%s' IN '%s' (data type: %s)\n", leftRst.c_str(), m_rightExpStr.c_str(), decodeDatatype(m_datatype.datatype).c_str());
    for (int i=0;i<m_inExpressions.size();i++){
      if ((!bDoAggFilter && m_inExpressions[i].containGroupFunc()) || (bDoAggFilter && !m_inExpressions[i].containGroupFunc() && !(m_inExpressions[i].m_type==LEAF&&m_inExpressions[i].m_expType==CONST)))
        return false;
      if (!m_inExpressions[i].evalExpression(rds, sResult, dts2, true)){
        trace(ERROR, "Failed to get result of IN element %s!\n", m_inExpressions[i].getEntireExpstr().c_str());
        return false;
      }
      if (anyDataCompare(leftRst, EQ, sResult, m_datatype) == 1){
        return true;
      }
    }
  }
  return false;
}

bool FilterC::compareExpression(RuntimeDataStruct & rds, vector< unordered_map< int,int > > & sideMatchedRowIDs){
  sideMatchedRowIDs.clear();
  return compareExpressionI(rds, sideMatchedRowIDs);
}

// join match, sideMatchedRowIDs returns the matched IDs
bool FilterC::joinMatch(RuntimeDataStruct & rds, vector< unordered_map< int,int > > & sideMatchedRowIDs)
{
  string sSideWorkID = "";
  if (m_leftExpression->getEntireExpstr().find("@R[")==0)
    sSideWorkID=m_leftExpression->getEntireExpstr().substr(3,m_leftExpression->getEntireExpstr().find("]")-3);
  else if (m_rightExpression->getEntireExpstr().find("@R[")==0)
    sSideWorkID=m_rightExpression->getEntireExpstr().substr(3,m_rightExpression->getEntireExpstr().find("]")-3);
  int sidWorkID=-1;
  if (sSideWorkID.empty() || !isInt(sSideWorkID))
    return false;
  else
    sidWorkID=atoi(sSideWorkID.c_str())-1;
  if (sidWorkID<0 || sidWorkID>=rds.sideDatasets->size())
    return false;
    
  //for (int sidWorkID=0; sidWorkID<rds.sideDatasets->size(); sidWorkID++){
  for (int i=0;i<(*rds.sideDatasets)[sidWorkID].size();i++){
    unordered_map< int,int > sideMatchedRowID;
    unordered_map< string, unordered_map<string,string> > sideDatarow;
    // construct current side work
    sideDatarow.insert(pair<string, unordered_map<string,string> >(intToStr(sidWorkID+1), (*rds.sideDatasets)[sidWorkID][i]));
    string leftRst = "", rightRst = "";
    DataTypeStruct dts1, dts2;
    unordered_map< string, unordered_map<string,string> >* oldSideDatarow = rds.sideDatarow;
    rds.sideDatarow = &sideDatarow;
    bool evaled = m_leftExpression->evalExpression(rds, leftRst, dts1, true) && m_rightExpression->evalExpression(rds, rightRst, dts2, true);
    rds.sideDatarow = oldSideDatarow;
    if (evaled){
      if (anyDataCompare(leftRst, m_comparator, rightRst, m_datatype) == 1){
        sideMatchedRowID.insert(pair< int,int >(sidWorkID,i));
        sideMatchedRowIDs.push_back(sideMatchedRowID);
      }
    }
  }
  //}
  return sideMatchedRowIDs.size()>0;
}

bool FilterC::compareTwoSideExp(RuntimeDataStruct & rds, vector< unordered_map< int,int > > & sideMatchedRowIDs)
{
  bool bResult = false;
  unordered_map< string, unordered_map<string,string> > * oldsideDatarow = rds.sideDatarow;
  if (m_leftExpression && m_rightExpression){
    string leftRst = "", rightRst = "";
    DataTypeStruct dts1, dts2;
    unordered_map< string, unordered_map<string,string> > sideDatarow;
    rds.sideDatarow = &sideDatarow;
    if (m_leftExpression->containRefVar() || m_rightExpression->containRefVar()){ // do join checking
      //if (sideMatchedRowIDs.size() == rds.sideDatasets->size()) { // already matched by other filter
      //  for (int i=0; i<rds.sideDatasets->size(); i++){
      //    sideDatarow.insert(pair<string, unordered_map<string,string> >(intToStr(i), (*rds.sideDatasets)[i][sideMatchedRowIDs[i]]));
      //  }
      //  if (m_leftExpression->evalExpression(rds, leftRst, dts1, true) && m_rightExpression->evalExpression(rds, rightRst, dts2, true)){
      //    bResult = anyDataCompare(leftRst, m_comparator, rightRst, m_datatype) == 1;
      //  }else
      //    bResult = false;
      //}else{ // nested loop join side work datasets
      bResult = joinMatch(rds, sideMatchedRowIDs);
      //}
    }else{
      if (m_leftExpression->evalExpression(rds, leftRst, dts1, true) && m_rightExpression->evalExpression(rds, rightRst, dts2, true)){
        bResult = anyDataCompare(leftRst, m_comparator, rightRst, m_datatype) == 1;
      }else
        bResult = false;
    }
  }else{
    bResult = false;
  }
  rds.sideDatarow = oldsideDatarow;
  return bResult;
}

// calculate an expression prediction. no predication or comparasion failed means alway false
bool FilterC::compareExpressionI(RuntimeDataStruct & rds, vector< unordered_map< int,int > > & sideMatchedRowIDs){
  bool result=false;
  if (m_type == BRANCH){
    if (!m_leftNode || !m_rightNode)
      return false;
    vector< unordered_map< int,int > > leftMatchedIDs, rightMatchedIDs;
    if (m_junction == AND){
      result = m_leftNode->compareExpressionI(rds, leftMatchedIDs) && m_rightNode->compareExpressionI(rds, rightMatchedIDs);
      if (leftMatchedIDs.size()>0 && rightMatchedIDs.size()>0){
        // merge the matched row IDs joined by different keys
        bool bIntersection = false; // if both join resut matched the same sidework data, need to get the intersection data set, otherswise merge two data set
        for (unordered_map< int,int >::iterator it=leftMatchedIDs[0].begin(); it!=leftMatchedIDs[0].end(); it++)
          if (rightMatchedIDs[0].find(it->first)!=rightMatchedIDs[0].end()){
            bIntersection = true;
            break;
          }
        unordered_map< int,int > mergedIDs;
        if (bIntersection){ // doing intersection
          for (int i=0; i<leftMatchedIDs.size(); i++){
            mergedIDs.clear();
            for (int j=0; j<rightMatchedIDs.size(); j++){
              bool bMatched = true;
              // check each key from left join result, if the key exists in right join result, then need to check their value to see if matched. If multiple keys exist in both sides, need to check all of them.
              for (unordered_map< int,int >::iterator it=leftMatchedIDs[i].begin(); it!=leftMatchedIDs[i].end(); it++){
                if (rightMatchedIDs[j].find(it->first)!=rightMatchedIDs[j].end() && it->second != rightMatchedIDs[j][it->first]){ // check the matched row id of the same sidework data set from both joined results.
                  bMatched = false;
                  break;
                }
              }
              if (bMatched){
                mergedIDs.insert(leftMatchedIDs[i].begin(),leftMatchedIDs[i].end());
                mergedIDs.insert(rightMatchedIDs[j].begin(),rightMatchedIDs[j].end());
                sideMatchedRowIDs.push_back(mergedIDs);
              }
            }
          }
        }else{ // doing merge
          // simply merge each rows.
          for (int i=0; i<leftMatchedIDs.size(); i++){
            for (int j=0; j<rightMatchedIDs.size(); j++){
              mergedIDs.clear();
              mergedIDs.insert(leftMatchedIDs[i].begin(),leftMatchedIDs[i].end());
              mergedIDs.insert(rightMatchedIDs[j].begin(),rightMatchedIDs[j].end());
              sideMatchedRowIDs.push_back(mergedIDs);
            }
          }
        }
      }else if (leftMatchedIDs.size()>0)
        sideMatchedRowIDs = leftMatchedIDs;
      else if (rightMatchedIDs.size()>0)
        sideMatchedRowIDs = rightMatchedIDs;
    }else{
      result = m_leftNode->compareExpressionI(rds, leftMatchedIDs) || m_rightNode->compareExpressionI(rds, rightMatchedIDs);
      //result = m_leftNode->compareExpressionI(rds, leftMatchedIDs);
      //result = m_rightNode->compareExpressionI(rds, rightMatchedIDs) || result;
      sideMatchedRowIDs.insert(sideMatchedRowIDs.begin(),leftMatchedIDs.begin(),leftMatchedIDs.end());
      sideMatchedRowIDs.insert(sideMatchedRowIDs.begin(),rightMatchedIDs.begin(),rightMatchedIDs.end());
      // simply merge each rows.
      //unordered_map< int,int > mergedIDs;
      //for (int i=0; i<leftMatchedIDs.size(); i++){
      //  for (int j=0; j<rightMatchedIDs.size(); j++){
      //    mergedIDs.clear();
      //    mergedIDs.insert(leftMatchedIDs[i].begin(),leftMatchedIDs[i].end());
      //    mergedIDs.insert(rightMatchedIDs[j].begin(),rightMatchedIDs[j].end());
      //    sideMatchedRowIDs.push_back(mergedIDs);
      //  }
      //}
    }
    return result;
  }else if(m_type == LEAF){
    if (m_leftExpression && m_leftExpression->m_type == LEAF && m_leftExpression->m_expType == FUNCTION && m_leftExpression->m_Function && (m_leftExpression->m_Function->m_funcID==ANYCOL || m_leftExpression->m_Function->m_funcID==ALLCOL)) { // left anycol/allcol
      if (m_rightExpression && m_rightExpression->m_type == LEAF && m_rightExpression->m_expType == FUNCTION && m_rightExpression->m_Function && (m_rightExpression->m_Function->m_funcID==ANYCOL || m_rightExpression->m_Function->m_funcID==ALLCOL)) {
        trace(ERROR, "ANYCOL/ALLCOL cannot be presented on both comparing side!\n");
        return false;
      }
      bool bResult = (m_leftExpression->m_Function->m_funcID==ALLCOL);
      vector<ExpressionC> vExpandedExpr = m_leftExpression->m_Function->expandForeach(rds.fieldvalues->size());
      FilterC tmpFilter;
      copyTo(&tmpFilter);
      // compare each field using a temporary filter
      for (int i=0; i<vExpandedExpr.size(); i++){
        vExpandedExpr[i].copyTo(tmpFilter.m_leftExpression);
        tmpFilter.m_leftExpression->analyzeColumns(m_fieldnames, m_fieldtypes, m_rawDatatype, m_sideDatatypes);
        bool bThis = tmpFilter.compareExpressionI(rds, sideMatchedRowIDs);
        bResult = m_leftExpression->m_Function->m_funcID==ALLCOL?(bResult&&bThis):(bResult||bThis);
      }
      return bResult;
    }
    if (m_rightExpression && m_rightExpression->m_type == LEAF && m_rightExpression->m_expType == FUNCTION && m_rightExpression->m_Function && (m_rightExpression->m_Function->m_funcID==ANYCOL || m_rightExpression->m_Function->m_funcID==ALLCOL)) { // right anycol/allcol
      if (m_leftExpression && m_leftExpression->m_type == LEAF && m_leftExpression->m_expType == FUNCTION && m_leftExpression->m_Function && (m_leftExpression->m_Function->m_funcID==ANYCOL || m_leftExpression->m_Function->m_funcID==ALLCOL)) {
        trace(ERROR, "ANYCOL/ALLCOL cannot be presented on both comparing side!\n");
        return false;
      }
      bool bResult = (m_rightExpression->m_Function->m_funcID==ALLCOL);
      vector<ExpressionC> vExpandedExpr = m_rightExpression->m_Function->expandForeach(rds.fieldvalues->size());
      FilterC tmpFilter;
      copyTo(&tmpFilter);
      // compare each field
      for (int i=0; i<vExpandedExpr.size(); i++){
        vExpandedExpr[i].copyTo(tmpFilter.m_rightExpression);
        bool bThis = tmpFilter.compareExpressionI(rds, sideMatchedRowIDs);
        bResult = m_rightExpression->m_Function->m_funcID==ALLCOL?(bResult&&bThis):(bResult||bThis);
      }
      return bResult;
    }
    if (m_comparator == IN || m_comparator == NOIN){
      return compareIn(rds)?(m_comparator == IN?true:false):(m_comparator == IN?false:true);
    }
    else{
      if (rds.aggFuncs && rds.aggFuncs->size()==0 && rds.anaFuncs && rds.anaFuncs->size()==0){ // in the matching the raw data process, dont compare aggregation function
        if (m_leftExpression && m_rightExpression){
          // do not filter if aggregation function involved
          if (m_leftExpression->containGroupFunc() || m_rightExpression->containGroupFunc() || m_leftExpression->containAnaFunc() || m_rightExpression->containAnaFunc())
            return true;
          else{ // no aggregation function in either left or right expression. do filter comparasion
            return compareTwoSideExp(rds, sideMatchedRowIDs);
          }
        }else
          return false;
      }else{ // matching the aggregation/analytic functions only
        if (m_leftExpression && m_rightExpression){
          // do not filter if no aggregation/analytic function involved (exempt const)
          if ((!m_leftExpression->containGroupFunc() && !m_leftExpression->containAnaFunc() && !(m_leftExpression->m_type==LEAF&&m_leftExpression->m_expType==CONST)) || (m_rightExpression->containGroupFunc() && m_rightExpression->containAnaFunc() && !(m_rightExpression->m_type==LEAF&&m_rightExpression->m_expType==CONST)))
            return true;
          else{ // aggregation/analytic function in either left or right expression. do filter comparasion
            if ((m_leftExpression->containGroupFunc() || m_rightExpression->containGroupFunc()) && rds.aggFuncs->size()==0) // return true if rds.aggFuncs is empty when comparing aggregation function
              return true;
            if ((m_leftExpression->containAnaFunc() || m_rightExpression->containAnaFunc()) && rds.anaFuncs->size()==0) // return true if anaFuncs is empty when comparing analytic function
              return true;
            return compareTwoSideExp(rds, sideMatchedRowIDs);
          }
        }else
          return false;
      }
    }
  }else{ 
    return false;
  }
  return result;
}
