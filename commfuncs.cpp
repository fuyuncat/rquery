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

#include "commfuncs.h"

vector<string> split(string str, char delim, char quoter, char escape) 
{
  vector<string> v;
  size_t i = 0, j = 0, begin = 0;
  bool quoted = false;
  while(i < str.size()) {
    if (str[i] == delim && i>0 && !quoted) {
      printf("found delim\n");
      v.push_back(std::string(str, begin, i));
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
    v.push_back(std::string(str, begin, str.size()));

  /*while(i < str.size()) {
    if(str[i] == delim || i == 0) {
      if(i + 1 < str.size() && str[i + 1] == escape) {
        j = begin + 1;
        while(j < str.size() && str[j++] != escape);
          v.push_back(std::string(str, begin, j - 1 - i));
        begin = j - 1;
        i = j - 1;
        continue;
      }

      j = begin + 1;
      while(j < str.size() && str[j++] != delim);
      v.push_back(std::string(str, begin, j - 1 - i - (i ? 1 : 0) ));
      begin = j;
    }
    ++i;
  }*/

  return v;
}
