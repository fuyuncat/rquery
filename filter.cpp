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
  
  m_metaDataAnzlyzed = false; // analyze column name to column id.
  m_expstrAnalyzed = false;

  m_comparators.push_back("!=");m_comparators.push_back(">=");m_comparators.push_back("<=");m_comparators.push_back("=");m_comparators.push_back(">");m_comparators.push_back("<");m_comparators.push_back(" LIKE ");m_comparators.push_back(" REGLIKE ");m_comparators.push_back(" NOLIKE ");m_comparators.push_back(" NOREGLIKE ");m_comparators.push_back(" IN ");m_comparators.push_back(" NOIN "); // "=", "<", ">" should be put after "<=" ">=" "!="
}

void FilterC::setExpstr(string expStr)
{
  m_expStr = expStr;
  buildFilter();
}

FilterC::FilterC()
{
  init();
}

FilterC::~FilterC()
{

}

FilterC::FilterC(string expStr)
{
  init();
  setExpstr(expStr);
}

FilterC::FilterC(FilterC* node)
{
  init();

  m_type = node->m_type;
  m_junction = node->m_junction;
  m_comparator = node->m_comparator;
  m_datatype = node->m_datatype;
  m_leftColId = node->m_leftColId;
  m_rightColId = node->m_rightColId;
  m_expStr = node->m_expStr;
  m_leftExpStr = node->m_leftExpStr;
  m_rightExpStr = node->m_rightExpStr;
  m_leftExpression = node->m_leftExpression;
  m_rightExpression = node->m_rightExpression;
  m_leftNode = node->m_leftNode;
  m_rightNode = node->m_rightNode;
  m_parentNode = node->m_parentNode;
  m_metaDataAnzlyzed = node->m_metaDataAnzlyzed;
  m_expstrAnalyzed = node->m_expstrAnalyzed;
  //predStr = node.predStr;
}

FilterC::FilterC(int junction, FilterC* leftNode, FilterC* rightNode)
{
  init();
  m_type = BRANCH;
  m_junction = junction;
  m_leftNode = leftNode;
  m_rightNode = rightNode;
  //m_leftNode = leftNode==NULL?NULL:new Prediction(leftNode);
  //m_rightNode = rightNode==NULL?NULL:new Prediction(rightNode);;
}

FilterC::FilterC(int comparator, int colId, string data)
{
  init();
  m_type = LEAF;
  m_comparator = comparator;
  m_leftColId = colId;
  m_rightExpStr = data;
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
      if (node->m_comparator == IN || node->m_comparator == NOIN){ // hard code for IN/NOIN,m_rightExpression is NULL, m_inExpressions contains IN expressions
        if (node->m_rightExpression){
          node->m_rightExpression->clear();
          delete node->m_rightExpression;
        }
        node->m_rightExpression = NULL;
        if (node->m_rightExpStr.length()<2 || node->m_rightExpStr[0]!='(' || node->m_rightExpStr[node->m_rightExpStr.length()-1]!=')'){
          trace(ERROR, "Invalid IN string '%s'\n", node->m_rightExpStr.c_str());
          return;
        }
        string sElements = node->m_rightExpStr.substr(1,node->m_rightExpStr.length()-2);
        vector<string> vElements = split(sElements,',',"''()",'\\',{'(',')'},false,true);
        for (int i=0;i<vElements.size();i++){
          string sResult, sElement = trim_copy(vElements[i]);
          if (sElement.empty()){
            trace(ERROR, "Empty IN element string!\n");
            return;
          }
          ExpressionC eElement(sElement);
          if (!eElement.expstrAnalyzed()){
            trace(ERROR, "Failed to analyze the expression of %s!\n", sElement.c_str());
            return;
          }
          m_inExpressions.push_back(eElement);
        }
      }else
        node->m_rightExpression = new ExpressionC(node->m_rightExpStr);
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
          delete m_leftNode;
          m_leftNode = NULL;
          return false;
        }
        m_rightNode = new FilterC(m_expStr.substr(i+splitor.length()));
        m_rightNode->m_parentNode = this;
        //trace(DEBUG,"Building rightNode\n");
        if (!m_rightNode->buildFilter(" OR ",quoters)){
          m_rightNode->clear();
          delete m_rightNode;
          m_rightNode = NULL;
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
bool FilterC::analyzeColumns(vector<string>* fieldnames, vector<DataTypeStruct>* fieldtypes, DataTypeStruct* rawDatatype){
  trace(DEBUG, "Analyzing columns in filter '%s'\n", m_expStr.c_str());
  if (!fieldnames || !fieldtypes){
    trace(ERROR, "(Filter)fieldnames or fieldtypes is NULL!\n");
    return UNKNOWN;
  }
  if (m_type == BRANCH){
    m_metaDataAnzlyzed = true;
    if (m_leftNode)
        m_metaDataAnzlyzed = m_leftNode->analyzeColumns(fieldnames, fieldtypes, rawDatatype) && m_metaDataAnzlyzed ;
    if (m_rightNode)
        m_metaDataAnzlyzed = m_rightNode->analyzeColumns(fieldnames, fieldtypes, rawDatatype) && m_metaDataAnzlyzed;
    if (!m_metaDataAnzlyzed)
        return m_metaDataAnzlyzed;
    //if (m_leftExpression)
    //  m_leftExpression->analyzeColumns(fieldnames, fieldtypes, rawDatatype);
    //if (m_rightExpression)
    //  m_rightExpression->analyzeColumns(fieldnames, fieldtypes, rawDatatype);
  }else if (m_type == LEAF){
    if (m_leftExpression){
      if (!m_leftExpression->expstrAnalyzed()){
        m_leftExpression->clear();
        delete m_leftExpression;
        m_leftExpression = new ExpressionC(m_leftExpStr);
      }
    }else
      m_leftExpression = new ExpressionC(m_leftExpStr);
    if (!m_leftExpression->expstrAnalyzed()){
      trace(ERROR, "Failed to analyze m_leftExpression of filter '%s'!\n", m_leftExpression->getEntireExpstr().c_str());
      m_leftExpression->clear();
      delete m_leftExpression;
      m_leftExpression = NULL;
      return false;
    }
    m_leftExpression->analyzeColumns(fieldnames, fieldtypes, rawDatatype);

    if (m_comparator == IN || m_comparator == NOIN){ // hard code for IN/NOIN,m_rightExpression is NULL, m_inExpressions contains IN expressions
      DataTypeStruct dts;
      dts.datatype = -99;
      for (int i=0; i<m_inExpressions.size(); i++){
        if (m_inExpressions[i].expstrAnalyzed()){
          m_inExpressions[i].analyzeColumns(fieldnames, fieldtypes, rawDatatype);
          if (dts.datatype == -99)
            dts = m_inExpressions[i].m_datatype;
          else
            dts = getCompatibleDataType(dts, m_inExpressions[i].m_datatype);
        }else
          return false;
      }
      m_datatype = getCompatibleDataType(m_leftExpression->m_datatype, dts);
    }else{
      if (m_rightExpression){
        if (!m_rightExpression->expstrAnalyzed()){
          m_rightExpression->clear();
          delete m_rightExpression;
          m_rightExpression = new ExpressionC(m_leftExpStr);
        }
      }else
        m_rightExpression = new ExpressionC(m_leftExpStr);
      if (!m_rightExpression->expstrAnalyzed()){
        trace(ERROR, "Failed to analyze m_rightExpression of filter '%s'!\n", m_rightExpression->getEntireExpstr().c_str());
        m_rightExpression->clear();
        delete m_rightExpression;
        m_rightExpression = NULL;
        return false;
      }
      m_rightExpression->analyzeColumns(fieldnames, fieldtypes, rawDatatype);
      m_datatype = getCompatibleDataType(m_leftExpression->m_datatype, m_rightExpression->m_datatype);
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

void FilterC::copyTo(FilterC* node){
  if (!node)
    return;
  else{
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

// clear predictin
void FilterC::clear(){
  if (m_leftNode){
    m_leftNode->clear();
    delete m_leftNode;
    m_leftNode = NULL;
  }
  if (m_rightNode){
    m_rightNode->clear();
    delete m_rightNode;
    m_rightNode = NULL;
  }
  m_type = UNKNOWN;
  m_datatype.datatype = UNKNOWN;
  m_datatype.extrainfo = "";
  m_junction = UNKNOWN;
  m_comparator = UNKNOWN;
  m_leftColId = -1;
  m_rightColId = -1;
  m_rightExpStr = "";
  m_leftExpStr = "";
  m_expStr = "";
  m_metaDataAnzlyzed = false;
  m_expstrAnalyzed = false;
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
      delete m_leftNode;
      m_leftNode = NULL;
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
      delete m_rightNode;
      m_rightNode = NULL;
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

bool FilterC::compareIn(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs)
{
  string leftRst, sResult;
  DataTypeStruct dts1, dts2;
  bool bDoAggFilter = (aggFuncs && aggFuncs->size()>0);
  // dont do filte in two scenarios: 1: aggFuncs prvoided, but no aggregation function involved in the expression (except CONST); 2: no aggFuncs provided, but aggregation function involved in the expression;
  if ((!bDoAggFilter && m_leftExpression && m_leftExpression->containGroupFunc()) || (bDoAggFilter && !m_leftExpression->containGroupFunc() && !(m_leftExpression->m_type==LEAF&&m_leftExpression->m_expType==CONST)))
    return true;
  if (m_inExpressions.size() == 0 || !m_leftExpression || !m_leftExpression->evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, leftRst, dts1, true)){
    trace(ERROR, "Failed to get the value to be compared in IN!\n"); 
    return false;
  }
  // trace(DEBUG, "Comparing '%s' IN '%s' (data type: %s)\n", leftRst.c_str(), m_rightExpStr.c_str(), decodeDatatype(m_datatype.datatype).c_str());

  for (int i=0;i<m_inExpressions.size();i++){
    if ((!bDoAggFilter && m_inExpressions[i].containGroupFunc()) || (bDoAggFilter && !m_inExpressions[i].containGroupFunc() && !(m_inExpressions[i].m_type==LEAF&&m_inExpressions[i].m_expType==CONST)))
      return false;
    if (!m_inExpressions[i].evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, sResult, dts2, true)){
      trace(ERROR, "Failed to get result of IN element %s!\n", m_inExpressions[i].getEntireExpstr().c_str());
      return false;
    }
    if (anyDataCompare(leftRst, EQ, sResult, m_datatype) == 1){
      return true;
    }
  }
  return false;
}

// calculate an expression prediction. no predication or comparasion failed means alway false
bool FilterC::compareExpression(vector<string>* fieldvalues, map<string,string>* varvalues, unordered_map< string,GroupProp >* aggFuncs, unordered_map< string,vector<string> >* anaFuncs){
  bool result=false;
  if (m_type == BRANCH){
    if (!m_leftNode || !m_rightNode)
      return false;
    if (m_junction == AND)
      return m_leftNode->compareExpression(fieldvalues, varvalues, aggFuncs, anaFuncs) && m_rightNode->compareExpression(fieldvalues, varvalues, aggFuncs, anaFuncs);
    else
      return m_leftNode->compareExpression(fieldvalues, varvalues, aggFuncs, anaFuncs) || m_rightNode->compareExpression(fieldvalues, varvalues, aggFuncs, anaFuncs);
  }else if(m_type == LEAF){
    if (m_comparator == IN || m_comparator == NOIN){
      return compareIn(fieldvalues, varvalues, aggFuncs, anaFuncs)?(m_comparator == IN?true:false):(m_comparator == IN?false:true);
    }
    else{
      if (aggFuncs && aggFuncs->size()==0 && anaFuncs && anaFuncs->size()==0){ // in the matching the raw data process, dont compare aggregation function
        if (m_leftExpression && m_rightExpression){
          // do not filter if aggregation function involved
          if (m_leftExpression->containGroupFunc() || m_rightExpression->containGroupFunc() || m_leftExpression->containAnaFunc() || m_rightExpression->containAnaFunc())
            return true;
          else{ // no aggregation function in either left or right expression. do filter comparasion
            string leftRst = "", rightRst = "";
            DataTypeStruct dts1, dts2;
            if (m_leftExpression->evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, leftRst, dts1, true) && m_rightExpression->evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, rightRst, dts2, true)){
              //trace(DEBUG2, "(1)Comparing '%s' %s '%s' (data type: %s)\n", leftRst.c_str(), decodeComparator(m_comparator).c_str(), rightRst.c_str(), decodeDatatype(m_datatype.datatype).c_str());
              return anyDataCompare(leftRst, m_comparator, rightRst, m_datatype) == 1;
            }else
              return false;
          }
        }else
          return false;
      }else{ // matching the aggregation/analytic functions only
        if (m_leftExpression && m_rightExpression){
          // do not filter if no aggregation/analytic function involved (exempt const)
          if ((!m_leftExpression->containGroupFunc() && !m_leftExpression->containAnaFunc() && !(m_leftExpression->m_type==LEAF&&m_leftExpression->m_expType==CONST)) || (m_rightExpression->containGroupFunc() && m_rightExpression->containAnaFunc() && !(m_rightExpression->m_type==LEAF&&m_rightExpression->m_expType==CONST)))
            return true;
          else{ // aggregation/analytic function in either left or right expression. do filter comparasion
            string leftRst = "", rightRst = "";
            DataTypeStruct dts1, dts2;
            if (m_leftExpression && m_rightExpression && m_leftExpression->evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, leftRst, dts1, true) && m_rightExpression->evalExpression(fieldvalues, varvalues, aggFuncs, anaFuncs, rightRst, dts2, true)){
              //trace(DEBUG2, "(2)Comparing '%s' %s '%s' (data type: %s)\n", leftRst.c_str(), decodeComparator(m_comparator).c_str(), rightRst.c_str(), decodeDatatype(m_datatype.datatype).c_str());
              return anyDataCompare(leftRst, m_comparator, rightRst, m_datatype) == 1;
            }else
              return false;
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
