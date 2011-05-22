/*******************************************************************************
#      ____               __          __  _      _____ _       _               #
#     / __ \              \ \        / / | |    / ____| |     | |              #
#    | |  | |_ __   ___ _ __ \  /\  / /__| |__ | |  __| | ___ | |__   ___      #
#    | |  | | '_ \ / _ \ '_ \ \/  \/ / _ \ '_ \| | |_ | |/ _ \| '_ \ / _ \     #
#    | |__| | |_) |  __/ | | \  /\  /  __/ |_) | |__| | | (_) | |_) |  __/     #
#     \____/| .__/ \___|_| |_|\/  \/ \___|_.__/ \_____|_|\___/|_.__/ \___|     #
#           | |                                                                #
#           |_|                                                                #
#                                                                              #
#                                (c) 2011 by                                   #
#           University of Applied Sciences Northwestern Switzerland            #
#                     Institute of Geomatics Engineering                       #
#                           martin.christen@fhnw.ch                            #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/

//------------------------------------------------------------------------------

#include "geo/MercatorQuadtree.h"
#include "geo/CoordinateTransformation.h"
#include "string/FilenameUtils.h"
#include <iostream>

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
   if (argc != 6)
   {
      std::cout << "Wrong arguments:\n";
      std::cout << FilenameUtils::ExtractBaseFileName(std::string(argv[0]));
      std::cout << " lng0 lat0 lng1 lat1 lod\n"; 
      return 0;
   }

   double lng0, lat0, lng1, lat1;
   int lod;

   lng0 = atof(argv[1]);
   lat0 = atof(argv[2]);
   lng1 = atof(argv[3]);
   lat1 = atof(argv[4]);
   lod = atoi(argv[5]);

   if (lng0 >= lng1) { std::cout << "error: lng1 must be greater than lng0\n"; return 0; }
   if (lat0 >= lat1) { std::cout << "error: lat1 must be greater than lat0\n"; return 0; }
   if (lod < 4) { std::cout << "error: level of detail must be atleast 4\n"; return 0; }


   MercatorQuadtree* pQuadtree = new MercatorQuadtree();

   int64 px0, py0, px1, py1, tx0, ty0, tx1, ty1;
   pQuadtree->WGS84ToPixel(lng0, lat0, lod, px0, py1);
   pQuadtree->WGS84ToPixel(lng1, lat1, lod, px1, py0);

   std::cout << "Number of pixels in specified range: " << (px1-px0+1)*(py1-py0+1) << "\n";

   pQuadtree->PixelToTileCoord(px0, py0, tx0, ty0);
   pQuadtree->PixelToTileCoord(px1, py1, tx1, ty1);

   std::cout << "Tile Coords: (" << tx0 << ", " << ty0 << ")-(" << tx1 << ", " << ty1 << ")\n";

   return 0;
}