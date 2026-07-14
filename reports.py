import sys
import os
import json 
import matplotlib.pyplot as plt 
import numpy as np

data = {}
fill = False

def parseArgs():
    global data
    global fill
    for i in range(1, len(sys.argv)):
        if (sys.argv[i] == "-f"):
            fill = True
            continue

        with open(sys.argv[i], "r") as f:
            title = os.path.basename(sys.argv[i]).split(".")[0]
            data[title] = json.load(f)

def getTestFromName(name, title):
    if title not in data: return None 

    for test in data[title]:
        if test["Name"] == name: return test

    return None

def plotTest(testName, title):
    test = getTestFromName(testName, title)
    testData = np.array(test["Data"])
    x = [i for i in range(len(testData))]
    ax = plt.plot(x, testData[:,0], label=title + "." + testName)
    if fill: plt.fill_between(x, y1=testData[:,1], y2=testData[:,2], alpha=0.4)

def plotAllTest(testName):
    for title in data:
        plotTest(testName, title)

parseArgs()

fig, ax = plt.subplots()

title = ""
for d in data:
    title += d + "/"

title = title[:-1]

ax.set_title(title)

plotTest("Quad double", "first")
plotAllTest("Condense and distill")
#plotTest("Kahan sum")

plt.legend()
plt.show()