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

#include "commfuncs.h"

class FilterC;

class FilterC
{
  public:

    FilterC();
    FilterC(FilterC* node);
    FilterC(int junction, FilterC* leftNode, FilterC* rightNode); // construct a branch
    FilterC(int comparator, int colId, string data); // construct a leaf

    ~FilterC();

    int type;       // 1: branch; 2: leaf
    int junction;   // if type is BRANCH, 1: and; 2: or. Otherwise, it's meaningless
    int comparator; // if type is LEAF, 1: ==; 2: >; 3: <; 4: !=; 5: >=; 6: <=; 7: LIKE; 8: REGLIKE. Otherwise, it's meaningless
    int leftColId;              // if type is LEAF, it's id of column on the left to be predicted. Otherwise, it's meaningless
    int rightColId;             // if type is LEAF, it's id of column on the right to be predicted. Otherwise, it's meaningless
    string leftExpression;    // if type is LEAF, it's id of column to be predicted. Otherwise, it's meaningless
    string rightExpression;   // if type is LEAF, it's data to be predicted. Otherwise, it's meaningless
    FilterC* leftNode;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
    FilterC* rightNode;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
    FilterC* parentNode;    // for all types except the root, it links to parent node. Otherwise, it's meaningless

    int getLeftHeight(); // get left tree Height
    int getRightHeight(); // get left tree Height
    void add(FilterC* node, int junction, bool leafGrowth, bool addOnTop); // add a NEW preiction into tree
    void dump();
    bool containsColId(int colId); // detect if predication contains special colId
    FilterC* getFirstPredByColId(int colId, bool leftFirst); // detect if predication contains special colId
    bool analyzeColumns(vector<string> m_fieldnames1, vector<string> m_fieldnames2); // analyze column ID & name from metadata
    bool columnsAnalyzed();
    FilterC* cloneMe();
    void copyTo(FilterC* node);
    std::set<int>  getAllColIDs(int side); // get all involved colIDs in this prediction
    map<int,string>  buildMap(); // build the prediction as a HashMap
    bool compareExpression(); // calculate an expression prediction
    int size(); // get all involved colIDs in this prediction
    void clear(); // clear predictin
    bool remove(FilterC* node); // remove a node from prediction. Note: the input node is the address of the node contains in current prediction
    void fillDataForColumns(map <string, string> & dataList, vector <string> columns); // build a data list for a set of column, keeping same sequence, fill the absent column with NULL

  private:
    bool metaDataAnzlyzed; // analyze column name to column id.
    
    void dump(int deep);

  protected:
    void init();
};

#endif // __FILTERC_H

