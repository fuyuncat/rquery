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
Command line mode:<br />
&nbsp;&nbsp;&nbsp;./rq [parameters] "Text to be parsed and queried"<br />
&nbsp;&nbsp;&nbsp;or<br />
&nbsp;&nbsp;&nbsp;echo "Text to be parsed and queried" | ./rq [parameters]<br />
&nbsp;&nbsp;&nbsp;or<br />
&nbsp;&nbsp;&nbsp;./rq [parameters] < file<br />
&nbsp;&nbsp;&nbsp;or<br />
&nbsp;&nbsp;&nbsp;./rq [parameters] file<br />
&nbsp;&nbsp;&nbsp;or<br />
&nbsp;&nbsp;&nbsp;./rq [parameters] folder<br />
<br />
Console mode:
&nbsp;&nbsp;&nbsp;./rq [parameters] --console<br />
&nbsp;&nbsp;&nbsp;Input any of below commands in the console mode.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;load file|folder : load a file or a folder<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;filemode buffer|line : Provide file read mode, default is buffer.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;skip <N> : How many bytes or lines (depends on the filemode) to be skipped.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;parse /regular string/ : Parse a regular expression string quoted by //<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;set <field datatype [date format],...> : Set field data type. Supported data types: LONG, INTEGER, DOUBLE, STRING, DATE.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;filter <filter conditions> : Filter the parsed records<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;select <field or expression,...> : Fields or expressions to be selected<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;group <field or expression,...> : Fields or expressions to be groupd for aggregation selections<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;sort <field or expression [asc|desc],...> : Sorting keys to decide order of the output records<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;limt <n | bottomN,topN> : Limited number records to be printed<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;run [query] : Run the preprocessed query of a provided query (refering to below part)<br />
<br />
Where the parameter could be any of below ones.<br />
&nbsp;&nbsp;&nbsp;--help|-h<br />
&nbsp;&nbsp;&nbsp;--query | -q <qeury string> : The query string to be used to parse and query the text content.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Syntax of query string:<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;parse /regular string/|set field datatype [date format],...|filter <ilter conditions|select field or expression,...|group field or expression,...|sort field or expression [asc|desc],...|limt n | bottomN,topN<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  The query parts are separated by |, they could be put in any order. You can provide one or more of them in a query, none of them is mandatary<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  parse /regular string/ : Parse a regular expression string quoted by //<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  set field datatype [date format],... : Set field data type. Supported data types: LONG, INTEGER, DOUBLE, STRING, DATE.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  filter filter conditions : Filter the parsed records<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  select field or expression,... : Fields or expressions to be selected<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  group field or expression,... : Fields or expressions to be groupd for aggregation selections<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  sort field or expression [asc|desc],... : Sorting keys to decide order of the output records<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  limt n | bottomN,topN : Limited number records to be printed<br />
<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Variables:<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;In any expression of select, filter, group, sort, variables can be used. The variables are in a @Var format. Currently, the variables can be used are,<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;@raw : The raw string of a parsed line<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;@file : The file name<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;@line : The line sequence number of (regular expression) matched lines<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;@row : The sequence number of output records<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;@filedN : The field of a parsed line, N is the sequence number of the field. It matches to the Capturing Group in the regular expression.<br />
<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Fields:<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Fields are the Capturing Group or Named Capturing Group in the regular expression. If it's a Named Capturing Group, the name can be used as the field name, or a variable @fieldN can be mapped to a Capturing Group. <br />
&nbsp;&nbsp;&nbsp;--fieldheader | -f on|off : Wheather print fields header(default) or not<br />
&nbsp;&nbsp;&nbsp;--readmode | -r buffer|line : File read mode, buffer(default) or line<br />
&nbsp;&nbsp;&nbsp;--buffsize | -b size : The buffer size when read mode is buffer, default is 16384<br />
&nbsp;&nbsp;&nbsp;--skip | -s number : How many lines or bytes to be skipped before start to parse the text content, default is 0<br />
&nbsp;&nbsp;&nbsp;--msglevel | -m level : The output message level, could be INFO, WARNING, ERROR, FATAL, default is ERROR<br />
<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Functions:<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Functions can be used in the expression. We current provide some essential normal functions and aggregation functions.
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;upper(str) : Normal function. Convert a string to upper case.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;lower(str) : Normal function. Convert a string to lower case.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;strlen(str) : Normal function. Return the length of a string.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;instr(str,sub) : Normal function. Return the position of a sub string in a string. Return -1 if caannot find the sub string<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;substr(str,pos,len) : Normal function. Get a substring of a string, start from pos. If len is not provide, get the sub string till the end of the string.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;replace(str,sub1,sub2) : Normal function. Replace all sub1 in a string with sub2.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Comparestr(str1,str2) : Normal function. Compare str1 to str2, case sensitive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;NocaseComparestr(str1,str2) : Normal function. Compare str1 to str2, case insensive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;floor(num) : Normal function. Get the floor integer number of a given float number.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ceil(num) : Normal function. Get the ceil integer number of a given float number.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;round(num) : Normal function. Round a given float number.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;log(num) : Normal function. Get the log result of a given float number.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;dateformat(date) : Normal function. Convert a date data to a string with the given format.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;timediff(date1,date2) : Normal function. Get the difference (in seconds) of two date.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;now() : Normal function. Get current date time.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Count(expr) : Aggregation function. Count the number of expr.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Sum(expr) : Aggregation function. Sum the number of expr.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Max(expr) : Aggregation function. Get the maximum value of expr.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Min(expr) : Aggregation function. Get the minimum value of expr.<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Average(expr) : Aggregation function. Get the average value of expr.<br />
<br />
# Example and scenarios
&nbsp;&nbsp;&nbsp;-- Query an apache or nginx access log, to get the number of hits from different clients, and the browser is Chrome or Firefox<br />
&nbsp;&nbsp;&nbsp;`./rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter agent reglike '*(Chrome|Firefox)*' | select host, count(1) | group host | sort count(1) desc" < access.log`
<br />
&nbsp;&nbsp;&nbsp;-- Searching folder logs, to get all access log from 127.0.0.1
&nbsp;&nbsp;&nbsp;`./rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter origip like '127.0.0.1*' | select truncdate(time,3600) " logs`
<br />
&nbsp;&nbsp;&nbsp;-- Login console, customize the query
   ```
   ./rq --console
   load /var/log/httpd/access_logs
   parse /\"(?P<origip>[^\n]*)\" (?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \"(?P<request>[^\n]*)\" (?P<status>[0-9]+) (?P<size>\S+) \"(?P<referrer>[^\n]*)\" \"(?P<agent>[^\n]*)\"/
   filter origip like '192.168.0.8*'
   select truncdate(time,3600)
   run
   ```
<br />
&nbsp;&nbsp;&nbsp;-- Get the hourly hits from nginx log
   ```
  rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/| select truncdate(time,3600), count(1) | group truncdate(time,3600)" /var/log/nginx/access.log-20220629         ERROR:Selection 'truncdate(time,3600)' does not exist in Group or invalid using aggregation function
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
<br />
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
