# rquery
&nbsp;&nbsp;&nbsp;RQuery is a regular expression (Capturing Group and Named Group supported) based text searching engine.<br />
&nbsp;&nbsp;&nbsp;Contact Email: fuyuncat@gmail.com<br />
<br />
# Install
&nbsp;&nbsp;&nbsp; Download source code zip<br />
&nbsp;&nbsp;&nbsp; Unzip the downloaded file<br />
&nbsp;&nbsp;&nbsp; make<br />
<br />
# Usage & Help Doc
- run modes
   - Command line mode:<br />
      - ./rq [parameters] "Text to be parsed and queried"<br />
      - echo "Text to be parsed and queried" | ./rq [parameters]<br />
      - ./rq [parameters] < file<br />
      - ./rq [parameters] file<br />
      - ./rq [parameters] folder<br />
   - Console mode:
      - ./rq [parameters] --console<br />
   &nbsp;&nbsp;&nbsp;Input any of below commands in the console mode.<br />
         - load file|folder : load a file or a folder<br />
         - filemode buffer|line : Provide file read mode, default is buffer.<br />
         - skip <N> : How many bytes or lines (depends on the filemode) to be skipped.<br />
         - parse /regular string/ : Parse a regular expression string quoted by //<br />
         - set <field datatype [date format],...> : Set field data type. Supported data types: LONG, INTEGER, DOUBLE, STRING, DATE.<br />
         - filter <filter conditions> : Filter the parsed records<br />
         - select <field or expression,...> : Fields or expressions to be selected<br />
         - group <field or expression,...> : Fields or expressions to be groupd for aggregation selections<br />
         - sort <field or expression [asc|desc],...> : Sorting keys to decide order of the output records<br />
         - limt <n | bottomN,topN> : Limited number records to be printed<br />
         - run [query] : Run the preprocessed query of a provided query (refering to below part)<br />
-Parameters
   - --help|-h<br />
   - --fieldheader | -f on|off : Wheather print fields header(default) or not<br />
   - --readmode | -r buffer|line : File read mode, buffer(default) or line<br />
   - --buffsize | -b size : The buffer size when read mode is buffer, default is 16384<br />
   - --skip | -s number : How many lines or bytes to be skipped before start to parse the text content, default is 0<br />
   - --msglevel | -m level : The output message level, could be INFO, WARNING, ERROR, FATAL, default is ERROR<br />
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
- Variables:
In any expression of select, filter, group, sort, variables can be used. The variables are in a @Var format. Currently, the variables can be used are,<br />
   - @raw : The raw string of a parsed line<br />
   - @file : The file name<br />
   - @line : The line sequence number of (regular expression) matched lines<br />
   - @row : The sequence number of output records<br />
   - @filedN : The field of a parsed line, N is the sequence number of the field. It matches to the Capturing Group in the regular expression.<br />
- Fields:<br />
Fields are the Capturing Group or Named Capturing Group in the regular expression. If it's a Named Capturing Group, the name can be used as the field name, or a variable @fieldN can be mapped to a Capturing Group. <br />
- Functions:<br />
Functions can be used in the expression. We current provide some essential normal functions and aggregation functions.
   - upper(str) : Normal function. Convert a string to upper case.<br />
   - lower(str) : Normal function. Convert a string to lower case.<br />
   - strlen(str) : Normal function. Return the length of a string.<br />
   - instr(str,sub) : Normal function. Return the position of a sub string in a string. Return -1 if caannot find the sub string<br />
   - substr(str,pos,len) : Normal function. Get a substring of a string, start from pos. If len is not provide, get the sub string till the end of the string.<br />
   - replace(str,sub1,sub2) : Normal function. Replace all sub1 in a string with sub2.<br />
   - Comparestr(str1,str2) : Normal function. Compare str1 to str2, case sensitive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br />
   - NocaseComparestr(str1,str2) : Normal function. Compare str1 to str2, case insensive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br />
   - floor(num) : Normal function. Get the floor integer number of a given float number.<br />
   - ceil(num) : Normal function. Get the ceil integer number of a given float number.<br />
   - round(num) : Normal function. Round a given float number.<br />
   - log(num) : Normal function. Get the log result of a given float number.<br />
   - dateformat(date) : Normal function. Convert a date data to a string with the given format.<br />
   - timediff(date1,date2) : Normal function. Get the difference (in seconds) of two date.<br />
   - now() : Normal function. Get current date time.<br />
   - truncdate(date,seconds) : Normal function. Truncate a date a number is multiple of the given second number.<br />
   - Count(expr) : Aggregation function. Count the number of expr.<br />
   - Sum(expr) : Aggregation function. Sum the number of expr.<br />
   - Max(expr) : Aggregation function. Get the maximum value of expr.<br />
   - Min(expr) : Aggregation function. Get the minimum value of expr.<br />
   - Average(expr) : Aggregation function. Get the average value of expr.<br />
# Example and scenarios
- Query an apache or nginx access log, to get the number of hits from different clients, and the browser is Chrome or Firefox<br />
`./rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter agent reglike '*(Chrome|Firefox)*' | select host, count(1) | group host | sort count(1) desc" < access.log`
- Searching folder logs, to get all access log from 127.0.0.1<br />
`./rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter origip like '127.0.0.1*' | select truncdate(time,3600) " logs`
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
