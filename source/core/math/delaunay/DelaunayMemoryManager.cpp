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

#include "DelaunayMemoryManager.h"
#include <iostream>

//-----------------------------------------------------------------------------

unsigned int DelaunayMemoryManager::_nTrianglesCount = 0;
unsigned int DelaunayMemoryManager::_nVerticesCount = 0;

//-----------------------------------------------------------------------------

math::DelaunayVertex* DelaunayMemoryManager::AllocVertex(double x, double y, double elevation, double weight)
{
   math::DelaunayVertex* pNewVertex;
   pNewVertex = new math::DelaunayVertex(x,y,elevation,weight);
   if (pNewVertex) 
      ++_nVerticesCount;

#ifdef DMM_MEMORY_DEBUG
   std::cout << "<b>Alloc Vertex(x,y,e,w)</b>\n";
   std::cout << "Triangles: " << _nTrianglesCount << ", Vertices: " << _nVerticesCount << "\n";
   std::cout << "Total Memory: " << double(_nTrianglesCount*sizeof(math::DelaunayTriangle) + _nVerticesCount*sizeof(math::DelaunayVertex))/1024.0/1024.0 << " MB\n";
#endif
   return pNewVertex;
}

//-----------------------------------------------------------------------------

math::DelaunayVertex* DelaunayMemoryManager::AllocVertex(const ElevationPoint& pt)
{
   math::DelaunayVertex* pNewVertex;
   pNewVertex = new math::DelaunayVertex(pt);
   if (pNewVertex) 
      ++_nVerticesCount;

#ifdef DMM_MEMORY_DEBUG
   std::cout << "<b>Alloc Vertex(pt)</b>\n";
   std::cout << "Triangles: " << _nTrianglesCount << ", Vertices: " << _nVerticesCount << "\n";
   std::cout << "Total Memory: " << double(_nTrianglesCount*sizeof(math::DelaunayTriangle) + _nVerticesCount*sizeof(math::DelaunayVertex))/1024.0/1024.0 << " MB\n";
#endif

   return pNewVertex;
}

//-----------------------------------------------------------------------------

math::DelaunayTriangle* DelaunayMemoryManager::AllocTriangle()
{
   math::DelaunayTriangle* pNewTriangle;
   pNewTriangle = new math::DelaunayTriangle();
   if (pNewTriangle) 
      ++_nTrianglesCount;

#ifdef DMM_MEMORY_DEBUG
   std::cout << "<b>Alloc Triangle()</b>\n";
   std::cout << "Triangles: " << _nTrianglesCount << ", Vertices: " << _nVerticesCount << "\n";
   std::cout << "Total Memory: " << double(_nTrianglesCount*sizeof(math::DelaunayTriangle) + _nVerticesCount*sizeof(math::DelaunayVertex))/1024.0/1024.0 << " MB\n";
#endif

   return pNewTriangle;
}

//-----------------------------------------------------------------------------

void DelaunayMemoryManager::Free(math::DelaunayVertex* v)
{
   if (v)
   {
      --_nVerticesCount;
      delete v;

#ifdef DMM_MEMORY_DEBUG
      std::cout << "<b>Free Vertex</b>\n";
      std::cout << "Triangles: " << _nTrianglesCount << ", Vertices: " << _nVerticesCount << "\n";
      std::cout << "Total Memory: " << double(_nTrianglesCount*sizeof(math::DelaunayTriangle) + _nVerticesCount*sizeof(math::DelaunayVertex))/1024.0/1024.0 << " MB\n";
#endif
   }
}

//-----------------------------------------------------------------------------

void DelaunayMemoryManager::Free(math::DelaunayTriangle* t)
{
   if (t)
   {
      --_nTrianglesCount;
      delete t;

#ifdef DMM_MEMORY_DEBUG
      std::cout << "<b>Free Triangle</b>\n";
      std::cout << "Triangles: " << _nTrianglesCount << ", Vertices: " << _nVerticesCount << "\n";
      std::cout << "Total Memory: " << double(_nTrianglesCount*sizeof(math::DelaunayTriangle) + _nVerticesCount*sizeof(math::DelaunayVertex))/1024.0/1024.0 << " MB\n";
#endif
   }
}

//-----------------------------------------------------------------------------

void DelaunayMemoryManager::DumpMemoryInfo()
{
   std::cout << "<b>Total Memory for Delaunay Structure</b>\n" << double(_nTrianglesCount*sizeof(math::DelaunayTriangle) + _nVerticesCount*sizeof(math::DelaunayVertex))/1024.0 << " KB\n";
   std::cout << "Vertices: " << _nVerticesCount << "\n";
   std::cout << "Triangles: " << _nTrianglesCount << "\n";
}

//-----------------------------------------------------------------------------

void DelaunayMemoryManager::DumpMemoryInfoShort()
{
   std::cout << "Memory Delaunay:</b>\n" << double(_nTrianglesCount*sizeof(math::DelaunayTriangle) + _nVerticesCount*sizeof(math::DelaunayVertex))/1024.0/1024.0 << " MB ";
   std::cout << "(vtx=" << _nVerticesCount << ", ";
   std::cout << "tri= " << _nTrianglesCount << ")\n";
}

//-----------------------------------------------------------------------------


double DelaunayMemoryManager::GetMemory()
{
   return double(_nTrianglesCount*sizeof(math::DelaunayTriangle) + _nVerticesCount*sizeof(math::DelaunayVertex))/1024.0/1024.0;

}

//-----------------------------------------------------------------------------
