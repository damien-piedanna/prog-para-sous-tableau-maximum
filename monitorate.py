from subprocess import STDOUT, check_output, TimeoutExpired, CalledProcessError
from os import path
from os.path import isfile
import subprocess
import random
import time

dataFolder="data/"
binFolder="bin/"
srcFolder="src/"

datasets=[2048,65536,131072,1048576,2097152]

def runDataset(name, i):
    output=""
    try:
        binFile=binFolder+name
        if not isfile(binFile):
            return "FILENOTFOUND"
        start = time.time()
        subprocess.check_output([binFile, dataFolder+"/test"+str(i)], stderr=STDOUT, timeout=15,universal_newlines=True).strip().replace('\n', '')
        return str(format(time.time() - start, ".4f"))
    except (TimeoutExpired) as e :
        return "ERR"

def runProject(name):
    result=""
    for d in datasets:
        generateTestFile(d)
        result = result + "====================\n"
        result = result + str(d) + "\n"
        result = result + runDataset(name, d) + "\n"
        result = result + runDataset(name + "-fopenmp", d) + "\n"
    return result

def monitorate(f):
    compile(f, "")
    compile(f, "-fopenmp")
    print(runProject(f))

def generateTestFile(num):
    if (not path.exists("data/test" + str(num))):
        f = open("data/test" + str(num), "a")
        f.truncate(0)

        for x in range(num):
            f.write(str(random.randint(-10000,10000)) + " ")

        f.close()

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
