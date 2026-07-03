import sys
import json 
import matplotlib.pyplot as plt 
import numpy as np

with open(sys.argv[1], "r") as f:
    data = json.load(f)

def getTestFromName(name):
    for test in data:
        if test["Name"] == name: return test
    return None

def plotTest(testName):
    test = getTestFromName(testName)
    testData = np.array(test["Data"])
    x = [i for i in range(len(testData))]
    ax = plt.plot(x, testData[:,0], label=testName)
    plt.fill_between(x, y1=testData[:,1], y2=testData[:,2], alpha=0.4)
    
plotTest("Quad double V2")
plotTest("Condense and distill V2")
#plotTest("Condense and distill V1")
#plotTest("Condense and distill V2")
plotTest("Condense and distill V6")

plt.legend()
plt.show()