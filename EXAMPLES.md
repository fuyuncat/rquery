# Functions usage
- isnull(expr) : Normal function. Return the position of a sub string in a string. Return -1 if caannot find the sub string<br/> 
   ```
   echo "aaa,bbb,,ddd,eee"|rq -f on -q "p /([^,]*),([^,]*),([^,]*),([^,]*),([^,^\n]*)/ | s @field1 as f1,isnull(@field1) as f1n,@field3 as f3,isnull(@field3) as f3n"
   ```
   Returns result:<br/>
   ```
   f1      f1n     f3      f3n
   --      ---     --      ---
   aaa     0               1
   Pattern matched 1 line(s).
   Selected 1 row(s).
   ```
- upper(str) : Normal function. Convert a string to upper case<br/>
   ```
   echo "asdasAWdsfasfsafsdaf, ssadflsdfSOFSF{SFLDF "|rq -q "s upper(@raw)"
   ```
   Returns result:<br/>
   ```
   ASDASAWDSFASFSAFSDAF, SSADFLSDFSOFSF{SFLDF 
   ```
   Anothe example:<br/>
   ```
   echo "aaa,bbb,,ddd,eee"|rq -q "p /([^,]*),([^,]*),([^,]*),([^,]*),([^,^\n]*)/ | s upper(@1)"
   ```
   Returns result:<br/>
   ```
   AAA
   ```
- lower(str) : Normal function. Convert a string to lower case<br/>
   ```
   echo "AAA,BBB,,DDD,EEE"|rq -q "p /([^,]*),([^,]*),([^,]*),([^,]*),([^,^\n]*)/ | s lower(@1)"
   ```
   Returns result:<br/>
   ```
   aaa
   ```
- substr(str,startpos[,len]) : Normal function. Get a substring of a string, start from pos. If len is not provide, get the sub string till the end of the string<br/>
   ```
   echo "AAABBBcccDDDEEE"|rq -q "s substr(@raw,6,3)"
   ```
   Returns result:<br/>
   ```
   ccc
   ```
   Another example,
   ```
   echo "AAABBBcccDDDEEE"|rq -q "s substr(@raw,6)"
   ```
   Returns result:<br/>
   ```
   cccDDDEEE
   ```
- instr(str,substr) : Normal function. Return the position of a sub string in a string. Return -1 if caannot find the sub string<br/>
   ```
   echo "AAABBBcccDDDEEE"|rq -q "s instr(@raw,'ccc')"
   ```
   Returns result:<br/>
   ```
   6
   ```
- strlen(str) : Normal function. Return the length of a string<br/>
   ```
   echo "AAABBBcccDDDEEE"|rq -q "s strlen(@raw,)"
   ```
   Returns result:<br/>
   ```
   15
   ```
- comparestr(str1, str2) : Normal function. Compare str1 to str2, case sensitive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br/>
   ```
   echo "Abc,aBC"|rq -q "p /(?P<col1>[^,]*),(?P<col2>[^,^\n]*)/ | s col1,col2,comparestr(col1,col2)"
   ```
   Returns result:<br/>
   ```
   Abc     aBC     -32
   ```
- nocasecomparestr(str1, str2) : Normal function. Compare str1 to str2, case insensive, return -1 if str1 less than str2, return 0 if str1 equal to str2, return 1 if str1 greater than str2<br/>
   ```
   echo "Abc,aBC"|rq -q "p /(?P<col1>[^,]*),(?P<col2>[^,^\n]*)/ | s col1,col2,nocasecomparestr(col1,col2)"
   ```
   Returns result:<br/>
   ```
   Abc     aBC     0
   ```
- replace(str, sub, new) : Normal function. Replace all sub1 in a string with sub2<br/>
   ```
   rq -q "s replace(@raw, 'aaa','111','bbb','222','ccc','333')" "sfasfaaadfscccasdfasfbbbsafsaaadf"
   ```
   Returns result:<br/>
   ```
   sfasf111dfs333asdfasf222safs111df
   ```
- regreplace(str, pattern, new) : Normal function. Replace all regular pattern in a string with sub (capturing group supported).<br/>
   ```
   echo "AAABBBcccDDDEEE"|rq -q "s regreplace(@raw,'(AAA|EEE)','333')"
   ```
   Returns result:<br/>
   ```
   333BBBcccDDD333
   ```
- regmatch(str, pattern, return_expr) : Normal function. Return an expression including the capturing groups matched a regular pattern. Use {N} to stand for the matched groups<br/>
   ```
   echo "AAA,111,ccc,222,EEE"|rq -q "s regmatch(@raw,'[^1]*111([^2]*)222[^\n]*','I found \"{1}\" .')"
   ```
   Returns result:<br/>
   ```
   I found ",ccc," .
   ```
- regcount(str,pattern) : Normal function. Get the number of regular pattern matchs in a string.<br />
   ```
   rq -q "s regcount('11.0.asf.12.12.12.12','\d+')" " "
   ```
   Returns result:<br/>
   ```
   6
   ```
- regget(str,pattern,idxnum[, matchseq]) : Normal function. Get the regular pattern matched string with specific sequence number in a string. if idxnum is a negtive number, it will start searching from the end of the string. The default value of matchseq is 0, which means return the whole matched string, it can also be a negative number.<br />
   ```
   rq -q "s regget('11.0.asf.12.12.18.12','(\d+)\.(\d+)',-1,2)" " "
   ```
   Returns result:<br/>
   ```
   12
   ```
- pad(seed, times) : Normal function. Construct a new string from seed multiple len times<br/>
   ```
   echo ""|rq -q "s pad('abcde',3)"
   ```
   Returns result:<br/>
   ```
   abcdeabcdeabcde
   ```
- countword(str,[ingnore_quoters]) : Normal function. Get the number of word in a string. Any substring separated by space/tab/newline/punctuation marks will be count as a word. if ingnore_quoters (in pairs, e.g. ''"") provided, the whole string between quoters will be counted as one word<br/>
   ```
   echo "'adasd wqe ' aaa,weq;\"qqeq?eqe12\" qw" | ./rq -q "s countword(@raw,'\'\'\"\"')"
   ```
   Returns result:<br/>
   ```
   5
   ```
- getword(str,wordnum,[ingnore_quoters]) : Normal function. Get a word specified sequence number in a string. Any substring separated by space/tab/newline/punctuation marks will be count as a word. if ingnore_quoters (in pairs, e.g. ''"") provided, the whole string between quoters will be counted as one word<br/>
   ```
   echo "'adasd wqe ' aaa,weq;\"qqeq?eqe12\" qw" | ./rq -q "s getword(@raw,3,'\'\'\"\"')"
   ```
   Returns result:<br/>
   ```
   weq
   ```
- getpart(str,delimiter,part_index[,quoters]) : Normal function. Get a part of a string splitted by delimiter. The optional parameter quoters defines the the quoters, it can be multiple pairs, the delimiters between the quoters will be skipped.<br/>
   ```
   rq -q "s getpart(@raw,'0#0',2)" "sfsaf0#0qqq0#0aaa"
   ```
   Returns result:<br/>
   ```
   qqq
   ```
- getparts(str,delimiter,startindex[,endindex][,quoters]) : Normal function. Get a group of parts of a string splitted by delimiter, and concatenant them using the delimiter. if startindex/endindex is a negtive number, it will start searching from the end of the string, if the calculated endindex is located before startindex, it will reverse concatenant the parts. The optional parameter quoters defines the the quoters, it can be multiple pairs, the delimiters between the quoters will be skipped.<br />
   ```
   rq -q "s getparts('/usr/lib/boost/lib','/',1, -2)" " "
   ```
   Returns result:<br/>
   ```
   /usr/lib/boost
   ```
- countpart(str,delimiter[,quoters]) : Normal function. Get the number parts in a string splitted by delimiter. if part_index is a negtive number, it will start searching from the end of the string. The optional parameter quoters defines the the quoters, it can be multiple pairs, the delimiters between the quoters will be skipped.<br />
   ```
   rq -q "s countpart(@raw,'0#0','[]')" "sfsaf0#0qqq0#0aa[0#0]a"
   ```
   Returns result:<br/>
   ```
   3
   ```
- countstr(str,substr) : Normal function. count occurences of substr in str.<br/>
   ```
   rq -q "s countstr(@raw,'123')" "asg123aad123,123;;wrew"
   ```
   Returns result:<br/>
   ```
   3
   ```
- findnth(str,sub[,Nth]) : Normal function. Find the position of Nth sub in str, if Nth is positive number, search from head, if Nth is negative, search from tail.<br />
   ```
   rq -q "s findnth(@raw,'me',1)" "meadsdfmeaasf me sfsme adfa me adf"
   ```
   Returns result:<br/>
   ```
   0
   ```
   Another example:<br/>
   ```
   rq -q "s findnth(@raw,'me',-2)" "meadsdfmeaasf me sfsme adfa me adf"
   ```
   Returns result:<br/>
   ```
   21
   ```
- revertstr(str) : Normal function. Convert a string to reverse sequence (e.g. abc -> cba).<br />
   ```
   rq -q "s revertstr(@raw)" "meadsdfmeaasf me sfsme adfa me adf"
   ```
   Returns result:<br/>
   ```
   fda em afda emsfs em fsaaemfdsdaem
   ```
- ascii(char) : Normal function. Get the ascii code of a char.<br />
   ```
   echo " " | rq -q "s ascii('a')"
   ```
   Returns result:<br/>
   ```
   97
   ```
- char(int) : Normal function. Get character of an ascii code number.<br />
   ```
   echo " " | rq -q "s char(97)"
   ```
   Returns result:<br/>
   ```
   a
   ```
- urlencode(url) : Normal function. Encode an URL string.<br />
   ```
   rq -q "s urlencode('https://github.com/fuyuncat/rquery/releases/download/v1.11/rquery-v1.11.el7.x86_64.rpm')" " "
   ```
   Returns result:<br/>
   ```
   https%3a%2f%2fgithub.com%2ffuyuncat%2frquery%2freleases%2fdownload%2fv1.11%2frquery-v1.11.el7.x86_64.rpm
   ```
- urldecode(encodedstr) : Normal function. Decode an encoded string to an URL string.<br />
   ```
   rq -q "s urldecode('https%3a%2f%2fgithub.com%2ffuyuncat%2frquery%2freleases%2fdownload%2fv1.11%2frquery-v1.11.el7.x86_64.rpm')" " "
   ```
   Returns result:<br/>
   ```
   https://github.com/fuyuncat/rquery/releases/download/v1.11/rquery-v1.11.el7.x86_64.rpm
   ```
- base64encode(str) : Normal function. Encode a string to base64 code.<br />
   ```
   rq -q "s base64encode('https://github.com/fuyuncat/rquery/releases/download/v1.11/rquery-v1.11.el7.x86_64.rpm')" " "
   ```
   Returns result:<br/>
   ```
   aHR0cHM6Ly9naXRodWIuY29tL2Z1eXVuY2F0L3JxdWVyeS9yZWxlYXNlcy9kb3dubG9hZC92MS4xMS9ycXVlcnktdjEuMTEuZWw3Lng4Nl82NC5ycG0=
   ```
- base64decode(encodedstr) : Normal function. Decode a base64 code string.<br />
   ```
   rq -q "s base64decode('aHR0cHM6Ly9naXRodWIuY29tL2Z1eXVuY2F0L3JxdWVyeS9yZWxlYXNlcy9kb3dubG9hZC92MS4xMS9ycXVlcnktdjEuMTEuZWw3Lng4Nl82NC5ycG0=')" " "
   ```
   Returns result:<br/>
   ```
   https://github.com/fuyuncat/rquery/releases/download/v1.11/rquery-v1.11.el7.x86_64.rpm
   ```
- md5(str) : Normal function. Get the MD5 value of a string.<br />
   ```
   rq -q "s md5('https://github.com/fuyuncat/rquery/releases/download/v1.11/rquery-v1.11.el7.x86_64.rpm')" " "
   ```
   Returns result:<br/>
   ```
   014f8cfb5f5c36ede34a9a5c0593a92f
   ```
- hash(str) : Normal function. Get the hash value of a string.<br />
   ```
   rq -q "s hash('https://github.com/fuyuncat/rquery/releases/download/v1.11/rquery-v1.11.el7.x86_64.rpm')" " "
   ```
   Returns result:<br/>
   ```
   14822156803265260030
   ```
- isip(str) : Normal function. Return 1 if the given str is an IP address, otherwise, return 0.<br />
   ```
   rq -q "s isip('192.168.168.255')" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- isipv6(str) : Normal function. Return 1 if the given str is an IPv6 address, otherwise, return 0.<br />
   ```
   rq -q "s isipv6('fe80::6e:f9ff:ff7a')" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- ismac(str) : Normal function. Return 1 if the given str is a MAC address, otherwise, return 0.<br />
   ```
   rq -q "s ismac('02:6e:f9:78:e9:cc')" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- duplicate(expr, dupNum[, delmiter]) : Normal function. Generate a string of specific number of the expression result.<br />
   ```
   rq -q "s duplicate(@raw, 3, '-')" name
   ```
   Returns result:<br/>
   ```
   name-name-name
   ```
- mod(num,div) : Normal function. Get the mod of a number.<br />
   ```
   echo " " | rq -q "s mod(255,7)"
   ```
   Returns result:<br/>
   ```
   3
   ```
- abs(num) : Normal function. Get the abs value of a given float number.<br />
   ```
   echo " " | rq -q "s abs(-12)"
   ```
   Returns result:<br/>
   ```
   12
   ```
- ceil(floatNum) : Normal function. Get the ceil integer number of a given float number<br/>
   ```
   echo "3.1415926"|rq -q "s ceil(@raw)"
   ```
   Returns result:<br/>
   ```
   4
   ```
- round(floatNum,scale) : Normal function. Round a given float number<br/>
   ```
   rq -q "s round(@raw,2)" "3.1415926"
   ```
   Returns result:<br/>
   ```
   3.14
   ```
- log(num) : Normal function. Get the log result of a given float number<br/>
   ```
   echo "2"|rq -q "s log(@raw)"
   ```
   Returns result:<br/>
   ```
   0.69314718055994529
   ```
- acos(x) : Normal function. Returns the principal value of the arc cosine of x, expressed in radians.<br/>
   ```
   echo "2"|rq -q "s log(@raw)"
   ```
   Returns result:<br/>
   ```
   0.69314718055994529
   ```
- acosh(x) : Normal function. Returns the nonnegative area hyperbolic cosine of x.<br/>
   ```
   echo "2"|rq -q "s log(@raw)"
   ```
   Returns result:<br/>
   ```
   0.69314718055994529
   ```
- asin(x) : Normal function. Returns the principal value of the arc sine of x, expressed in radians.<br/>
   ```
   rq -q "s asin(0.5)*180/3.1415926" " "
   ```
   Returns result:<br/>
   ```
   30.00000051174484
   ```
- asinh(x) : Normal function. Returns the area hyperbolic sine of x.<br/>
   ```
   rq -q "s asinh(3.626860)" " "
   ```
   Returns result:<br/>
   ```
   1.9999998915933479
   ```
- atan(x) : Normal function. Returns the principal value of the arc tangent of x, expressed in radians.<br/>
   ```
   rq -q "s atan(1)*180/3.1415926" " "
   ```
   Returns result:<br/>
   ```
   45.000000767617259
   ```
- atan2(x, y) : Normal function. Returns the principal value of the arc tangent of y/x, expressed in radians.<br/>
   ```
   rq -q "s atan2(10,-10)*180/3.1415926" " "
   ```
   Returns result:<br/>
   ```
   135.00000230285175
   ```
- atanh(x) : Normal function. Returns the area hyperbolic tangent of x.<br/>
   ```
   rq -q "s atanh(0.761594)" " "
   ```
   Returns result:<br/>
   ```
   0.99999962865416925
   ```
- cbrt(x) : Normal function. Returns the cubic root of x.<br/>
   ```
   rq -q "s cbrt(27)" " "
   ```
   Returns result:<br/>
   ```
   3.0000000000000004
   ```
- coypsign(x, y) : Normal function. Returns a value with the magnitude of x and the sign of y.<br/>
   ```
   rq -q "s copysign ( 10.0,-1.0)" " "
   ```
   Returns result:<br/>
   ```
   -10
   ```
- cos(x) : Normal function. Returns the cosine of an angle of x radians.<br/>
   ```
   rq -q "s cos(60*3.1415926/180)" " "
   ```
   Returns result:<br/>
   ```
   0.5000000154700408
   ```
- cosh(x) : Returns the hyperbolic cosine of x.<br/>
   ```
   rq -q "s log(2), cosh(log(2))" " "
   ```
   Returns result:<br/>
   ```
   0.69314718055994529     1.25
   ```
- erf(x) : Normal function. Returns the error function value for x.<br />
   ```
   rq -q "s erf(1)" " "
   ```
   Returns result:<br/>
   ```
   0.84270079294971489
   ```
- exp(x) : Normal function. Returns the base-e exponential function of x, which is e raised to the power x: ex.<br />
   ```
   rq -q "s exp(5)" " "
   ```
   Returns result:<br/>
   ```
   148.4131591025766
   ```
- exp2(x) : Normal function. Returns the base-2 exponential function of x, which is 2 raised to the power x: 2x.<br />
   ```
   rq -q "s exp2(8)" " "
   ```
   Returns result:<br/>
   ```
   256
   ```
- fma(x,y,z) : Normal function. Returns x*y+z.<br />
   ```
   rq -q "s fma(10,20,30)" " "
   ```
   Returns result:<br/>
   ```
   230
   ```
- fmod(numer, denom) : Normal function. Returns the floating-point remainder of numer/denom (rounded towards zero), fmod = numer - tquot * denom, Where tquot is the truncated (i.e., rounded towards zero) result of: numer/denom.<br />
   ```
   rq -q "s fmod(5.3,2)" " "
   ```
   Returns result:<br/>
   ```
   1.2999999999999998
   ```
- fpclassify(x) : Normal function. Returns a value of type of x. INFINITE: Positive or negative infinity (overflow), NAN: Not-A-Number, ZERO: Value of zero, SUBNORMAL: Sub-normal value (underflow), NORMAL: Normal value (none of the above), <br />
   ```
   rq -q "s fpclassify(1/0)" " "
   ```
   Returns result:<br/>
   ```
   INFINITE
   ```
- hypot(x) : Normal function. Returns the hypotenuse of a right-angled triangle whose legs are x and y.<br />
   ```
   rq -q "s hypot(3, 4)" " "
   ```
   Returns result:<br/>
   ```
   5
   ```
- ilogb(x) : Normal function. Returns the integral part of the logarithm of |x|, using FLT_RADIX as base for the logarithm.<br />
   ```
   rq -q "s ilogb(10)" " "
   ```
   Returns result:<br/>
   ```
   3
   ```
- isfinite(x) : Normal function. Returns a non-zero value (true) if x is finite; and zero (false) otherwise.<br />
   ```
   rq -q "s isfinite(1.0/0.0)" " "
   ```
   Returns result:<br/>
   ```
   0
   ```
- isinf(x) : Normal function. Returns a non-zero value (true) if x is an infinity; and zero (false) otherwise.<br />
   ```
   rq -q "s isinf(1.0/0.0)" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- isnormal(x) : Normal function. Returns a non-zero value (true) if x is normal; and zero (false) otherwise.<br />
   ```
   rq -q "s isnormal(1)" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- lgamma(x) : Normal function. Returns the natural logarithm of the absolute value of the gamma function of x.<br />
   ```
   rq -q "s lgamma(0.5)" " "
   ```
   Returns result:<br/>
   ```
   0.57236494292470008
   ```
- log10(x) : Normal function. Returns the common (base-10) logarithm of x.<br />
   ```
   rq -q "s log10(1000)" " "
   ```
   Returns result:<br/>
   ```
   3
   ```
- log2(x) : Normal function. Returns the binary (base-2) logarithm of x.<br />
   ```
   rq -q "s log2(1024)" " "
   ```
   Returns result:<br/>
   ```
   10
   ```
- pow(base, exponent) : Normal function. Returns base raised to the power exponent. <br />
   ```
   rq -q "s pow(32.01, 1.54)" " "
   ```
   Returns result:<br/>
   ```
   208.03669140538651
   ```
- remainder(numer, denom) : Normal function. Returns the floating-point remainder of numer/denom (rounded to nearest): remainder = numer - rquot * denom, Where rquot is the result of: numer/denom, rounded toward the nearest integral value (with halfway cases rounded toward the even number).<br />
   ```
   rq -q "s remainder(5.3, 2)" " "
   ```
   Returns result:<br/>
   ```
   -0.70000000000000018
   ```
- scalbln(x) : Normal function. Scales x by FLT_RADIX raised to the power of n, returning the result of computing: scalbn(x,n) = x * FLT_RADIX^n<br />
   ```
   rq -q "s scalbln(1.5,4)" " "
   ```
   Returns result:<br/>
   ```
   24
   ```
- scalbn(x) : Normal function. Scales x by FLT_RADIX raised to the power of n, returning the same as: scalbn(x,n) = x * FLT_RADIX^n<br />
   ```
   rq -q "s scalbn(1.5,4)" " "
   ```
   Returns result:<br/>
   ```
   24
   ```
- sin(x) : Normal function. Returns the sine of an angle of x radians.<br />
   ```
   rq -q "s sin(30*PI()/180)" " "
   ```
   Returns result:<br/>
   ```
   0.49999999999999994
   ```
- sinh(x) : Normal function. Returns the hyperbolic sine of x.<br />
   ```
   rq -q "s sinh(0.693147)" " "
   ```
   Returns result:<br/>
   ```
   0.74999977430008058
   ```
- sqrt(x) : Normal function. Returns the square root of x.<br />
   ```
   rq -q "s sqrt(1024)" " "
   ```
   Returns result:<br/>
   ```
   32
   ```
- tan(x) : Normal function. Returns the tangent of an angle of x radians.<br />
   ```
   rq -q "s tan(45 * pi() / 180.0)" " "
   ```
   Returns result:<br/>
   ```
   0.99999999999999989
   ```
- tanh(x) : Normal function. Returns the hyperbolic tangent of x.<br />
   ```
   rq -q "s tanh(0.693147)" " "
   ```
   Returns result:<br/>
   ```
   0.59999988444162233
   ```
- tgamma(x) : Normal function. Returns the gamma function of x.<br />
   ```
   rq -q "s tgamma(0.5)" " "
   ```
   Returns result:<br/>
   ```
   1.7724538509055161
   ```
- pi() : Normal function. Returns the value of PI.<br />
   ```
   rq -q "s pi()" " "
   ```
   Returns result:<br/>
   ```
   3.1415926535897931
   ```
- timediff(datetime1,datetime2) : Normal function. Get the difference (in seconds) of two date<br/>
   ```
   echo "2022-07-29:18:00:00 2022-07-28:08:18:00"|rq -q "p /([^ ]*) ([^\n]*)/ | s @1,@2,timediff(@1,@2)"
   ```
   Returns result:<br/>
   ```
   2022-07-29:18:00:00     2022-07-28:08:18:00     121320
   ```
- addtime(date, number, unit) : Normal function. Increase a datetime, unit can be s-second(default),m-minute,h-hour,d-day,n-month,y-year, number can be positive or negative interger.<br/>
   ```
   rq -q "s addtime('2022/08/18:08:00:18 +1000', -1, 'd')" " "
   ```
   Returns result:<br/>
   ```
   2022/08/17:08:00:18 +1000
   ```
- dateformat(datetime,format) : Normal function. Convert a date data to a string with the given format<br/>
   ```
   echo "2022-07-29:18:00:00"|rq -q "s @raw,dateformat(@raw,'%d/%b/%Y')"
   ```
   Returns result:<br/>
   ```
   2022-07-29:18:00:00     29/Jul/2022
   ```
- truncdate(datetime,seconds) : Normal function. Truncate a date a number is multiple of the given second number<br/>
   ```
   echo "2022-07-29:18:56:36"|rq -q "s @raw,truncdate(@raw,3600)"
   ```
   Returns result:<br/>
   ```
   2022-07-29:18:56:36     2022-07-29:18:00:00
   ```
- now([anything]) : Normal function. Get current date time<br/>
   ```
   echo ""|rq -q "s now()"
   ```
   Returns result:<br/>
   ```
   2022-07-29:18:56:36     2022-07-29:18:00:00
   ```
- truncdateu(date,unit) : Normal function. Truncate a date to a specific unit. s:second, m:minute, h:hour, d:day, b:month.<br/>
   ```
   rq -q "s now(), truncdateu(now(),'d')" " "
   ```
   Returns result:<br/>
   ```
   2022-11-18 19:17:31     2022-11-01 00:00:00
   ```
- isleap(date) : Normal function. If the year of the date is a leap year, return 1, otherwise, return 0.<br/>
   ```
   rq -q "s isleap('2022-11-18')" " "
   ```
   Returns result:<br/>
   ```
   0
   ```
- weekday(date) : Normal function. Return the the week day of the date, 1: Monday ... 7: Sunday.<br/>
   ```
   rq -q "s weekday(now())" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- monthfirstday(date) : Normal function. Return the date of the first day of the month.<br/>
   ```
   rq -q "s monthfirstday(now())" " "
   ```
   Returns result:<br/>
   ```
   2022-11-01 00:00:00
   ```
- monthfirstmonday(date) : Normal function. Return the date of the first Monday of the month.<br/>
   ```
   rq -q "s monthfirstmonday(now())" " "
   ```
   Returns result:<br/>
   ```
   2022-11-07 19:19:55
   ```
- yearweek(date) : Normal function. Return the week number of the date in the year.<br/>
   ```
   rq -q "s yearweek(now())" " "
   ```
   Returns result:<br/>
   ```
   46
   ```
- yearday(date) : Normal function. Return the day number of the date in the year.<br/>
   ```
   rq -q "s yearday(now())" " "
   ```
   Returns result:<br/>
   ```
   317
   ```
- datetolong(date) : Normal function. Return the seconds since 1970-01-01 00:00:00.<br/>
   ```
   rq -q "s datetolong(now())" " "
   ```
   Returns result:<br/>
   ```
   1668453667
   ```
- longtodate(seconds) : Normal function. Conver the a number of seconds since 1970-01-01 00:00:00 to a date.<br/>
   ```
   rq -q "s longtodate(1670483858)" " "
   ```
   Returns result:<br/>
   ```
   2022-12-08 07:17:38
   ```
- localtime(date) : Normal function. Convert a time in another timezone (e.g. 1988-08-08 18:18:58 +800) to local time.<br/>
   ```
   rq -q "s localtime('2022-11-18 17:11:22 +0800')" " "
   ```
   Returns result:<br/>
   ```
   2022-11-18 20:11:22
   ```
- gmtime(date, gmtoffset) : Normal function. Convert a local time to another timezone time (e.g. 1988-08-08 18:18:58 +800), gmtoffset's range is from -12 to 12.<br/>
   ```
   rq -q "s now(), gmtime(now(),+8)" " "
   ```
   Returns result:<br/>
   ```
   2022-11-15 12:02:03     2022-11-15 09:02:03 +800
   ```
- random([min,][max]) : Normal function. Generate a random integer. If no parameter provided, the range is from 1 to 100. Providing one parameter means rang from 1 to max.<br/>
   ```
   rq -q "s random()" " "
   ```
   Returns result:<br/>
   ```
   58
   ```
   Another example:<br/>
   ```
   rq -q "s random(101,108)" " "
   ```
   Returns result:<br/>
   ```
   106
   ```
   One more example:<br/>
   ```
   rq -q "s random(8)" " "
   ```
   Returns result:<br/>
   ```
   6
   ```
- randstr(len[,flags]) : Normal function. Generate a random string. len: string length (default 8); flags (default uld) includes: u:upper alphabet;l:lower alphabet;d:digit;m:minus;n:unlderline;s:space;x:special(\`~!@#$%^&\*+/\|;:'"?/);b:Brackets([](){}<>); A lower flag stands for optional choice, a upper flag stands for compulsory choice. <br/>
   ```
   rq -q "s randstr(16,'Udx')" " "
   ```
   Returns result:<br/>
   ```
   P@R32YOM*Z16R3R5`
   ```
- trimleft(str[,char]) : Normal function. Trim all char from left of the string, if char is not provided, all space (including tab) will be trimmed.<br/>
   ```
   echo "aaa,  bbb  " | rq -q "p d/,/ | s @2, '\''+trimleft(@2,' ')+'\''"
   ```
   Returns result:<br/>
   ```
     bbb   'bbb  '
   ```
   One more example:<br/>
   ```
   echo "aaa,|||bbb|||" | rq -q "p d/,/ | s @2, trimleft(@2,'|',0)"
   ```
   Returns result:<br/>
   ```
   |||bbb|||       ||bbb|||
   ```
- trimright(str[,char]) : Normal function. Trim all char from right of the string, if char is not provided, all space (including tab) will be trimmed.<br/>
   ```
   echo "aaa,  bbb  " | rq -q "p d/,/ | s @2, '\''+trimright(@2,' ')+'\''"
   ```
   Returns result:<br/>
   ```
     bbb   '  bbb'
   ```
   One more example:<br/>
   ```
   echo "aaa,|||bbb|||" | rq -q "p d/,/ | s @2, trimright(@2,'|',0)"
   ```
   Returns result:<br/>
   ```
   |||bbb|||       |||bbb||
   ```
- trim(str[,char]) : Normal function. Trim all char from the string, if char is not provided, all space (including tab) will be trimmed.<br/>
   ```
   echo "aaa,  bbb  " | ./rq -q "p d/,/ | s @2, '\''+trim(@2,' ')+'\''"
   ```
   Returns result:<br/>
   ```
     bbb   'bbb'
   ```
   One more example:<br/>
   ```
   echo "aaa,|||bbb|||" | rq -q "p d/,/ | s @2, trim(@2,'|',0)"
   ```
   Returns result:<br/>
   ```
   |||bbb|||       ||bbb||
   ```
- camelstr(str) : Normal function. Convert a string to camel string (First letter of each word is upper case).<br />
   ```
   rq -q "s camelstr(@raw)" "asdfWd,sfsea2123wd"
   ```
   Returns result:<br/>
   ```
   Asdfwd,Sfsea2123Wd
   ```
- snakestr(str) : Normal function. Convert a string to snake string (First letter of each sentence is upper case).<br />
   ```
   rq -q "s snakestr(@raw)" "asdfWd,sfsea2123wd"
   ```
   Returns result:<br/>
   ```
   Asdfwd,sfsea2123wd
   ```
- fieldname(fieldid) : Normal function. Return the filed name of a field (column).<br />
   ```
   rq -n -q "p d/,/|s @1,@2,@3, foreach(4,%,switch(comparenum($,0),1,fieldname(#)+':'+$,'')) | f anycol(4,%,tofloat($))>0" samples/numbers.csv
   ```
   Returns result:<br/>
   ```
   2000-01-29      3PXI5   37685                                                                           METRIC_A:1      METRIC_B:22.37  METRIC_C:23.91
   2000-01-29      3PXI6   37686                                                                           METRIC_A:1      METRIC_B:30.00  METRIC_C:40.14
   2000-01-29      3PXJ1   37691                                                                           METRIC_A:1      METRIC_B:25.00  METRIC_C:51.13
   ```
- concat(str1,str2,[...]) : Normal function. Concatenate multiple strings. <br />
   ```
   rq -q "s concat(@1,'+',@2,'+',@3)" "aaa bbb ccc"
   ```
   Returns result:<br/>
   ```
   aaa+bbb+ccc                                                                           METRIC_A:1      METRIC_B:25.00  METRIC_C:51.13
   ```
- concatcol(start,end,expr[,step,delmiter]) : Normal function (Macro function implemented). Concatenate multiple field expressions. $ stands for GROUP expression when GROUP involved), # stands for field sequence, % stands for the largest field sequence ID, % can be involved in an expression.<br />
   ```
   rq -q "s concatcol(1,%,$,1,',')" "aaa bbb ccc"
   ```
   Returns result:<br/>
   ```
   aaa,bbb,ccc
   ```
- calcol(start,end,expr[,step,operation]) : Normal function (Macro function implemented). Caluclate multiple field expressions. $ stands for GROUP expression when GROUP involved), # stands for field sequence, % stands for the largest field sequence ID, % can be involved in an expression. Operations can be SUM/AVERAGE/MAX/MIN/COUNT/UNIQUECOUNT.<br />
   ```
   rq -q "s calcol(1,%,$,1,'sum')" "111 222 333"
   ```
   Returns result:<br/>
   ```
   666
   ```
- comparenum(num1,num2) : Normal function. Compare two numbers, return -1 if num1 less than num2, return 0 if num1 equal to num2, return 1 if num1 greater than num2<br />
   ```
   rq -q "s comparenum(5.8,1.8)" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- comparedate(date1,date2[,dateformat]) : Normal function. Compare two dates, return -1 if date1 less than date2, return 0 if date1 equal to date2, return 1 if date1 greater than date2. date1 and date2 should have the same dateformat.<br />
   ```
   rq -q "s comparedate('2022/Aug/18 18:18:58', '2022/Aug/18 18:58:18')" " "
   ```
   Returns result:<br/>
   ```
   -1
   ```
- toint(str) : Normal function. Conver a string to integer.<br />
   ```
   rq -q "s toint('888')" " "
   ```
   Returns result:<br/>
   ```
   888
   ```
- tolong(str) : Normal function. Conver a string to long.<br />
   ```
   rq -q "s tolong('888')" " "
   ```
   Returns result:<br/>
   ```
   888
   ```
- tofloat(str) : Normal function. Conver a string to float.<br />
   ```
   rq -q "s tofloat('888.888')" " "
   ```
   Returns result:<br/>
   ```
   888.888
   ```
- tostr(expr) : Normal function. Conver any data type to string.<br />
   ```
   rq -q "s datatype(tostr(888)), tostr(888)" " "
   ```
   Returns result:<br/>
   ```
   STRING  888
   ```
- todate(str[,dateformat]) : Normal function. Conver any string to date.<br />
   ```
   rq -q "s rq -q "s datatype(todate('2022/Aug/18 15:18:58'))" " "" " "
   ```
   Returns result:<br/>
   ```
   DATE
   ```
   Another example:<br/>
   ```
   rq -q "s datatype(todate('1988/08/18','%Y/%m/%d'))" " "
   ```
   Returns result:<br/>
   ```
   DATE
   ```
- dectohex(num) : Normal function. Conver a decimal number to hex formatted string.<br />
   ```
   rq -q "s dectohex(888)" " "
   ```
   Returns result:<br/>
   ```
   378
   ```
- hextodec(str) : Normal function. Conver a hex number to decimal number.<br />
   ```
   rq -q "s hextodec(378)" " "
   ```
   Returns result:<br/>
   ```
   888
   ```
- dectobin(num) : Normal function. Conver a decimal number to binary formatted string.<br />
   ```
   rq -q "s dectobin(888)" " "
   ```
   Returns result:<br/>
   ```
   1101111000
   ```
- bintodec(str) : Normal function. Conver a binary number to decimal number.<br />
   ```
   rq -q "s bintodec('1101111000')" " "
   ```
   Returns result:<br/>
   ```
   888
   ```
- datatype(expr) : Normal function. Return the date type of the expression.<br/>
   ```
   echo " " | ./rq -q "s 8+1.32e+8, datatype(1.32e+8), datatype('2009-12-08')"
   ```
   Returns result:<br/>
   ```
   132000008       DOUBLE  DATE
   ```
- detectdt(str) : Normal function. Detect the data type of a string.<br/>
   ```
   rq -q "s detectdt('2022-11-18')" " "
   ```
   Returns result:<br/>
   ```
   DATE
   ```
- islong(str) : Normal function. Check if a string can be a long value.<br/>
   ```
   rq -q "s islong('11.0')" " "
   ```
   Returns result:<br/>
   ```
   0
   ```
- isdouble(str) : Normal function. Check if a string can be a double value.<br/>
   ```
   rq -q "s isdouble('11.0')" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- isdate(str) : Normal function. Check if a string can be a date value.<br/>
   ```
   rq -q "s isdate('2022-11-18')" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- isstring(str) : Normal function. Check if a string can be a string value.<br/>
   ```
   rq -q "s isstring('11.0')" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- when(condition1,return1[,condition2,return2...],else): Normal function. if condition1 is fulfilled, then return return1, etc.. If none matched, return "else".<br />
   ```
   rq -q "s when(@1>0 and 1=1,@2,@3)" "1 aaa bbb"
   ```
   Returns result:<br/>
   ```
   aaa
   ```
- switch(expr, case1, return1[, case2, return2 ...][, default]) : Normal function. if input equal to case1, then return return1, etc.. If none matched, return default or return input if no default provided. Similar to SWITCH CASE statement.<br/>
   ```
   echo "aaa
   bbb
   ccc
   aaa
   ccc"| rq -q "s switch(@raw,'aaa',111,'bbb',222)"
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
   ccc"| rq -q "s switch(@raw,'aaa',111,'bbb',222,000)"
   ```
   Returns result:<br/>
   ```
   111
   222
   000
   111
   000
   ```
- greatest(expr1, expr2[, expr3...]) : Normal function. Return the largest one of the given expressions<br/>
   ```
   echo "aaa,bbb,,ddd,eee"|rq -q "p /([^,]*),([^,]*),([^,]*),([^,]*),([^,^\n]*)/ | s greatest(@1,@2,@3,@4,@5)"
   ```
   Returns result:<br/>
   ```
   eee
   ```
   Another example:<br/>
   ```
   rq -n -q "p d/\t/ | s foreach(1,%,$), greatest(foreach(2,%,$))  " samples/matrix.tsv
   ```
   Returns result:<br/>
   ```
   a       0.1     0.5     0.3     0.0     0.5
   b       0.9     0.2     0.4     0.7     0.9
   c       0.2     0.0     0.6     0.5     0.6
   d       0.0     0.5     0.3     0.1     0.5
   ```
- least(expr1, expr2[, expr3...]) : Normal function. urn the smallest one of the given expressions<br/>
   ```
   echo "aaa,bbb,ccc,ddd,eee"|rq -q "p /([^,]*),([^,]*),([^,]*),([^,]*),([^,^\n]*)/ | s least(@1,@2,@3,@4,@5)"
   ```
   Returns result:<br/>
   ```
   aaa
   ```
   Another example:<br/>
   ```
   rq -n -q "p d/\t/ | s foreach(1,%,$), least(foreach(2,%,$))  " samples/matrix.tsv
   ```
   Returns result:<br/>
   ```
   a       0.1     0.5     0.3     0.0     0.0
   b       0.9     0.2     0.4     0.7     0.2
   c       0.2     0.0     0.6     0.5     0.0
   d       0.0     0.5     0.3     0.1     0.0
   ```
- sumall(expr1[,expr2...]) : Normal function. Sumarize the result of the input expressions, the parameter can be a foreach function. <br/>
   ```
   rq -n -q "p d/\t/ | s foreach(1,%,$), round(sumall(foreach(2,%,$)),1)  " samples/matrix.tsv 
   ```
   Returns result:<br/>
   ```
   a       0.1     0.5     0.3     0.0     0.9
   b       0.9     0.2     0.4     0.7     2.2
   c       0.2     0.0     0.6     0.5     1.3
   d       0.0     0.5     0.3     0.1     0.9
   ```
- floor(floatNum) : Normal function. Get the floor integer number of a given float number<br/>
   ```
   echo "3.1415926"|rq -q "s floor(@raw)"
   ```
   Returns result:<br/>
   ```
   3
   ```
- eval(expr_str) : Normal function. Eval the input string as an expression.<br/>
   ```
   echo "(2*10-2)/10.0" | ./rq -q "s eval(@raw)"
   ```
   Returns result:<br/>
   ```
   1.8
   ```
- exec(expr_str) : Normal function. Run a system command and return the result.<br/>
   ```
   rq -q "s @line, exec('date')" " "
   ```
   Returns result:<br/>
   ```
   1       Fri Nov 11 13:32:49 AEDT 2022
   ```
- myips([startseq,[,endseq[,delimiter]]]) : Normal function. Return the IPs of the local machine, startseq is the start sequence number of the IPs, endseq is the end sequence number of the IPs. if startseq/endseq is a negtive number, it will count from the end of the array. delimiter is the character used to separate multiple IPs, default is '|'.<br/>
   ```
   rq -q "s myips()" " "
   ```
   Returns result:<br/>
   ```
   192.168.168.168
   ```
- hostname() : Normal function. Return the hostname of the local machine.<br/>
   ```
   rq -q "s hostname()" " "
   ```
   Returns result:<br/>
   ```
   mylinux.server
   ```
- appendFile(content, file) : Normal function. Append content to a file, return 1 if successed, 0 if failed.<br />
   Split columns of a CSV file to different files:<br/>
   ```
   [ rquery]$ cat samples/Master.CSV
   Datestamp|Date|Code|Status
   20211212|6/6/20|Active|Off
   20220926|6/6/20|Active|Off
   [ rquery]$ ./rq -n -q "P d/\|/ | s foreach(1,%,appendFile($+'\n','/tmp/'+fieldname(#)))" samples/Master.CSV
   1       1       1       1
   1       1       1       1
   [ rquery]$ cat /tmp/Datestamp
   20211212
   20220926
   [ rquery]$ cat /tmp/Date
   6/6/20
   6/6/20
   [ rquery]$ cat /tmp/Code
   Active
   Active
   ```
- isfile(filepath) : Normal function. Return 1 if the given path is a file, otherwise, return 0.<br/>
   ```
   rq -q "s isfile('samples/dupids.txt')" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- isfolder(filepath) : Normal function. Return 1 if the given path is a folder, otherwise, return 0.<br/>
   ```
   rq -q "s isfolder('samples/dupids.txt')" " "
   ```
   Returns result:<br/>
   ```
   0
   ```
- fileexist(filepath) : Normal function. Return 1 if the given filepath exists, otherwise, return 0<br/>
   ```
   rq -q "s fileexist('samples/dupids.txt')" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- filesize(filepath) : Normal function. Return the size of the given file, return -1 if file does not exist.<br/>
   ```
   rq -q "s filesize('samples/dupids.txt')" " "
   ```
   Returns result:<br/>
   ```
   38
   ```
- renamefile(filepath) : Normal function. Rename the file if it exists.<br/>
   ```
   rq -q "s renamefile('samples/aaaaaa', 'samples/bbbbbb')" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- rmfile(filepath) : Normal function. Remove the file if it exists.<br/>
   ```
   rq -q "s rmfile('samples/bbbbbb')" " "
   ```
   Returns result:<br/>
   ```
   1
   ```
- rcount([sideid][,fieldid][,value_expr]) : Normal function. Return the size of the side work data set. sideid is the id the side work, fieldid is id the field in the side work, value_expr is the value of a member. If no parameter is provided, it will return the number of side works; if only sideid is provided, it will return the data set size of the specified side work; if only sideid and fieldid, it will return the data set size of the specified side work; if all three parameter are provided, it will return the number of specified member value in the specified field in the data work. <br/>
   ```
   rq -v "r:@1" -q "s @raw, rcount(1,1,@raw) " samples/dupids.txt
   ```
   Returns result:<br/>
   ```
   1231    1
   1231    2
   11      1
   23      1
   11      2
   255     1
   21      1
   121     1
   21      2
   1232    1
   ```
- rmember(sideid,fieldid,seqnum) : Normal function. Return the member value of the specified side work, field id and sequence number. sideid is the id the side work, fieldid is id the field in the side work, seqnum is the member sequence number. <br/>
   ```
   rq -v "r:@1" -q "s @line, @raw, rmember(1,1,@line) " samples/dupids.txt
   ```
   Returns result:<br/>
   ```
   1       1231    1231
   2       1231    1231
   3       11      11
   4       23      23
   5       11      11
   6       255     255
   7       21      21
   8       121     121
   9       21      21
   10      1232    1232
   ```
- rmembers(sideid,fieldid,startseq[,endseq[,delimiter]]) : Normal function. Return the member values of the specified side work, field id and sequence number range. sideid is the id the side work, fieldid is id the field in the side work, startseq is the start sequence number of the members, endseq is the end sequence number of the members. if startseq/endseq is a negtive number, it will count from the end of the array. delimiter is the character used to separate multiple members, default is '|'. <br/>
   ```
   /rq -v "r:@1" -q "s @raw, rcount(1,1,@raw), rmember(1,1,@line), rmembers(1,1,1,@line)" samples/filelink_dupids.txt
   ```
   Returns result:<br/>
   ```
   1231    1       1231    1231
   1231    2       1231    1231|1231
   11      1       11      1231|1231|11
   23      1       23      1231|1231|11|23
   11      2       11      1231|1231|11|23|11
   255     1       255     1231|1231|11|23|11|255
   21      1       21      1231|1231|11|23|11|255|21
   121     1       121     1231|1231|11|23|11|255|21|121
   21      2       21      1231|1231|11|23|11|255|21|121|21
   1232    1       1232    1231|1231|11|23|11|255|21|121|21|1232
   ```
- rmemberid(sideid,fieldid,value_expr) : Normal function. Return the sequence number of the first matched member value of the specified side work and field id. sideid is the id the side work, fieldid is id the field in the side work, value_expr is the value of a member. <br/>
   ```
   rq -v "r:@raw" -q "s @raw, rmemberid(1,1,@raw) " samples/dupids.txt
   ```
   Returns result:<br/>
   ```
   1231    1
   1231    1
   11      3
   23      4
   11      3
   255     6
   21      7
   121     8
   21      7
   1232    10
   ```
- sum(expr) - Aggregation function. Sum the number of expr<br/>
   ```
   echo "deptA 2022Jun 123
   deptB 2022Jun 22
   deptC 2022Jun 33
   deptA 2022Jul 56
   deptC 2022Jul 78"|rq -q "p /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | s depar,sum(completed) | g depar"
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
   deptC 2022Jul 78"|rq -q "p /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | s depar,count(completed) | g depar"
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
   deptC 2022Jul 78"|rq -q "p /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | s depar,average(completed) | g depar"
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
   deptC 2022Jul 78"|rq -q "p /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | s depar,max(completed) | g depar"
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
   deptC 2022Jul 78"|rq -q "p /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | s depar,min(completed) | g depar"
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
   deptC 2022Jul 78"|rq -q "p /(?P<depar>[^ ]*) (?P<date>[^ ]*) (?P<completed>[^\n]*)/ | s uniquecount(depar)"
   ```
   Returns result:<br/>
   ```
   3
   ```
- grouplist([distinct ]expr[,delimiter][,asc|desc]) - Aggregation function. Combine the specific expr in a group to a string. distinct is a key word to indicate if the elements should be distinct, delimiter is a string to be the delimiter, asc|desc keywords indicate whether do sorting.<br />
   ```
   rq -q "p d/ /\"\"[]/r | s @4,grouplist(distinct 'ip:'+@1,', ',asc),count(1),uniquecount(@1) | g @4" timezone.log
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:10:41 +0700]    ip:192.168.1.1  2       1
   [22/Jul/2022:01:10:41 +1000]    ip:192.168.1.1  1       1
   [22/Jul/2022:01:10:41 +1100]    ip:192.168.1.1  1       1
   [22/Jul/2022:01:10:41 -0700]    ip:192.168.1.2, ip:192.168.1.3  3       2
   [22/Jul/2022:01:10:41 -0800]    ip:192.168.1.1, ip:192.168.1.2, ip:192.168.1.3  3       3
   ```
- Rank([group1[,group2]...];[sort1 [asc|desc][,sort2 [asc|desc]]...]) : Analytic function. The the rank of a sorted expression in a group.<br />
   ```
   rq -q "p d/ /\"\"[]/r | s @4,@1,rank(@4,@1;) | o @4,@1 " timezone.log
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     1
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     2
   [22/Jul/2022:01:10:41 +1000]    192.168.1.1     1
   [22/Jul/2022:01:10:41 +1100]    192.168.1.1     1
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     1
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     2
   [22/Jul/2022:01:10:41 -0700]    192.168.1.3     3
   [22/Jul/2022:01:10:41 -0800]    192.168.1.1     1
   [22/Jul/2022:01:10:41 -0800]    192.168.1.2     2
   [22/Jul/2022:01:10:41 -0800]    192.168.1.3     3
   ```
   Another example:<br/>
   ```
   rq -q "p d/ /\"\"[]/r | s @4,@1,rank(@4,@1),count(1) | g @4,@1 | o @4,@1 " timezone.log
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     1       2
   [22/Jul/2022:01:10:41 +1000]    192.168.1.1     1       1
   [22/Jul/2022:01:10:41 +1100]    192.168.1.1     1       1
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     1       2
   [22/Jul/2022:01:10:41 -0700]    192.168.1.3     2       1
   [22/Jul/2022:01:10:41 -0800]    192.168.1.1     1       1
   [22/Jul/2022:01:10:41 -0800]    192.168.1.2     2       1
   [22/Jul/2022:01:10:41 -0800]    192.168.1.3     3       1
   ```
- Denserank([group1[,group2]...];[sort1 [asc|desc][,sort2 [asc|desc]]...]) : Analytic function. The the dense rank of a sorted expression in a group.<br />
   ```
   rq -q "p d/ /\"\"[]/r | s @4,@1,rank(@4,@1),denserank(@4,@1) | o @4,@2,@3,@1,rank(@4,@1)" timezone.log 
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     1       1
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     2       1
   [22/Jul/2022:01:10:41 +1000]    192.168.1.1     1       1
   [22/Jul/2022:01:10:41 +1100]    192.168.1.1     1       1
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     1       1
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     2       1
   [22/Jul/2022:01:10:41 -0700]    192.168.1.3     3       3
   [22/Jul/2022:01:10:41 -0800]    192.168.1.1     1       1
   [22/Jul/2022:01:10:41 -0800]    192.168.1.2     2       2
   [22/Jul/2022:01:10:41 -0800]    192.168.1.3     3       3
   ```
- Nearby(expr;[sort1 [asc|desc][,sort2 [asc|desc]];distance;default...]) : Analytic function. Get the value of nearby rows, if distance is negative, it returns value of previous N row, if distance is positive, it returns value of next N row.<br />
   ```
   rq -q "p d/ /\"\"[]/r | s @4,@1,nearby(@1;@4,@1;-1;'NULL'),nearby(@1;@4,@1;2;'NULL') | o @1,@4" timezone.log
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     NULL    192.168.1.1
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     192.168.1.1     192.168.1.1
   [22/Jul/2022:01:10:41 +1000]    192.168.1.1     192.168.1.1     192.168.1.1
   [22/Jul/2022:01:10:41 +1100]    192.168.1.1     192.168.1.1     192.168.1.2
   [22/Jul/2022:01:10:41 -0800]    192.168.1.1     192.168.1.1     192.168.1.2
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     192.168.1.1     192.168.1.2
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     192.168.1.2     192.168.1.3
   [22/Jul/2022:01:10:41 -0800]    192.168.1.2     192.168.1.2     192.168.1.3
   [22/Jul/2022:01:10:41 -0700]    192.168.1.3     192.168.1.2     NULL
   [22/Jul/2022:01:10:41 -0800]    192.168.1.3     192.168.1.3     NULL
   ```
- Counta([group1,group2...];expr) : Analytic function. Count the number of expr of each group.<br />
   ```
   rq -q "p d/ /\"\"[]/r | s @4,@1,counta(@4;@1) | o @4,@1 " timezone.log
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     2
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     2
   [22/Jul/2022:01:10:41 +1000]    192.168.1.1     1
   [22/Jul/2022:01:10:41 +1100]    192.168.1.1     1
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     3
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     3
   [22/Jul/2022:01:10:41 -0700]    192.168.1.3     3
   [22/Jul/2022:01:10:41 -0800]    192.168.1.1     3
   [22/Jul/2022:01:10:41 -0800]    192.168.1.2     3
   [22/Jul/2022:01:10:41 -0800]    192.168.1.3     3
   ```
- Uniquecounta([group1,group2...];expr) : Analytic function. Count the number of unique expr of each group.<br />
   ```
   rq -q "p d/ /\"\"[]/r | s @4,@1,uniquecounta(@4;@1) | o @4,@1 " timezone.log
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     1
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     1
   [22/Jul/2022:01:10:41 +1000]    192.168.1.1     1
   [22/Jul/2022:01:10:41 +1100]    192.168.1.1     1
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     2
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     2
   [22/Jul/2022:01:10:41 -0700]    192.168.1.3     2
   [22/Jul/2022:01:10:41 -0800]    192.168.1.1     3
   [22/Jul/2022:01:10:41 -0800]    192.168.1.2     3
   [22/Jul/2022:01:10:41 -0800]    192.168.1.3     3
   ```
- Suma([group1,group2...];expr) : Analytic function. Sum the expr of each group.<br />
   ```
   rq -q "p d/ /\"\"[]/r | s @4,@1,suma(@4;@7) | o @4,@1 " timezone.log
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     344
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     344
   [22/Jul/2022:01:10:41 +1000]    192.168.1.1     172
   [22/Jul/2022:01:10:41 +1100]    192.168.1.1     172
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     516
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     516
   [22/Jul/2022:01:10:41 -0700]    192.168.1.3     516
   [22/Jul/2022:01:10:41 -0800]    192.168.1.1     516
   [22/Jul/2022:01:10:41 -0800]    192.168.1.2     516
   [22/Jul/2022:01:10:41 -0800]    192.168.1.3     516
   ```
- Averagea([group1,group2...];expr) : Analytic function. Caluclate average of expr of each group.<br />
   ```
   rq -q "p d/ /\"\"[]/r | s @4,@1,averagea(@4;@7) | o @4,@1 " timezone.log
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     172
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     172
   [22/Jul/2022:01:10:41 +1000]    192.168.1.1     172
   [22/Jul/2022:01:10:41 +1100]    192.168.1.1     172
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     172
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     172
   [22/Jul/2022:01:10:41 -0700]    192.168.1.3     172
   [22/Jul/2022:01:10:41 -0800]    192.168.1.1     172
   [22/Jul/2022:01:10:41 -0800]    192.168.1.2     172
   [22/Jul/2022:01:10:41 -0800]    192.168.1.3     172
   ```
- maxa([group1,group2...];expr) : Analytic function. Get the maximum value of expr of each group.<br />
   ```
   rq -q "p d/ /\"\"[]/r | s @4,@1,maxa(@4;@1) | o @4,@1 " timezone.log
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     192.168.1.1
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     192.168.1.1
   [22/Jul/2022:01:10:41 +1000]    192.168.1.1     192.168.1.1
   [22/Jul/2022:01:10:41 +1100]    192.168.1.1     192.168.1.1
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     192.168.1.3
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     192.168.1.3
   [22/Jul/2022:01:10:41 -0700]    192.168.1.3     192.168.1.3
   [22/Jul/2022:01:10:41 -0800]    192.168.1.1     192.168.1.3
   [22/Jul/2022:01:10:41 -0800]    192.168.1.2     192.168.1.3
   [22/Jul/2022:01:10:41 -0800]    192.168.1.3     192.168.1.3
   ```
- mina([group1,group2...];expr) : Analytic function. Get the minimum value of expr of each group.<br />
   ```
   rq -q "p d/ /\"\"[]/r | s @4,@1,mina(@4;@1) | o @4,@1 " timezone.log
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     192.168.1.1
   [22/Jul/2022:01:10:41 +0700]    192.168.1.1     192.168.1.1
   [22/Jul/2022:01:10:41 +1000]    192.168.1.1     192.168.1.1
   [22/Jul/2022:01:10:41 +1100]    192.168.1.1     192.168.1.1
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     192.168.1.2
   [22/Jul/2022:01:10:41 -0700]    192.168.1.2     192.168.1.2
   [22/Jul/2022:01:10:41 -0700]    192.168.1.3     192.168.1.2
   [22/Jul/2022:01:10:41 -0800]    192.168.1.1     192.168.1.1
   [22/Jul/2022:01:10:41 -0800]    192.168.1.2     192.168.1.1
   [22/Jul/2022:01:10:41 -0800]    192.168.1.3     192.168.1.1
   ```
- foreach(beginid,endid,macro_expr) : Macro function. make a macro expression list for all fields from beginid to endid. $ stands for field ($ stands for GROUP expression when GROUP involved), # stands for field sequence, % stands for the largest field sequence ID, % can be involved in an expression. For example, foreach(%-2,2,substr($,2,3)+#) will make this expression list: substr(@field{N-2},2,3)+N..,substr(@field3,2,3)+3,substr(@field2,2,3)+2. It can only be used in "select" and "sort". It cannot be a part of expression.<br />
   ```
   rq -q "p d/ /r| select @%,@1,foreach(%-1,%,$) | f @%>=5" test.cpp
   ```
   Returns result:<br/>
   ```
   5               !=      &other){
   5               new     A(other.mya->m_id);
   5               B&      other)
   5               !=      &other){
   5               new     A(other.mya->m_id);
   5               new     A(id);
   8               "%Y%m%d",       &tm);
   5               >       bbb;
   5               =       B();
   9               bbb2[bbb2.size()-1].mya->m_id,  myb.mya->m_id);
   5               %d\n",  bbb2[i].mya->m_id);
   ```
- coltorow(exp1[,exp2 ... ] ) : Macro function. Make the columns to rows. Accept multiple parameter, also accept foreach(). The row number will be the maximum number of parameter of all coltorow functions.<br />
   ```
   rq -q "s @1,coltorow(@2,@3,@4),@5" "aaa bbb ccc ddd eee fff"
   ```
   Returns result:<br/>
   ```
   aaa     bbb     eee
           ccc
           ddd
   ```
   Another example:<br/>
   ```
   rq -f -q "s @1,coltorow(@2,@3,@4) as f2 ,@5" "aaa bbb ccc ddd eee fff"
   ```
   Returns result:<br/>
   ```
   @FIELD1 F2      @FIELD5
   ------- --      -------
   aaa     bbb     eee
           ccc
           ddd
   Pattern matched 0 line(s).
   Selected 0 row(s).
   ```
- anycol(start,end,expr[,step]) : Macro function. Can be used in filter only, to check any field fulfil a condition, e.g. anycol(1,%,$). $ stands for GROUP expression when GROUP involved), # stands for field sequence, % stands for the largest field sequence ID, % can be involved in an expression.>0.<br />
   ```
   rq -q "p d/ /r | s foreach(1,%,$,2) | f anycol(1,%,$,2) > 0" samples/anyall.txt
   ```
   Returns result:<br/>
   ```
   1       123212  0       0       0
   121     -1321   -11
   123     2231    2
   2131    131
   11      12      21
   ```
- allcol(start,end,expr[,step]) : Macro function. Can be used in filter only, to check all field fulfil a condition, e.g. allcol(1,%,$). $ stands for GROUP expression when GROUP involved), # stands for field sequence, % stands for the largest field sequence ID, % can be referred in an expression.<br />
   ```
   rq -q "p d/ /r | s foreach(1,%,$,2) | f allcol(1,%,$,2) > 0" samples/anyall.txt
   ```
   Returns result:<br/>
   ```
   123     2231    2
   2131    131
   11      12      21
   ```
- root(expr) : Hierarchy function. Returns the expression of the root node.<br />
   ```
   rq -n -q "s switch(comparenum(@level,0),1,'+','')+pad('-',@level-1)+@4+'('+@3+')', 'Root:'+root(@4) | h k:@1;p:@2" samples/loop.txt
   ```
   Returns result:<br/>
   ```
   /(Root) Root:/
   +Sub1(Branch)   Root:/
   +-file1(Leaf)   Root:/
   +-file2(Leaf)   Root:/
   +-file3(Leaf)   Root:/
   +Sub2(Branch)   Root:/
   +-file4(Leaf)   Root:/
   +-file5(Leaf)   Root:/
   Loop1(Branch)   Root:Loop1
   +Loop2(Branch)  Root:Loop1
   ```
- parent(expr) : Hierarchy function. Returns the expression of the parent node.<br />
   ```
   rq -n -q "s switch(comparenum(@level,0),1,'+','')+pad('-',@level-1)+@4+'('+@3+')', 'Parent:'+parent(@4) | h k:@1;p:@2" samples/loop.txt
   ```
   Returns result:<br/>
   ```
   /(Root) Parent:
   +Sub1(Branch)   Parent:/
   +-file1(Leaf)   Parent:Sub1
   +-file2(Leaf)   Parent:Sub1
   +-file3(Leaf)   Parent:Sub1
   +Sub2(Branch)   Parent:/
   +-file4(Leaf)   Parent:Sub2
   +-file5(Leaf)   Parent:Sub2
   Loop1(Branch)   Parent:
   +Loop2(Branch)  Parent:Loop1
   ```
- path(expr[,connector]) : Hierarchy function. Returns an path (of the expression) from root to current node, connector is used for connecting nodes, default is '/'.<br />
   ```
   rq -n -q "s switch(comparenum(@level,0),1,'+','')+pad('-',@level-1)+@4+'('+@3+')', 'Full path:'+path(@4,'-') | h k:@1;p:@2" samples/loop.txt
   ```
   Returns result:<br/>
   ```
   /(Root) Full path:/
   +Sub1(Branch)   Full path:/-Sub1
   +-file1(Leaf)   Full path:/-Sub1-file1
   +-file2(Leaf)   Full path:/-Sub1-file2
   +-file3(Leaf)   Full path:/-Sub1-file3
   +Sub2(Branch)   Full path:/-Sub2
   +-file4(Leaf)   Full path:/-Sub2-file4
   +-file5(Leaf)   Full path:/-Sub2-file5
   Loop1(Branch)   Full path:Loop1
   +Loop2(Branch)  Full path:Loop1-Loop2
   ```

# Examples and scenarios
- Get lines containing specific string (equal to grep)<br/>
   ```
   rq -q "select @raw | filter @raw like '*strptime*'" test.cpp
   ```
   Returns result:<br/>
   ```
     printf("strptime is defined!\n");
     char * c = strptime("20220728", "%Y%m%d", &tm);
   ```
- Get lines matched a regular pattern (equal to grep -E or egrep)<br/>
   ```
   rq -q "select @raw | filter @raw reglike '(struct|char)'" test.cpp
   ```
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
- A very simple way to query apache/nginx logs.<br/>
   ```
   rq -q "p d/ /\"\"[]/ | s @6,count(1) |g @6 | o 1 " logs/g_access_log.log
   ```
   Returns result:<br/>
   ```
   200     626
   302     4073
   403     25
   404     59
   406     1
   ```
   Another example: A simple way to search hourly hit counts from apache/nginx logs..<br/>
   ```
   rq -q "p d/ /\"\"[]/ | t @4 date '[%d/%b/%Y:%H:%M:%S %z]' | s truncdate(@4,3600), count(1) | g truncdate(@4,3600) | o 1" logs/g_access_log.log
   ```
   Returns result:<br/>
   ```
   [22/Jul/2022:01:00:00 +0000]    40
   [22/Jul/2022:03:00:00 +0000]    4
   [22/Jul/2022:05:00:00 +0000]    108
   [22/Jul/2022:07:00:00 +0000]    77
   [22/Jul/2022:08:00:00 +0000]    74
   [22/Jul/2022:09:00:00 +0000]    41
   [22/Jul/2022:10:00:00 +0000]    4422
   [22/Jul/2022:11:00:00 +0000]    2
   [22/Jul/2022:12:00:00 +0000]    1
   [22/Jul/2022:16:00:00 +0000]    2
   [22/Jul/2022:18:00:00 +0000]    6
   [22/Jul/2022:19:00:00 +0000]    1
   [22/Jul/2022:23:00:00 +0000]    6
   ```
- Generate a simple hourly load graph by querying apache/nginx logs.<br/>
   ```
   rq -p on -q "p d/ /\"\"[]/ | t @5 date '[%d/%b/%Y:%H:%M:%S %z]' | select dateformat(truncdate(@5,3600))+':'+pad('#',round(count(1)/1000)) | group truncdate(@5,3600) | sort 1 asc " access.log.20220788
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
- Get fields number of each line using delmiter matching a file.<br/>
   ```
   rq -f on -q "p d/ /\"\"/ | select @% as filednum, strlen(@raw) as linelen" access.log
   ```
   Returns result:<br/>
   ```
   filednum        linelen
   --------        -------
   1       65
   11      516
   11      547
   11      542
   11      574
   11      536
   11      523
   11      546
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
- Get the line count of each file in a folder including subfolders<br/>
   ```
   rq -c yes -q "s @file,count(1) | group @file" logs/
   ```
   Returns result:<br/>
   ```
   logs/access.log 692
   logs/g_access_log.loh   4855
   logs/result.lst 722
   logs/sublogs/timezone.log       10
   ```
- Get fields from multibytes content<br/>
   ```
   rq -q "p d/,/ | s @1,@2,@3" multicode.txt
   ```
   Returns result:<br/>
   ```
   en      english Hello
   cn      中文    你好
   kr      한국어    안녕하세요
   jp      日本語  こんにちは
   gr      Deutsch Hallo
   ru      Русский Привет
   ```
- Use wildcard pattern to query a csv file, read the first line as the field names<br/>
   ```
   rq -n y -q "p w/*,*,*/|s Year, Industry_aggregation_NZSIOC" survey_small.csv
   ```
   Returns result:<br/>
   ```
   2012    Level 1
   2013    Level 1
   2015    Level 3
   2016    Level 1
   2017    Level 6
   2018    Level 1
   2019    Level 1
   2020    Level 1
   2021    Level 8
   ```
- Use delmiter pattern to query a file list<br/>
   ```
   ls -l | ./rq -q "p d/ /r|s @1,@2,@9"
   ```
   Returns result:<br/>
   ```
   total   460652
   -rw-------.     1       aaa.txt
   -rw-------.     1       access.log
   -rw-------.     1       apache.log
   -rw-------.     1       commfuncs.cpp
   -rw-------.     1       commfuncs.h
   -rw-------.     1       commfuncs.o
   ...
   ```
- Get the file count number and total size of each user in a specific folder<br/>
   ```
   ls -l $folder/ | rq -q "parse /(\S+)[ ]{1,}(\S+)[ ]{1,}(?P<owner>\S+)[ ]{1,}(?P<group>\S+)[ ]{1,}(?P<size>\S+)[ ]{1,}(\S+)[ ]{1,}(\S+)[ ]{1,}(\S+)[ ]{1,}(\S+)/ | s owner, count(1), sum(size) | g owner"
   ```
   Returns result:<br/>
   ```
   nginx   20      16436
   root    505     3127798
   ```
- Filter empty lines<br/>
   ```
   rq -q "f isnull(@raw)!=1" test.cpp
   ```
   Returns result:<br/>
   ```
   main ()
   {
     struct tm;
   #if __XSI_VISIBLE
     printf("strptime is defined!\n");
     char * c = strptime("20220728", "%Y%m%d", &tm);
   #endif
   }
   ```
- Pre set data type for fields<br/>
   ```
   rq -q "p /\\\"(?P<origip>[^\n]*)\\\" (?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/ | set time date '%d/%b/%Y:%H:%M:%S %z'| s @raw | filter time='29/Jun/2022:16:58:18 +1000'" logs/access.log
   ```
   Returns result:<br/>
   ```
   "123.456.789.111 987.654.321.999" 192.168.1.1 - - [29/Jun/2022:16:58:18 +1000] "GET /index.html HTTP/1.1" 200 - "https://mysite.com.au/index.html" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36"
   "123.456.789.222 987.654.321.888" 192.168.1.1 - - [29/Jun/2022:16:58:18 +1000] "GET /logo.gif HTTP/1.1" 200 - "https://mysite.com.au/index.html" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36"
   "123.456.789.333 987.654.321.777" 192.168.1.1 - - [29/Jun/2022:16:58:18 +1000] "GET /login.php HTTP/1.1" 200 - "https://mysite.com.au/index.html" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36"
   ```
- Set variable and use it in the query<br/>
   ```
   rq -v var:print -q "s @raw | filter @raw like '*'+@var+'*'" test.cpp
   ```
   Returns result:<br/>
   ```
     printf("strptime is defined!\n");
   ```
   Dynamic variables example:<br/>
   ```
   rq -v "v1:0:tolong(@v1)+tolong(switch(@raw,'#####',1,0));v2:1:@v2+@line" -q "s @v1,@v2,@raw" samples/test_file.txt
   ```
   Returns result:<br/>
   ```
   0       1       aaaaa
   0       2       bbbbb
   0       4       ccccc
   0       7       #####
   1       11      ddddd
   1       16      eeeee
   1       22      fffff
   1       29      #####
   ```
- Get all INFO WARNING and ERROR messages and write them to a logfile<br/>
   ```
   rq -m info -l /tmp/rquery.log -v folder:$(pwd) -q "s @raw | filter @raw = @folder/'*'" file
   ```
   Returns result:<br/>
   ```
   cat /tmp/rquery.log
   ERROR:(2)Quoters in 's @raw | filter @raw = @folder/'*'' are not paired!
   ERROR:Operation / is not supported for STRING data type!
   ```
- Show the query progress (Useful for processing large files)<br/>
   ```
   rq -p on -q "p /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter agent reglike '*(Chrome|Firefox)*' | s host, count(1) | g host | sort count(1) desc" a_very_large_file.log
   ```
   Shows progress<br/>
   ```
   2883584 bytes(0.88%) read in 1.888000 seconds.
   ```
- Display the output in jason format<br/>
   ```
   rq -o json -q "p /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|s status, count(1) | g status" access.log
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
   rq -r line -s 1000 -q "s @raw | filter @raw like '*192.168.1.1*'" access.log
   ```
   Rquery will start from line 1001 to process the file<br/>
- Generate 10 usernames and passwords.<br/>
   ```
   rq -q "s @dupid, randstr(2,'ul')+randstr(8,'uld'), randstr(8,'uldnxb')+tostr(random()) | d 10" name
   ```
   Result output<br/>
   ```
   1       toJhx526Bp9t    r8`1Q~{2340
   2       irETD2x1aG2Q    _g)2g#"}826
   3       jxEFRX1XDAvH    '_8I0>IHW98
   4       wWC0ygV65hmi    _:5R_!_Rl92
   5       QzjIap7WFcAl    5K03$[v5H75
   6       ouYiegc3oxQk    IPd_!r>~:22
   7       LGS5mjL6ov2L    0>u[[iba?39
   8       vkSUqasxYmmT    7'2f52I`f96
   9       HskOuBzH6y1O    5cDu_]_M_81
   10      RHFuq4eo4ks2    e_k"/53_$39
   ```
- Detect more rows to get the accurate data types<br/>
   By default, only the first row is chosen to be used to detect the data types. Therefore, below query will treat all rows as LONG.<br/>
   ```
   echo "123
   333
   aaa
   ada
   134" | ./rq -m error -q "s @raw-100 "
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
   134" | ./rq -m error -d 3 -q "s @raw-100 "
   ERROR:Operation - is not supported for STRING data type!
   ERROR:Operation - is not supported for STRING data type!
   ERROR:Operation - is not supported for STRING data type!
   23
   233



   ```
- Query an apache or nginx access log, to get the number of hits from different clients, and the browser is Chrome or Firefox<br/>
   ```
   rq -q "p /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter agent reglike '(Chrome|Firefox)' | s host, count(1) | g host | sort count(1) desc" < access.log
   ```
   Returns result:<br/>
   ```
   10.10.1.111     329
   10.10.2.222     148
   10.10.3.333     2
   ```
- Searching folder logs, to get all access log from 127.0.0.1<br/>
  ```
  rq -q "p /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/|filter host = '127.0.0.1' | s @raw " logs
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
   rq -f on -q "p /(?P<host>\S+) (\S+) (?P<user>\S+) \[(?P<time>[^\n]+)\] \\\"(?P<request>[^\n]*)\\\" (?P<status>[0-9]+) (?P<size>\S+) \\\"(?P<referrer>[^\n]*)\\\" \\\"(?P<agent>[^\n]*)\\\"/| s truncdate(time,3600), count(1) | g truncdate(time,3600)" /var/log/nginx/access.log-20220629
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
- From one file (timezone.log) get rows that only have "hostip" existing in another file (tz1.log) <br/>
   ```
   rq -q "p d/ /\"\"[]/r |m @1 as aaa, @4 as bbb where @fileid=1 | s @1, @4 | f @fileid=2 and @1 in @r[1][aaa]" tz1.log timezone.log
   ```
   Returns result:<br/>
   ```
   192.168.1.2     [22/Jul/2022:01:10:41 -0700]
   192.168.1.1     [22/Jul/2022:01:10:41 +1100]
   192.168.1.1     [22/Jul/2022:01:10:41 -0800]
   192.168.1.1     [22/Jul/2022:01:10:41 +0700]
   192.168.1.1     [22/Jul/2022:01:10:41 +0700]
   192.168.1.1     [22/Jul/2022:01:10:41 +1000]
   192.168.1.2     [22/Jul/2022:01:10:41 -0800]
   192.168.1.2     [22/Jul/2022:01:10:41 -0700]
   ```
- Join query data from three files <br/>
   ```
   rq -q "p d/ /\"\"[]/r |m @1 as aaa, @5 as bbb where @fileid=1;@1 as aaa, @5 as ccc where @fileid=2 | s @file,@fileid,@line,@fileline,@1,@r[2][ccc] | f @fileid=3 and @1=@r[2][aaa]" tz1.log tz2.log timezone.log
   ```
   Returns result:<br/>
   ```
   timezone.log    3       7       1       192.168.1.2     "GET textb_222.pdf"
   timezone.log    3       7       1       192.168.1.2     "GET textc_222.pdf"
   timezone.log    3       8       2       192.168.1.1     "GET texta_111.pdf"
   timezone.log    3       10      4       192.168.1.1     "GET texta_111.pdf"
   timezone.log    3       11      5       192.168.1.1     "GET texta_111.pdf"
   timezone.log    3       12      6       192.168.1.1     "GET texta_111.pdf"
   timezone.log    3       14      8       192.168.1.1     "GET texta_111.pdf"
   timezone.log    3       15      9       192.168.1.2     "GET textb_222.pdf"
   timezone.log    3       15      9       192.168.1.2     "GET textc_222.pdf"
   timezone.log    3       16      10      192.168.1.2     "GET textb_222.pdf"
   timezone.log    3       16      10      192.168.1.2     "GET textc_222.pdf"
   ```
- Join query data on multiple keys <br/>
   ```
   rq -q "m @1, @2, @3 where @fileid=1 | s @1, @2, @r[1][3] | f @fileid=2 and @1=@r[1][1] and @2=@r[1][2]" samples/mkeys1.txt samples/mkmain.txt
   ```
   Returns result:<br/>
   ```
   tom     cat     ttt_ccc
   jerry   mouse   jjj_mmm
   ```
- Using the other two files JOIN query result to join query the main file. <br/>
   ```
   rq -q "p d/ /\"\"[]/r |m @1 as aaa, @5 as bbb where @fileid=1;@1 as aaa, @5 as ccc where @fileid=2 and @1 in @r[1][aaa]  | s @file,@fileid,@line,@fileline,@1,@r[2][ccc] | f @fileid=3 and @1=@r[2][aaa]" tz1.log tz2.log timezone.log
   ```
   Returns result:<br/>
   ```
   timezone.log    3       7       1       192.168.1.2     "GET textb_222.pdf"
   timezone.log    3       8       2       192.168.1.1     "GET texta_222.pdf"
   timezone.log    3       10      4       192.168.1.1     "GET texta_222.pdf"
   timezone.log    3       11      5       192.168.1.1     "GET texta_222.pdf"
   timezone.log    3       12      6       192.168.1.1     "GET texta_222.pdf"
   timezone.log    3       14      8       192.168.1.1     "GET texta_222.pdf"
   timezone.log    3       15      9       192.168.1.2     "GET textb_222.pdf"
   timezone.log    3       16      10      192.168.1.2     "GET textb_222.pdf"
   ```
- Get one row for each distincted IP address from a log file. <br/>
   ```
   rq -q "p d/ /\"\"[]/r |m @1 as aaa where @fileid=1 | s @1, @4, @5 | f @1 noin @r[1][aaa]" tz1.log 
   ```
   Returns result:<br/>
   ```
   192.168.1.2     [22/Jul/2022:01:10:41 -0700]    "GET imagesa_111.jpg"
   192.168.1.1     [22/Jul/2022:01:10:41 +1100]    "GET imagesb_111.jpg"
   ```
- Get the last line ID, 3 columns of all rows that have 5 or more columns.<br />
   Method 1: Using foreach macro function:<br/>
   ```
   rq -q "p d/ /r| select @line,foreach(%-2,%,$) | f @%>=5" test.cpp
   ```
   Returns result:<br/>
   ```
   34      (this   !=      &other){
   38      =       new     A(other.mya->m_id);
   54      operator=(const B&      other)
   56      (this   !=      &other){
   60      =       new     A(other.mya->m_id);
   73      =       new     A(id);
   82      strptime("20220728",    "%Y%m%d",       &tm);
   85      std::string,B   >       bbb;
   88      myb2    =       B();
   96      %d)\n", bbb2[bbb2.size()-1].mya->m_id,  myb.mya->m_id);
   99      id:     %d\n",  bbb2[i].mya->m_id);
   ```
   Method 2: Using @N variable:<br/>
   ```
   rq -q "p d/ /r| select @line,@(N-2),@(N-1),@N | f @%>=5" test.cpp
   ```
   Returns result:<br/>
   ```
   34      (this   !=      &other){
   38      =       new     A(other.mya->m_id);
   54      operator=(const B&      other)
   56      (this   !=      &other){
   60      =       new     A(other.mya->m_id);
   73      =       new     A(id);
   82      strptime("20220728",    "%Y%m%d",       &tm);
   85      std::string,B   >       bbb;
   88      myb2    =       B();
   96      %d)\n", bbb2[bbb2.size()-1].mya->m_id,  myb.mya->m_id);
   99      id:     %d\n",  bbb2[i].mya->m_id);
   ```
- Split the search result into different files named after a field content. <br/>
   ```
   rq -p on -q "s @raw|>'/tmp/'+@1+'.log'" logs/g_access_log.loh
   ```
   Returns result:<br/>
   ```
   'logs/g_access_log.loh': 1165657 bytes(100.00%) read in 0.270000 seconds.
   [rquery]$ wc -l /tmp/10.65.12.181.log
   2105 /tmp/10.65.12.181.log
   [rquery]$ wc -l /tmp/10.65.22.232.log
   2679 /tmp/10.65.22.232.log
   [rquery]$ wc -l logs/g_access_log.loh
   4784 logs/g_access_log.loh
   ```
- Further filter the reseult set using extrafilter <br/>
   Without extrafilter:<br/>
   ```
   [ rquery]$ ./rq -q "s @1, count(1) | g @1" timezone.log
   192.168.1.1     5
   192.168.1.2     3
   192.168.1.3     2
   ```
   Using extrafilter:<br/>
   ```
   [ rquery]$ ./rq -q "s @1, count(1) | g @1 | e @2>=3 trim @1+':'+@2" timezone.log
   192.168.1.1:5
   192.168.1.2:3
   ```
- Show hierarchy (tree) structure <br/>
   Hierarchy data:<br/>
   ```
   [ rquery]$ cat samples/tree.txt
   ID PID Name Value
   1 0 Root /
   2 1 Branch Sub1
   3 1 Branch Sub2
   4 2 Leaf file1
   5 2 Leaf file2
   6 2 Leaf file3
   7 3 Leaf file4
   8 3 Leaf file5
   ```
   Show the tree structure:<br/>
   ```
   [ rquery]$ ./rq -n -q "s switch(comparenum(@level,0),1,'+','')+pad('-',@level-1)+@4+'('+@3+')' | h k:@1;p:@2" samples/tree.txt
   /(Root)
   +Sub2(Branch)
   +-file5(Leaf)
   +-file4(Leaf)
   +Sub1(Branch)
   +-file3(Leaf)
   +-file2(Leaf)
   +-file1(Leaf)
   ```
- Generate a summary report for those selected output rows <br/>
   ```
   rq -n -q "p d/\t/ | s foreach(1,%,$) | r 2:sum,3:average,4:count,5:max | l 2,3" samples/matrix.tsv
   ```
   Return reseult:<br/>
   ```
   b       0.9     0.2     0.4     0.7
   c       0.2     0.0     0.6     0.5
           1.10    0.10    2.00    0.70
   ```
- Use @R as a dynamic variable to get the unique data <br/>
   Original data:<br/>
   ```
   [ rquery]$ cat samples/dupids.txt
   1231
   1231
   11
   23
   11
   255
   21
   121
   21
   1232
   ```
   Filtered reseult:<br/>
   ```
   [ rquery]$ rq -n -v "r:@v;v:0:@1" -q "s @raw | f @1 noin @r[1][1]" samples/dupids.txt
   1231
   11
   23
   255
   21
   121
   1232
   ```
- Use @R as a dynamic variable to get the duplicated data <br/>
   Original data:<br/>
   ```
   [ rquery]$ cat samples/dupids.txt
   1231
   1231
   11
   23
   11
   255
   21
   121
   21
   1232
   ```
   Filtered reseult:<br/>
   ```
   [ rquery]$ ./rq -v "r:@v;v:0:@1" -q "s @raw | f @1 in @r[1][1]" samples/dupids.txt -m error
   1231
   11
   21
   ```
-- Find out the top Swap usage processes <br/>
   ```
   # rq -v "pid:1:trim(getpart(@file,'/',2))" -q "s @pid, @2, when(isnull(@pid), '', exec('ps -p '+@pid+' -o args | grep -v COMMAND')) | f trim(@1)='VmSwap:' and @pid reglike '\d+' " /proc/*/status -m error | sort -k 2nr
   1286    9676    /usr/lib/polkit-1/polkitd --no-debug
   13231   5872    php-fpm: master process (/etc/php-fpm.conf)
   13236   4516    php-fpm: pool www
   1184    4476    php-fpm: pool www
   29363   4288    /usr/bin/python3 -c from pyovpn.log.logworker import start ; start()
   13243   4280    php-fpm: pool www
   13237   4272    php-fpm: pool www
   29465   4268    php-fpm: pool www
   13235   4252    php-fpm: pool www
   13238   4176    php-fpm: pool www
   ...
   ```   
-- Define macro function to make the query commands simple and clear <br/>
   ```
   # rq -u "myfunc:substr(~a~,instr(~a~,'.')+1,~b=3~)" -q "s myfunc(@1,5), myfunc(@2)" "asds.12345678 aasf.abcdefgh"
   12345   abc
   ```   
