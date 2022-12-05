from cProfile import label
from concurrent.futures import thread
import subprocess 
import time
import numpy as np
import matplotlib.pyplot as plt
import math
def plot(threads):
    t = []

    with open('results.txt') as f:
        lines = f.readlines()
    for i in range(len(lines)):
        lines[i] = lines[i].replace(" ", "")
        lines[i] = lines[i].replace("\n", "")
        t.append(float(lines[i]))

    tl = np.arange(0,len(t),1)
    plt.plot(tl,t,label=str(threads))


def testFunction(start, end, n,threads):

    time.sleep(2)
    f = open("results.txt","w")
    f.close()
    processes = []

    temp = np.random.uniform(low=start, high=end, size=(n,))
    temp = np.sort(temp)
    temp = np.around(temp, decimals=3, out=None)
    sampl = []
    for i in range(len(temp)):
        if(i == 0):
            sampl.append(temp[i]-0)
        else:
            sampl.append(temp[i]-temp[i-1])
    
    start = time.time()
    for i in sampl:
        p = subprocess.Popen(["./client",'-k',str(16),"127.0.0.1:8081"])
        processes.append(p)
        time.sleep(i)
    end = time.time()

    for p in processes:
        p.wait()
    
    print("Total running time : ",end - start,"seconds")

for i in range(2,5):
    print("Launch with ",math.pow(2,i)," threads")
    testFunction(0.,2.,2000,math.pow(2,i))
    subprocess.Popen(["fuser",'-k','8081/tcp'])
    time.sleep(2)
    plot(math.pow(2,i))
    plt.yscale("log")
plt.legend()
plt.show()
