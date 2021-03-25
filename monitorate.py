from subprocess import STDOUT, check_output, TimeoutExpired, CalledProcessError
from os import path
from os.path import isfile
import subprocess
import random
import time

dataFolder="data/"
binFolder="bin/"
srcFolder="src/"

datasets=[16,2048,65536,131072,1048576,1048577,2097152,4194304,8388608]

def runDataset(name, i):
    output=""
    try:
        binFile=binFolder+name
        if not isfile(binFile):
            return "FILE_NOT_FOUND"
        start = time.time()
        output = subprocess.check_output([binFile, dataFolder+"/test"+str(i)], stderr=STDOUT, timeout=15,universal_newlines=True).strip().replace('\n', '')
        result = str(format(time.time() - start, ".4f"))
        with open(dataFolder+"/result"+str(i)) as f:
            if f.readline().strip().replace('\n', '')==output:
                return result
            else:
                return "BAD_RESULT"
    except  CalledProcessError as e :
        output=e.output.strip().replace('\n', '')
        if (e.returncode < 0) :
            return "FILE_NOT_FOUND"
        with open(dataFolder+"/result"+str(i)) as f:
            if f.readline().strip().replace('\n', '')==output:
                return result
            else:
                return "BAD_RESULT"
    except (TimeoutExpired) as e :
        return "TIMED_OUT"

def runProject(name):
    result=""
    for d in datasets:
        generateFiles(d)
        result = result + "====================\n"
        result = result + str(d) + "\n"
        result = result + runDataset(name, d) + "\n"
        result = result + runDataset(name + "-fopenmp", d) + "\n"
    return result

def monitorate(f):
    compile(f, "")
    compile(f, "-fopenmp")
    print(runProject(f))

def generateFiles(num):
    tab = []
    for x in range(num):
        tab.append(random.randint(-10000,10000))

    if (not path.exists("data/test" + str(num))):
        f = open("data/test" + str(num), "a")
        f.truncate(0)

        for nb in tab:
            f.write(str(nb) + " ")

        f.close()

    if (not path.exists("data/result" + str(num))):
        f = open("data/result" + str(num), "a")
        f.truncate(0)

        f.write(maxSubArray(tab))

        f.close()

def maxSubArray(tab):
    start = 0
    end = len(tab)
    max_ending_at_i = max_seen_so_far = tab[start]
    max_left_at_i = max_left_so_far = start
    max_right_so_far = start + 1
    for i in range(start + 1, end):
        if max_ending_at_i > 0:
            max_ending_at_i += tab[i]
        else:
            max_ending_at_i = tab[i]
            max_left_at_i = i
        if max_ending_at_i > max_seen_so_far:
            max_seen_so_far = max_ending_at_i
            max_left_so_far = max_left_at_i
            max_right_so_far = i + 1

    res = str(max_seen_so_far) + " "
    for i in range (max_left_so_far, max_right_so_far):
        res = res + str(tab[i]) + " "

    return res

def compile(f, option):
    name=binFolder + f
    try:
        if (option==""):
            output = subprocess.check_output(["gcc","-std=c99", "-o" , name,  srcFolder+ f+".c" ,"-lm"], stderr=STDOUT, universal_newlines=True).strip().replace('\n', '')
        else:
            output = subprocess.check_output(["gcc","-std=c99", "-o" , name + option,  srcFolder+ f+".c" ,"-lm", option], stderr=STDOUT, universal_newlines=True).strip().replace('\n', '')
        if (output==""):
            resultCompilation=1
    except  CalledProcessError as e :
        print("Compilation error")

monitorate("piedanna")
