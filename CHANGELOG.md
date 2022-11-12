# Change Log
All change logs will be documented here.<br/>
   # [1.1] 2022-11-12
   Enhancement: optimized runtime methods (evalExpression, runFunction) using a runtime data struct. <br/>
   # [1.03] 2022-11-11
   New feature: new function exec(expr_str) : Normal function. Run a system command and return the result. <br/>
   Bux fix: Variable field size may cause incorrect data type detected. <br/>
   Bux fix: "-" cannot be set as a delimiter. <br/>
   # [1.03] 2022-11-10
   Bux fix: option detecttyperows does not work as expected. <br/>
   # [1.02] 2022-11-10
   Enhancement: The dynamic variables will be calculated BEFORE filter (the filter can use the current calculated variables), it also keep the order of input. <br/>
   New feature: New functions: rcount([sideid][,fieldid][,value_expr]), rmember(sideid,fieldid,seqnum), rmemberid(sideid,fieldid,value_expr). <br/>
   # [1.01] 2022-11-07
   Enhancement: FOREACH can be used in the TRIM clause in the EXTRAFILTER. <br/>
   Enhancement: Extrafilter will use less memory. <br/>
   Fxied bug: Selection sequence number in SORT does not work properly; Aggregation function in output file expression is not evaled properly. <br/>
   # [1.01] 2022-11-03
   Enhancement: The main filter will not be applied to the dynamic variable @R, instead, it can define its own filter (like a sidework filter), if multiple variable @R defined, only the first defined filter works. <br/>
   # [1.0] 2022-11-02
   Enhancement: @R can be used in the user defined dynamic variable, it will be working as the first sidework datasets. <br/>
   # [0.993] 2022-11-02
   Enhancement: getpart will return empty string if the given index is out of range. <br/>
   Bux fix: when function does not work properly if the condition filter is not const. <br/>
   Bux fix: delimiter parsing cannot have r and s flags at the same time. <br/>
   Bux fix: Some functions (e.g. switch) does not work properly in dynamic variable. <br/>
   Enhancement: extra filer will use the selection data type to compare data. <br/>
   # [0.992] 2022-11-02
   Enhancement: Compare two ANY date type data as STRING. <br/>
   # [0.991] 2022-11-01
   New feature: new query option: report|r SelectionIndex1:AggregationOp1[,SelectionIndex2:AggregationOp2] -- Generate a summary of specified selections. <br/>
   New feature: new function sumall(expr1[,expr2...]) : Normal function. Sumarize the result of the input expressions, the parameter can be a foreach function. <br/>
   Enhancement: functions greatest and least can accept foreach as a parameter now. <br/>
   Enhancement: function round accept the second parameter as scale now. <br/>
   Enhancement: function replace can replace mutiple string pairs now. <br/>
   New feature: new function eval(expr_str) : Normal function. Eval the input string as an expression. <br/>
   Bug fix: Expression goes wrong if quotered 2 or more operator, e.g. (a*b-c)/d . <br/>
   # [0.99] 2022-10-31
   Fixed bug: Multiple FOREACH functions do not work properly. <br/>
   New feature: new function when(condition1,return1[,condition2,return2...],else): Normal function. if condition1 is fulfilled, then return return1, etc.. If none matched, return "else". <br/>
   New feature: new function getword(str,delimiter,part_index) : Normal function. Get a part of a string splitted by delimiter. <br/>
   Enhancement: hierarchy (tree) query can work with sort now. <br/>
   Enhancement: hierarchy (tree) query new variable @ISLEAF indicate if the node is a leaf. <br/>
   # [0.983] 2022-10-28
   New feature: new hierarchy variable @NODEID: an unique sequence id of the node of the tree. <br/>
   New feature: new function : root(expr) : Hierarchy function. Returns the expression of the root node.<br/>
   New feature: new function : parent(expr) : Hierarchy function. Returns the expression of the parent node.<br/>
   New feature: new function : path(expr[,connector]) : Hierarchy function. Returns an path (of the expression) from root to current node, connector is used for connecting nodes, default is '/'.<br/>
   New feature: new delimiter parsing flag : s : The leading&trail space will be reserved when the delmiter is space.. <br/>
   # [0.983] 2022-10-27
   Enhancement: Improved performance of hierarchy(tree) query. <br/>
   # [0.982] 2022-10-26
   Enhancement: extrafilter has a new clause: trim, to specify the selections after trimmed the result set. <br/>
   New feature: New query option tree|h k:expr1[,expr2...];p:expr1[,expr2...] : Provide keys and parent keys to construct tree stucture. tree cannot work with group/sort/unique. <br/>
   # [0.981] 2022-10-25
   New feature: New query option extrafilter|e <extra filter conditions> : Provide filter conditions to filt the resultset.<br/>
   New feature: New function appendFile(content, file) : Normal function. Append content to a file, return 1 if successed, 0 if failed.<br />
   # [0.981] 2022-10-23
   New feature: New function concat(str1,str2,[...]) : Normal function. Concatenate multiple strings. <br/>
   New feature: New function concatcol(start,end,expr[,step,delmiter]) : Normal function (Macro function implemented). Concatenate multiple field expressions. <br/>
   New feature: New function calcol(start,end,expr[,step,operation]) : Normal function (Macro function implemented). Caluclate multiple field expressions. <br/>
   # [0.98] 2022-10-23
   New feature: New function fieldname(fieldid) : Normal function. Return the filed name of a field (column). <br/>
   Fixed bug: Space didnot work as a field delmiter. <br/>
   # [0.98] 2022-10-23
   New feature: New function anycol(start,end,expr[,step]) : Macro function. Can be used in filter only, to check any field fulfil condition, e.g. anycol(1,%,$)>0. <br/>
   New feature: New function allcol(start,end,expr[,step]) : Macro function. Can be used in filter only, to check all field fulfil condition, e.g. allcol(1,%,$)>0. <br/>
   Enhancement: Function foreach has a new parameter: step, default is 1. <br/>
   # [0.98] 2022-10-21
   New feature: New function countstr(str,substr) : Normal function. count occurences of substr in str. <br/>
   # [0.973] 2022-10-21
   Enhancement: coltorow function works properly with foreach as a parameter when processing dynamic fields. <br/>
   # [0.973] 2022-10-20
   Enhancement: Dynamic variables can be set to be a global when processing multiple files. e.g. v1:0:@v1+1:g  <br/>
   # [0.972] 2022-10-20
   Enhancement: Dynamic variables will be reset for each file when processing multiple files. <br/>
   # [0.972] 2022-10-19
   New feature: New function comparenum(num1,num2) : Normal function. Compare two numbers; comparedate(date1,date2[,dateformat]) : Normal function. Compare two dates. <br/>
   # [0.971] 2022-10-19
   Enhancement: Dynamic variables can quote other variables defined previously, e.g. v1:0:tolong(@v1)+tolong(switch(@raw,'#####',1,0));v2:1:@v2+@line;v3:0:tolong(@v1)+tolong(@v2). <br/>
   Fixed bug: Expression priority is wrong when multiple subtract operations involved. <br/>
   Fixed bug: Output file append mode does not work properly. <br/>
   # [0.97] 2022-10-18
   New feature: Variable can be a dynamic variable now (not supported in aggregation and analytic functions yet). e.g. v1:1:@v1+1, @v1 has an initial value 0, it will be plused one for each matched row. <br/>
   # [0.97] 2022-10-13
   Enhancement: New parameter added to trim/trimleft/trimright to indicate should trim all repeated characters or only trim once. <br/>
   # [0.962] 2022-10-13
   New feature: New function: todate(str[,dateformat]) : Normal function. Conver any string to date. <br/>
   Fixed bug: Variable @R does not get correct data type. <br/>
   Fixed bug: Expression priority is wrong when subtract is calculated before plus. <br/>
   # [0.963] 2022-10-12
   Enhancement: "\'" will be escaped to "'", "\t" will be escaped to tab, "\n" will be escaped to newline, "\r" will be escaped to return, in string expressions. <br/>
   # [0.962] 2022-09-02
   Fixed bug: field sequence id of sidework(meanwhile) query referred by variable @R is incorrect. <br/>
   # [0.962] 2022-09-01
   New feature: new option: --delimiter | -i <string> : Specify the delimiter of the fields, TAB will be adapted if this option is not provided. <br/>
   # [0.961] 2022-08-30
   New feature: New function: coltorow(exp1[,exp2 ... ] ) : Macro function. Make the columns to rows. Accept multiple parameter, also accept foreach(). The row number will be the maximum number of parameter of all coltorow functions. <br/>
   # [0.96] 2022-08-28
   Enhancement: The flag options (fieldheader/progress/nameline/recursive) dont requir a following value any more, it will turn on the option by default. <br/>
   # [0.96] 2022-08-27
   New feature: '>' '>>' indicate output files. <br/>
   # [0.953] 2022-08-26
   Fixed bug: User defined date format cannot be recognized if special character quoted timezone. <br/>
   Enhancement: '\t' will be treated as TAB, '\/' will be treated as '/' in wildcard/delimiter parsing pattern. <br/>
   Fixed bug: Sorting gets incorrect order in some situations. <br/>
   Fixed bug: Analytic function doesnot work properly with group. <br/>
   Fixed bug: Nearby doesnot get the correct order sequence. <br/>
   # [0.953] 2022-08-25
   Fixed bug: Expanded selections from foreach marco function might be set in an incorrect position in some situations. <br/>
   # [0.952] 2022-08-25
   Enhancement: foreach() marco function can invole % in an express, e.g. %-2 <br/>
   New feature: New variable: @N : The last field, it can be an expression, e.g. @(N-3) .<br/>
   # [0.951] 2022-08-25
   Fixed bug: Operator priority is not correct in some expressions. <br/>
   # [0.95] 2022-08-24
   New feature: New function : toint(str) : Normal function. Conver a string to integer. <br/>
   New feature: New function : tolong(str) : Normal function. Conver a string to long. <br/>
   New feature: New function : tofloat(str) : Normal function. Conver a string to float. <br/>
   New feature: New function : tostr(expr) : Normal function. Conver any data type to string. <br/>
   New feature: New function : dectohex(num) : Normal function. Conver a decimal number to hex formatted string. <br/>
   New feature: New function : hextodec(str) : Normal function. Conver a hex number to decimal number. <br/>
   New feature: New function : dectobin(num) : Normal function. Conver a decimal number to binary formatted string. <br/>
   New feature: New function : bintodec(str) : Normal function. Conver a binary number to decimal number. <br/>
   New feature: New function : addtime(date, number, unit) : Normal function. Increase a datetime, unit can be s-second(default),m-minute,h-hour,d-day,n-month,y-year, number can be positive or negative interger. <br/>
   Fixed bug: datatype() doesnot return correct data type in some situations. <br/>
   Fixed bug: Analytic function is ignored in the filter in some situations. <br/>
   # [0.943] 2022-08-23
   New feature: New function : mod(num,div) : Normal function. Get the mod of a number; abs(num) : Normal function. Get the abs value of a given float number <br/>
   New feature: New function : ascii(char) : Normal function. Get the ascii code of a char; char(int) : Normal function. Get character of an ascii code number <br/>
   # [0.942] 2022-08-22
   New feature: New function : findnth(str,sub,Nth) : Normal function. Find the position of Nth sub in str, if Nth is positive number, search from head, if Nth is negative, search from tail. <br/>
   New feature: New function : revertstr(str) : Normal function. Convert a string to reverse sequence (e.g. abc -> cba). <br/>
   Fixed bug: Failed to read content from pipeline. <br/>
   # [0.942] 2022-08-21
   Enhancement: Main query result set can be used as a side work data set to do filtering. <br/>
   Fixed bug: Intermedium side work data set doesnot get other side work data set correctly. <br/>
   # [0.941] 2022-08-20
   Enhancement: Side work data set id starting from 1, to align with other sequence numbers. <br/>
   # [0.941] 2022-08-19
   New feature: Accept wildcard in the file name. <br/>
   # [0.94] 2022-08-18
   New feature: Join query two or more files using new query command "meanwhile". <br/>
   # [0.94] 2022-08-16
   New feature: new variable @FILELINE stands for line number of current file; new variable @FILEID stands for sequence number of current file.<br />
   New feature: Accept multiple files<br />
   Fixed bug: Analytic function may cause crash if data contain variable number fileds in each rows.<br />
   # [0.933] 2022-08-16
   New feature: New analytic functions: counta([group1,group2...];expr), uniquecounta([group1,group2...];expr), suma([group1,group2...];expr), averagea([group1,group2...];expr), maxa([group1,group2...];expr), mina([group1,group2...];expr)<br />
   # [0.933] 2022-08-15
   Enhancement: Analytic function and aggregation function can be compared in the filter.<br />
   Fixed bug: Analytic function doesnt work with aggregation function after aggregation function optimized.<br />
   # [0.932] 2022-08-15
   Enhancement: Optimized aggregation function implement.<br />
   New feature: New function: Grouplist([distinct ]expr[,delimiter][,asc|desc]) : Aggregation function. Combine the specific expr in a group to a string. distinct is a key word to indicate if the elements should be distinct, delimiter is a string to be the delimiter, asc|desc keywords indicate whether do sorting<br />
   # [0.932] 2022-08-13
   Fixed bug: Messy code when reading large files.<br />
   # [0.932] 2022-08-12
   Enhancement: Improved performance of uniquecount()<br />
   New feature: New function: camelstr(str) : Normal function. Convert a string to camel string (First letter of each word is upper case).<br />
   New feature: New function: snakestr(str) : Normal function. Convert a string to snake string (First letter of each sentence is upper case).<br />
   # [0.931] 2022-08-12
   Fixed bug: sorting large data may crashed when date type involved<br />
   Enhancement: Optimized memory usage and solved potential memory leak issue<br />
   # [0.931] 2022-08-10
   Enhancement: Make the data structure of analytic function more optimized, saved memory. <br/>
   New feature: new analytic function: Nearby(expr;[sort1 [asc|desc][,sort2 [asc|desc]];distance;default...]) : Analytic function. Get the value of nearby rows, if distance is negative, it returns value of previous N row, if distance is positive, it returns value of next N row.<br />
   # [0.931] 2022-08-09
   Fixed bug: analytic function doesnot get correct result if it is involved in an expression<br />
   New feature: new analytic function: denserank([group1[;group2]...],[sort1 [asc|desc][;sort2 [asc|desc]]...]) : Analytic function. The the dense rank of a sorted expression in a group.<br />
   # [0.93] 2022-08-08
   New feature: new analytic function: Rank([group1[;group2]...],[sort1 [asc|desc][;sort2 [asc|desc]]...]) : Analytic function. The the rank of a sorted expression in a group.<br />
   # [0.93] 2022-08-07
   Enhancement: Slightly improve performance when no matching pattern(parse) provided! <br/>
   # [0.923] 2022-08-07
   Enhancement: Sorting performance is improved a lot! <br/>
   # [0.923] 2022-08-04
   Enhancement: Delimiter pattern now parse end of line as an empty field if the line is ended with a delimiter <br/>
   # [0.922] 2022-08-04
   Fixed bug: Quoters is not escaped in string.<br/>
   Fixed bug: Quoters failed to be evaled in an expression.<br/>
   Fixed bug: Filed data type is not set correct when using @N abbrevation.<br/>
   New feature: new function: trimleft(str[,char]) -- Trim all char from left of the string, if char is not provided, all space (including tab) will be trimmed.<br/>
   New feature: new function: trimright(str[,char]) -- Trim all char from right of the string, if char is not provided, all space (including tab) will be trimmed.<br/>
   New feature: new function: trim(str[,char]) -- Trim all char from the string, if char is not provided, all space (including tab) will be trimmed.<br/>
   New feature: new function: datatype(expr) -- Return the date type of the expression.<br/>
   Enhancement: scientific notation number, e.g.1.58e+8 can be recognized.<br/>
   # [0.921] 2022-08-03
   Fixed bug: wildcard/delmiter searching might cause infinite loop in very rare scenarios<br/>
   New feature: new variable @% for the number of fields.
   # [0.92] 2022-08-03
   New feature: new function: countword(str,[ingnore_quoters]) -- Get the number of word in a string. Any substring separated by space/tab/newline/punctuation marks will be count as a word. if ingnore_quoters (in pairs, e.g. ''"") provided, the whole string between quoters will be counted as one word<br/>
   New feature: new function: getword(str,wordnum,[ingnore_quoters]) -- Get a word specified sequence number in a string. Any substring separated by space/tab/newline/punctuation marks will be count as a word. if ingnore_quoters (in pairs, e.g. ''"") provided, the whole string between quoters will be counted as one word<br/>
   New feature: new function: random([min,][max]) -- Generate a random integer. If no parameter provided, the range is from 1 to 100. Providing one parameter means rang from 1 to max.<br/>
   New feature: new function: randstr(len,flags) -- Generate a random string. len: string length (default 8); flags (default uld) includes: u:upper alphabet;l:lower alphabet;d:digit;m:minus;n:unlderline;s:space;x:special(\`~!@#$%^&\*+/\|;:'"?/);b:Brackets([](){}<>); A lower flag stands for optional choice, a upper flag stands for compulsory choice<br/>
   Fixed bug: wildcard/delmiter searching might read multiple lines in an once.<br/>
   Fixed bug: wildcard/delmiter searching might skip empty lines.<br/>
   # [0.913] 2022-08-02
   New feature: @N can be used for abbrevation of @fieldN<br/>
   New option: --nameline | -n Yes/no : Specify the first line should be used for filed names (useful for csv files). Default is no.<br/>
   Enhancement: Variable number of field can be searched & matched now.<br/>
   Fixed bug: User defined variable is not set correctly.<br/>
   Fixed bug: wildcard/delmiter searching might cause infinite loop in very rare scenarios<br/>
   # [0.913] 2022-08-01
   Fixed bug: sorting doesnot work properly when multiple sort keys involved<br/>
   Enhancement: // no long be used for regular expression string, {} no long be used for date string, they all use '' now.<br/>
   New feature: New searching pattern: w/<WildCardExpr>/, wildcard '\*' stands for a field, e.g. w/\*abc\*,\*/<br/>
   New feature: New searching pattern: d/<Delmiter>/[quoters/][r], Delmiter splits fields, delmiter between quoters will be skipped, r at the end of pattern means the delmiter is repeatable, e.g. d/ /""/<br/>
   # [0.912] 2022-08-01
   Enhancement: Query commands accept one letter abbrivation, e.g. s stands for 'select'<br/>
   New feature: Macro function foreach(beginid,endid,macro_expr) is introduced.<br/>
   # [0.912] 2022-07-31
   Added Version Info<br/>
   Added workflow actions to create CentOS7/8&MacOS rpm and zip files.<br/>
   Enhancement: query option is optional now; Set select @raw when no "select" provided.<br/>
   # [0.912] 2022-07-29
   New feature: New option: -c|--recursive <yes|no> -- Wheather recursively read subfolder of a folder (default NO).<br/>
   # [0.911] 2022-07-29
   New feature: New option: -d | --detecttyperows How many matched rows will be used for detecting data types, default is 1<br/>
   Added examples.md<br/>
   Fixed bug: Incorrectly detected data type for @RAW<br/>
   Fixed bug: Some DATE data can not be detected the correct data type and date format<br/>
   Fixed bug: Functions without paramter dont work properly.<br/>
   # [0.911] 2022-07-28
   Fixed bug: LIKE missed quoters<br/>
   Fixed bug: SET field data type does not set the correct date format for DATE type.<br/>
   Fixed bug: UNIQUE is ignored in some scenarios<br/>
   # [0.91] 2022-07-27
   New feature: UNIQUE searching command to make the returned result unique.<br/>
   New console command: clear: clear all inputs<br/>
   # [0.91] 2022-07-26
   New feature: Regmatch returns an expression including the capturing groups matched a regular pattern.<br/>
   Fixed bug: the escaped quoter is missing in a string<br/>
   Fixed bug: filter incorrectly read quoters in the string<br/>
   # [0.903] 2022-07-25
   Fixed Bug: incorrect timezone converting<br/>
   Improved date cast performance<br/>
   # [0.902] 2022-07-19
   New feature: Using -v or --variable to pass variables in<br/>
   Fixed bug: Json format becomes messy when limt N,M involved<br/>
   Fixed bug: Selected row number is incorrect when limt N,M involved<br/>
   Fixed compile warnings<br/>
   # [0.901] 2022-07-18
   Fixed compile warnings
   New feature: added switch,pad,greatest,least functions <br/>
   # [0.90] 2022-07-18
   New feature: added isnull(arg) function <br/>
   # [0.893] 2022-07-18
   New feature: Select can have a alias name. Syntax: select <expression> as <alias> ...<br/>
   # [0.892] 2022-07-17
   Fix defect: Unable to handle aggregation function (only) without group field<br/>
   # [0.892] 2022-07-15
   Hot fix: replace bug; nested function calls<br/>
   Hot fix: infinite loop<br/>
   # [0.891] 2022-07-15
   Fix bug: quoters mismatched; Added Regreplace function<br/>
   Fix bug: failed to do reglike comparing<br/>
   # [0.89] 2022-07-14
   Json output format<br/>
   Added log file option; added progress option<br/>
   Huge performance improvement! Reduced the temp data set size of grouping.<br/>
   Implement Hash Map<br/>
   # [0.88] 2022-07-11
   File process bug fix<br/>
   Console bug fix<br/>
   Added Console mode<br/>
   # [0.01] 2022-07-08
   Project initializing.<br/>
