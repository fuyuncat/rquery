# Examples and scenarios
- Get lines containing specific string (equal to grep)<br/>
`rq -q "select @raw | filter @raw like '*strptime*'" test.cpp`<br/>
Returns result:<br/>
   ```
     printf("strptime is defined!\n");
     char * c = strptime("20220728", "%Y%m%d", &tm);
   ```
- Get lines matched a regular pattern (equal to grep -E or egrep)<br/>
`rq -q "select @raw | filter @raw reglike '(struct|char)'" test.cpp`<br/>
Returns result:<br/>
   ```
    struct tm;
    char * c = strptime("20220728", "%Y%m%d", &tm);
   ```
- Get unique output (equal to uniq)<br/>
   ```
   echo "ccc
   bbb
   aaa
   ccc
   bbb" | rq -q "select @raw | unique"
   ```
   Returns result:<br/>
   ```
   ccc
   bbb
   aaa
   ```
- Get unique and sorted output (equal to uniq&sort)<br/>
   ```
   echo "ccc
   bbb
   aaa
   ccc
   bbb" | rq -q "select @raw | unique | sort @raw"
   ```
   Returns result:<br/>
   ```
   aaa
   bbb
   ccc
   ```
- Get the specific lines<br/>
   ```
   rq -q "select @raw | limit 5,7" test.cpp
   ```
   Returns result:<br/>
   ```
     struct tm;
   #if __XSI_VISIBLE
     printf("strptime is defined!\n");
   ```
- Get the file count number and total size of each user in a specific folder<br/>
   ```ls -l $folder/ | rq -q "parse /(\S+)[ ]{1,}(\S+)[ ]{1,}(?P<owner>\S+)[ ]{1,}(?P<group>\S+)[ ]{1,}(?P<size>\S+)[ ]{1,}(\S+)[ ]{1,}(\S+)[ ]{1,}(\S+)[ ]{1,}(\S+)/ | select owner, count(1), sum(size) | group owner"
   ```
   Returns result:<br/>
   ```
   nginx   20      16436
   root    505     3127798
   ```
- Pre set data type for fields<br/>
   ```rq -q "parse /\\\"(?P<origip>[^\n]*)\\\" (?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/ | set time date '%d/%b/%Y:%H:%M:%S %z'| select @raw | filter time={29/Jun/2022:16:58:18 +1000}" logs/access.log
   ```
   Returns result:<br/>
   ```
   "123.456.789.111 987.654.321.999" 192.168.1.1 - - [29/Jun/2022:16:58:18 +1000] "GET /index.html HTTP/1.1" 200 - "https://mysite.com.au/index.html" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36"
   "123.456.789.222 987.654.321.888" 192.168.1.1 - - [29/Jun/2022:16:58:18 +1000] "GET /logo.gif HTTP/1.1" 200 - "https://mysite.com.au/index.html" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36"
   "123.456.789.333 987.654.321.777" 192.168.1.1 - - [29/Jun/2022:16:58:18 +1000] "GET /login.php HTTP/1.1" 200 - "https://mysite.com.au/index.html" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36"
   ```
- Set variable and use it in the query<br/>
   ```
   rq -v var:print -q "select @raw | filter @raw like '*'+@var+'*'" test.cpp
   ```
   Returns result:<br/>
   ```
     printf("strptime is defined!\n");
   ```
- Get all INFO WARNING and ERROR messages and write them to a logfile<br/>
   ```
   rq -m info -l /tmp/rquery.log -v folder:$(pwd) -q "select @raw | filter @raw = @folder/'*'" file
   ```
   Returns result:<br/>
   ```
   cat /tmp/rquery.log
   ERROR:(2)Quoters in 'select @raw | filter @raw = @folder/'*'' are not paired!
   ERROR:Operation / is not supported for STRING data type!
   ```
- Show the query progress (Useful for processing large files)<br/>
   ```
   rq -p on -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter agent reglike '*(Chrome|Firefox)*' | select host, count(1) | group host | sort count(1) desc" a_very_large_file.log
   ```
   Shows progress<br/>
   ```
   2883584 bytes(0.88%) read in 1.888000 seconds.
   ```
- Display the output in jason format<br/>
   ```
   rq -o json -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|select status, count(1) | group status" access.log
   ```
   Returns result:<br/>
   ```
   {
           "records": [
                   {
                           "status": 200,
                           "count(1)": 450
                   },
                   {
                           "status": 302,
                           "count(1)": 20
                   },
                   {
                           "status": 303,
                           "count(1)": 6
                   },
                   {
                           "status": 304,
                           "count(1)": 149
                   },
                   {
                           "status": 400,
                           "count(1)": 3
                   },
                   {
                           "status": 401,
                           "count(1)": 1
                   },
                   {
                           "status": 404,
                           "count(1)": 4
                   },
                   {
                           "status": 500,
                           "count(1)": 5
                   }
           ],
           "MatchedLines": 638,
           "SelectedRows": 8
   }
   ```
- Skip first 1000 lines when processing a file<br/>
   ```
   rq -r line -s 1000 -q "select @raw | filter @raw like '*192.168.1.1*'" access.log
   ```
   Rquery will start from line 1001 to process the file<br/>
- Detect more rows to get the accurate data types<br/>
   By default, only the first row is chosen to be used to detect the data types. Therefore, below query will treat all rows as LONG.<br/>
   ```
   echo "123
   333
   aaa
   ada
   134" | ./rq -m error -q "select @raw-100 "
   ERROR:Invalid LONG data detected!
   ERROR:Invalid LONG data detected!
   23
   233
   24514368
   24514368
   34
   ```
   Choose first 3 rows to detect the data types, it will set the data type as STRING eventually.<br/>
   ```
   echo "123
   333
   aaa
   ada
   134" | ./rq -m error -d 3 -q "select @raw-100 "
   ERROR:Operation - is not supported for STRING data type!
   ERROR:Operation - is not supported for STRING data type!
   ERROR:Operation - is not supported for STRING data type!
   23
   233



   ```
- Query an apache or nginx access log, to get the number of hits from different clients, and the browser is Chrome or Firefox<br/>
   ```
   rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter agent reglike '(Chrome|Firefox)' | select host, count(1) | group host | sort count(1) desc" < access.log
   ```
   Returns result:<br/>
   ```
   10.10.1.111     329
   10.10.2.222     148
   10.10.3.333     2
   ```
- Searching folder logs, to get all access log from 127.0.0.1<br/>
  ```
  rq -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter host = '127.0.0.1' | select @raw " logs
  ```
   Returns result:<br/>
   ```
   127.0.0.1 - - [29/Jun/2022:16:58:18 +1000] "GET /index.html HTTP/1.1" 200 - "https://mysite.com.au/index.html" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36"
   127.0.0.1 - - [29/Jun/2022:16:58:28 +1000] "GET /index.html HTTP/1.1" 200 - "https://mysite.com.au/index.html" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36"
   127.0.0.1 - - [29/Jun/2022:16:58:38 +1000] "GET /index.html HTTP/1.1" 200 - "https://mysite.com.au/index.html" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36"
   ```
- Login console, customize the query<br/>
   ```
   rq --console
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
- Get the hourly hits from nginx log<br/>
   ```
   rq -f on -q "parse /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/| select truncdate(time,3600), count(1) | group truncdate(time,3600)" /var/log/nginx/access.log-20220629
   ```
   Returns result:<br/>
   ```
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

# Functions usage
- isnull(expr) - Return the position of a sub string in a string. Return -1 if caannot find the sub string<br/> 
   ```
   echo "aaa,bbb,,ddd,eee"|rq -f on -q "parse /([^,]*),([^,]*),([^,]*),([^,]*),([^,^\n]*)/ | select @field1 as f1,isnull(@field1) as f1n,@field3 as f3,isnull(@field3) as f3n"
   ```
   Returns result:<br/>
   ```
   f1      f1n     f3      f3n
   --      ---     --      ---
   aaa     0               1
   Pattern matched 1 line(s).
   Selected 1 row(s).
   ```
- upper(str) - Convert a string to upper case<br/>
   ```
   echo "aaa,bbb,,ddd,eee"|rq -q "parse /([^,]*),([^,]*),([^,]*),([^,]*),([^,^\n]*)/ | select upper(@field1)"
   ```
   Returns result:<br/>
   ```
   AAA
   ```
- lower(str) - Convert a string to lower case<br/>
   ```
   echo "AAA,BBB,,DDD,EEE"|rq -q "parse /([^,]*),([^,]*),([^,]*),([^,]*),([^,^\n]*)/ | select lower(@field1)"
   ```
   Returns result:<br/>
   ```
   aaa
   ```
- substr(str,startpos[,len]) - Get a substring of a string, start from pos. If len is not provide, get the sub string till the end of the string<br/>
   ```
   echo "AAABBBcccDDDEEE"|rq -q "select substr(@raw,6,3)"
   ```
   Returns result:<br/>
   ```
   ccc
   ```
   Another example,
   ```
   echo "AAABBBcccDDDEEE"|rq -q "select substr(@raw,6)"
   ```
   Returns result:<br/>
   ```
   cccDDDEEE
   ```
- instr(str,substr) - Return the position of a sub string in a string. Return -1 if caannot find the sub string<br/>
   ```
   echo "AAABBBcccDDDEEE"|rq -q "select instr(@raw,'ccc')"
   ```
   Returns result:<br/>
   ```
   6
   ```
- strlen(str) - Return the length of a string<br/>
   ```
   echo "AAABBBcccDDDEEE"|rq -q "select strlen(@raw,)"
   ```
   Returns result:<br/>
   ```
   15
   ```
- comparestr(str1, str2) - Compare str1 to str2, case sensitive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br/>
   ```
   echo "Abc,aBC"|rq -q "parse /(?P<col1>[^,]*),(?P<col2>[^,^\n]*)/ | select col1,col2,comparestr(col1,col2)"
   ```
   Returns result:<br/>
   ```
   Abc     aBC     -32
   ```
- nocasecomparestr(str1, str2) - Compare str1 to str2, case insensive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br/>
   ```
   echo "Abc,aBC"|rq -q "parse /(?P<col1>[^,]*),(?P<col2>[^,^\n]*)/ | select col1,col2,nocasecomparestr(col1,col2)"
   ```
   Returns result:<br/>
   ```
   Abc     aBC     0
   ```
- replace(str, sub, new) - Replace all sub1 in a string with sub2<br/>
   ```
   echo "AAABBBcccDDDEEE"|rq -q "select replace(@raw,'ccc','333')"
   ```
   Returns result:<br/>
   ```
   AAABBB333DDDEEE
   ```
- regreplace(str, pattern, new) - Replace all regular pattern in a string with sub (capturing group supported).<br/>
   ```
   echo "AAABBBcccDDDEEE"|rq -q "select regreplace(@raw,'(AAA|EEE)','333')"
   ```
   Returns result:<br/>
   ```
   333BBBcccDDD333
   ```
- regmatch(str, pattern, return_expr) - Return an expression including the capturing groups matched a regular pattern. Use {N} to stand for the matched groups<br/>
   ```
   echo "AAA,111,ccc,222,EEE"|rq -q "select regmatch(@raw,'[^1]*111([^2]*)222[^\n]*','I found \"{1}\" .')"
   ```
   Returns result:<br/>
   ```
   I found ",ccc," .
   ```
- pad(seed, times) - Construct a new string from seed multiple len times<br/>
   ```
   echo ""|rq -q "select pad('abcde',3)"
   ```
   Returns result:<br/>
   ```
   abcdeabcdeabcde
   ```
- switch(expr, case1, return1[, case2, return2 ...][, default]) - if input equal to case1, then return return1, etc.. If none matched, return default or return input if no default provided. Similar to SWITCH CASE statement.<br/>
   ```
   echo "aaa
   bbb
   ccc
   aaa
   ccc"| rq -q "select switch(@raw,'aaa',111,'bbb',222)"
   ```
   Returns result:<br/>
   ```
   111
   222
   ccc
   111
   ccc
   ```
   Another example:<br/>
   ```
   echo "aaa
   bbb
   ccc
   aaa
   ccc"| rq -q "select switch(@raw,'aaa',111,'bbb',222,000)"
   ```
   Returns result:<br/>
   ```
   111
   222
   000
   111
   000
   ```
- greatest(expr1, expr2[, expr3...]) - Return the largest one of the given expressions<br/>
   ```
   echo "aaa,bbb,,ddd,eee"|rq -q "parse /([^,]*),([^,]*),([^,]*),([^,]*),([^,^\n]*)/ | select greatest(@field1,@field2,@field3,@field4,@field5)"
   ```
   Returns result:<br/>
   ```
   eee
   ```
- least(expr1, expr2[, expr3...]) - urn the smallest one of the given expressions<br/>
   ```
   echo "aaa,bbb,ccc,ddd,eee"|rq -q "parse /([^,]*),([^,]*),([^,]*),([^,]*),([^,^\n]*)/ | select least(@field1,@field2,@field3,@field4,@field5)"
   ```
   Returns result:<br/>
   ```
   aaa
   ```
- floor(floatNum) - Get the floor integer number of a given float number<br/>
   ```
   echo "3.1415926"|rq -q "select floor(@raw)"
   ```
   Returns result:<br/>
   ```
   3
   ```
- ceil(floatNum) - Get the ceil integer number of a given float number<br/>
   ```
   echo "3.1415926"|rq -q "select ceil(@raw)"
   ```
   Returns result:<br/>
   ```
   4
   ```
- round(floatNum) - Round a given float number<br/>
   ```
   echo "3.1415926"|rq -q "select round(@raw*10)/10"
   ```
   Returns result:<br/>
   ```
   3
   ```
- log(num) - Get the log result of a given float number<br/>
   ```
   echo "1000"|rq -q "select log(@raw)"
   ```
   Returns result:<br/>
   ```
   3
   ```
- timediff(datetime1,datetime2) - Get the difference (in seconds) of two date<br/>
   ```
   echo "2022-07-29:18:00:00 2022-07-28:08:18:00"|rq -q "parse /([^ ]*) ([^\n]*)/ | select @field1,@field2,timediff(@field1,@field2)"
   ```
   Returns result:<br/>
   ```
   2022-07-29:18:00:00     2022-07-28:08:18:00     121320
   ```
- dateformat(datetime,format) - Convert a date data to a string with the given format<br/>
   ```
   echo "2022-07-29:18:00:00"|rq -q "select @raw,dateformat(@raw,'%d/%b/%Y')"
   ```
   Returns result:<br/>
   ```
   2022-07-29:18:00:00     29/Jul/2022
   ```
- truncdate(datetime,seconds) - Truncate a date a number is multiple of the given second number<br/>
   ```
   echo "2022-07-29:18:56:36"|rq -q "select @raw,truncdate(@raw,3600)"
   ```
   Returns result:<br/>
   ```
   2022-07-29:18:56:36     2022-07-29:18:00:00
   ```
- now() - Get current date time<br/>
   ```
   echo ""|rq -q "select now()"
   ```
   Returns result:<br/>
   ```
   2022-07-29:18:56:36     2022-07-29:18:00:00
   ```
- sum(expr) - Aggregation function. Sum the number of expr<br/>
   ```
   echo "deptA 2022Jun 123
   deptB 2022Jun 22
   deptC 2022Jun 33
   deptA 2022Jul 56
   deptC 2022Jul 78"|rq -q "parse /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | select depar,sum(completed) | group depar"
   ```
   Returns result:<br/>
   ```
   deptA   179
   deptB   22
   deptC   111
   ```
- count(expr) - Aggregation function. Count the number of expr<br/>
   ```
   echo "deptA 2022Jun 123
   deptB 2022Jun 22
   deptC 2022Jun 33
   deptA 2022Jul 56
   deptC 2022Jul 78"|rq -q "parse /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | select depar,count(completed) | group depar"
   ```
   Returns result:<br/>
   ```
   deptA   2
   deptB   1
   deptC   2
   ```
- average(expr) - Aggregation function. Get the average value of expr<br/>
   ```
   echo "deptA 2022Jun 123
   deptB 2022Jun 22
   deptC 2022Jun 33
   deptA 2022Jul 56
   deptC 2022Jul 78"|rq -q "parse /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | select depar,average(completed) | group depar"
   ```
   Returns result:<br/>
   ```
   deptA   89.5
   deptB   22
   deptC   55.5
   ```
- max(expr) - Aggregation function. Get the maximum value of expr<br/>
   ```
   echo "deptA 2022Jun 123
   deptB 2022Jun 22
   deptC 2022Jun 33
   deptA 2022Jul 56
   deptC 2022Jul 78"|rq -q "parse /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | select depar,max(completed) | group depar"
   ```
   Returns result:<br/>
   ```
   deptA   123
   deptB   22
   deptC   78
   ```
- min(expr) - Aggregation function. Get the minimum value of expr<br/>
   ```
   echo "deptA 2022Jun 123
   deptB 2022Jun 22
   deptC 2022Jun 33
   deptA 2022Jul 56
   deptC 2022Jul 78"|rq -q "parse /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | select depar,min(completed) | group depar"
   ```
   Returns result:<br/>
   ```
   deptA   56
   deptB   22
   deptC   33
   ```
- uniquecount(expr) - Aggregation function. Count the number of distinct expr.<br/>
   ```
   echo "   echo "deptA 2022Jun 123
   deptB 2022Jun 22
   deptC 2022Jun 33
   deptA 2022Jul 56
   deptC 2022Jul 78"|rq -q "parse /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | select uniquecount(depar)"

   ```
   Returns result:<br/>
   ```
   3
   ```
