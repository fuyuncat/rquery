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

    map<string,string> parseparam(string parameterstr);

  private:
    map<string,string> m_queryparts;
    string m_filterString;
    FilterC prediction;
    int analyzedPos;

    const static vector<string> junctionWords;
    const static vector<string> junctionSplitors;
    const static vector<string> comparators; // ">=", "<=" should be before ">", "<"


  protected:
    void init();
    // map<string,string> parsequery(string raw);
};

#endif // __PARSERC_H

