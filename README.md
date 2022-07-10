# rquery
  RQuery is a regular expression (Capturing Group and Named Group supported) based text searching engine.
  Contact Email: fuyuncat@gmail.com

# Usage & Help Doc
Command line mode:
  ./rq [parameters] "Text to be parsed and queried"
  or
  echo "Text to be parsed and queried" | ./rq [parameters]
  or
  ./rq [parameters] < file
  or
  ./rq [parameters] file
  or
  ./rq [parameters] folder

Console mode:
  ./rq [parameters] --console
  Input any of below commands in the console mode.
    load file|folder : load a file or a folder
    filemode buffer|line : Provide file read mode, default is buffer.
    skip <N> : How many bytes or lines (depends on the filemode) to be skipped.
    parse /regular string/ : Parse a regular expression string quoted by //
    set <field datatype [date format],...> : Set field data type
    filter <filter conditions> : Filter the parsed records
    select <field or expression,...> : Fields or expressions to be selected
    group <field or expression,...> : Fields or expressions to be groupd for aggregation selections
    sort <field or expression [asc|desc],...> : Sorting keys to decide order of the output records
    limt <n | bottomN,topN> : Limited number records to be printed
    run [query] : Run the preprocessed query of a provided query (refering to below part)

Where the parameter could be any of below ones.
  --help|-h
  --query | -q <qeury string> : The query string to be used to parse and query the text content.
    Syntax of query string:
      parse /regular string/|set <field datatype [date format],...>|filter <filter conditions>|select <field or expression,...>|group <field or expression,...>|sort <field or expression [asc|desc],...>|limt <n | bottomN,topN>
      The query parts are separated by |, they could be put in any order. You can provide one or more of them in a query, none of them is mandatary
        parse /regular string/ : Parse a regular expression string quoted by //
        set <field datatype [date format],...> : Set field data type
        filter <filter conditions> : Filter the parsed records
        select <field or expression,...> : Fields or expressions to be selected
        group <field or expression,...> : Fields or expressions to be groupd for aggregation selections
        sort <field or expression [asc|desc],...> : Sorting keys to decide order of the output records
        limt <n | bottomN,topN> : Limited number records to be printed
    Variables:
      In any expression of select, filter, group, sort, variables can be used. The variables are in a @Var format. Currently, the variables can be used are,
        @raw : The raw string of a parsed line
        @file : The file name
        @line : The line sequence number of (regular expression) matched lines
        @row : The sequence number of output records
        @filedN : The field of a parsed line, N is the sequence number of the field. It matches to the Capturing Group in the regular expression.
    Fields:
      Fields are the Capturing Group or Named Capturing Group in the regular expression. If it's a Named Capturing Group, the name can be used as the field name, or a variable @fieldN can be mapped to a Capturing Group. 
  --fieldheader | -f on|off : Wheather print fields header(default) or not
  --readmode | -r buffer|line : File read mode, buffer(default) or line
  --buffsize | -b size : The buffer size when read mode is buffer, default is 16384
  --skip | -s number : How many lines or bytes to be skipped before start to parse the text content, default is 0
  --msglevel | -m level : The output message level, could be INFO, WARNING, ERROR, FATAL, default is ERROR

# Example and scenarios
  # Query an apache or nginx access log, to get the number of hits from different clients, and the browser is Chrome or Firefox
  ./rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter agent reglike '*(Chrome|Firefox)*' | select host, count(1) | group host | sort count(1) desc" < access.log

  # Searching folder logs, to get all access log from 127.0.0.1
  ./rq -m debug1 -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter origip like '127.0.0.1*' | select truncdate(time,3600) " logs
  
  # Login console, customize the query
  ./rq --console
  load /var/log/httpd/access_logs
  parse /\"(?P<origip>[^\n]*)\" (?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \"(?P<request>[^\n]*)\" (?P<status>[0-9]+) (?P<size>\S+) \"(?P<referrer>[^\n]*)\" \"(?P<agent>[^\n]*)\"/
  filter origip like '192.168.0.8*'
  select truncdate(time,3600)
  run
  
# Dependencies
  This engine currently depends on boost, we are planning to remove this dependency in the near future.

# Compile Environment
  The alpha version has been compiled in CentOS 7.
  The g++ version is 4.8.5 20150623

# License
  Our source code is available under the GNU GPLv3 license.
  
