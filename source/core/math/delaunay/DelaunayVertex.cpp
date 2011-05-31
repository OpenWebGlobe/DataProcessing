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

#include "DelaunayVertex.h"
#include "DelaunayMemoryManager.h"


namespace math
{
   //--------------------------------------------------------------------------
   DelaunayVertex::DelaunayVertex() 
   { 
      _Init(); _pt.x = _pt.y = _pt.elevation = _pt.weight = 0.0; 
   }
   //--------------------------------------------------------------------------
   DelaunayVertex::DelaunayVertex(double x, double y, double elevation, double weight)
   {
      _Init(); _pt.x = x; _pt.y = y; _pt.elevation = elevation; _pt.weight = weight; 
   }
   //--------------------------------------------------------------------------
   DelaunayVertex::DelaunayVertex(const ElevationPoint& pt)
   {
      _Init(); _pt = pt;
   }
   //--------------------------------------------------------------------------
   DelaunayVertex::~DelaunayVertex()
   {

   }
   //--------------------------------------------------------------------------
   void DelaunayVertex::_Init()
   {
      _bInfinite = false;
      _nRef = 0;
      _nId = -1;
   }
   //--------------------------------------------------------------------------
   void DelaunayVertex::SetInifite()
   {
      _bInfinite = true;
   }
   //--------------------------------------------------------------------------
   bool DelaunayVertex::IsInfinite()
   {
      return _bInfinite;
   }
   //--------------------------------------------------------------------------
   void DelaunayVertex::SetId(int nId)
   {
      _nId = nId;
   }
   //--------------------------------------------------------------------------
   int  DelaunayVertex::GetId()
   {
      return _nId;
   }
   //--------------------------------------------------------------------------
   void DelaunayVertex::IncRef()
   {
      _nRef++;
   }
   //--------------------------------------------------------------------------
   bool DelaunayVertex::DecRef()
   {
      _nRef--;
      if (_nRef == 0)
      {
         // This vertex is no longer referenced
         //
         DelaunayMemoryManager::Free(this);
         return true;
      }

      return false;
   }
   //--------------------------------------------------------------------------
   int DelaunayVertex::GetRefCount()
   {
      return _nRef;
   }
   //--------------------------------------------------------------------------
}