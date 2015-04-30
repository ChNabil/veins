import libxml2 #xml library
import sys     #arguments

if len(sys.argv) != 2:
        print "Usage: %s path-to-sumo-net-file"
        exit(1)

doc = libxml2.parseFile(sys.argv[1]) #first argument is the sumo net file
#XPath query for the attrib we want
results = doc.xpathEval("//location/@origBoundary")
#there should be exactly one location node in this file, and it contains the original coordinates
if len(results) != 1:
        print 'error: ' + results
else:
        print ("*.manager.traceCoords=\"%s\""% str(results[0]).split('"')[1])
