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
#include <unordered_map>
#include "commfuncs.h"

class ExpressionC;

class FunctionC;

class ExpressionC
{
  public:

    ExpressionC();
    ExpressionC(const string & expString);
    ExpressionC(ExpressionC* node);
    ExpressionC(ExpressionC* leftNode, ExpressionC* rightNode); // construct a branch
    ExpressionC(const int & operate, const int & colId, const string & data); // construct a leaf
    ExpressionC(const ExpressionC& other);

    ~ExpressionC();
    ExpressionC& operator=(const ExpressionC& other);

    void copyTo(ExpressionC* node) const;

    short int m_type;       // 1: BRANCH; 2: LEAF
    short int m_operate;    // if type is BRANCH, 1: +; 2: -; 3: *; 4: /; 5: ^; . Otherwise, it's meaningless
    DataTypeStruct m_datatype;   // 1: STRING; 2: LONG; 3: INTEGER; 4: DOUBLE; 5: DATE; 6: TIMESTAMP; 7: BOOLEAN. Otherwise, it's meaningless
    short int m_expType;    // if type is LEAF, the expression string type. 1: CONST, 2: COLUMN, 3: VARIABLE, 4:FUNCTION, 5:MACROPARA
    short int m_funcID; // function ID when expression type is FUNCTION
    string m_expStr;  // if type is LEAF, the expression string, either be a CONST, COLUMN or FUNCTION; if type is BRANCH, it's the full expression string
    int m_colId;      // if type is LEAF, and the expression string type is COLUMN. it's id of column. Otherwise, it's meaningless
    ExpressionC* m_leftNode; // if type is BRANCH, it links to the left child node. Otherwise, it's meaningless
    ExpressionC* m_rightNode;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
    ExpressionC* m_parentNode;    // for all types except the root, it links to parent node. Otherwise, it's meaningless
    FunctionC* m_Function;  // if m_expType is FUNCTION, it points to a function class, otherwise, it's NULL

    void setExpstr(const string & expString); // set expression string and analyze the string
    int getLeftHeight(); // get left tree Height
    int getRightHeight(); // get left tree Height
    void add(ExpressionC* node, int op, bool leafGrowth, bool addOnTop); // add a NEW expression into tree
    void dump() const;
    string getEntireExpstr() const;
    bool containsColId(const int & colId); // detect if predication contains special colId
    ExpressionC* getFirstPredByColId(const int & colId, const bool & leftFirst); // detect if predication contains special colId
    DataTypeStruct analyzeColumns(vector<string>* fieldnames, vector<DataTypeStruct>* fieldtypes, DataTypeStruct* rawDatatype, unordered_map< string, unordered_map<string,DataTypeStruct> >* sideDatatypes); // analyze column ID & name from metadata, return data type of current node
    bool columnsAnalyzed();
    bool expstrAnalyzed();
    bool groupFuncOnly(); // check if the expression contains group (aggregation) function only except CONST
    bool containGroupFunc(); // check if the expression contains group (aggregation) function
    bool containAnaFunc(); // check if the expression contains analytic function
    bool containRefVar(); // check if the expression contains reference variable @R[][]
    int getSideWorkID(); // if the expression contains reference variable @R[][], return the first found side work ID (the first subscribe)
    void getAllColumnNames(vector<string> & fieldnames);  // get all potential column/variable (upper case)
    bool inColNamesRange(const vector<string> & fieldnames); // check if all column/variable in a given list of names (upper case).
    ExpressionC* cloneMe();
    unordered_set<int>  getAllColIDs(const int & side); // get all involved colIDs in this prediction
    unordered_map<int,string>  buildMap(); // build the prediction as a HashMap
    int size(); // get all involved colIDs in this prediction
    void clear(); // clear predictin
    bool remove(ExpressionC* node); // remove a node from prediction. Note: the input node is the address of the node contains in current prediction
    void fillDataForColumns(unordered_map <string, string> & dataList, const vector <string> & columns); // build a data list for a set of column, keeping same sequence, fill the absent column with NULL
    bool evalAnalyticFunc(unordered_map< string,string > * anaResult, string & sResult); // get expression result from pre evaled analytic function results
    bool evalExpression(RuntimeDataStruct & rds, string & sResult, DataTypeStruct & dts, const bool & getresultonly); // calculate this expression. fieldnames: column names; fieldvalues: column values; varvalues: variable values; sResult: return result. column names are upper case; skipRow: wheather skip @row or not. extrainfo so far for date format only
    bool mergeConstNodes(string & sResult); // merge const expression, reduce calculation during matching. If merged successfully, return true, sResult returns result.
    bool getAggFuncs(unordered_map< string,GroupProp > & aggFuncs); // get the full list of aggregation functions in the expression.
    bool getAnaFuncs(unordered_map< string,vector<ExpressionC> > & anaFuncs, unordered_map< string, vector<int> > & anaGroupNums); // get the full list of analytic functions in the expression.
    bool getTreeFuncs(unordered_map< string,vector<ExpressionC> > & treeFuncs); // get the full list of hierarchy (tree) functions in the expression.
    void setTreeFuncs(unordered_map< string,string > * treeFuncVals); // set result for the tree functions
    FunctionC* getAnaFunc(const string & funcExpStr); // search analytic function in this expression using the function expression string.
    bool calAggFunc(const GroupProp & aggGroupProp, FunctionC* function, string & sResult); // calculate final aggregation function result

  private:
    bool m_metaDataAnzlyzed; // analyze column name to column id.
    bool m_expstrAnalyzed; // if expression string analyzed

    static std::set<char> m_operators; // "^", "*", "/" should be before "+", "-"
    vector<string>* m_fieldnames;  // all nodes (parent & children) point to the same address!!!
    vector<DataTypeStruct>* m_fieldtypes;     // all nodes (parent & children) point to the same address!!!
    DataTypeStruct* m_rawDatatype;

    //unordered_map< string,string > m_macroParaDefault; // the initial value of the user defined macro function parameters.
    ExpressionC* m_macroParaDefExpr; // if m_expType is MACROPARA, the default expression of the parameter.

    void dump(const int & deep) const;
    bool buildExpression();  // build expression class from the expression string
    void alignChildrenDataType(); // align children datatype with current datatype
    bool existLeafNode(ExpressionC* node); // check if exist leaf node
    ExpressionC* getTopParent(); // get the top parent node
    ExpressionC* BuildTree(string expStr, ExpressionC* newNode, ExpressionC* parentNode, bool isLeftChild); // build a BTree from an expression string
    bool buildLeafNode(string expStr, ExpressionC* node); // build a Leaf Node from an (atom) expression string
    bool isMacroInExpression();

  protected:
    void init();
};

#endif // __EXPRESSIONC_H

