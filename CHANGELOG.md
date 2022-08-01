# Change Log
All change logs will be documented here.
   # [0.913] 2022-08-01
   Fixed bug: sorting doesnot work properly when multiple sort keys involved
   # [0.912] 2022-08-01
   Enhancement: Query commands accept one letter abbrivation, e.g. s stands for 'select'
   New feature: Macro function foreach(beginid,endid,macro_expr) is introduced.
   # [0.912] 2022-07-31
   Added Version Info
   Added workflow actions to create CentOS7/8&MacOS rpm and zip files.
   Enhancement: query option is optional now; Set select @raw when no "select" provided.
   # [0.912] 2022-07-29
   New feature: New option: -c|--recursive <yes|no> -- Wheather recursively read subfolder of a folder (default NO).<br/>
   # [0.911] 2022-07-29
   New feature: New option: -d | --detecttyperows How many matched rows will be used for detecting data types, default is 1<br/>
   Added examples.md
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
