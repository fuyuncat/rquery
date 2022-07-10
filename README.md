# rquery
  RQuery is a regular expression (Capturing Group and Named Group supported) based text searching engine.<br />
  Contact Email: fuyuncat@gmail.com<br />

# Usage & Help Doc
Command line mode:<br />
  ./rq [parameters] "Text to be parsed and queried"<br />
  or<br />
  echo "Text to be parsed and queried" | ./rq [parameters]<br />
  or<br />
  ./rq [parameters] < file<br />
  or<br />
  ./rq [parameters] file<br />
  or<br />
  ./rq [parameters] folder<br />

Console mode:
  ./rq [parameters] --console<br />
  Input any of below commands in the console mode.<br />
    load file|folder : load a file or a folder<br />
    filemode buffer|line : Provide file read mode, default is buffer.<br />
    skip <N> : How many bytes or lines (depends on the filemode) to be skipped.<br />
    parse /regular string/ : Parse a regular expression string quoted by //<br />
    set <field datatype [date format],...> : Set field data type<br />
    filter <filter conditions> : Filter the parsed records<br />
    select <field or expression,...> : Fields or expressions to be selected<br />
    group <field or expression,...> : Fields or expressions to be groupd for aggregation selections<br />
    sort <field or expression [asc|desc],...> : Sorting keys to decide order of the output records<br />
    limt <n | bottomN,topN> : Limited number records to be printed<br />
    run [query] : Run the preprocessed query of a provided query (refering to below part)<br />

Where the parameter could be any of below ones.<br />
  --help|-h<br />
  --query | -q <qeury string> : The query string to be used to parse and query the text content.<br />
    Syntax of query string:<br />
      parse /regular string/|set field datatype [date format],...|filter <ilter conditions|select field or expression,...|group field or expression,...|sort field or expression [asc|desc],...|limt n | bottomN,topN<br />
      The query parts are separated by |, they could be put in any order. You can provide one or more of them in a query, none of them is mandatary<br />
        parse /regular string/ : Parse a regular expression string quoted by //<br />
        set field datatype [date format],... : Set field data type<br />
        filter filter conditions : Filter the parsed records<br />
        select field or expression,... : Fields or expressions to be selected<br />
        group field or expression,... : Fields or expressions to be groupd for aggregation selections<br />
        sort field or expression [asc|desc],... : Sorting keys to decide order of the output records<br />
        limt n | bottomN,topN : Limited number records to be printed<br />
    Variables:<br />
      In any expression of select, filter, group, sort, variables can be used. The variables are in a @Var format. Currently, the variables can be used are,<br />
        @raw : The raw string of a parsed line<br />
        @file : The file name<br />
        @line : The line sequence number of (regular expression) matched lines<br />
        @row : The sequence number of output records<br />
        @filedN : The field of a parsed line, N is the sequence number of the field. It matches to the Capturing Group in the regular expression.<br />
    Fields:<br />
      Fields are the Capturing Group or Named Capturing Group in the regular expression. If it's a Named Capturing Group, the name can be used as the field name, or a variable @fieldN can be mapped to a Capturing Group. <br />
  --fieldheader | -f on|off : Wheather print fields header(default) or not<br />
  --readmode | -r buffer|line : File read mode, buffer(default) or line<br />
  --buffsize | -b size : The buffer size when read mode is buffer, default is 16384<br />
  --skip | -s number : How many lines or bytes to be skipped before start to parse the text content, default is 0<br />
  --msglevel | -m level : The output message level, could be INFO, WARNING, ERROR, FATAL, default is ERROR<br />

# Example and scenarios
  # Query an apache or nginx access log, to get the number of hits from different clients, and the browser is Chrome or Firefox<br />
  ./rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter agent reglike '*(Chrome|Firefox)*' | select host, count(1) | group host | sort count(1) desc" < access.log<br />

  # Searching folder logs, to get all access log from 127.0.0.1
  ./rq -m debug1 -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter origip like '127.0.0.1*' | select truncdate(time,3600) " logs<br />
  
  # Login console, customize the query
  ./rq --console<br />
  load /var/log/httpd/access_logs<br />
  parse /\"(?P<origip>[^\n]*)\" (?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \"(?P<request>[^\n]*)\" (?P<status>[0-9]+) (?P<size>\S+) \"(?P<referrer>[^\n]*)\" \"(?P<agent>[^\n]*)\"/<br />
  filter origip like '192.168.0.8*'<br />
  select truncdate(time,3600)<br />
  run<br />
  
# Dependencies
  This engine currently depends on boost, we are planning to remove this dependency in the near future.<br />

# Compile Environment
  The alpha version has been compiled in CentOS 7.<br />
  The g++ version is 4.8.5 20150623<br />

# License
  Our source code is available under the GNU GPLv3 license.<br />
  
