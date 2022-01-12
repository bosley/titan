import re
import glob
import sys
import subprocess

if len(sys.argv) < 2:
    print("Expected path to titan binary")
    exit(1)

titan = sys.argv[1]

print(titan)

def indicate(result, test):
    print(result, test)

def run_item(item):
    print("-"*10)
    error_code = item.split("_")[0]
    result = subprocess.run([titan, item], stdout=subprocess.PIPE)
    
    if result.returncode != 1:
        print("[FAIL]", item, "Incorrect return code")
        exit(1)

    expected = "Error : " + error_code
    if expected not in result.stdout.decode("utf-8") :
        print("[FAIL]", item, "Incorrect error code")
        exit(1)

    print("[PASS]", item)

check_list = glob.glob("*.tl")

for item in check_list:
    run_item(item)

exit(0)
