/*******************************************************************************
//
//        File: parser.h
// Description: Parser class header
//       Usage: parser.h
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/

#ifndef __PARSERC_H
#define __PARSERC_H

#include <vector>
#include <string>
#include "commfuncs.h"
#include "filter.h"

class ParserC
{
  public:

    ParserC();
    ~ParserC();

    bool isJunctionWord(string word);
    FilterC* buildFilter(string initialString);

    map<string,string> parseparam(string parameterstr);
    void dumpQueryparts();

  private:
    map<string,string> m_queryparts;
    //string m_filterStr;
    int analyzedPos;

    static vector<string> junctionWords;
    static vector<string> junctionSplitors;
    static vector<string> comparators; // ">=", "<=" should be before ">", "<"

    void buildLeafNodeFromStr(FilterC* node, string str);
    bool buildFilter(FilterC* node, string initialString, string splitor, string quoters = "()"); // split input command line into pieces; \ is escape char, " and splitor could be escaped.

  protected:
    void init();
    // map<string,string> parsequery(string raw);
};

#endif // __PARSERC_H

