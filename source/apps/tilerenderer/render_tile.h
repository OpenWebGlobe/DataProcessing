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
#                           robert.wueest@fhnw.ch                              #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/
// This is the triangulate version without mpi intended for regular 
// workstations. Multi cores are supported (OpenMP) and highly recommended.
//------------------------------------------------------------------------------
// Some code adapted from: generate_tiles.py
// Found at: http://trac.openstreetmap.org/browser/applications/rendering/mapnik
//------------------------------------------------------------------------------
#ifndef _RENDER_TILE_H
#define _RENDER_TILE_H
#include "google_projection.h"
#include <mapnik/projection.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/envelope.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/map.hpp>
 
//------------------------------------------------------------------------------
inline void _renderTile(std::string tile_uri, mapnik::Map m, int x, int y, int zoom, GoogleProjection tileproj, mapnik::projection prj, bool verbose = false)
{
   if(verbose)
   {
#define MAPNIK_DEBUG
   }
   // Calculate pixel positions of bottom-left & top-right
   ituple p0(x * 256, (y + 1) * 256);
   ituple p1((x + 1) * 256, y * 256);

   // Convert to LatLong (EPSG:4326)
   dtuple l0 = tileproj.pixel2GeoCoord(p0, zoom);
   dtuple l1 = tileproj.pixel2GeoCoord(p1, zoom);

   // Convert to map projection (e.g. mercator co-ords EPSG:900913)
   dtuple c0(l0.a,l0.b);
   dtuple c1(l1.a,l1.b);
   prj.forward(c0.a, c0.b);
   prj.forward(c1.a, c1.b);

   // Bounding box for the tile

   mapnik::Envelope<double> bbox = mapnik::Envelope<double>(c0.a,c0.b,c1.a,c1.b);

   m.resize(256, 256);
   m.zoomToBox(bbox);
   m.set_buffer_size(128);

   // Render image with default Agg renderer
   mapnik::Image32 buf(m.getWidth(),m.getHeight());
   mapnik::agg_renderer<mapnik::Image32> ren(m,buf);
   ren.apply();
   mapnik::save_to_file<mapnik::ImageData32>(buf.data(),tile_uri,"png");
      if(verbose)
   {
#undef MAPNIK_DEBUG
   }
}

#endif