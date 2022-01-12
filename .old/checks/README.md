# Pass / Fail


The following directories contain files meant to test semantic failures and semantic passes.

## semantic failures

The failures directory attempts to generate all possible error codes for specific situations. The files are named as follows: 

  "error"_test_name.tl

When the run.py script is executed, and a path to the titan compiler is supplied, the script will iterate over all of the *.tl files in the directory and ensure the following:

  1) The return code for the titan compiler is '1'
  2) The resulting titan error code (listed in the file name) is present in the output


## semantic passes

Similar to failures, the run.py script executed with a path to the titan compiler will execute all *.tl files. It will ensure the following:

  1) The return code for the titan compiler is '0'
  2) The resulting output does not contain any errors

Features newly added to the analyzer must have a file listed here to ensure that it is working correctly
