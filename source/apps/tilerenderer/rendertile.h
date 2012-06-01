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
#ifndef _RENDERTILE_H
#define _RENDERTILE_H
#include "google_projection.h"
#include <mapnik/projection.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#ifndef MAPNIK_2
	#include <mapnik/envelope.hpp>
#endif
#include <mapnik/image_util.hpp>
#include <mapnik/map.hpp>
#include <io/FileSystem.h>
#include <image/ImageLoader.h>

class TileRenderer
{
public:
	static void RenderTile(
		std::string			tile_uri, 
		mapnik::Map			m, 
		int					x, 
		int					y, 
		int					zoom, 
		GoogleProjection	tileproj, 
		mapnik::projection	prj, 
		bool				verbose = false, 
		bool				overrideTile = true, 
		bool				lockEnabled = false, 
		std::string			compositionLayerPath = "", 
		std::string			compositionMode = "overLay", 
		double				compositionAlpha = 1.0
		);
protected:
#ifndef MAPNIK_2
	static void Compose(std::string compositionLayerPath, std::string compositionMode, double compositionAlpha, int width, int height, mapnik::Image32* buf,int zz, int xx, int yy);
#else
	static void Compose(std::string compositionLayerPath, std::string compositionMode, double compositionAlpha, int width, int height, mapnik::image_32* buf,int zz, int xx, int yy);
#endif
};
#endif
