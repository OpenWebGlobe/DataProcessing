

#include "PointCloudReader.h"
#include <cassert>
#include "math/mathutils.h"
#include "geo/CoordinateTransformation.h"
#include "math/CloudPoint.h"
#include "string/StringUtils.h"
#include "string/FilenameUtils.h"
#include "io/FileSystem.h"
#include "xml/xml.h"
#include <boost/tokenizer.hpp>

//-----------------------------------------------------------------------------

PointCloudReader::PointCloudReader()
{
   _line.resize(4096);
   _separators = " ,\t;"; // tokens for value separation in ASCII point cloud
}

//-----------------------------------------------------------------------------

PointCloudReader::~PointCloudReader()
{

}

//-----------------------------------------------------------------------------

bool PointCloudReader::Open(const std::string& sFilename, int nSourceEPSG)
{
   if (_ifstream.is_open())
   {
      _ifstream.close();
   }

   _nSourceEPSG = nSourceEPSG;
   _sFilenameA = sFilename;
   _ptsread = 0;

   _ifstream.open(sFilename.c_str());


   return _ifstream.good();
}

//-----------------------------------------------------------------------------

void PointCloudReader::Close()
{
   _ifstream.close();
}

//-----------------------------------------------------------------------------

bool PointCloudReader::ReadPoint(CloudPoint& point)
{
   char* line = &_line[0];
   std::vector<double> vOut;

   while (!_ifstream.eof())
   {
      _ifstream.getline(line, 4095);
      std::string string_line(line);
      Tokenize(string_line, _separators, vOut);
      size_t numColumns = vOut.size();

      if (_ptsread == 0) // first point!
      {
         if (numColumns == 3)
         {
            // Reading XYZ Point Cloud
            _pct = PCT_XYZ;
         }
         if (numColumns == 4)
         {
            // Reading XYZI Point Cloud
            _pct = PCT_XYZI;
         }
         else if (numColumns == 6)
         {
            // Reading XYZRGB Point Cloud!
            _pct = PCT_XYZRGB;
         }
         else if (numColumns == 7)
         {
            // Reading XYZIRGB Point Cloud!
            _pct = PCT_XYZIRGB;
         }
         else
         {
             // pc is not valid!
            _pct = PCT_INVALID;
            return false;
         }
      }

      if (vOut.size() >= 3 && vOut.size() == numColumns)
      {
         point.x = vOut[0];
         point.y = vOut[1];
         point.elevation = vOut[2];

         if (_pct == PCT_XYZI)
         {
            point.intensity = (int)vOut[3];
         }
         else if (_pct == PCT_XYZRGB)
         {
            point.r = (unsigned char)vOut[3];
            point.g = (unsigned char)vOut[4];
            point.b = (unsigned char)vOut[5];
         }
         else if (_pct == PCT_XYZIRGB)
         {
            point.intensity = (int)vOut[3];
            point.r = (unsigned char)vOut[4];
            point.g = (unsigned char)vOut[5];
            point.b = (unsigned char)vOut[6];
         }

         _ptsread++;
         return true;
      }
   }

   return false;
}

//------------------------------------------------------------------------------



