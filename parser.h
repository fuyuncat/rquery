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

class ParserC
{
  public:

    ParserC();
    ~ParserC();

    map<string,string> parseparam(string parameterstr);
  private:

  protected:
    void init();
    map<string,string> parsequery(string raw);
};

#endif // __PARSERC_H

