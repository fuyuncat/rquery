/*******************************************************************************
//
//        File: commfuncs.h
// Description: common functions
//       Usage: commfuncs.h
//     Created: 18/06/2022
//      Author: Wei Huang
//       Email: fuyuncat@gmail.com
//
//------------- Change History ------------- 
//  18/06/2022: Created
//
*******************************************************************************/

#include <regex.h>
#include <vector>
#include <string>

using namespace std;

vector<string> split(string str, char delim = ' ', char quoter = '\"', char escape = '\\');
