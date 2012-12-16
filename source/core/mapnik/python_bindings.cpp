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

#include <iostream>
#include <boost/filesystem.hpp>
#include <mapnik/map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#ifndef MAPNIK_2
  #include <mapnik/filter_factory.hpp>
  #include <mapnik/envelope.hpp>
#else
  #include <mapnik/expression_evaluator.hpp>
  #include <mapnik/expression_grammar.hpp>
  #include <mapnik/expression_node.hpp>
  #include <mapnik/expression_string.hpp>
#endif
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/proj_transform.hpp>

#ifdef _MSC_VER
 #define PYTHON_MAPNIK_API __declspec(dllexport)
 #define _WIN_
#else
 #define PYTHON_MAPNIK_API
#endif
 



extern "C"
{

   PYTHON_MAPNIK_API char* test(char * a)
   {
      return a; 
   }

   PYTHON_MAPNIK_API char* PyRenderTile(char * mapnik_dir, char* mapdef, int w, int h, double lon0, double lat0, double lon1, double lat1, char* output)
   {
      std::stringstream plugin_path;
      mapnik::projection mapnikProj;
      using namespace mapnik;
      try
      {
         mapnik::Map map(w, h);
#ifdef _WIN_
   #ifdef _DEBUG
         plugin_path <<  mapnik_dir << "input/debug/";
   #else
         plugin_path << mapnik_dir << "input/release/";   
   #endif
#else
	plugin_path << mapnik_dir << "input/";
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
         //std::cout << mapdef << std::flush;
         load_map_string(map,mapdef);
         std::cout << "....Ok!\n" << std::flush;

         mapnikProj = projection(map.srs());

         // Convert to map projection (e.g. mercator co-ords EPSG:900913)
         mapnikProj.forward(lon0, lat0);
         mapnikProj.forward(lon1, lat1);

         // Bounding box for the tile
#ifndef MAPNIK_2
         mapnik::Envelope<double> bbox = mapnik::Envelope<double>(lon0,lat0,lon1,lat1);
	 map.resize(w,h);
	 map.zoomToBox(bbox);
	 map.set_buffer_size(128);
	 mapnik::Image32 buf(map.getWidth(),map.getHeight());
	 mapnik::agg_renderer<mapnik::Image32> ren(map,buf);
#else	 
	 mapnik::box2d<double> bbox(lon0,lat0,lon1,lat1);
         map.resize(w, h);
         map.zoom_to_box(bbox);
         map.set_buffer_size(128);
   
         mapnik::image_32 buf(map.width(),map.height());
         mapnik::agg_renderer<mapnik::image_32> ren(map,buf);
#endif
         ren.apply();
         for(size_t i = 0; i < w*h*4; i++)
         {
            output[i] = buf.raw_data()[i];
         }
         //mapnik::save_to_file<mapnik::ImageData32>(buf.data(),"bla.png","png");
         return (char*)"S";
      }
      catch ( const mapnik::config_error & ex )
      {
         std::cout << "### Configuration ERROR: " << ex.what() << std::flush;
         return (char*)"E1";
      }
      catch ( const std::exception & ex )
      {
         std::cout << "### std::exception: " << ex.what() << std::flush;
         return (char*)"E2";
      }
      catch ( ... )
      {
         std::cout << "### Unknown exception." << std::flush;
         return (char*)"E3";
      }
   }
}
