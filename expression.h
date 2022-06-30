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
    ExpressionC(string expStr);
    ExpressionC(ExpressionC* node);
    ExpressionC(ExpressionC* leftNode, ExpressionC* rightNode); // construct a branch
    ExpressionC(int operate, int colId, string data); // construct a leaf

    ~ExpressionC();

    int type;       // 1: branch; 2: leaf
    int operate; // 1: +; 2: -; 3: *; 4: /; 5: ^; . Otherwise, it's meaningless
    int datatype;   // if type is LEAF, 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
    int leftColId;              // if type is LEAF, it's id of column on the left to be predicted. Otherwise, it's meaningless
    int rightColId;             // if type is LEAF, it's id of column on the right to be predicted. Otherwise, it's meaningless
    string leftExpStr;    // if type is LEAF, it's id of column to be predicted. Otherwise, it's meaningless
    string rightExpStr;   // if type is LEAF, it's data to be predicted. Otherwise, it's meaningless
    ExpressionC* leftNode;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
    ExpressionC* rightNode;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
    ExpressionC* parentNode;    // for all types except the root, it links to parent node. Otherwise, it's meaningless

    int getLeftHeight(); // get left tree Height
    int getRightHeight(); // get left tree Height
    void add(ExpressionC* node, bool leafGrowth, bool addOnTop); // add a NEW preiction into tree
    void dump();
    bool containsColId(int colId); // detect if predication contains special colId
    ExpressionC* getFirstPredByColId(int colId, bool leftFirst); // detect if predication contains special colId
    bool analyzeColumns(vector<string> m_fieldnames1, vector<string> m_fieldnames2); // analyze column ID & name from metadata
    bool columnsAnalyzed();
    ExpressionC* cloneMe();
    void copyTo(ExpressionC* node);
    std::set<int>  getAllColIDs(int side); // get all involved colIDs in this prediction
    map<int,string>  buildMap(); // build the prediction as a HashMap
    auto evalExpression(); // calculate an expression prediction
    int size(); // get all involved colIDs in this prediction
    void clear(); // clear predictin
    bool remove(ExpressionC* node); // remove a node from prediction. Note: the input node is the address of the node contains in current prediction
    void fillDataForColumns(map <string, string> & dataList, vector <string> columns); // build a data list for a set of column, keeping same sequence, fill the absent column with NULL

  private:
    bool metaDataAnzlyzed; // analyze column name to column id.
    string m_expressionStr;
    
    void dump(int deep);

  protected:
    void init();
};

#endif // __EXPRESSIONC_H

