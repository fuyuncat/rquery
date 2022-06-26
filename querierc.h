/*******************************************************************************
//
//        File: querierc.h
// Description: Querier class header
//       Usage: querierc.h
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/

#ifndef __QUERIERC_H
#define __QUERIERC_H

#include <regex.h>
#include <vector>
#include <string>
//#include <boost/regex.hpp>

using namespace std;

class QuerierC
{
  public:

    QuerierC();
    ~QuerierC();

    int boostmatch( string regexp, string str, vector<string> *result = NULL);
    int boostmatch(const string regexp, const string str, map<string,string> & result);

  protected:

    void init();
};

#endif // __QUERIERC_H

