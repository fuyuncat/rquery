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

#include <vector>
#include <string>
#include "commfuncs.h"

class FilterC;

class FilterC
{
  public:

    FilterC();
    FilterC(FilterC* node);
    Prediction(int junction, FilterC* leftNode, FilterC* rightNode); // construct a branch
    Prediction(int comparator, int colId, string data); // construct a leaf

    ~FilterC();

    int type;       // 1: branch; 2: leaf
    int junction;   // if type is BRANCH, 1: and; 2: or. Otherwise, it's meaningless
    int comparator; // if type is LEAF, 1: ==; 2: >; 3: <; 4: !=; 5: >=; 6: <=. Otherwise, it's meaningless
    int leftColId;              // if type is LEAF, it's id of column on the left to be predicted. Otherwise, it's meaningless
    int rightColId;             // if type is LEAF, it's id of column on the right to be predicted. Otherwise, it's meaningless
    string leftExpression;    // if type is LEAF, it's id of column to be predicted. Otherwise, it's meaningless
    string rightExpression;   // if type is LEAF, it's data to be predicted. Otherwise, it's meaningless
    FilterC* leftNode;      // if type is BRANCH, it links to left child node. Otherwise, it's meaningless
    FilterC* rightNode;     // if type is BRANCH, it links to right child node. Otherwise, it's meaningless
    FilterC* parentNode;    // for all types except the root, it links to parent node. Otherwise, it's meaningless

  private:
    bool metaDataAnzlyzed; // analyze column name to column id.

  protected:
    void init();
    map<string,string> parsequery(string raw);
};

#endif // __FILTERC_H

