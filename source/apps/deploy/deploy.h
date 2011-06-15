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

#ifndef _DEPLOY_H
#define _DEPLOY_H

#include "og.h"
#include "errors.h"
#include "io/TarWriter.h"
#include "app/Logger.h"
#include "app/ProcessingSettings.h"
#include <system/Utils.h>
#include <iostream>
#include <fstream>

enum ELayerType
{
   IMAGE_LAYER,
   ELEVATION_LAYER,
};

enum EOuputImageFormat
{
   OUTFORMAT_PNG,
   OUTFORMAT_JPG,
};

enum EOutputElevationFormat
{
   OUTFORMAT_JSON,
};

namespace Deploy
{

   void DeployImageLayer(boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, const std::string& sLayer, const std::string& sPath, bool bArchive, EOuputImageFormat imageformat);

   void DeployElevationLayer(boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, const std::string& sLayer, const std::string& sPath, bool bArchive, EOutputElevationFormat elevationformat);

}


#endif