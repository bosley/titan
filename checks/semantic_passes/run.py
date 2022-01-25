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
    result = subprocess.run([titan, "-a", "-n", item], stdout=subprocess.PIPE)
    
    unexpected = "Error : "
    if unexpected in result.stdout.decode("utf-8") :
        print("[FAIL]", item, "Error(s) in file")
        print(result.stdout.decode("utf-8"))
        exit(1)

    if result.returncode != 0:
        print("[FAIL]", item, "Incorrect return code")
        exit(1)

    print("[PASS]", item)

check_list = glob.glob("*.tl")

for item in check_list:
    run_item(item)

exit(0)
