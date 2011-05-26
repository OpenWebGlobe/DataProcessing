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


#include "ogprocess.h"
#include "app/ProcessingSettings.h"
#include "geo/MercatorQuadtree.h"
#include "geo/CoordinateTransformation.h"
#include "string/FilenameUtils.h"
#include "string/StringUtils.h"
#include "geo/ImageLayerSettings.h"
#include "io/FileSystem.h"
#include "image/ImageWriter.h"
#include "app/Logger.h"
#include "math/mathutils.h"
#include <iostream>
#include <boost/program_options.hpp>
#include <sstream>
#include <omp.h>

//-----------------------------------------------------------------------------
// ERROR CODES:

// App Specific:
#define ERROR_GDAL               2;    // gdal-data directory not found
#define ERROR_CONFIG             3;    // wrong configuration (setup.xml) (processing path or log-path is wrong)
#define ERROR_PARAMS             4;    // wrong parameters
#define ERROR_IMAGELAYERSETTINGS 5;    // can't load imagelayersettings. (image layer probably doesn't exist)

// General Errors:
#define ERROR_OUTOFMEMORY        101;  // not enough memory

//------------------------------------------------------------------------------
const int tilesize = 256;
const double dHanc = 1.0/(double(tilesize)-1.0);
const double dWanc = 1.0/(double(tilesize)-1.0);


//------------------------------------------------------------------------------
// Image Operations (will be moved)

inline void _ReadImageDataMem(unsigned char* buffer, int bufferwidth, int bufferheight, int x, int y, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a)
{
   if (x<0) x = 0;
   if (y<0) y = 0;
   if (x>bufferwidth-1) x = bufferwidth-1;
   if (y>bufferheight-1) y = bufferheight-1;

   *r = buffer[bufferwidth*3*y+3*x];
   *g = buffer[bufferwidth*3*y+3*x+1];
   *b = buffer[bufferwidth*3*y+3*x+2];
   *a = 255;
}

//------------------------------------------------------------------------------

inline void _ReadImageValueBilinear(unsigned char* buffer, int bufferwidth, int bufferheight, double x, double y, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a)
{
   double uf = math::Fract<double>(x);
   double vf = math::Fract<double>(y);
   int nPixelX = int(x);
   int nPixelY = int(y);

   int u00,v00,u10,v10,u01,v01,u11,v11;
   u00 = nPixelX;
   v00 = nPixelY;
   u10 = nPixelX+1;
   v10 = nPixelY;
   u01 = nPixelX;
   v01 = nPixelY+1;
   u11 = nPixelX+1;
   v11 = nPixelY+1;

   unsigned char r00,g00,b00,a00;
   unsigned char r10,g10,b10,a10;
   unsigned char r01,g01,b01,a01;
   unsigned char r11,g11,b11,a11;

   _ReadImageDataMem(buffer, bufferwidth, bufferheight, u00,v00,&r00,&g00,&b00,&a00);
   _ReadImageDataMem(buffer, bufferwidth, bufferheight, u10,v10,&r10,&g10,&b10,&a10);
   _ReadImageDataMem(buffer, bufferwidth, bufferheight, u01,v01,&r01,&g01,&b01,&a01);
   _ReadImageDataMem(buffer, bufferwidth, bufferheight, u11,v11,&r11,&g11,&b11,&a11);

   double rd, gd, bd, ad;

   rd = (double(r00)*(1-uf)*(1-vf)+double(r10)*uf*(1-vf)+double(r01)*(1-uf)*vf+double(r11)*uf*vf)+0.5;
   gd = (double(g00)*(1-uf)*(1-vf)+double(g10)*uf*(1-vf)+double(g01)*(1-uf)*vf+double(g11)*uf*vf)+0.5;
   bd = (double(b00)*(1-uf)*(1-vf)+double(b10)*uf*(1-vf)+double(b01)*(1-uf)*vf+double(b11)*uf*vf)+0.5;
   ad = (double(a00)*(1-uf)*(1-vf)+double(a10)*uf*(1-vf)+double(a01)*(1-uf)*vf+double(a11)*uf*vf)+0.5;

   rd = math::Clamp<double>(rd, 0.0, 255.0);
   gd = math::Clamp<double>(gd, 0.0, 255.0);
   bd = math::Clamp<double>(bd, 0.0, 255.0);
   ad = math::Clamp<double>(ad, 0.0, 255.0);

   *r = (unsigned char) rd;
   *g = (unsigned char) gd;
   *b = (unsigned char) bd;
   *a = (unsigned char) ad;
}


//------------------------------------------------------------------------------

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
   po::options_description desc("Program-Options");
   desc.add_options()
       ("image", po::value<std::string>(), "image to add")
       ("srs", po::value<std::string>(), "spatial reference system for input files")
       ("layer", po::value<std::string>(), "name of layer to add the data")
       ("fill", "fill empty parts, don't overwrite already existing data")
       ("overwrite", "overwrite existing data")
       ("numthreads", po::value<int>(), "force number of threads")
       ("verbose", "verbose output")
       ;

   po::variables_map vm;

   bool bError = false;

   try
   {
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);
   }
   catch (std::exception&)
   {
      bError = true;
   }

   std::string sImagefile;
   std::string sSRS;
   std::string sLayer;
   bool bFill = false;
   bool bOverwrite = false;
   bool bVerbose = false;

   //---------------------------------------------------------------------------
   // init options:

   boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

   if (!qSettings)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   //---------------------------------------------------------------------------

   boost::shared_ptr<Logger> qLogger =  ProcessingUtils::CreateLogger("adddata", qSettings);

   if (!qLogger)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   //---------------------------------------------------------------------------

   if (!vm.count("image") || !vm.count("srs") || !vm.count("layer"))
   {
      bError = true;
   }
   else
   {
      sImagefile = vm["image"].as<std::string>();
      sSRS = vm["srs"].as<std::string>();
      sLayer = vm["layer"].as<std::string>();
   }

   if (vm.count("verbose"))
   {
      bVerbose = true;
   }

   if (vm.count("overwrite") && vm.count("fill"))
   {
      bError = true; // can't overwrite and fill at same time!
   }

   if (vm.count("overwrite"))
   {
      bOverwrite = true;
   }

   if (vm.count("numthreads"))
   {
      int n = vm["numthreads"].as<int>();
      if (n>0 && n<65)
      {
         std::ostringstream oss; 
         oss << "Forcing number of threads to " << n;
         qLogger->Info(oss.str());
         omp_set_num_threads(n);
      }


   }

   if (vm.count("fill"))
   {
      bFill = true;
   }

   if (!bFill && !bOverwrite)
   {
      bError = true; // needs atleast one option (fill or overwrite)
   }

   //---------------------------------------------------------------------------
   if (bError)
   {
      qLogger->Error("Wrong parameters!");
      std::ostringstream sstr;
   
      sstr << desc;
      qLogger->Info("\n" + sstr.str());

      return ERROR_PARAMS;
   }

   //---------------------------------------------------------------------------
   if (StringUtils::Left(sSRS, 5) != "EPSG:")
   {
      qLogger->Error("only srs starting with EPSG: are currently supported");
      return 1;
   }

   int epsg = atoi(sSRS.c_str()+5);

   //---------------------------------------------------------------------------

   DataSetInfo oInfo;

   if (!ProcessingUtils::init_gdal())
   {
      qLogger->Error("gdal-data directory not found!");
      return ERROR_GDAL;
   } 

   //---------------------------------------------------------------------------
   // Retrieve ImageLayerSettings:
   std::ostringstream oss;

   std::string sImageLayerDir = FilenameUtils::DelimitPath(qSettings->GetPath()) + sLayer;
   std::string sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sImageLayerDir) + "tiles");

   boost::shared_ptr<ImageLayerSettings> qImageLayerSettings = ImageLayerSettings::Load(sImageLayerDir);
   if (!qImageLayerSettings)
   {
      qLogger->Error("Failed retrieving image layer settings! Make sure to create it using 'createlayer'.");
      ProcessingUtils::exit_gdal();
      return ERROR_IMAGELAYERSETTINGS;
   }

   int lod = qImageLayerSettings->GetMaxLod();
   int64 layerTileX0, layerTileY0, layerTileX1, layerTileY1;
   qImageLayerSettings->GetTileExtent(layerTileX0, layerTileY0, layerTileX1, layerTileY1);

   if (bVerbose)
   {
      oss << "\nImage Layer:\n";
      oss << "     name = " << qImageLayerSettings->GetLayerName() << "\n";
      oss << "   maxlod = " << lod << "\n";
      oss << "   extent = " << layerTileX0 << ", " << layerTileY0 << ", " << layerTileX1 << ", " << layerTileY1 << "\n";
   }


   //---------------------------------------------------------------------------

   boost::shared_ptr<CoordinateTransformation> qCT;
   qCT = boost::shared_ptr<CoordinateTransformation>(new CoordinateTransformation(epsg, 3785));

   clock_t t0,t1;
   t0 = clock();

   ProcessingUtils::RetrieveDatasetInfo(sImagefile, qCT.get(), &oInfo, bVerbose);

   if (!oInfo.bGood)
   {
      qLogger->Error("Failed retrieving info!");
   }

   if (bVerbose)
   {
      oss << "Loaded image info:\n   Image Size: w= " << oInfo.nSizeX << ", h= " << oInfo.nSizeY << "\n";
      oss << "   dest: " << oInfo.dest_lrx << ", " << oInfo.dest_lry << ", " << oInfo.dest_ulx << ", " << oInfo.dest_uly << "\n";
      qLogger->Info(oss.str());
      oss.str("");
   }

   boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());
  
   int64 px0, py0, px1, py1;
   qQuadtree->MercatorToPixel(oInfo.dest_ulx, oInfo.dest_uly, lod, px0, py0);
   qQuadtree->MercatorToPixel(oInfo.dest_lrx, oInfo.dest_lry, lod, px1, py1);

   int64 imageTileX0, imageTileY0, imageTileX1, imageTileY1;
   qQuadtree->PixelToTileCoord(px0, py0, imageTileX0, imageTileY0);
   qQuadtree->PixelToTileCoord(px1, py1, imageTileX1, imageTileY1);

   if (bVerbose)
   {
      oss << "\nTile Coords (image):";
      oss << "   (" << imageTileX0 << ", " << imageTileY0 << ")-(" << imageTileX1 << ", " << imageTileY1 << ")\n";
      qLogger->Info(oss.str());
      oss.str("");
   }

   // check if image is outside layer
   if (imageTileX0 > layerTileX1 || 
       imageTileY0 > layerTileY1 ||
       imageTileX1 < layerTileX0 ||
       imageTileY1 < layerTileY0)
   {
      qLogger->Info("The dataset is outside of the layer and not being added!");
      ProcessingUtils::exit_gdal();
      return 0;
   }

   // clip tiles to layer extent
   imageTileX0 = math::Max<int64>(imageTileX0, layerTileX0);
   imageTileY0 = math::Max<int64>(imageTileY0, layerTileY0);
   imageTileX1 = math::Min<int64>(imageTileX1, layerTileX1);
   imageTileY1 = math::Min<int64>(imageTileY1, layerTileY1);

   // Load image 
   boost::shared_array<unsigned char> vImage = ProcessingUtils::ImageToMemoryRGB(oInfo);
   unsigned char* pImage = vImage.get();

   if (!vImage)
   {
      qLogger->Error("Can't load image into memory!\n");
      return ERROR_OUTOFMEMORY;
   }

   //
   boost::shared_array<unsigned char> vTile = boost::shared_array<unsigned char>(new unsigned char[tilesize*tilesize*4]);
   unsigned char* pTile = vTile.get();
   if (!vTile) { return ERROR_OUTOFMEMORY;}

   // iterate through all tiles and create them
   for (int64 xx = imageTileX0; xx <= imageTileX1; ++xx)
   {
      for (int64 yy = imageTileY0; yy <= imageTileY1; ++yy)
      {
         std::string sQuadcode = qQuadtree->TileCoordToQuadkey(xx,yy,lod);
         std::string sTilefile = sTileDir + sQuadcode + ".png";
         sTilefile = FilenameUtils::MakeHierarchicalFileName(sTilefile, 2);

         if (bVerbose)
         {
            std::stringstream sst;
            sst << "processing " << sQuadcode << " (" << xx << ", " << yy << ")";
            qLogger->Info(sst.str());
         }

         //---------------------------------------------------------------------
         // #todo: LOCK this tile
         // #todo: if tile is LOCKED -> wait
         //---------------------------------------------------------------------

         //---------------------------------------------------------------------
         // #todo:
         // if mode is --fill: (bFill)
         //      * load possibly existing tile into vTile
         // ...  * if there is none, clear vTile (memset with 0)
         // if mode is --overwrite (bOverwrite)
         //      * just overwrite tiles
         //_--------------------------------------------------------------------

         double px0m, py0m, px1m, py1m;
         qQuadtree->QuadKeyToMercatorCoord(sQuadcode, px0m, py0m, px1m, py1m);

         double ulx = px0m;
         double uly = py1m;
         double lrx = px1m;
         double lry = py0m;

         double anchor_Ax = ulx; 
         double anchor_Ay = lry;
         double anchor_Bx = lrx; 
         double anchor_By = lry;
         double anchor_Cx = lrx; 
         double anchor_Cy = uly;
         double anchor_Dx = ulx; 
         double anchor_Dy = uly;

         // avoid calculating transformation per pixel using anchor point method
         qCT->TransformBackwards(&anchor_Ax, &anchor_Ay);
         qCT->TransformBackwards(&anchor_Bx, &anchor_By);
         qCT->TransformBackwards(&anchor_Cx, &anchor_Cy);
         qCT->TransformBackwards(&anchor_Dx, &anchor_Dy);

         /*if (bVerbose)
         {
            std::stringstream sst;
            sst << "Anchor points:\nA(" << anchor_Ax << ", " << anchor_Ay << ")" 
                               << "\nB(" << anchor_Bx << ", " << anchor_By << ")" 
                               << "\nC(" << anchor_Cx << ", " << anchor_Cy << ")"
                               << "\nD(" << anchor_Dx << ", " << anchor_Dy << ")\n";
            qLogger->Info(sst.str());
         }*/

         // write current tile
         for (int ty=0;ty<tilesize;++ty)
         {
            for (int tx=0;tx<tilesize;++tx)
            {
                double dx = (double)tx*dWanc;
                double dy = (double)ty*dHanc;
                double xd = (anchor_Ax*(1.0-dx)*(1.0-dy)+anchor_Bx*dx*(1.0-dy)+anchor_Dx*(1.0-dx)*dy+anchor_Cx*dx*dy);
                double yd = (anchor_Ay*(1.0-dx)*(1.0-dy)+anchor_By*dx*(1.0-dy)+anchor_Dy*(1.0-dx)*dy+anchor_Cy*dx*dy);

                // pixel coordinate in original image
                double dPixelX = (oInfo.affineTransformation_inverse[0] + xd * oInfo.affineTransformation_inverse[1] + yd * oInfo.affineTransformation_inverse[2]);
                double dPixelY = (oInfo.affineTransformation_inverse[3] + xd * oInfo.affineTransformation_inverse[4] + yd * oInfo.affineTransformation_inverse[5]);
                unsigned char r,g,b,a;

                // out of image -> set transparent
                if (dPixelX<0 || dPixelX>oInfo.nSizeX ||
                    dPixelY<0 || dPixelY>oInfo.nSizeY)
                {
                     r = g = b = a = 0;
                }
                else
                {
                     // read pixel in image pImage[dPixelX, dPixelY] (biliear, bicubic or nearest neighbour)
                     // and store as r,g,b
                     _ReadImageValueBilinear(pImage, oInfo.nSizeX, oInfo.nSizeY, dPixelX, dPixelY, &r, &g, &b, &a);
                }

                size_t adr=4*ty*tilesize+4*tx; // currently RGB for testing purposes!
                pTile[adr+0] = b;  
                pTile[adr+1] = g;  
                pTile[adr+2] = r; 
                pTile[adr+3] = a;
            }
         }

         // save tile (pTile)
         if (bVerbose)
         {
            qLogger->Info("Storing tile: " + sTilefile);
         }

         ImageWriter::WritePNG(sTilefile, pTile, tilesize, tilesize);
      }
   }

   //---------------------------------------------------------------------------


   //---------------------------------------------------------------------------
   t1=clock();

   std::ostringstream out;
   out << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";
   qLogger->Info(out.str());

   ProcessingUtils::exit_gdal();

   return 0;
}



//------------------------------------------------------------------------------

