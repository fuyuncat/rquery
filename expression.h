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
    int m_expType;    // if type is LEAF, the expression string type. 1: CONST, 2: COLUMN, 3: FUNCTION
    string m_expStr;  // if type is LEAF, the expression string, either be a CONST, COLUMN or FUNCTION; if type is BRANCH, it's the full expression string
    int m_colId;      // if type is LEAF, and the expression string type is COLUMN. it's id of column. Otherwise, it's meaningless
    ExpressionC* m_leftNode;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
    ExpressionC* m_rightNode;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
    ExpressionC* m_parentNode;    // for all types except the root, it links to parent node. Otherwise, it's meaningless

    int getLeftHeight(); // get left tree Height
    int getRightHeight(); // get left tree Height
    void add(ExpressionC* node, int op, bool leafGrowth, bool addOnTop); // add a NEW expression into tree
    void dump();
    bool containsColId(int colId); // detect if predication contains special colId
    ExpressionC* getFirstPredByColId(int colId, bool leftFirst); // detect if predication contains special colId
    int analyzeColumns(vector<string> m_fieldnames, vector<int> m_fieldtypes); // analyze column ID & name from metadata, return data type of current node
    bool columnsAnalyzed();
    ExpressionC* cloneMe();
    void copyTo(ExpressionC* node);
    std::set<int>  getAllColIDs(int side); // get all involved colIDs in this prediction
    map<int,string>  buildMap(); // build the prediction as a HashMap
    string evalExpression(); // calculate an expression prediction
    int size(); // get all involved colIDs in this prediction
    void clear(); // clear predictin
    bool remove(ExpressionC* node); // remove a node from prediction. Note: the input node is the address of the node contains in current prediction
    void fillDataForColumns(map <string, string> & dataList, vector <string> columns); // build a data list for a set of column, keeping same sequence, fill the absent column with NULL

  private:
    bool m_metaDataAnzlyzed; // analyze column name to column id.
    
    void dump(int deep);

  protected:
    void init();
};

#endif // __EXPRESSIONC_H

