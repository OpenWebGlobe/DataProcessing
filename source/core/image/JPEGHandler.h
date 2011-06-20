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

#ifndef _JPEGHANDLER_H
#define _JPEGHANDLER_H

#include "og.h"
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <map>
#include <vector>


class OPENGLOBE_API JPEGHandler
{
public:
   /*!
   * \brief Decompress JPEG and store it in memory.
   * \param inJpeg pointer to jpeg data
   * \param inJpegSize size of jpeg data in bytes
   * \param outRGB rgb image output (shared array)
   * \param outWidth width of rgb image output
   * \param outHeight height of rgb image output
   * \return 
   */
   static bool JpegToRGB(unsigned char* inJpeg, int inJpegSize, boost::shared_array<unsigned char>& outRGB, int& outWidth, int& outHeight);
   //---------------------------------------------------------------------------
   /*!
   * \brief
   * \param inRgb pointer to rgb data
   * \param inWidth width of rgb image
   * \param inHeight height of rgb image
   * \param inQuality Quality of compression 0 for worst, 100 for best.
   * \param outJpeg jpeg output.
   * \param outJpegSize size in bytes of rgb output.
   * \return 
   */
   static bool RGBToJpeg(unsigned char* inRgb, int inWidth, int inHeight, int inQuality, boost::shared_array<unsigned char>& outJpeg, int& outJpegSize);


};


#endif