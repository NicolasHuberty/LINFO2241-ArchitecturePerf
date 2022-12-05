import numpy as np
import matplotlib.pyplot as plt


t = []

with open('results.txt') as f:
    lines = f.readlines()
for i in range(len(lines)):
    lines[i] = lines[i].replace(" ", "")
    lines[i] = lines[i].replace("\n", "")
    t.append(float(lines[i]))

tl = np.arange(0,len(t),1)
plt.plot(tl,t,"r-")
plt.title("Test")
#plt.yticks(np.arange(0, max(t)+1, 1.0))

plt.ylabel("Temps d'exÃ©cution")
plt.xlabel("Clients")
plt.show()

