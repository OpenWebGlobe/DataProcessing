#!/usr/bin/python

import urllib2
import sys
import os
import os.path
import tarfile

#-------------------------------------------------------------------------------
# OS specific setupls

if sys.platform == 'win32':
   print "Windows detected..."
   dest = "dataprocessing_external_win32.tar.gz"
elif sys.platform == 'darwin':
   print "MacOS X detected..."
   dest = "dataprocessing_external_osx.tar.gz"
elif 'linux' in sys.platform:
   print "Linux detected..."
   dest = "dataprocessing_external_linux.tar.gz"
else:
   print "unsupported system: " + sys.platform
   dest = "dataprocessing_external.tar.gz"

#-------------------------------------------------------------------------------
# FUNCTION: DOWNLOAD FILE
#-------------------------------------------------------------------------------
def download(url, filename):
   print "Fetching " + url
   webfile = urllib2.urlopen(url)
   diskfile = open(filename,"wb")
   diskfile.write(webfile.read())
   diskfile.close()
   webfile.close()
   

#-------------------------------------------------------------------------------
# MAIN
#-------------------------------------------------------------------------------
# Download external integration
url = "https://github.com/downloads/OpenWebGlobe/DataProcessing/" + dest

#if (os.path.isfile(dest)):
#   print "externals are already downloaded..."
#else:
#   download(url, dest)
#   print "Extracting externals..."
#   tar = tarfile.open(dest)
#   tar.extractall("../")
#   tar.close()
#   print "Ok."

#print "Done."

print "this script doesn't work yet..."   

