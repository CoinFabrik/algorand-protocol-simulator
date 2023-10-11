import pandas as pd
import re
import csv
import random

def createNetworkFile(nPartNodes, nRelayNodes):
    fileStr = str(nPartNodes) + " " + str(nRelayNodes)
    
    #partNode connections to relays
    for n in range(0, nPartNodes):
        relays = list(range(0, nRelayNodes))
        random.shuffle(relays)
        fileStr += '\n'
        fileStr += str(relays[0]) + " 0" + " 0 "
        fileStr += str(relays[1]) + " 0" + " 0 "
        fileStr += str(relays[2]) + " 0" + " 0 "
        fileStr += str(relays[3]) + " 0" + " 0 "

    #relay to relay connections
    relayConnMatrix = []
    for relayID in range(0, nRelayNodes):
        relayConnMatrix.append([False for _ in range(0, nRelayNodes)])
    for relayID in range(0, nRelayNodes):
        relays = list(range(0, nRelayNodes))
        random.shuffle(relays)
        for i in range(0, 10):
            relayConnMatrix[relayID][relays[i]] = True
            relayConnMatrix[relays[i]][relayID] = True
    for relayID in range(0, nRelayNodes):
        fileStr += '\n'
        for i in range(0, nRelayNodes):
            if (i != relayID and relayConnMatrix[relayID][i] == True):
                fileStr += str(i) + " 0" + " 0 "
                
    #save to file  
    file = open('test_network.nf', 'w')
    file.write(fileStr)    
    file.close()




createNetworkFile(1600, 120)
# createNetworkFile(900, 120)