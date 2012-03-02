#!/usr/bin/python
################################################################################
#      ____               __          __  _      _____ _       _               #
#     / __ \              \ \        / / | |    / ____| |     | |              #
#    | |  | |_ __   ___ _ __ \  /\  / /__| |__ | |  __| | ___ | |__   ___      #
#    | |  | | '_ \ / _ \ '_ \ \/  \/ / _ \ '_ \| | |_ | |/ _ \| '_ \ / _ \     #
#    | |__| | |_) |  __/ | | \  /\  /  __/ |_) | |__| | | (_) | |_) |  __/     #
#     \____/| .__/ \___|_| |_|\/  \/ \___|_.__/ \_____|_|\___/|_.__/ \___|     #
#           | |                                                                #
#           |_|                                                                #
#                                                                              #
#                            Point Cloud Converter                             #
#                               Version 1.0.1                                  #
#                                                                              #
#                                 (c) 2011 by                                  #
#           University of Applied Sciences Northwestern Switzerland            #
#                     Institute of Geomatics Engineering                       #
#                           martin.christen@fhnw.ch                            #
################################################################################
#     Licensed under MIT License. Read the file LICENSE for more information   @
################################################################################

import sys
import csv


if len(sys.argv) < 3:
   print('usage:\n')
   print('pointcloudconv inputfile outputfile')
   sys.exit()
   

inputfile = sys.argv[1]
outputfile = sys.argv[2]


csvreader = csv.reader(open(inputfile, 'r'), delimiter=' ')
writer = open(outputfile, 'w')


for row in csvreader:
   if len(row) == 7:   # Input: X Y Z I R G B
	   x = row[0]
	   y = row[1]
	   z = row[2]
	   #intensity = row[3]
	   r = "%1.3f"%(float(row[4]) / 255.0)
	   g = "%1.3f"%(float(row[5]) / 255.0)
	   b = "%1.3f"%(float(row[6]) / 255.0)
	   a = "1"
	   outline = x + "," + y + "," + z + "," + r + "," + g + "," + b + "," + a + ","
	   writer.write(outline + "\n")
		
	  
	  
	  



  



