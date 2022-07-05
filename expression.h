/*******************************************************************************
//
//        File: expression.h
// Description: Expression class header
//       Usage: expression.h
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/

#ifndef __EXPRESSIONC_H
#define __EXPRESSIONC_H

#include "expression.h"
#include "commfuncs.h"

class ExpressionC;

class ExpressionC
{
  public:

    ExpressionC();
    ExpressionC(string expString);
    ExpressionC(ExpressionC* node);
    ExpressionC(ExpressionC* leftNode, ExpressionC* rightNode); // construct a branch
    ExpressionC(int operate, int colId, string data); // construct a leaf

    ~ExpressionC();

    int m_type;       // 1: BRANCH; 2: LEAF
    int m_operate;    // if type is BRANCH, 1: +; 2: -; 3: *; 4: /; 5: ^; . Otherwise, it's meaningless
    int m_datatype;   // 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
    int m_expType;    // if type is LEAF, the expression string type. 1: CONST, 2: COLUMN, 3: VARIABLE, 4:FUNCTION
    string m_expStr;  // if type is LEAF, the expression string, either be a CONST, COLUMN or FUNCTION; if type is BRANCH, it's the full expression string
    int m_colId;      // if type is LEAF, and the expression string type is COLUMN. it's id of column. Otherwise, it's meaningless
    ExpressionC* m_leftNode;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
    ExpressionC* m_rightNode;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
    ExpressionC* m_parentNode;    // for all types except the root, it links to parent node. Otherwise, it's meaningless

    void setExpstr(string expString); // set expression string and analyze the string
    int getLeftHeight(); // get left tree Height
    int getRightHeight(); // get left tree Height
    void add(ExpressionC* node, int op, bool leafGrowth, bool addOnTop); // add a NEW expression into tree
    void dump();
    string getEntireExpstr();
    bool containsColId(int colId); // detect if predication contains special colId
    ExpressionC* getFirstPredByColId(int colId, bool leftFirst); // detect if predication contains special colId
    int analyzeColumns(vector<string>* fieldnames, vector<int>* fieldtypes); // analyze column ID & name from metadata, return data type of current node
    bool columnsAnalyzed();
    bool expstrAnalyzed();
    bool groupFuncOnly();
    bool compatibleExp(ExpressionC comExp);
    ExpressionC* cloneMe();
    void copyTo(ExpressionC* node);
    std::set<int>  getAllColIDs(int side); // get all involved colIDs in this prediction
    map<int,string>  buildMap(); // build the prediction as a HashMap
    int size(); // get all involved colIDs in this prediction
    void clear(); // clear predictin
    bool remove(ExpressionC* node); // remove a node from prediction. Note: the input node is the address of the node contains in current prediction
    void fillDataForColumns(map <string, string> & dataList, vector <string> columns); // build a data list for a set of column, keeping same sequence, fill the absent column with NULL
    bool evalExpression(vector<string>* fieldnames, vector<string>* fieldvalues, map<string,string>* varvalues, string & sResult); // calculate this expression. fieldnames: column names; fieldvalues: column values; varvalues: variable values; sResult: return result. column names are upper case; skipRow: wheather skip @row or not
    bool mergeConstNodes(string & sResult); // merge const expression, reduce calculation during matching. If merged successfully, return true, sResult returns result.

  private:
    bool m_metaDataAnzlyzed; // analyze column name to column id.
    bool m_expstrAnalyzed; // if expression string analyzed

    void dump(int deep);
    bool buildExpression();  // build expression class from the expression string
    void alignChildrenDataType(); // align children datatype with current datatype

    static std::set<char> m_operators; // "^", "*", "/" should be before "+", "-"
    vector<string>* m_fieldnames;  // all nodes (parent & children) point to the same address!!!
    vector<int>* m_fieldtypes;     // all nodes (parent & children) point to the same address!!!

  protected:
    void init();
};

#endif // __EXPRESSIONC_H

