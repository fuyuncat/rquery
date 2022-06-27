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
    QuerierC(string regexstr);
    ~QuerierC();

    void setregexp(string regexstr);
    void setrawstr(string rawstr);
    int searchNext();
    int searchAll();
    void output();
    void outputAndClean();
    int boostmatch( vector<string> *result = NULL);
    int boostmatch( map<string,string> & result);

  private:
    string m_regexstr;
    sregex m_regexp;
    string m_rawstr;
    regex_constants::match_flag_type m_searchflags;
    
    //vector<namesaving_smatch> m_results;
    vector< vector<string> > m_results;

  protected:
    void init();
    void formatoutput(namesaving_smatch matches);
};

#endif // __QUERIERC_H

