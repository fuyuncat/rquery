# rquery
&nbsp;&nbsp;&nbsp;RQuery is a tool to search text content/file/folder using regular/delimiter/wildcard expression parttern, and filter/group calculate/sort the matched result. One command can do what grep/xgrep/sort/uniq/awk/wc/sed/cut/tr can do and more. <br />
&nbsp;&nbsp;&nbsp;Contact Email: fuyuncat@gmail.com <br/>
&nbsp;&nbsp;&nbsp;Visit our wiki page to get the help document: https://github.com/fuyuncat/rquery/wiki <br/>
<br />
# Install
The latest version can be downloaded here: https://github.com/fuyuncat/rquery/releases <br />
- Install method 1<br />
&nbsp;&nbsp;&nbsp; download the rpm file <br />
&nbsp;&nbsp;&nbsp; sudo rpm -Uhv <downloaded_rpm_file> <br />

- Install method 2<br />
&nbsp;&nbsp;&nbsp; download the compiled zip file <br />
&nbsp;&nbsp;&nbsp; unzip the compiled zip file <br />
&nbsp;&nbsp;&nbsp; chmod ugo+x rq<br />
&nbsp;&nbsp;&nbsp; cp rq /usr/bin/<br />
&nbsp;&nbsp;&nbsp; or directly run it from local.<br />

- Install method 3<br />
&nbsp;&nbsp;&nbsp; Download source code zip<br />
&nbsp;&nbsp;&nbsp; Unzip the downloaded file<br />
&nbsp;&nbsp;&nbsp; sudo yum -y install boost-devel<br />
&nbsp;&nbsp;&nbsp; make<br />
&nbsp;&nbsp;&nbsp; make install<br />

# Usage & Help Doc
- run modes
   - Command line mode:<br />
      - ./rq [options] "Text to be parsed and queried"<br />
      - echo "Text to be parsed and queried" | ./rq [parameters]<br />
      - ./rq [options] < file<br />
      - ./rq [options] file<br />
      - ./rq [options] folder<br />
   - Console mode:
      - ./rq [options] --console<br />
   &nbsp;&nbsp;&nbsp;Input any of below commands in the console mode.<br />
         - load file|folder : load a file or a folder<br />
         - filemode buffer|line : Provide file read mode, default is buffer.<br />
         - skip <N> : How many bytes or lines (depends on the filemode) to be skipped.<br />
         - detecttyperows <N> How many matched rows will be used for detecting data types, default is 1 <br />
         - parse /parsing string/ : Choose one of three mode to match the content.<br />
            - // quotes a regular expression pattern string to parse the content; <br />
            - w/<WildCardExpr>/[quoters/] quotes wildcard expression to parse the content, wildcard '\*' stands for a field, e.g. w/\*abc\*,\*/. substrings between two \* are the spliters, spliter between quoters will be skipped; <br />
            - d/<Delmiter>/[quoters/][r][s] quotes delmiter to parse the content, Delmiter splits fields, delmiter between quoters will be skipped, flag r means the delmiter is repeatable; flag s means the leading&trail space will be reserved when the delmiter is space. e.g. d/ /""/ <br />
         - set <field datatype [date format],...> : Set field data type. Supported data types: LONG, INTEGER, DOUBLE, STRING, DATE.<br />
         - filter <filter conditions> : Filter the parsed records<br />
         - extrafilter <extra filter conditions> : Provide filter conditions to filt the resultset. @N refer to Nth selection of the result set.<br />
         - select <field or expression,...> : Fields or expressions to be selected<br />
         - meanwhile <actions when searching data> -- Provide actions to be done while doing searching. The result set can be used for two or more files JOIN or IN query.<br />
         - group <field or expression,...> : Fields or expressions to be groupd for aggregation selections<br />
         - sort <field or expression [asc|desc],...> : Sorting keys to decide order of the output records<br />
         - limt <n | bottomN,topN> : Limited number records to be printed<br />
         - unique : Make the returned resutl unique.<br />
         - tree k:expr1[,expr2...];p:expr1[,expr2...] : Provide keys and parent keys to construct tree stucture. tree cannot work with group/sort/unique. variable @level stands for the level of the node in the tree; @nodeid for an unique sequence id of the node of the tree. <br />
         - report SelectionIndex1:AggregationOp1[,SelectionIndex2:AggregationOp2] -- Generate a summary of specified selections. <br />
         - -v|--variable \"name1:value1[:expression1[:g]][;name2:value2[:expression2[:g]]..]\" -- Pass variable to rquery, variable name can be any valid word except the reserved words, RAW,FILE,ROW,LINE,FILELINE,FILEID. Using @name to refer to the variable. variable can be a dynamic variable if an expression passed, e.g. v1:1:@v1+1, @v1 has an initial value 0, it will be plused one for each matched row. Dynamic variable is not supported in the aggregation & analytic functions in current version. 'g' flag of a dynamic variable indecate it is a global variable when processing multiple files<br />
         - run [query] : Run the preprocessed query of a provided query (refering to below part)<br />
- options
   - --help|-h<br />
   - --fieldheader | -f on|off : Wheather print fields header or not(default).<br />
   - --progress | -p <on|off> -- Wheather show the processing progress or not(default).<br />
   - --outputformat | -o <text|json> -- Provide output format, default is text.<br />
   - --readmode | -r buffer|line : File read mode, buffer(default) or line.<br />
   - --buffsize | -b size : The buffer size when read mode is buffer, default is 16384.<br />
   - --skip | -s number : How many lines or bytes to be skipped before start to parse the text content, default is 0.<br />
   - --variable | -v "name1:value1[;name2:value2..]|name1:initvalue1:expression1[;name2:initvalue2:expression2...]|r:expression1[:filter][;r:expression2...]" -- Pass variable to rquery, variable name can be any valid word except the reserved words, RAW,FILE,ROW,LINE. Using @name to refer to the variable.<br />
   - --detecttyperows | -d <N> : How many matched rows will be used for detecting data types, default is 1.<br />
   - --delimiter | -i <string> : Specify the delimiter of the fields, TAB will be adapted if this option is not provided <br/>
   - --msglevel | -m level : The output message level, could be INFO, WARNING, ERROR, FATAL, default is FATAL.<br />
   - --nameline | -n Yes/no : Specify the first line should be used for filed names (useful for csv files). Default is no.<br />
   - --recursive | -c <yes|no> -- Wheather recursively read subfolder of a folder (default NO).<br />
   - --query | -q <qeury string> : The query string to be used to parse and query the text content.<br />
- Syntax of query string:
   - parse /parsing string/|set field datatype [date format],...|filter <ilter conditions|select field or expression,...|group field or expression,...|sort field or expression [asc|desc],...|limt n | bottomN,topN<br />
 The query parts are separated by |, they could be put in any order. You can provide one or more of them in a query, none of them is mandatary<br />
      - parse|p /parsing string/ : Choose one of three mode to match the content.<br />
         - // quotes a regular expression pattern string to parse the content; <br />
         - w/<WildCardExpr>/[quoters/] quotes wildcard expression to parse the content, wildcard '\*' stands for a field, e.g. w/\*abc\*,\*/. substrings between two \* are the spliters, spliter between quoters will be skipped; <br />
         - d/<Delmiter>/[quoters/][r][s] quotes delmiter to parse the content, Delmiter splits fields, delmiter between quoters will be skipped, flag r means the delmiter is repeatable; flag s means the leading&trail space will be reserved when the delmiter is space. e.g. d/ /""/ <br />
      - set|t field datatype [date format],... : Set field data type. Supported data types: LONG, INTEGER, DOUBLE, STRING, DATE.<br />
      - filter|f <filter conditions> : Filter the parsed records<br />
      - extrafilter|e <extra filter conditions> [ trim <selections ...>] : Provide filter conditions to filt the resultset. @N refer to Nth selection of the result set. the trim clause specifys the selections after trimmed the result set.<br />
      - meanwhile|m <actions when searching data> -- Provide actions to be done while doing searching. The result set can be used for two or more files JOIN or IN query.<br />
      - select|s field or expression,... : Fields or expressions to be selected<br />
      - group|g field or expression,... : Fields or expressions to be groupd for aggregation selections<br />
      - sort|o field or expression [asc|desc],... : Sorting keys to decide order of the output records<br />
      - limtl n | bottomN,topN : Limited number records to be printed<br />
      - unique|u : Make the returned resutl unique. <br />
      - tree|h k:expr1[,expr2...];p:expr1[,expr2...] : Provide keys and parent keys to construct tree stucture. tree cannot work with group/sort/unique. variable @level stands for the level of the node in the tree; @nodeid for an unique sequence id of the node of the tree; @ISLEAF indicate if the node is a leaf. <br />
      - report|r SelectionIndex1:AggregationOp1[,SelectionIndex2:AggregationOp2] -- Generate a summary of specified selections. <br />
      - \>|>> -- Set output files, if not set, output to standard terminal (screen). > will overwrite existing files, >> will append to existing file. <br />
- Variables:
In any expression of select, filter, group, sort, variables can be used. The variables are in a @Var format. Currently, the variables can be used are,<br />
   - @raw : The raw string of a parsed line<br />
   - @file : The file name<br />
   - @fileid : The file sequence number<br />
   - @line : The line sequence number of all matched lines<br />
   - @fileline : The line sequence number of current file matched lines<br />
   - @row : The sequence number of output records<br />
   - @fieldN : The field of a parsed line, N is the sequence number of the field. It matches to the Capturing Group in the regular expression.<br />
   - @R[side work id][filed name/id] : The referred side work data set queried by MEANWHILE command. "side work id" is the sequence id of the side work query in MEANWHILE command, starting from 1; "filed name/id" is the field name, alias or ID (starting from 1). Note: if R is defined in the user defined dynamic variable, the first dataset of @R will be the user defined variable values, the index of the dataset retrieved from MEANWHILE will start from 2.<br />
   - @% : Number of the fields.<br />
   - @N : The last field, it can be an expression, e.g. @(N-3) .<br />
   - @nodeid : An unique sequence id of the node of the tree in the hierarchical query.<br />
   - @isleaf : Indicate if the node is a leaf in the hierarchical query .<br />
   - @level : The level of the node in the tree in the hierarchical query .<br />
- Fields:<br />
Fields are the Capturing Group or Named Capturing Group in the regular expression. If it's a Named Capturing Group, the name can be used as the field name, or a variable @N or @fieldN can be mapped to a Capturing Group. <br />
- Functions:<br />
Functions can be used in the expression. We current provide some essential normal functions and aggregation functions.
   - upper(str) : Normal function. Convert a string to upper case.<br />
   - lower(str) : Normal function. Convert a string to lower case.<br />
   - strlen(str) : Normal function. Return the length of a string.<br />
   - isnull(expr) : Normal function. Return 1 if the size of the give parameter is 0, otherwise return 0<br />
   - instr(str,sub) : Normal function. Return the position of a sub string in a string. Return -1 if caannot find the sub string<br />
   - substr(str,pos,len) : Normal function. Get a substring of a string, start from pos. If len is not provide, get the sub string till the end of the string.<br />
   - replace(str,replace1,new1[,replace2,new2...]) : Normal function. Replace all replace strings in a string with new strings.<br />
   - regreplace(str,pattern,sub) : Normal function. Replace all regular pattern in a string with sub (capturing group supported).<br />
   - regmatch(str,pattern,sub) : Normal function. Return an expression including the capturing groups matched a regular pattern. Use {N} to stand for the matched groups<br />
   - regcount(str,pattern) : Normal function. Get the number of regular pattern matchs in a string.<br />
   - regget(str,pattern,idxnum) : Normal function. Get the regular pattern matched string with specific sequence number in a string. if idxnum is a negtive number, it will start searching from the end of the string.<br />
   - comparestr(str1,str2) : Normal function. Compare str1 to str2, case sensitive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br />
   - nocaseComparestr(str1,str2) : Normal function. Compare str1 to str2, case insensive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br />
   - countword(str,[ingnore_quoters]) : Normal function. Get the number of word in a string. Any substring separated by space/tab/newline/punctuation marks will be count as a word. if ingnore_quoters (in pairs, e.g. ''"") provided, the whole string between quoters will be counted as one word<br />
   - getword(str,wordnum,[ingnore_quoters]) : Normal function. Get a word specified sequence number in a string. Any substring separated by space/tab/newline/punctuation marks will be count as a word. if ingnore_quoters (in pairs, e.g. ''"") provided, the whole string between quoters will be counted as one word. if wordnum is a negtive number, it will start searching from the end of the string.<br />
   - getpart(str,delimiter,part_index) : Normal function. Get a part of a string splitted by delimiter. if part_index is a negtive number, it will start searching from the end of the string.<br />
   - countpart(str,delimiter) : Normal function. Get the number parts in a string splitted by delimiter. if part_index is a negtive number, it will start searching from the end of the string.<br />
   - countstr(str,substr) : Normal function. count occurences of substr in str.<br />
   - trimleft(str[,char][,repeat(1|0)]) : Normal function. Trim all char from left of the string, if char is not provided, all space (including tab) will be trimmed.<br />
   - trimright(str[,char][,repeat(1|0)]) : Normal function. Trim all char from right of the string, if char is not provided, all space (including tab) will be trimmed.<br/>
   - trim(str[,char][,repeat(1|0]) : Normal function. Trim all char from the string, if char is not provided, all space (including tab) will be trimmed.<br/>
   - concat(str1,str2,[...]) : Normal function. Concatenate multiple strings. <br/>
   - concatcol(start,end,expr[,step,delmiter]) : Normal function (Macro function implemented). Concatenate multiple field expressions. $ stands for GROUP expression when GROUP involved), # stands for field sequence, % stands for the largest field sequence ID, % can be involved in an expression.<br/>
   - calcol(start,end,expr[,step,operation]) : Normal function (Macro function implemented). Caluclate multiple field expressions. $ stands for GROUP expression when GROUP involved), # stands for field sequence, % stands for the largest field sequence ID, % can be involved in an expression. Operations can be SUM/AVERAGE/MAX/MIN/COUNT/UNIQUECOUNT.<br/>
   - sumall(expr1[,expr2...]) : Normal function. Sumarize the result of the input expressions, the parameter can be a foreach function. <br/>
   - rcount([sideid][,fieldid][,value_expr]) : Normal function. Return the size of the side work data set. sideid is the id the side work, fieldid is id the field in the side work, value_expr is the value of a member. If no parameter is provided, it will return the number of side works; if only sideid is provided, it will return the data set size of the specified side work; if only sideid and fieldid, it will return the data set size of the specified side work; if all three parameter are provided, it will return the number of specified member value in the specified field in the data work. <br/>
   - rmember(sideid,fieldid,seqnum) : Normal function. Return the member value of the specified side work, field id and sequence number. sideid is the id the side work, fieldid is id the field in the side work, seqnum is the member sequence number. if seqnum is a negtive number, it will start searching from the end of the array.<br/>
   - rmemberid(sideid,fieldid,value_expr) : Normal function. Return the sequence number of the first matched member value of the specified side work and field id. sideid is the id the side work, fieldid is id the field in the side work, value_expr is the value of a member. <br/>
   - comparenum(num1,num2) : Normal function. Compare two numbers, return -1 if num1 less than num2, return 0 if num1 equal to num2, return 1 if num1 greater than num2<br />
   - comparedate(date1,date2[,dateformat]) : Normal function. Compare two dates, return -1 if date1 less than date2, return 0 if date1 equal to date2, return 1 if date1 greater than date2. date1 and date2 should have the same dateformat.<br />
   - datatype(expr) : Normal function. Return the date type of the expression. If the expresion is a field or function, like @1, it will return the data type of the field/function.<br/>
   - detectdt(str) : Normal function. Detect the data type of a string.<br/>
   - islong(str) : Normal function. Check if a string can be a long value.<br/>
   - isDouble(str) : Normal function. Check if a string can be a double value.<br/>
   - isDate(str) : Normal function. Check if a string can be a date value.<br/>
   - isstring(str) : Normal function. Check if a string can be a string value.<br/>
   - toint(str) : Normal function. Conver a string to integer.<br/>
   - tolong(str) : Normal function. Conver a string to long.<br/>
   - tofloat(str) : Normal function. Conver a string to float.<br/>
   - tostr(expr) : Normal function. Conver any data type to string.<br/>
   - todate(str[,dateformat]) : Normal function. Conver any string to date.<br/>
   - dectohex(num) : Normal function. Conver a decimal number to hex formatted string.<br/>
   - hextodec(str) : Normal function. Conver a hex number to decimal number.<br/>
   - dectobin(num) : Normal function. Conver a decimal number to binary formatted string.<br/>
   - bintodec(str) : Normal function. Conver a binary number to decimal number.<br/>
   - pad(seed,len) : Normal function. Construct a new string from seed multiple len times.<br />
   - floor(num) : Normal function. Get the floor integer number of a given float number.<br />
   - ceil(num) : Normal function. Get the ceil integer number of a given float number.<br />
   - round(num,scale) : Normal function. Round a given float number.<br />
   - log(num) : Normal function. Get the log result of a given float number.<br />
   - mod(num,div) : Normal function. Get the mod of a number.<br />
   - abs(num) : Normal function. Get the abs value of a given float number.<br />
   - random([min,][max]) : Normal function. Generate a random integer. If no parameter provided, the range is from 1 to 100. Providing one parameter means rang from 1 to max.<br />
   - randstr(len,flags) : Normal function. Generate a random string. len: string length (default 8); flags (default uld) includes: u:upper alphabet;l:lower alphabet;d:digit;m:minus;n:unlderline;s:space;x:special(\`~!@#$%^&\*+/\|;:'"?/);b:Brackets([](){}<>); A lower flag stands for optional choice, a upper flag stands for compulsory choice. <br />
   - camelstr(str) : Normal function. Convert a string to camel string (First letter of each word is upper case).<br />
   - snakestr(str) : Normal function. Convert a string to snake string (First letter of each sentence is upper case).<br />
   - findnth(str,sub[,Nth]) : Normal function. Find the position of Nth sub in str, if Nth is positive number, search from head, if Nth is negative, search from tail.<br />
   - revertstr(str) : Normal function. Convert a string to reverse sequence (e.g. abc -> cba).<br />
   - fieldname(fieldid) : Normal function. Return the filed name of a field (column).<br />
   - ascii(char) : Normal function. Get the ascii code of a char.<br />
   - char(int) : Normal function. Get character of an ascii code number.<br />
   - dateformat(date) : Normal function. Convert a date data to a string with the given format.<br />
   - timediff(date1,date2) : Normal function. Get the difference (in seconds) of two date.<br />
   - addtime(date, number, unit) : Normal function. Increase a datetime, unit can be s-second(default),m-minute,h-hour,d-day,n-month,y-year, number can be positive or negative interger.<br />
   - now() : Normal function. Get current date time.<br />
   - truncdate(date,seconds) : Normal function. Truncate a date a number is multiple of the given second number.<br />
   - switch(input,case1,return1[,case2,return2...][,default]): Normal function. if input equal to case1, then return return1, etc.. If none matched, return default or return input if no default provided. Similar to SWITCH CASE statement.<br />
   - when(condition1,return1[,condition2,return2...],else): Normal function. if condition1 is fulfilled, then return return1, etc.. If none matched, return "else".<br />
   - greatest(expr1[,expr2,...]) : Normal function. Return the largest one of the given expressions. The expression can be a foreach function.<br />
   - least(expr1[,expr2,...]) : Normal function. Return the smallest one of the given expressions. The expression can be a foreach function.<br />
   - appendFile(content, file) : Normal function. Append content to a file, return 1 if successed, 0 if failed.<br />
   - eval(expr_str) : Normal function. Eval the input string as an expression.<br />
   - exec(expr_str) : Normal function. Run a system command and return the result.<br />
   - Count(expr) : Aggregation function. Count the number of expr.<br />
   - Uniquecount(expr) : Aggregation function. Count the number of distinct expr.<br />
   - Sum(expr) : Aggregation function. Sum the number of expr.<br />
   - Max(expr) : Aggregation function. Get the maximum value of expr.<br />
   - Min(expr) : Aggregation function. Get the minimum value of expr.<br />
   - Average(expr) : Aggregation function. Get the average value of expr.<br />
   - Grouplist([distinct ]expr[,delimiter][,asc|desc]) : Aggregation function. Combine the specific expr in a group to a string. distinct is a key word to indicate if the elements should be distinct, delimiter is a string to be the delimiter, asc|desc keywords indicate whether do sorting.<br />
   - Rank([group1[,group2]...];[sort1 [asc|desc][,sort2 [asc|desc]]...]) : Analytic function. The the rank of a sorted expression in a group.<br />
   - Denserank([group1[,group2]...];[sort1 [asc|desc][,sort2 [asc|desc]]...]) : Analytic function. The the dense rank of a sorted expression in a group.<br />
   - Nearby(expr;[sort1 [asc|desc][,sort2 [asc|desc]];distance;default...]) : Analytic function. Get the value of nearby rows, if distance is negative, it returns value of previous N row, if distance is positive, it returns value of next N row.<br />
   - Counta([group1,group2...];expr) : Analytic function. Count the number of expr of each group.<br />
   - Uniquecounta([group1,group2...];expr) : Analytic function. Count the number of unique expr of each group.<br />
   - Suma([group1,group2...];expr) : Analytic function. Sum the expr of each group.<br />
   - Averagea([group1,group2...];expr) : Analytic function. Caluclate average of expr of each group.<br />
   - maxa([group1,group2...];expr) : Analytic function. Get the maximum value of expr of each group.<br />
   - mina([group1,group2...];expr) : Analytic function. Get the minimum value of expr of each group.<br />
   - foreach(beginid,endid,macro_expr[,step]) : Macro function. make a macro expression list for all fields from beginid to endid. $ stands for field ($ stands for GROUP expression when GROUP involved), # stands for field sequence, % stands for the largest field sequence ID, % can be involved in an expression. For example, foreach(%-2,2,substr($,2,3)+#) will make this expression list: substr(@field{N-2},2,3)+N..,substr(@field3,2,3)+3,substr(@field2,2,3)+2. It can only be used in "select" and "sort". It cannot be a part of expression.<br />
   - coltorow(exp1[,exp2 ... ] ) : Macro function. Make the columns to rows. Accept multiple parameter, also accept foreach(). The row number will be the maximum number of parameter of all coltorow functions. <br />
   - anycol(start,end,expr[,step]) : Macro function. Can be used in filter only, to check any field fulfil a condition, e.g. anycol(1,%,$). $ stands for GROUP expression when GROUP involved), # stands for field sequence, % stands for the largest field sequence ID, % can be involved in an expression.<br />
   - allcol(start,end,expr[,step]) : Macro function. Can be used in filter only, to check all field fulfil a condition, e.g. allcol(1,%,$). $ stands for GROUP expression when GROUP involved), # stands for field sequence, % stands for the largest field sequence ID, % can be involved in an expression.<br />
   - root(expr) : Hierarchical function. Returns the expression of the root node.<br /> 
   - parent(expr) : Hierarchical function. Returns the expression of the parent node.<br /> 
   - path(expr[,connector]) : Hierarchical function. Returns an path (of the expression) from root to current node, connector is used for connecting nodes, default is '/'.<br /> 
# Example and scenarios
- Query an apache or nginx access log, to get the number of hits from different clients, and the browser is Chrome or Firefox<br />
`./rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter agent reglike '(Chrome|Firefox)' | select host, count(1) | group host | sort count(1) desc" < access.log`
- Searching folder logs, to get all access log from 127.0.0.1<br />
`./rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter host = '127.0.0.1' | select @raw " logs`
- Login console, customize the query
   ```
   ./rq --console
   Welcome to RQuery v0.88a
   Author: Wei Huang; Contact Email: fuyuncat@gmail.com

   rquery >load access.log
   File is loaded.
   rquery >parse /\"(?P<origip>[^\n]*)\" (?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \"(?P<request>[^\n]*)\" (?P<status>[0-9]+) (?P<size>\S+) \"(?P<referrer>[^\n]*)\" \"(?P<agent>[^\n]*)\"/
   Regular expression string is provided.
   rquery >group status
   Group expressions are provided.
   rquery >select status, count(1)
   Selection is provided.
   rquery >sort count(1) desc
   Sorting keys are provided.
   rquery >limit 5
   Output limit has been set up.
   rquery >run
   status  count(1)
   ------  --------
   200     450
   304     149
   302     20
   303     6
   500     5
   ```
- Generate a simple hourly load graph by querying apache/nginx logs.<br/>
   ```
   rq -p on -q "p d/ /\"\"[]/ | t @5 date '[%d/%b/%Y:%H:%M:%S %z]' | select dateformat(truncdate(@5,3600))+':'+pad('#',round(count(1)/1000)) | group truncdate(@5,3600) | sort 1 asc " access.log.20220708
   ```
   Returns result:<br/>
   ```
   [08/Jul/2022:10:00:00 +0000]:#################################################################################
   [08/Jul/2022:11:00:00 +0000]:##########################################################################################
   [08/Jul/2022:12:00:00 +0000]:#################################################################################
   [08/Jul/2022:13:00:00 +0000]:#################################################################################
   [08/Jul/2022:14:00:00 +0000]:###################################################################################################
   [08/Jul/2022:15:00:00 +0000]:################################################################################################
   [08/Jul/2022:16:00:00 +0000]:#####################################################################
   [08/Jul/2022:17:00:00 +0000]:###############################
   [08/Jul/2022:18:00:00 +0000]:######################
   [08/Jul/2022:19:00:00 +0000]:#####################
   [08/Jul/2022:20:00:00 +0000]:######################
   [08/Jul/2022:21:00:00 +0000]:###################
   [08/Jul/2022:22:00:00 +0000]:###############
   [08/Jul/2022:23:00:00 +0000]:################
   [09/Jul/2022:00:00:00 +0000]:#############
   [09/Jul/2022:01:00:00 +0000]:#############
   [09/Jul/2022:02:00:00 +0000]:#############
   [09/Jul/2022:03:00:00 +0000]:##############
   [09/Jul/2022:04:00:00 +0000]:###########
   [09/Jul/2022:05:00:00 +0000]:######
   [09/Jul/2022:06:00:00 +0000]:############
   [09/Jul/2022:07:00:00 +0000]:##########
   [09/Jul/2022:08:00:00 +0000]:###########
   [09/Jul/2022:09:00:00 +0000]:###################
   ```
More examples can be found here: https://github.com/fuyuncat/rquery/blob/main/EXAMPLES.md <br />
# Dependencies
&nbsp;&nbsp;&nbsp;No dependency required. <br />
<br />
# Compile Environment
&nbsp;&nbsp;&nbsp;The alpha version has been compiled in CentOS 7.<br />
&nbsp;&nbsp;&nbsp;The g++ version is 4.8.5 20150623<br />
<br />
# License
&nbsp;&nbsp;&nbsp;Our source code is available under the GNU GPLv3 license.<br />
<br />
# Change logs
&nbsp;&nbsp;&nbsp;https://github.com/fuyuncat/rquery/blob/main/CHANGELOG.md <br />
