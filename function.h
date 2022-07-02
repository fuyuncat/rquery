/*******************************************************************************
//
//        File: function.h
// Description: Function class header
//       Usage: function.h
//     Created: 28/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  28/06/2022: Created
//
*******************************************************************************/

#ifndef __FUNCTIONC_H
#define __FUNCTIONC_H

#include "function.h"
#include "commfuncs.h"

class ExpressionC;

class FunctionC
{
  public:

    FunctionC();
    FunctionC(string expString);
    FunctionC(FunctionC* node);
    FunctionC(FunctionC* leftNode, FunctionC* rightNode); // construct a branch
    FunctionC(int operate, int colId, string data); // construct a leaf

    ~FunctionC();

    int m_type;       // 1: BRANCH; 2: LEAF
    int m_operate;    // if type is BRANCH, 1: +; 2: -; 3: *; 4: /; 5: ^; . Otherwise, it's meaningless
    int m_datatype;   // 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
    int m_expType;    // if type is LEAF, the expression string type. 1: CONST, 2: COLUMN, 3: FUNCTION
    string m_expStr;  // if type is LEAF, the expression string, either be a CONST, COLUMN or FUNCTION; if type is BRANCH, it's the full expression string
    int m_colId;      // if type is LEAF, and the expression string type is COLUMN. it's id of column. Otherwise, it's meaningless
    FunctionC* m_leftNode;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
    FunctionC* m_rightNode;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
    FunctionC* m_parentNode;    // for all types except the root, it links to parent node. Otherwise, it's meaningless
    
    bool runFunction(string & sResult);
    bool runFunction(vector<string>* fieldnames, map<string,string>* fieldvalues, map<string,string>* varvalues, string & sResult);

    int getLeftHeight(); // get left tree Height
    int getRightHeight(); // get left tree Height
    void add(FunctionC* node, int op, bool leafGrowth, bool addOnTop); // add a NEW expression into tree
    void dump();
    bool containsColId(int colId); // detect if predication contains special colId
    FunctionC* getFirstPredByColId(int colId, bool leftFirst); // detect if predication contains special colId
    int analyzeColumns(vector<string> fieldnames, vector<int> fieldtypes); // analyze column ID & name from metadata, return data type of current node
    bool columnsAnalyzed();
    FunctionC* cloneMe();
    void copyTo(FunctionC* node);
    std::set<int>  getAllColIDs(int side); // get all involved colIDs in this prediction
    map<int,string>  buildMap(); // build the prediction as a HashMap
    int size(); // get all involved colIDs in this prediction
    void clear(); // clear predictin
    bool remove(FunctionC* node); // remove a node from prediction. Note: the input node is the address of the node contains in current prediction
    void fillDataForColumns(map <string, string> & dataList, vector <string> columns); // build a data list for a set of column, keeping same sequence, fill the absent column with NULL

  private:
    bool m_metaDataAnzlyzed; // analyze column name to column id.

    void dump(int deep);
    void checkDataType();

  protected:
    void init();
};

#endif // __FUNCTIONC_H

