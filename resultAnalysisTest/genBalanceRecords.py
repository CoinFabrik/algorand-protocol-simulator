import pandas as pd
import re
import csv
import random
import numpy as np

MIN_BALANCE = 100000



def createBalanceFile2():
    
    pass




def createBalanceFile(nAccounts, nOnlineAccounts, TotalBalance, nPartNodes):
    #file structure:
    #accID balance status partNodeConnection
    
    addedTotalBalance = 0
    addedOnlineBalance = 0
    
    # Generate Distribution:

    bodyStr = ""    
    accDict = {}
    for accID in range(0, nAccounts):
        lineStr = str(accID) + " "
        
        # if (accID % 10000 == 0):
        #     print(accID)
        
        # randomNum = np.random.normal(loc=TotalBalance/2, scale=TotalBalance/2)
        # while(randomNum < 0):
        #     randomNum = np.random.normal(loc=TotalBalance/2, scale=TotalBalance/2)
        # bal = int(np.round(randomNum))
        bal = random.randint(1, int(2*TotalBalance/nAccounts))
        # bal = TotalBalance/(nAccounts)

        if bal < MIN_BALANCE:
            bal = MIN_BALANCE
        
        accDict[accID] = bal

        lineStr += str(bal) + " "
        
        if accID > nOnlineAccounts:
            lineStr += "0 "
        else:
            lineStr += "1 "
            addedOnlineBalance += bal

        addedTotalBalance += bal

        lineStr += str(random.randint(0, nPartNodes-1)) + "\n"
        bodyStr += lineStr
    
    # bodyStr = ""
    # startStr = str(addedTotalBalance) + " " + str(addedOnlineBalance) + "\n" 
    # fileStr = startStr + bodyStr
    fileStr = bodyStr[:-1]

    #save to file  
    file = open('test_balance.bf', 'w')
    file.write(fileStr)
    file.close()
    
    print(addedTotalBalance)
    return accDict


nAccounts =  300000 + 300
accDict = createBalanceFile(nAccounts, 90, 1836000000 * 1e6, 1600)

# nAccounts =  900
# accDict = createBalanceFile(nAccounts, 900, 100000*2000, 900)




# import matplotlib.pyplot as plt
# import numpy as np

# sorted_dict = sorted(accDict.items(), key=lambda x:x[1])
# keys = [s[0] for s in sorted_dict]
# values = [s[1] for s in sorted_dict]

  
# fig = plt.figure(figsize = (100, 5))
 
# # creating the bar plot
# plt.bar(list(range(0, len(values))), values, color ='maroon',
#         width = 0.4)
 
# plt.xlabel("Accounts")
# plt.ylabel("Balance (microalgos)")
# plt.title("Starting balance distribution per account, sorted")
# plt.show()