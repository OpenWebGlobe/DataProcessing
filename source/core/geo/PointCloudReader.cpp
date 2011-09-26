

#include "PointCloudReader.h"
#include <cassert>
#include "math/mathutils.h"
#include "geo/CoordinateTransformation.h"
#include "math/CloudPoint.h"
#include "string/StringUtils.h"
#include "string/FilenameUtils.h"
#include "io/FileSystem.h"
#include "xml/xml.h"

//-----------------------------------------------------------------------------

PointCloudReader::PointCloudReader()
{

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

   _ifstream.open(sFilename.c_str());

   return _ifstream.good();
}

//-----------------------------------------------------------------------------

void PointCloudReader::Close()
{
   _ifstream.close();
}

//-----------------------------------------------------------------------------

//bool PointCloudReader::_ConvertToBinaryFromXYZ(std::ostream& file, int nDestEpsg, uint64& out_nPointsWritten, double& inout_xmin, double& inout_ymin, double &inout_zmin, double& inout_xmax, double& inout_ymax, double &inout_zmax)
//{
//   _minIntensity = 1e20;
//   _maxIntensity = -1e20;
//   
//   bool bFirst = true;
//   CoordinateTransformation* pCT = 0;
//
//   if (_nSourceEPSG != 0)
//   {
//      if (_nSourceEPSG != 3395 && _nSourceEPSG != 3785 && _nSourceEPSG != nDestEpsg)
//         pCT = new CoordinateTransformation(_nSourceEPSG, nDestEpsg);
//   }
//  
//   std::ifstream myfile(_sFilenameA.c_str(), std::ios::in);
//   char line[4096];
//   std::string sLine;
//   CloudPoint oPoint;
//
//   int numColumns = 0;
//   int curPoint = 0;
//
//   if (myfile.good())
//   {
//      while (!myfile.eof())
//      {
//         std::vector<double> vOut;
//         std::string sSeparator;
//        
//         myfile.getline(line, 4095);
//         sLine = line;
//         curPoint++;
//         if (/*(curPoint%5 == 0) &&*/ sLine.size()>0)
//         {
//            Tokenize(sLine, ' ', vOut);
//            
//            if (bFirst)
//            {
//               bFirst = false;
//               numColumns = vOut.size();  // number of columns MUST be constant! otherwise the point is wrong!!
//            
//               if (numColumns == 3)
//               {
//                  _minIntensity = _maxIntensity = 0; // no intensity!!
//                  std::cout << "Reading XYZ Point Cloud!\n";
//                  _pct = PCT_XYZ;
//               }
//               if (numColumns == 4)
//               {
//                  std::cout << "Reading XYZI Point Cloud!\n";
//                  _pct = PCT_XYZI;
//               }
//               else if (numColumns == 6)
//               {
//                  std::cout << "Reading XYZRGB Point Cloud!\n";
//                  _pct = PCT_XYZRGB;
//               }
//               else if (numColumns == 7)
//               {
//                  std::cout << "Reading XYZIRGB Point Cloud!\n";
//                  _pct = PCT_XYZIRGB;
//               }
//               else if (numColumns == 10)
//               {
//                  std::cout << "Reading XYZOIRGB Pount Cloud!\n";
//                  std::cout << "** WARNING ** ORIENTATION IS CURRENTLY UNSUPPORTED!\n";
//                  _pct = PCT_XYZOIRGB;
//               }
//               else
//               {
//                  std::cout << "Invalid Point Cloud file!\n";
//                  _pct = PCT_INVALID;
//                  return false;
//               }
//            
//            }
//
//            if (vOut.size() >= 3 && vOut.size() == numColumns && _pct != PCT_INVALID)
//            {
//               oPoint.x = vOut[0];
//               oPoint.y = vOut[1];
//               oPoint.elevation = vOut[2];
//
//               if (pCT)
//               {
//                  pCT->Transform(&oPoint.x, &oPoint.y);
//               }
//
//               if (_pct == PCT_XYZI)
//               {
//                  oPoint.intensity = (int)vOut[3];
//                  _minIntensity = math::Min<int>(oPoint.intensity, _minIntensity);
//                  _maxIntensity = math::Max<int>(oPoint.intensity, _maxIntensity);
//               }
//               else if (_pct == PCT_XYZRGB)
//               {
//                  oPoint.r = (unsigned char)vOut[3];
//                  oPoint.g = (unsigned char)vOut[4];
//                  oPoint.b = (unsigned char)vOut[5];
//               }
//               else if (_pct == PCT_XYZIRGB)
//               {
//                  oPoint.intensity = (int)vOut[3];
//                  oPoint.r = (unsigned char)vOut[4];
//                  oPoint.g = (unsigned char)vOut[5];
//                  oPoint.b = (unsigned char)vOut[6];
//                  
//                  _minIntensity = math::Min<int>(oPoint.intensity, _minIntensity);
//                  _maxIntensity = math::Max<int>(oPoint.intensity, _maxIntensity);
//               }
//               else if (_pct == PCT_XYZOIRGB)
//               {
//                  // #TODO: TRANSFORM ORIENTATION!!!
//                  oPoint.ox = vOut[3];
//                  oPoint.oy = vOut[4];
//                  oPoint.oz = vOut[5];
//
//                  oPoint.intensity = (int)vOut[6];
//                  oPoint.r = (unsigned char)vOut[7];
//                  oPoint.g = (unsigned char)vOut[8];
//                  oPoint.b = (unsigned char)vOut[9];
//                  
//                  _minIntensity = math::Min<int>(oPoint.intensity, _minIntensity);
//                  _maxIntensity = math::Max<int>(oPoint.intensity, _maxIntensity);
//               }
//
//               WriteCloudPoint(file, oPoint);
//               out_nPointsWritten++;
//
//               inout_xmin = math::Min<double>(inout_xmin, oPoint.x);
//               inout_ymin = math::Min<double>(inout_ymin, oPoint.y);
//               inout_zmin = math::Min<double>(inout_zmin, oPoint.elevation);
//               
//               inout_xmax = math::Max<double>(inout_xmax, oPoint.x);
//               inout_ymax = math::Max<double>(inout_ymax, oPoint.y);
//               inout_zmax = math::Max<double>(inout_zmax, oPoint.elevation);
//            }
//         }
//      }
//
//   }
//   myfile.close();
//
//   if (pCT)
//   {
//      delete pCT;
//   }
//   return true;
//}

