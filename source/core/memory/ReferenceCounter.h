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

#ifndef _REFERENCECOUNTER_H
#define _REFERENCECOUNTER_H

#include "og.h"
#include <cassert>

// This is a manually controlled reference counter. The programmer is responsible
// To increase or decrease references and object deletion.
// This reference counter can be used for cyclic data structures.

class OPENGLOBE_API ReferenceCounter
{
   public:
      ReferenceCounter(){}
      virtual ~ReferenceCounter(){}
	   
      virtual void IncRef() = 0;
      virtual bool DecRef() = 0;
      virtual int  GetRefCount() = 0;
	   
};



#endif