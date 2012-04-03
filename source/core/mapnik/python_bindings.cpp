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

#include "render_tile.h"
#include "google_projection.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <mapnik/map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/envelope.hpp>
#include <mapnik/proj_transform.hpp>

extern "C"
{
   __declspec(dllexport) char* test(char * a)
   {
      return a; 
   }

   __declspec(dllexport) char* PyRenderTile(char * mapnik_dir, char* mapdef, char* path, int w, int h, double lon0, double lat0, double lon1, double lat1)
   {
      std::stringstream plugin_path;
      mapnik::projection mapnikProj;
      using namespace mapnik;
      try
      {
         mapnik::Map map(w, h);
   #ifdef _DEBUG
         plugin_path <<  mapnik_dir << "input/debug/";
   #else
         plugin_path << mapnik_dir << "input/release/";   
   #endif
         datasource_cache::instance()->register_datasources(plugin_path.str().c_str());
         std::stringstream font_dir;
         font_dir << mapnik_dir << "fonts/dejavu-fonts-ttf-2.30/ttf/";
         if (boost::filesystem3::exists( font_dir.str() ) )
         {
            boost::filesystem3::directory_iterator end_itr; // default construction yields past-the-end
            for ( boost::filesystem3::directory_iterator itr( font_dir.str() );
               itr != end_itr;
               ++itr )
            {
               if (!boost::filesystem3::is_directory(itr->status()) )
               {
                  freetype_engine::register_font(itr->path().string());
               }
            }
            std::cout << "....Ok!\n" << std::flush;
         } else { std::cout << "#Error# Font directory not found!\n" << std::flush; }
         //---------------------------------------------------------------------------
         // -- Generate map container
         map.set_background(color_factory::from_string("white"));
         std::cout << "..parse map file definitions.....";
         load_map_string(map,mapdef);
         std::cout << "....Ok!\n" << std::flush;

         mapnikProj = projection(map.srs());

         // Convert to map projection (e.g. mercator co-ords EPSG:900913)
         dtuple c0(lon0,lat0);
         dtuple c1(lon1,lat1);
         mapnikProj.forward(c0.a, c0.b);
         mapnikProj.forward(c1.a, c1.b);

         // Bounding box for the tile

         mapnik::Envelope<double> bbox = mapnik::Envelope<double>(c0.a,c0.b,c1.a,c1.b);

         map.resize(w, h);
         map.zoomToBox(bbox);
         map.set_buffer_size(128);

         // Render image with default Agg renderer
   
         mapnik::Image32 buf(map.getWidth(),map.getHeight());
         mapnik::agg_renderer<mapnik::Image32> ren(map,buf);
         ren.apply();
         mapnik::save_to_file<mapnik::ImageData32>(buf.data(),path,"png");
      }
      catch ( const mapnik::config_error & ex )
      {
         std::cout << "### Configuration ERROR: " << ex.what() << std::flush;
         return 0;
      }
      catch ( const std::exception & ex )
      {
         std::cout << "### std::exception: " << ex.what() << std::flush;
         return 0;
      }
      catch ( ... )
      {
         std::cout << "### Unknown exception." << std::flush;
         return 0;
      }
      return 0;
   }
}
