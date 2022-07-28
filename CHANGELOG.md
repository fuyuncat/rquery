# Change Log
All change logs will be documented here.
   # [0.911] 2022-07-28
   Fixed bug: LIKE missed quoters
   Fixed bug: SET field data type does not set the correct date format for DATE type.
   # [0.91] 2022-07-27
   New feature: UNIQUE searching command to make the returned result unique.
   New console command: clear: clear all inputs
   # [0.91] 2022-07-26
   New feature: Regmatch returns an expression including the capturing groups matched a regular pattern.
   Fixed bug: the escaped quoter is missing in a string
   Fixed bug: filter incorrectly read quoters in the string
   # [0.903] 2022-07-25
   Fixed Bug: incorrect timezone converting
   Improved date cast performance
   # [0.902] 2022-07-19
   New feature: Using -v or --variable to pass variables in
   Fixed bug: Json format becomes messy when limt N,M involved
   Fixed bug: Selected row number is incorrect when limt N,M involved
   Fixed compile warnings
   # [0.901] 2022-07-18
   Fixed compile warnings
   New feature: added switch,pad,greatest,least functions 
   # [0.90] 2022-07-18
   New feature: added isnull(arg) function 
   # [0.893] 2022-07-18
   New feature: Select can have a alias name. Syntax: select <expression> as <alias> ...
   # [0.892] 2022-07-17
   Fix defect: Unable to handle aggregation function (only) without group field
   # [0.892] 2022-07-15
   Hot fix: replace bug; nested function calls
   Hot fix: infinite loop
   # [0.891] 2022-07-15
   Fix bug: quoters mismatched; Added Regreplace function
   Fix bug: failed to do reglike comparing
   # [0.89] 2022-07-14
   Json output format
   Added log file option; added progress option
   Huge performance improvement! Reduced the temp data set size of grouping.
   Implement Hash Map
   # [0.88] 2022-07-11
   File process bug fix
   Console bug fix
   Added Console mode
   # [0.01] 2022-07-08
   Project initializing.
