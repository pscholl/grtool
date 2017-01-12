% grt-score-tool
%
%

# NAME

grt-score-tool

# SYNOPSIS
grt-score-tool [-h] [-v] [--recursive] [-i] [-r] [-t] [--sql SQL]  
               [--where SQL] [--dump FILE] [--load [FILE [FILE ...]]]  
               [--sqlitefunctions FILE] [--special] [--table NAME]  
               [--limit-columns N] [--limit-rows N]  
               [--select-columns COLS] [-a] [-d] [--order-by COL]  
               [-asc] [-lt N] [-get N] [--file-key KEY] [--format FMT]  
               [files [files ...]]

parse scoring files  
files generated via [grt score -f]

# DESCRIPTION
A database tool for parsing multiple score files created by [grt score -f] into an in-memory sqlite database and printing the formatted contents of the table.

# OPTIONS
positional arguments:
-  files                 files to parse (stdin reads one path per line, e.g. per [find]) (default: None)

optional arguments:
-  -h, --help            show this help message and exit
-  -v, --verbose         be verbose (default: False)
-  --recursive           enable recursive file glob (default: False)
-  -i, --info            print info on loaded table (default: False)
-  -r, --raw             raw output, no tabulate (default: False)
-  -t, --total           only load total scores, no per-class scores (default: False)
-  --sql SQL             perform SQL term on loaded table (default: None)
-  --where SQL           add where clause to sql term (default: None)
-  --dump FILE           dump created database to SQL text format (default: None)
-  --load [FILE [FILE ...]] load database from SQL dump (default: None)
-  --sqlitefunctions FILE SQL extension functions; path to shared object file (default: ./libsqlitefunctions)
-  --special             special function; see code; for advanced usage (default: False)

table arguments:
-  --table NAME          specify the table name (default: scores)
-  --limit-columns N     limit table printing to first N columns (default: 0)
-  --limit-rows N        limit table printing to first N rows (default: 0)
-  --select-columns COLS limit table printing to selected columns, format:
  - 1,2,4,6,...
  - [column1],[column2],...  
overrides --limit-columns (default: None)
-  -a, --aggregate       aggregate on selected columns (default: False)
-  -d, --distinct        get distinct values from selected columns (default: False)
-  --order-by COL        order table by COL (default: None)
-  -asc, --ascending     ascending order instead of descending (default: False)
-  -lt N, --lessthan N   only print rows with [--order-by] value less than N (default: None)
-  -get N, --greaterequalthan N only print rows with [--order-by] value greater or equal than N (default: None)
-  --file-key KEY        if set, will try to break the file names according to the specified key  
format: [sep][key1][sep][key2]...  
exactly one key can be a merged key (as '[key]'), which will merge remaining name even if it contains [sep]  
(default: None)
-  --format FMT          tabulate table format, no effect if --raw (default: simple)

# EXAMPLES

## SQLite extension functions
For aggregating purposes, some functions need to be loaded from an external extenison module since sqlite does not support them natively. The `extension-functions.c` source needs to be compiled, for example on linux systems with the following command:
```
gcc -fPIC -lm -shared extension-functions.c -o libsqlitefunctions.so
```
See the source file for more info and compile commands for other systems. The resulting shared object file can be set for the module via the --sqlitefunctions option.

## file key parsing
For better parsing of multiple score files with special filenames the --file-key option can be used to set a file key. This key will then be used to try and parse each file name into additional columns in the database. For example, suppose a score file is named `KMeans_42_wrist_100_mean`. A file key `_classifier_sequence_modality_window_feature` will then add 5 new columns to the table and populate them with the corresponding values from the file name. Note the prepended additional separator in the key. If the file name includes a single field with values that also include the separator, the option to specify one merged key also exists. For example a file name `this_is_an_example_file_name` will be parsed with the key `_col1_[col2]_col3` into the three columns with the values `col1=this`, `col2=is_an_example_file` and `col3=name`.
