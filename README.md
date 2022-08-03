# rquery
&nbsp;&nbsp;&nbsp;RQuery is a tool to search string block/file/folder using regular/delmiter/wildcard/ expression parttern, and filter/group calculate/sort the matched result. One command can do what grep/xgrep/sort/uniq/awk/wc/sed/cut can do and more. <br />
&nbsp;&nbsp;&nbsp;Contact Email: fuyuncat@gmail.com<br />
<br />
# Install
The latest version can be downloaded here: https://github.com/fuyuncat/rquery/releases <br />
- Install method 1<br />
&nbsp;&nbsp;&nbsp; Download source code zip<br />
&nbsp;&nbsp;&nbsp; Unzip the downloaded file<br />
&nbsp;&nbsp;&nbsp; sudo yum -y install boost-devel<br />
&nbsp;&nbsp;&nbsp; make<br />

- Install method 2<br />
&nbsp;&nbsp;&nbsp; download the rpm file <br />
&nbsp;&nbsp;&nbsp; unzip the rpm file <br />
&nbsp;&nbsp;&nbsp; sudo rpm -ihv <downloaded_rpm_file> <br />

- Install method 3<br />
&nbsp;&nbsp;&nbsp; download the compiled zip file <br />
&nbsp;&nbsp;&nbsp; unzip the compiled zip file <br />
&nbsp;&nbsp;&nbsp; chmod ugo+x rq<br />
&nbsp;&nbsp;&nbsp; cp rq /usr/bin/<br />
&nbsp;&nbsp;&nbsp; or directly run it from local.<br />
<br />
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
         - detecttyperows <N> How many matched rows will be used for detecting data types, default is 1
         - parse /regular string/ : Choose one of three mode to match the content.<br />
            - // quotes a regular expression pattern string to parse the content; 
            - w/<WildCardExpr>/[quoters/] quotes wildcard expression to parse the content, wildcard '\*' stands for a field, e.g. w/\*abc\*,\*/. substrings between two \* are the spliters, spliter between quoters will be skipped; 
            - d/<Delmiter>/[quoters/][r] quotes delmiter to parse the content, Delmiter splits fields, delmiter between quoters will be skipped, r at the end of pattern means the delmiter is repeatable, e.g. d/ /""/
         - set <field datatype [date format],...> : Set field data type. Supported data types: LONG, INTEGER, DOUBLE, STRING, DATE.<br />
         - filter <filter conditions> : Filter the parsed records<br />
         - select <field or expression,...> : Fields or expressions to be selected<br />
         - group <field or expression,...> : Fields or expressions to be groupd for aggregation selections<br />
         - sort <field or expression [asc|desc],...> : Sorting keys to decide order of the output records<br />
         - limt <n | bottomN,topN> : Limited number records to be printed<br />
         - unique : Make the returned resutl unique.<br />
         - var "name1:value1[:datatyp1][ name2:value2[:datatyp2]..]" -- Pass variable to rquery, variable name can be any valid word except the reserved words, RAW,FILE,ROW,LINE. Using @name to refer to the variable.<br />
         - run [query] : Run the preprocessed query of a provided query (refering to below part)<br />
- options
   - --help|-h<br />
   - --fieldheader | -f on|off : Wheather print fields header or not(default).<br />
   - --progress | -p <on|off> -- Wheather show the processing progress or not(default).<br />
   - --outputformat | -o <text|json> -- Provide output format, default is text.<br />
   - --readmode | -r buffer|line : File read mode, buffer(default) or line.<br />
   - --buffsize | -b size : The buffer size when read mode is buffer, default is 16384.<br />
   - --skip | -s number : How many lines or bytes to be skipped before start to parse the text content, default is 0.<br />
   - --variable | -v "name1:value1[ name2:value2..]" -- Pass variable to rquery, variable name can be any valid word except the reserved words, RAW,FILE,ROW,LINE. Using @name to refer to the variable.<br />
   - --detecttyperows | -d <N> : How many matched rows will be used for detecting data types, default is 1.<br />
   - --msglevel | -m level : The output message level, could be INFO, WARNING, ERROR, FATAL, default is FATAL.<br />
   - --nameline | -n Yes/no : Specify the first line should be used for filed names (useful for csv files). Default is no.<br />
   - --query | -q <qeury string> : The query string to be used to parse and query the text content.<br />
- Syntax of query string:
   - parse /regular string/|set field datatype [date format],...|filter <ilter conditions|select field or expression,...|group field or expression,...|sort field or expression [asc|desc],...|limt n | bottomN,topN<br />
 The query parts are separated by |, they could be put in any order. You can provide one or more of them in a query, none of them is mandatary<br />
      - parse /regular string/ : Parse a regular expression string quoted by //<br />
      - set field datatype [date format],... : Set field data type. Supported data types: LONG, INTEGER, DOUBLE, STRING, DATE.<br />
      - filter filter conditions : Filter the parsed records<br />
      - select field or expression,... : Fields or expressions to be selected<br />
      - group field or expression,... : Fields or expressions to be groupd for aggregation selections<br />
      - sort field or expression [asc|desc],... : Sorting keys to decide order of the output records<br />
      - limt n | bottomN,topN : Limited number records to be printed<br />
      - unique : Make the returned resutl unique. <br />
      - msglevel : The output message level, could be INFO, WARNING, ERROR, FATAL, default is FATAL.<br />
      - progress <on|off> -- Wheather show the processing progress or not(default).<br />
      - format <text|json> -- Provide output format, default is text.<br />
- Variables:
In any expression of select, filter, group, sort, variables can be used. The variables are in a @Var format. Currently, the variables can be used are,<br />
   - @raw : The raw string of a parsed line<br />
   - @file : The file name<br />
   - @line : The line sequence number of (regular expression) matched lines<br />
   - @row : The sequence number of output records<br />
   - @filedN : The field of a parsed line, N is the sequence number of the field. It matches to the Capturing Group in the regular expression.<br />
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
   - replace(str,sub1,sub2) : Normal function. Replace all sub1 in a string with sub2.<br />
   - regreplace(str,pattern,sub) : Normal function. Replace all regular pattern in a string with sub (capturing group supported).<br />
   - regmatch(str,pattern,sub) : Normal function. Return an expression including the capturing groups matched a regular pattern. Use {N} to stand for the matched groups<br />
   - Comparestr(str1,str2) : Normal function. Compare str1 to str2, case sensitive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br />
   - NocaseComparestr(str1,str2) : Normal function. Compare str1 to str2, case insensive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br />
   - countword(str,[ingnore_quoters]) : Normal function. Get the number of word in a string. Any substring separated by space/tab/newline/punctuation marks will be count as a word. if ingnore_quoters (in pairs, e.g. ''"") provided, the whole string between quoters will be counted as one word<br />
   - getword(str,wordnum,[ingnore_quoters]) : Normal function. Get a word specified sequence number in a string. Any substring separated by space/tab/newline/punctuation marks will be count as a word. if ingnore_quoters (in pairs, e.g. ''"") provided, the whole string between quoters will be counted as one word<br />
   - pad(seed,len) : Normal function. Construct a new string from seed multiple len times.<br />
   - floor(num) : Normal function. Get the floor integer number of a given float number.<br />
   - ceil(num) : Normal function. Get the ceil integer number of a given float number.<br />
   - round(num) : Normal function. Round a given float number.<br />
   - log(num) : Normal function. Get the log result of a given float number.<br />
   - dateformat(date) : Normal function. Convert a date data to a string with the given format.<br />
   - timediff(date1,date2) : Normal function. Get the difference (in seconds) of two date.<br />
   - now() : Normal function. Get current date time.<br />
   - truncdate(date,seconds) : Normal function. Truncate a date a number is multiple of the given second number.<br />
   - switch(input,case1,return1[,case2,result2...][,default]): Normal function. if input equal to case1, then return return1, etc.. If none matched, return default or return input if no default provided. Similar to SWITCH CASE statement.<br />
   - greatest(expr1,expr2[,...]) : Normal function. Return the largest one of the given expressions.<br />
   - least(expr1,expr2[,...]) : Normal function. Return the smallest one of the given expressions.<br />
   - Count(expr) : Aggregation function. Count the number of expr.<br />
   - Uniquecount(expr) : Aggregation function. Count the number of distinct expr.<br />
   - Sum(expr) : Aggregation function. Sum the number of expr.<br />
   - Max(expr) : Aggregation function. Get the maximum value of expr.<br />
   - Min(expr) : Aggregation function. Get the minimum value of expr.<br />
   - Average(expr) : Aggregation function. Get the average value of expr.<br />
   - foreach(beginid,endid,macro_expr) : Macro function. make a macro expression list for all fields from beginid to endid. $ stands for field ($ stands for GROUP expression when GROUP involved), # stands for field sequence, % stands for the largest field sequence ID. For example, foreach(%,2,substr($,2,3)+#) will make this expression list: substr(@fieldN,2,3)+N..,substr(@field3,2,3)+3,substr(@field2,2,3)+2. It can only be used in "select" and "sort". It cannot be a part of expression.
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
- Get the hourly hits from nginx log
   ```
    rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/| select truncdate(time,3600), count(1) | group truncdate(time,3600)" /var/log/nginx/access.log-20220629
    truncdate(time,3600)    count(1)
    --------------------    --------
    28/Jun/2022:10:00:00 +1000      261
    28/Jun/2022:11:00:00 +1000      77
    28/Jun/2022:12:00:00 +1000      250
    28/Jun/2022:13:00:00 +1000      165
    28/Jun/2022:14:00:00 +1000      42
    28/Jun/2022:15:00:00 +1000      121
    28/Jun/2022:16:00:00 +1000      238
    28/Jun/2022:17:00:00 +1000      118
    28/Jun/2022:18:00:00 +1000      81
    28/Jun/2022:19:00:00 +1000      106
    28/Jun/2022:20:00:00 +1000      311
    28/Jun/2022:21:00:00 +1000      86
    28/Jun/2022:22:00:00 +1000      64
    28/Jun/2022:23:00:00 +1000      63
    29/Jun/2022:00:00:00 +1000      51
    29/Jun/2022:01:00:00 +1000      76
    29/Jun/2022:02:00:00 +1000      32
   ```
   More examples can be found here: https://github.com/fuyuncat/rquery/blob/main/EXAMPLES.md <br />
# Dependencies
&nbsp;&nbsp;&nbsp;This engine currently depends on boost, we are planning to remove this dependency in the near future.<br />
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
