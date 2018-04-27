#!/bin/python3

### Parameters (externalize as arguments in a later version...)
amountOfRuns   = 25
printAllRates  = False
plotIndividual = False
pattern        = "../simulations/lust/results-old-2/myconf-%d.sca"


#run with individual files (deprecated)
import sys
if len(sys.argv)>1 and sys.argv[1]:
  print("Warning, running in deprecated mode...")
  runname=sys.argv[1]


####################### Parsing methods ########################
# These functions relate directly to the regex dictionary below.
################################################################
import re
def params(matchobj, i):
  for x in matchobj.groupdict():
    try:
      simParams[i][x]=int(matchobj.groupdict()[x])
    except:
      simParams[i][x]=matchobj.groupdict()[x]

def startTime(matchobj, i):
  ID = int(matchobj.group("ID"))
  if not ID in parsedData[i]:
    parsedData[i][ID]={}
  parsedData[i][ID]['startTime'] = matchobj.group("startTime")
  pass

def stopTime(matchobj, i):
  ID = int(matchobj.group("ID"))
  if not ID in parsedData[i]:
    parsedData[i][ID] = {}
  parsedData[i][ID]['stopTime'] = matchobj.group("stopTime")

resProcessor = re.compile(r'.*?(?P<value>[0-9]+(\.[0-9]+)?)')
def appl(matchobj, i):
  ID = int(matchobj.group("ID"))
  if not ID in parsedData[i]:
    parsedData[i][ID]={}

  res = matchobj.group(2)
  try:
    value = float(resProcessor.match(res).group('value'))
  except AttributeError:
    if 'nan' in res: #nan or -nan happens when dividing by 0 (?)
      value = -1
    else:
      print("Error, could not extract value from following string:")
      print(res)

  if "DetRateART" in res:
    parsedData[i][ID]['ARTDetectionRate']=value;
  elif "DetRateeART" in res:
    parsedData[i][ID]['eARTDetectionRate']=value;
  elif "DetRateExchange" in res:
    parsedData[i][ID]['ExchangeDetectionRate']=value;

  #eART
  elif "eARTtruePositivesScalar" in res:
    parsedData[i][ID]['eARTTPRate']=value;
  elif "eARTfalseNegativesScalar" in res:
    parsedData[i][ID]['eARTFNRate']=value;
  elif "eARTtrueNegativesScalar" in res:
    parsedData[i][ID]['eARTTNRate']=value;
  elif "eARTfalsePositivesScalar" in res:
    parsedData[i][ID]['eARTFPRate']=value;

  #ART
  elif "ARTtruePositivesScalar" in res:
    parsedData[i][ID]['ARTTPRate']=value;
  elif "ARTfalseNegativesScalar" in res:
    parsedData[i][ID]['ARTFNRate']=value;
  elif "ARTtrueNegativesScalar" in res:
    parsedData[i][ID]['ARTTNRate']=value;
  elif "ARTfalsePositivesScalar" in res:
    parsedData[i][ID]['ARTFPRate']=value;

  #Exchange
  elif "ExchangeTruePositivesScalar" in res:
    parsedData[i][ID]['ExchangeTPRate']=value;
  elif "ExchangeFalseNegativesScalar" in res:
    parsedData[i][ID]['ExchangeFNRate']=value;
  elif "ExchangeTrueNegativesScalar" in res:
    parsedData[i][ID]['ExchangeTNRate']=value;
  elif "ExchangeFalsePositivesScalar" in res:
    parsedData[i][ID]['ExchangeFPRate']=value;

  #Merged
  elif "MergedTruePositivesScalar" in res:
    parsedData[i][ID]['MergedTPRate']=value;
  elif "MergedFalseNegativesScalar" in res:
    parsedData[i][ID]['MergedFNRate']=value;
  elif "MergedTrueNegativesScalar" in res:
    parsedData[i][ID]['MergedTNRate']=value;
  elif "MergedFalsePositivesScalar" in res:
    parsedData[i][ID]['MergedFPRate']=value;

  elif "SenderIsAttackerScalarCounter" in res:
    parsedData[i][ID]['ReceivedMaliciousMessages']=value;
  elif "SenderIsNotAttackerScalarCounter" in res:
    parsedData[i][ID]['ReceivedBenignMessages']=value;
  elif "AttackersNumCntr" in res:
    parsedData[i][ID]['AmountOfAttackers']=value;
  elif "IamAnAttacker" in res:
    if 'isAttacker' in parsedData[i][ID]:
      print("Warning, key isAttacker already exists for " + str(ID))
    parsedData[i][ID]['isAttacker']=(value==0);
  else:
    print("Note: unparsed scalar value:")
    print(res)

lineProcessors = {
    #re.compile(r'^attr iterationvars2 "\$t=(?P<trafficLoadClass>[0-9]+), \$0=(?P=trafficLoadClass), \$1=(?P=trafficLoadClass), \$2=(?P<ARTVariance>[0-9]+), \$3=(?P<LDMExchangeEnabled>true|false), \$4=(?P<maliciousProbability>[0-9]+(\.[0-9]+)?), \$5=(?P<modification>.*?), \$repetition=(?P<rep>[0-9]+)"') : params,
    re.compile(r'^attr iterationvars2 "\$0=(?P<ARTThreshold>[0-9]+), \$repetition=(?P<rep>[0-9]+)"') : params,
    re.compile(r'^scalar.*\[(?P<ID>[0-9]+)\].*startTime.*?(?P<startTime>[0-9]+(\.[0-9]+)?)') : startTime,
    re.compile(r'^scalar.*\[(?P<ID>[0-9]+)\].*stopTime.*?(?P<stopTime>[0-9]+(\.[0-9]+)?)') : stopTime,
    re.compile(r'^scalar.*\[(?P<ID>[0-9]+)\]\.appl(.*)') : appl
    }

################### End of parsing methods #####################

################# Processing individual runs ###################

simParams = []
parsedData = []
plotData={}
print("ART_threshold;ARTMR;ARTBR;eARTMR;eARTBR;exchangeMR;exchangeBR;mergedMR;mergedBR;MR;BR")
for i in range(0,amountOfRuns):
  simParams.append({})
  parsedData.append({})
  runname=pattern%(i)
  with open(runname, 'r') as f:
    for line in f:
      for regex in lineProcessors:
        matchobj = regex.match(line)
        if matchobj:
          lineProcessors[regex](matchobj, i)
          break
    totalBenignReceived=0
    totalMaliciousReceived=0
    detectedARTMaliciousReceptions=0.0
    detectedARTBenignReceptions=0.0
    detectedeARTMaliciousReceptions=0.0
    detectedeARTBenignReceptions=0.0
    detectedExchangeMaliciousReceptions=0.0
    detectedExchangeBenignReceptions=0.0
    detectedMergedMaliciousReceptions=0.0
    detectedMergedBenignReceptions=0.0
    for ID in parsedData[i]:
      rb = parsedData[i][ID].get('ReceivedBenignMessages',0)
      rm = parsedData[i][ID].get('ReceivedMaliciousMessages',0)
      arttpr = parsedData[i][ID].get('ARTTPRate',0.0)
      artfpr = parsedData[i][ID].get('ARTFPRate',0.0)
      earttpr = parsedData[i][ID].get('eARTTPRate',0.0)
      eartfpr = parsedData[i][ID].get('eARTFPRate',0.0)
      exchangetpr = parsedData[i][ID].get('ExchangeTPRate',0.0)
      exchangefpr = parsedData[i][ID].get('ExchangeFPRate',0.0)
      mergedtpr = parsedData[i][ID].get('MergedTPRate',0.0)
      mergedfpr = parsedData[i][ID].get('MergedFPRate',0.0)
      totalBenignReceived    += rb
      totalMaliciousReceived += rm
      detectedARTMaliciousReceptions      += arttpr * rm
      detectedARTBenignReceptions         += artfpr * rb
      detectedeARTMaliciousReceptions     += earttpr * rm
      detectedeARTBenignReceptions        += eartfpr * rb
      detectedExchangeMaliciousReceptions += exchangetpr * rm
      detectedExchangeBenignReceptions    += exchangefpr * rb
      detectedMergedMaliciousReceptions   += mergedtpr * rm
      detectedMergedBenignReceptions      += mergedfpr * rb

    if plotIndividual:
      plotData[i]=[detectedARTMaliciousReceptions, totalMaliciousReceived, detectedARTBenignReceptions, totalBenignReceived]
    else:
      threshold = simParams[i]['ARTThreshold']
      if threshold not in plotData:
        plotData[threshold]=[detectedARTMaliciousReceptions, totalMaliciousReceived, detectedARTBenignReceptions, totalBenignReceived]
      else:
        detectedARTMaliciousReceptions+=plotData[threshold][0]
        totalMaliciousReceived+=plotData[threshold][1]
        detectedARTBenignReceptions+=plotData[threshold][2]
        totalBenignReceived+=plotData[threshold][3]
        plotData[threshold]=[detectedARTMaliciousReceptions, totalMaliciousReceived, detectedARTBenignReceptions, totalBenignReceived]

    if printAllRates:
        print("%d;%f;%f;%f;%f;%f;%f;%f;%f"%(\
            simParams.get('ARTThreshold',0),\
            (detectedARTMaliciousReceptions/totalMaliciousReceived),\
            (detectedARTBenignReceptions/totalBenignReceived),\
            (detectedeARTMaliciousReceptions/totalMaliciousReceived),\
            (detectedeARTBenignReceptions/totalBenignReceived),\
            (detectedExchangeMaliciousReceptions/totalMaliciousReceived),\
            (detectedExchangeBenignReceptions/totalBenignReceived),\
            (detectedMergedMaliciousReceptions/totalMaliciousReceived),\
            (detectedMergedBenignReceptions/totalBenignReceived)\
            ))

############# End processing individual runs ###################
###################### Plotting code ###########################
import matplotlib.pyplot as plt

x = []
TPgraph=[]
FPgraph=[]
for item in plotData:
  x.append(item)
  TPgraph.append(plotData[item][0]/plotData[item][1])
  FPgraph.append(plotData[item][2]/plotData[item][3])

plt.scatter(x,TPgraph)
plt.savefig('TruePositives.png')
plt.close()
plt.scatter(x,FPgraph)
plt.savefig('FalsePositives.png')
plt.close()
