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
#include "rendertile.h"

//------------------------------------------------------------------------------
void TileRenderer::RenderTile(std::string tile_uri, mapnik::Map m, int x, int y, int zoom, GoogleProjection tileproj, mapnik::projection prj, bool verbose, bool overrideTile, bool lockEnabled, std::string compositionLayerPath, std::string compositionMode, double compositionAlpha)
{
   if(!overrideTile && FileSystem::FileExists(tile_uri))
   {
      return;
   }
   else
   {
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
#ifndef MAPNIK_2
      mapnik::Envelope<double> bbox = mapnik::Envelope<double>(c0.a,c0.b,c1.a,c1.b);
      m.resize(256,256);
      m.zoomToBox(bbox);
#else
      mapnik::box2d<double> bbox(c0.a,c0.b,c1.a,c1.b);
      m.resize(256,256);
      m.zoom_to_box(bbox);
#endif
      m.set_buffer_size(128);

      // Render image with default Agg renderer

#ifndef MAPNIK_2
      mapnik::Image32 buf(m.getWidth(),m.getHeight());
      mapnik::agg_renderer<mapnik::Image32> ren(m,buf);
#else
      mapnik::image_32 buf(m.width(), m.height());
      mapnik::agg_renderer<mapnik::image_32> ren(m,buf);
#endif
      ren.apply();
      if(lockEnabled)
      {
         int lockhandle = FileSystem::Lock(tile_uri);
#ifndef MAPNIK_2
		 Compose(compositionLayerPath, compositionMode, compositionAlpha, m.getWidth(),m.getHeight(),&buf);
         mapnik::save_to_file<mapnik::ImageData32>(buf.data(),tile_uri,"png");
#else
	 Compose(compositionLayerPath, compositionMode, compositionAlpha, m.width(),m.height(),&buf);
	 mapnik::save_to_file<mapnik::image_data_32>(buf.data(),tile_uri,"png");
#endif
         FileSystem::Unlock(tile_uri, lockhandle);
      }
      else
      {
#ifndef MAPNIK_2
		 Compose(compositionLayerPath, compositionMode, compositionAlpha, m.getWidth(),m.getHeight(),&buf);
         mapnik::save_to_file<mapnik::ImageData32>(buf.data(),tile_uri,"png");
#else
		Compose(compositionLayerPath, compositionMode, compositionAlpha, m.width(),m.height(),&buf);
		mapnik::save_to_file<mapnik::image_data_32>(buf.data(),tile_uri,"png");
#endif
      }
   }
}

#ifndef MAPNIK_2
void TileRenderer::Compose(std::string compositionLayerPath, std::string compositionMode, double compositionAlpha, int width, int height, mapnik::Image32* buf)
#else
void TileRenderer::Compose(std::string compositionLayerPath, std::string compositionMode, double compositionAlpha, int width, int height, mapnik::image_32* buf)
#endif
{
	 // COMPOSITION MODE
	if(compositionLayerPath != "")
	{
		//std::stringstream str;
		//str<< compositionLayerPath << "/tiles/" << zoom << "/" << x << "/" << y << ".png";
		ImageObject compImg;
		if(ImageLoader::LoadFromDisk(Img::Format_PNG, compositionLayerPath, Img::PixelFormat_RGBA,compImg))
		{
			if(compositionMode == "unify")
			{
				for(size_t y = 0; y < height; y++)
				{
					for(size_t x = 0; x < width; x++)
					{
						unsigned char comp[4];
						int adr = 4*y*width+4*x;
						compImg.ReadPixel4(x,y,comp[0],comp[1],comp[2],comp[3]);
						unsigned int compInt = 0x00000000;
						compInt = compInt | comp[3];
						compInt = (compInt << 8) | comp[2];
						compInt = (compInt << 8) | comp[1];
						compInt = (compInt << 8) | comp[0];
						unsigned char alpha = buf->raw_data()[adr+3];
						//buf.setPixel(x,y,compInt);
						buf->blendPixel(x,y,compInt,(int)alpha);
					}
				}
			}
			else if(compositionMode == "overlay")
			{
				for(size_t y = 0; y < height; y++)
				{
					for(size_t x = 0; x < width; x++)
					{
						unsigned char comp[4];
						int adr = 4*y*width+4*x;
						compImg.ReadPixel4(x,y,comp[0],comp[1],comp[2],comp[3]);
						unsigned int compInt = 0x00000000;
						compInt = compInt | comp[3];
						compInt = (compInt << 8) | comp[2];
						compInt = (compInt << 8) | comp[1];
						compInt = (compInt << 8) | comp[0];
						unsigned char alpha = buf->raw_data()[adr+3];
						//buf.setPixel(x,y,compInt);
						buf->blendPixel(x,y,compInt,(int)comp[3]);
					}
				}
			}
		}
	}
}