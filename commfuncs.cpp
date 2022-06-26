/*******************************************************************************
//
//        File: commfuncs.cpp
// Description: common functions
//       Usage: commfuncs.cpp
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/
#include <stdio.h>
#include "commfuncs.h"

namesaving_smatch::namesaving_smatch(const string pattern)
{
    string pattern_str = pattern;
    sregex capture_pattern = sregex::compile("\\?P?<(\\w+)>");
    sregex_iterator words_begin = sregex_iterator(pattern_str.begin(), pattern_str.end(), capture_pattern);
    sregex_iterator words_end = sregex_iterator();

    for (sregex_iterator i = words_begin; i != words_end; i++)
    {
        string name = (*i)[1].str();
        m_names.push_back(name);
    }
}

namesaving_smatch::~namesaving_smatch() { }

vector<string>::const_iterator namesaving_smatch::names_begin() const
{
    return m_names.begin();
}

vector<string>::const_iterator namesaving_smatch::names_end() const
{
    return m_names.end();
}

vector<string> namesaving_smatch::m_names;


vector<string> split(string str, char delim, char quoter, char escape) 
{
  vector<string> v;
  size_t i = 0, j = 0, begin = 0;
  bool quoted = false;
  while(i < str.size()) {
    if (str[i] == delim && i>0 && !quoted) {
      printf("found delim\n");
      v.push_back(string(str, begin, i));
      begin = i+1;
    }
    if (str[i] == quoter) {
      printf("found quoter\n");
      if (i>0 && str[i-1]!=escape)
        quoted = !quoted;
    }
    ++i;
  }
  if (begin<str.size())
    v.push_back(string(str, begin, str.size()));

  /*while(i < str.size()) {
    if(str[i] == delim || i == 0) {
      if(i + 1 < str.size() && str[i + 1] == escape) {
        j = begin + 1;
        while(j < str.size() && str[j++] != escape);
          v.push_back(string(str, begin, j - 1 - i));
        begin = j - 1;
        i = j - 1;
        continue;
      }

      j = begin + 1;
      while(j < str.size() && str[j++] != delim);
      v.push_back(string(str, begin, j - 1 - i - (i ? 1 : 0) ));
      begin = j;
    }
    ++i;
  }*/

  return v;
}
