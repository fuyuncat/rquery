# Change Log
All change logs will be documented here.<br/>
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
