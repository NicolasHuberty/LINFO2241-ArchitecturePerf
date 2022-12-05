import numpy as np
import matplotlib.pyplot as plt


t = []

with open('results8.txt') as f:
    lines = f.readlines()
for i in range(len(lines)):
    lines[i] = lines[i].replace(" ", "")
    lines[i] = lines[i].replace("\n", "")
    t.append(float(lines[i]))
t2 = []

with open('resultssimd8.txt') as f:
    lines = f.readlines()
for i in range(len(lines)):
    lines[i] = lines[i].replace(" ", "")
    lines[i] = lines[i].replace("\n", "")
    t2.append(float(lines[i]))

t3 = []
with open('results128.txt') as f:
    lines = f.readlines()
for i in range(len(lines)):
    lines[i] = lines[i].replace(" ", "")
    lines[i] = lines[i].replace("\n", "")
    t3.append(float(lines[i]))

t4 = []
with open('resultssimd128.txt') as f:
    lines = f.readlines()
for i in range(len(lines)):
    lines[i] = lines[i].replace(" ", "")
    lines[i] = lines[i].replace("\n", "")
    t4.append(float(lines[i]))

tl = np.arange(0,len(t),1)
def smooth(y, box_pts):
    box = np.ones(box_pts)/box_pts
    y_smooth = np.convolve(y, box, mode='same')
    return y_smooth
plt.plot(tl[:900],np.convolve(t,np.ones(25)/25,mode='same')[:900],alpha=0.4,color="b",label="Optimized server key size 8")
plt.plot(tl[:900],np.convolve(t2,np.ones(25)/25,mode='same')[:900],"r-",alpha=0.4,label="Optimized Server with SIMD key size 8")
plt.plot(tl[:900],np.convolve(t3,np.ones(25)/25,mode='same')[:900],color="b",label="Optimized Server key size 128")
plt.plot(tl[:900],np.convolve(t4,np.ones(25)/25,mode='same')[:900],"r-",label="Optimized Server with SIMD key size 128")
plt.title("Answer time with a key size of 1024")
#plt.yticks(np.arange(0, max(t)+1, 1.0))
#plt.yscale('log')
plt.ylabel("Execution Time (s)")
plt.xlabel("Clients")
plt.legend()
plt.savefig("graph.png")