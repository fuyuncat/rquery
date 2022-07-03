/*******************************************************************************
//
//        File: filter.h
// Description: Filter class header
//       Usage: filter.h
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/

#ifndef __FILTERC_H
#define __FILTERC_H

#include "expression.h"
#include "commfuncs.h"

class FilterC;

class FilterC
{
  public:

    FilterC();
    FilterC(string expStr);
    FilterC(FilterC* node);
    FilterC(int junction, FilterC* leftNode, FilterC* rightNode); // construct a branch
    FilterC(int comparator, int colId, string data); // construct a leaf

    ~FilterC();

    int m_type;       // 1: branch; 2: leaf
    int m_junction;   // if type is BRANCH, 1: and; 2: or. Otherwise, it's meaningless
    int m_comparator; // if type is LEAF, 1: ==; 2: >; 3: <; 4: !=; 5: >=; 6: <=; 7: LIKE; 8: REGLIKE. Otherwise, it's meaningless
    int m_datatype;   // if type is LEAF, 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
    int m_leftColId;              // if type is LEAF, it's id of column on the left to be predicted. Otherwise, it's meaningless
    int m_rightColId;             // if type is LEAF, it's id of column on the right to be predicted. Otherwise, it's meaningless
    string m_expStr;
    string m_leftExpStr;    // if type is LEAF, it's id of column to be predicted. Otherwise, it's meaningless
    string m_rightExpStr;   // if type is LEAF, it's data to be predicted. Otherwise, it's meaningless
    ExpressionC* m_leftExpression; // meaningful only if type is LEAF
    ExpressionC* m_rightExpression; // meaningful only if type is LEAF
    FilterC* m_leftNode;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
    FilterC* m_rightNode;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
    FilterC* m_parentNode;    // for all types except the root, it links to parent node. Otherwise, it's meaningless

    void setExpstr(string expStr);
    void buildFilter();  // build Filter from expression string
    int getLeftHeight(); // get left tree Height
    int getRightHeight(); // get left tree Height
    void add(FilterC* node, int junction, bool leafGrowth, bool addOnTop); // add a NEW preiction into tree
    void dump();
    bool containsColId(int colId); // detect if predication contains special colId
    FilterC* getFirstPredByColId(int colId, bool leftFirst); // detect if predication contains special colId
    bool analyzeColumns(vector<string>* fieldnames, vector<int>* fieldtypes); // analyze column ID & name from metadata
    bool columnsAnalyzed();
    bool expstrAnalyzed();
    FilterC* cloneMe();
    void copyTo(FilterC* node);
    std::set<int>  getAllColIDs(int side); // get all involved colIDs in this prediction
    map<int,string>  buildMap(); // build the prediction as a HashMap
    int size(); // get all involved colIDs in this prediction
    void clear(); // clear predictin
    bool remove(FilterC* node); // remove a node from prediction. Note: the input node is the address of the node contains in current prediction
    void fillDataForColumns(map <string, string> & dataList, vector <string> columns); // build a data list for a set of column, keeping same sequence, fill the absent column with NULL
    void mergeExprConstNodes();  // merge const in the expressions

    bool compareExpression(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues); // calculate an expression prediction. no predication or comparasion failed means alway false

  private:
    bool m_metaDataAnzlyzed; // analyze column name to column id.
    bool m_expstrAnalyzed;   // if expression string analyzed
    static vector<string> m_comparators; // ">=", "<=" should be before ">", "<"
    
    void dump(int deep);
    void buildLeafNodeFromStr(FilterC* node, string str); // build a leaf node
    bool buildFilter(string splitor, string quoters); // build current filter class from the expression string

  protected:
    void init();
};

#endif // __FILTERC_H

