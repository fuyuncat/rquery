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
#include "commfuncs.h"
//#include <boost/regex.hpp>

using namespace std;

class QuerierC
{
  public:

    QuerierC();
    QuerierC(string regexp);
    ~QuerierC();

    void setregexp(string regexp);
    void setrawstr(string rawstr);
    int searchNext();
    void output();
    void outputAndClean();
    int boostmatch( vector<string> *result = NULL);
    int boostmatch( map<string,string> & result);

  private:
    string m_regexp;
    sregex m_rexp;
    string m_rawstr;
    regex_constants::match_flag_type m_searchflags;
    
    vector<namesaving_smatch> m_results;

  protected:
    void init();
};

#endif // __QUERIERC_H

