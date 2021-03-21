import random
import sys

f = open("data/test_" + sys.argv[1], "a")
f.truncate(0)

for x in range(int(sys.argv[1])):
    f.write(str(random.randint(int(sys.argv[2]),int(sys.argv[3]))) + " ")

f.close()