#!/usr/bin/python

import sys

numBanks = 64

cmdDict = {"ACT":"A","READ":"R","WRITE":"W","PRE":"P","REF":"F"}

bankStates = []
for ii in range(numBanks):
    bankStates.append(".")

inFileName = sys.argv[1]

inFile = open(inFileName)

sys.stdout.write('%10s' % " ")
for ii in range(numBanks):
    sys.stdout.write('%3d' % ii)
print

for line in inFile:
    if line[0] != '@': # very simple format checking
        continue
    line = line.rstrip()
    grep = line.split()
    cycle = grep[0][1:]
    cmd = grep[1]

    doPrint = True
    ref = False
    if cmd == 'REF' and len(grep)>3:
        addr = ""
        ref = True
        bankList = grep[3:]

        for ii in bankList:
            if int(ii) >= numBanks:
                print "Increase numBanks!",ii,"detected, max is",numBanks-1
            bankStates[int(ii)] = cmdDict[cmd]
    else:
        if cmd != 'REF':
            addr = grep[3]
            bankId = int(grep[12])
            if bankId >= numBanks:
                print "Increase numBanks!",bankId,"detected, max is",numBanks-1
            bankStates[bankId] = cmdDict[cmd]
        else:
            doPrint = False

    if doPrint:
        sys.stdout.write('%9s' % cycle)
        sys.stdout.write(" ")
        for ii in range(numBanks):
            sys.stdout.write('%3s' % bankStates[ii])
        print " ",addr

        #reset active banks to | and precharge banks to .
        if ref:
            for ii in bankList:
                bankStates[int(ii)] = "."
        else:
            if bankStates[bankId] == "A" or bankStates[bankId] == "W" or bankStates[bankId] == "R":
                bankStates[bankId] = "|"
            if bankStates[bankId] == "P" or bankStates[bankId] == "F":
                bankStates[bankId] = "."

        

    
