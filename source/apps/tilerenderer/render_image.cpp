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

#if defined(HAVE_CAIRO)
// cairo
#include <mapnik/cairo_renderer.hpp>
#include <cairomm/surface.h>
#endif

#include <iostream>
#include <boost/filesystem.hpp>


int main ( int argc , char** argv)
{    
    if (argc != 3)
    {
        std::cout << "needs at least two argument";
        return EXIT_SUCCESS;
    }
    
    using namespace mapnik;
    try {
        std::cout << " generating map ... \n";
        std::string mapnik_dir(argv[1]);
		std::string map_file(argv[2]);
		std::string map_uri = "image.png";

		projection merc = projection("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over");
		projection longlat = projection("+proj=latlong +datum=WGS84");

		int z = 5;

#ifdef _DEBUG
		std::string plugin_path = mapnik_dir + "/input/debug/";
#else
		std::string plugin_path = mapnik_dir + "/input/release/";	
#endif
		datasource_cache::instance()->register_datasources(plugin_path.c_str()); 
		std::string font_dir = mapnik_dir + "/fonts/dejavu-fonts-ttf-2.30/ttf/";
        std::cout << " looking for DejaVuSans fonts in... " << font_dir << "\n";
		if (boost::filesystem3::exists( font_dir ) )
		{
			boost::filesystem3::directory_iterator end_itr; // default construction yields past-the-end
			for ( boost::filesystem3::directory_iterator itr( font_dir );
				itr != end_itr;
				++itr )
			{
				if (!boost::filesystem3::is_directory(itr->status()) )
				{
					freetype_engine::register_font(itr->path().string());
				}
			}
		}


        Map m(5000,3800);
        m.set_background(color_factory::from_string("white"));
		load_map(m,map_file);
		m.set_srs(merc.params());
		double bounds[4] = {5.955870,46.818020,10.492030,47.808380};
		double dummy = 0.0;
	
		proj_transform transform = proj_transform(longlat,merc);
		
		transform.forward(bounds[0],bounds[1],dummy);
		transform.forward(bounds[2],bounds[3],dummy);
		Envelope<double> bbox = Envelope<double>(bounds[0],bounds[1],bounds[2],bounds[3]);
        
		m.zoomToBox(bbox);

		Image32 buf(m.getWidth(),m.getHeight());
        agg_renderer<Image32> ren(m,buf);
        ren.apply();
        save_to_file<ImageData32>(buf.data(),map_uri,"png");
    }
    catch ( const mapnik::config_error & ex )
    {
        std::cerr << "### Configuration error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch ( const std::exception & ex )
    {
        std::cerr << "### std::exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch ( ... )
    {
        std::cerr << "### Unknown exception." << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
