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

#ifndef _DELAUNAY_LOCATIONSTRUCTURE_H
#define _DELAUNAY_LOCATIONSTRUCTURE_H

#include "og.h"
#include "DelaunayMemoryManager.h"
#include "DelaunayTriangle.h"
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace math
{
   //--------------------------------------------------------------------------

   enum EDelaunayLocationAlgorithms
   {
      DELAUNAYLOCATION_LINEARLIST = 0,          // not accelerated location function, triangles are stored in a list.
      DELAUNAYLOCATION_QUADTREE_HIERARCHY = 1,  // accelerated using quadtree hierachy   
      DELAUNAYLOCATION_KDTREE_HIERARCHY = 2,    // accelerated using kd-tree hierachy (also known as BHI or SKD-TREE)
   };

   //--------------------------------------------------------------------------

   class OPENGLOBE_API IDelaunayLocationStructure
   {
   public:
      IDelaunayLocationStructure();
      virtual ~IDelaunayLocationStructure() {}

      //! Add Triangle to Structure. This shouldn't be called from outside and will be removed at one point
      //! This method is available to create supersimplex triangle, however, this should be part of this class
      //! in future!
      virtual void AddTriangle(DelaunayTriangle* pTriangle) = 0;

      //! Get triangle (and its relation to) at specified 2D point.
      virtual DelaunayTriangle* GetTriangleAt(double x, double y, ePointTriangleRelation& eRelation) = 0;
   
      //! Traverse Structure and call function for every triangle. Traversal order is not important.   
      virtual void Traverse(boost::function<void(DelaunayTriangle*)> callback) = 0;


      //! Spatial traverse Structure and call function for every triangle that has atleast one point within the specified axis aligned rectangular boundary. 
      //! \param xmin min x value of axis aligned rectangular boundary
      //! \param ymin min y value of axis aligned rectangular boundary
      //! \param xmax max x value of axis aligned rectangular boundary
      //! \param ymax max y value of axis aligned rectangular boundary
      //! \param callback function to be called for every triangle inside bounding rect
      virtual void SpatialTraverse(double xmin, double ymin, double xmax, double ymax, boost::function<void(DelaunayTriangle*)> callback) = 0;


      //! Delete a triangle from memory. The delaunay trianglulation still exists, but
      //! the triangle is removed from memory.
      void DeleteMemory(DelaunayTriangle* pTri);

      //! Insert new Vertex into triangulation
      //! pStartTriangle is a hint.
      //! Returns true if a point was actually inserted.
      DelaunayTriangle* InsertVertex(DelaunayVertex* pVertex, DelaunayTriangle* pStartTriangle);

      //! Create instance of a location structure using specified algorithm
      static boost::shared_ptr<IDelaunayLocationStructure>   CreateLocationStructure(double xmin, double ymin, double xmax, double ymax, EDelaunayLocationAlgorithms eAlgorithm = DELAUNAYLOCATION_LINEARLIST);
   
      //! Set Epsilon for point distance
      void SetEpsilon(double epsilon) {_dEpsilon = epsilon;}

   protected:
      //! Remove a triangle from acceleration structure
      virtual void RemoveTriangle(DelaunayTriangle* pTriangle) = 0;

      double _dEpsilon;
   private:
      DelaunayTriangle* _InsertPointToTriangulation(DelaunayVertex* pVertex);

   };
}

#endif

