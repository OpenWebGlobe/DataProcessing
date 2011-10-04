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

#ifndef A_OCTOCODE_H
#define A_OCTOCODE_H

#include "og.h"
#include <cmath>
#include <cassert>
#include <string>
#include <sstream>
#include <math/vec3.h>
#include <math/vec2.h>

//! \class Octocode
//! \brief Class for Octocode management
//! \author Martin Christen, martin.christen@fhnw.ch
class Octocode
{
public:
   Octocode(){}
   virtual ~Octocode(){}

   //---------------------------------------------------------------------------
   //! Returns parent Octocode
   static std::string GetParent(const std::string& sOctocode)
   {
      if (sOctocode.length()<2)
      {
         assert(false);
         return std::string();
      }

      return sOctocode.substr(0,sOctocode.length()-1);
   }
   
   //---------------------------------------------------------------------------
   //! Get normaliized box coordinates from a octocode
   static bool OctocodeToNormalizedCoord(const std::string& sOctocode, vec3<double>& v0, vec3<double>& v1)
   {
      double scale = 1.0;
      vec3<double> vec(0,0,0);

      int nlevelOfDetail =  sOctocode.length();
      
      for (int i = 0; i < nlevelOfDetail; i++)
      {
         scale /= 2.0;

         switch (sOctocode[i])
         {
         case '0':
            //_OctoCoordTranslate(vec, 0, 0, 0 );
            break;
         case '1':
            _OctoCoordTranslate(vec, scale, 0, 0);
            break;   
         case '2':
            _OctoCoordTranslate(vec, 0, scale, 0);
            break;
         case '3':
            _OctoCoordTranslate(vec, scale, scale,0 );
            break;
         case '4':
            _OctoCoordTranslate(vec, 0, 0, scale );
            break;
         case '5':
            _OctoCoordTranslate(vec, scale, 0, scale);
            break;   
         case '6':
            _OctoCoordTranslate(vec, 0, scale, scale);
            break;
         case '7':
            _OctoCoordTranslate(vec, scale, scale, scale );
            break;
        
         default:
            //wrong octocode!
            return false;   
         }
      }
      
      v0 = vec;
      v1.x = v0.x+scale;
      v1.y = v0.y+scale;
      v1.z = v0.z+scale;
      return true;
   }
   
   //---------------------------------------------------------------------------
   //! \param Get Voxel Space Index (VSI) of an octocode
   static bool OctocodeToIndex(const std::string& sOctocode, unsigned int& out_tileX, unsigned int& out_tileY, unsigned int& out_tileZ, unsigned int& out_levelOfDetail)
   {
      out_tileX = out_tileY = out_tileZ = 0;
      out_levelOfDetail = (unsigned int)sOctocode.length();
      
      assert(out_levelOfDetail<=32);
      
      if (out_levelOfDetail>32)
         return false;
      
      for (int i = out_levelOfDetail; i > 0; i--)
      {
         unsigned int mask = 1 << (i - 1);
         switch (sOctocode[out_levelOfDetail - i])
         {
         case '0':
            break;

         case '1':
            out_tileX |= mask;
            break;

         case '2':
            out_tileY |= mask;
            break;

         case '3':
            out_tileX |= mask;
            out_tileY |= mask;
            break;
         
         case '4':
            out_tileZ |= mask;
            break;

         case '5':
            out_tileX |= mask;
            out_tileZ |= mask;
            break;

         case '6':
            out_tileY |= mask;
            out_tileZ |= mask;
            break;

         case '7':
            out_tileX |= mask;
            out_tileY |= mask;
            out_tileZ |= mask;
            break;   

         default:
            return false;   
         }
      }

      return true;
   }
   
   //---------------------------------------------------------------------------
   
   //Convert Voxel Space Index to Octocode
   static std::string IndexToOctocode(unsigned int TileX, unsigned int TileY, unsigned int TileZ, unsigned int levelofdetail)
   {
      std::ostringstream  oss;
     

      for (int i=levelofdetail; i>0;i--)
      {
         char digit = '0';
         int mask = 1 << (i-1);
         if ((TileX & mask) != 0)
         {
            digit++;
         }
         if ((TileY & mask) != 0)
         {
            digit+=2;
         }
         if ((TileZ & mask) != 0)
         {
            digit+=4;
         }

         oss << digit;
      }

      return oss.str();
   }
   
   //---------------------------------------------------------------------------
   // If speed is important, always use array indices instead of string based octocodes
   // however, these are limited to 21 levels!
   
   static uint64 ToArrayIndex(unsigned int i, unsigned int j, unsigned int k, unsigned int level)
   {
      assert(level < 22); // maximal level to support uint64
      uint64 nMaxTile = ULLCONST(1) << (uint64)level;
      return nMaxTile*nMaxTile*k + nMaxTile*j + i;
   }
   
   //---------------------------------------------------------------------------
   
   static void FromArrayIndex(unsigned int level, uint64 nArrayIndex, unsigned int& out_i, unsigned int& out_j, unsigned int& out_k)
   {
      assert(level < 22); // maximal level to support uint64
      uint64 nMaxTile = ULLCONST(1) << (uint64)level;
     
      out_k = (unsigned int)(nArrayIndex / (nMaxTile*nMaxTile));
      unsigned int tmp = (unsigned int)(nArrayIndex - out_k*nMaxTile*nMaxTile); 
      out_j = (unsigned int)(tmp / nMaxTile);
      out_i = (unsigned int)(tmp - out_j*nMaxTile);
   }
   //---------------------------------------------------------------------------

private:
   static inline void _OctoCoordTranslate(vec3<double>& oVec, double fX, double fY, double fZ)
   {
      oVec.x += fX;
      oVec.y += fY;
      oVec.z += fZ;
   }

};


#endif