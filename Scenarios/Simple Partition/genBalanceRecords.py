import pandas as pd
import re
import csv
import random

def createBalanceFile(nAccounts, nOnlineAccounts, TotalBalance, nPartNodes):
    #file structure:
    #accID balance status partNodeConnection
    
    addedTotalBalance = 0
    addedOnlineBalance = 0
    
    bodyStr = ""
    for accID in range(0, nAccounts):
        lineStr = str(accID) + " "
        
        bal = random.randint(1, 2*TotalBalance/nAccounts)
        lineStr += str(bal) + " "
        
        if accID > nOnlineAccounts:
            lineStr += "0 "
        else:
            lineStr += "1 "
            addedOnlineBalance += bal

        addedTotalBalance += bal

        lineStr += str(random.randint(0, nPartNodes-1)) + "\n"        
        bodyStr += lineStr
    
    startStr = str(addedTotalBalance) + " " + str(addedOnlineBalance) + "\n" 
    # fileStr = startStr + bodyStr
    fileStr = bodyStr

    #save to file  
    file = open('test_balance.bf', 'w')
    file.write(fileStr)
    file.close()




createBalanceFile(10*400, 10*400, 10*400*1000000, 400)