BOOST=../../external/boost_1_46_1
CFLAGS=-fopenmp -I$(BOOST) -I../../source/core -I/usr/include/gdal
# FIXME there are far too many hardcode paths in the line below
LIBS=\
     -Wl,--whole-archive ../../bin/libOpenWebGlobeProcessing.a -Wl,--no-whole-archive \
     $(BOOST)/bin.v2/libs/date_time/build/gcc-4.4.3/release/link-static/threading-multi/libboost_date_time.a \
     $(BOOST)/bin.v2/libs/filesystem/build/gcc-4.4.3/release/link-static/threading-multi/libboost_filesystem.a \
     $(BOOST)/bin.v2/libs/program_options/build/gcc-4.4.3/release/link-static/threading-multi/libboost_program_options.a \
     $(BOOST)/bin.v2/libs/thread/build/gcc-4.4.3/release/link-static/threading-multi/libboost_thread.a \
     $(BOOST)/bin.v2/libs/system/build/gcc-4.4.3/release/link-static/threading-multi/libboost_system.a \
     -lgdal1.7.0 \
     -lxerces-c

all: \
	../../bin/adddata \
	../../bin/calcextent \
	../../bin/createlayer \
	../../bin/deploy \
	../../bin/locktest \
	../../bin/resample \
	../../bin/triangulate

../../bin/adddata: \
	../../source/apps/adddata/main.cpp \
	../../bin/elevationdata.o \
	../../bin/imagedata.o \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< ../../bin/elevationdata.o ../../bin/imagedata.o $(LIBS)

../../bin/calcextent: \
	../../source/apps/calcextent/main.cpp \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< $(LIBS)

../../bin/createlayer: \
	../../source/apps/createlayer/main.cpp \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< $(LIBS)

../../bin/deploy: \
	../../source/apps/deploy/main.cpp \
	../../bin/deploy.o \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< ../../bin/deploy.o $(LIBS)

../../bin/locktest: \
	../../source/apps/locktest/main.cpp \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< $(LIBS)

../../bin/resample: \
	../../source/apps/resample/main.cpp \
	../../bin/resample.o \
	../../bin/resample_elevation.o \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< ../../bin/resample.o ../../bin/resample_elevation.o $(LIBS)

../../bin/resample_mpi: \
	../../source/apps/resample/main_mpi.cpp \
	../../bin/resample.o \
	../../bin/resample_elevation.o \
	../../bin/libOpenWebGlobeProcessing.a
	mpic++ -o $@ $(CFLAGS) $< ../../bin/resample.o ../../bin/resample_elevation.o $(LIBS)

../../bin/triangulate: \
	../../source/apps/triangulate/main.cpp \
	../../bin/triangulate.o \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< ../../bin/triangulate.o $(LIBS)

../../bin/libOpenWebGlobeProcessing.a: \
	../../bin/Logger.o \
	../../bin/ProcessingSettings.o \
	../../bin/CoordinateTransformation.o \
	../../bin/ElevationLayerSettings.o \
	../../bin/ElevationReader.o \
	../../bin/ElevationTile.o \
	../../bin/ImageLayerSettings.o \
	../../bin/MercatorQuadtree.o \
	../../bin/ProcessStatus.o \
	../../bin/ImageHandler.o \
	../../bin/ImageLoader.o \
	../../bin/ImageWriter.o \
	../../bin/JPEGHandler.o \
	../../bin/CommonPath.o \
	../../bin/FileSystem.o \
	../../bin/TarWriter.o \
	../../bin/ogprocess.o \
	../../bin/DelaunayLocationStructure.o \
	../../bin/DelaunayMemoryManager.o \
	../../bin/DelaunayTriangle.o \
	../../bin/DelaunayTriangulation.o \
	../../bin/DelaunayVertex.o \
	../../bin/Predicates.o \
	../../bin/ElevationPoint.o \
	../../bin/GeoCoord.o \
	../../bin/vec3.o \
	../../bin/ConvertUTF.o \
	../../bin/ConvertUTF.o \
	../../bin/FilenameUtils.o \
	../../bin/StringUtils.o \
	../../bin/Timer.o \
	../../bin/Utils.o \
	../../bin/BaseTypeConversion.o \
	../../bin/ClassExport.o \
	../../bin/PropertyBase.o \
	../../bin/Tokenizer.o
	ar rcs $@ $?

../../bin/Logger.o: ../../source/core/app/Logger.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ProcessingSettings.o: ../../source/core/app/ProcessingSettings.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/CoordinateTransformation.o: ../../source/core/geo/CoordinateTransformation.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ElevationLayerSettings.o: ../../source/core/geo/ElevationLayerSettings.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ElevationReader.o: ../../source/core/geo/ElevationReader.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ElevationTile.o: ../../source/core/geo/ElevationTile.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ImageLayerSettings.o: ../../source/core/geo/ImageLayerSettings.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ProcessStatus.o: ../../source/core/geo/ProcessStatus.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/MercatorQuadtree.o: ../../source/core/geo/MercatorQuadtree.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ImageHandler.o: ../../source/core/image/ImageHandler.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ImageLoader.o: ../../source/core/image/ImageLoader.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ImageWriter.o: ../../source/core/image/ImageWriter.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/JPEGHandler.o: ../../source/core/image/JPEGHandler.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/CommonPath.o: ../../source/core/io/CommonPath.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/FileSystem.o: ../../source/core/io/FileSystem.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/TarWriter.o: ../../source/core/io/TarWriter.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ogprocess.o: ../../source/core/ogprocess.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/DelaunayLocationStructure.o: ../../source/core/math/delaunay/DelaunayLocationStructure.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/DelaunayMemoryManager.o: ../../source/core/math/delaunay/DelaunayMemoryManager.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/DelaunayTriangle.o: ../../source/core/math/delaunay/DelaunayTriangle.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/DelaunayTriangulation.o: ../../source/core/math/delaunay/DelaunayTriangulation.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/DelaunayVertex.o: ../../source/core/math/delaunay/DelaunayVertex.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/Predicates.o: ../../source/core/math/delaunay/Predicates.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ElevationPoint.o: ../../source/core/math/ElevationPoint.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/GeoCoord.o: ../../source/core/math/GeoCoord.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/vec3.o: ../../source/core/math/vec3.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ConvertUTF.o: ../../source/core/string/ConvertUTF.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/FilenameUtils.o: ../../source/core/string/FilenameUtils.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/StringUtils.o: ../../source/core/string/StringUtils.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/Timer.o: ../../source/core/system/Timer.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/Utils.o: ../../source/core/system/Utils.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/BaseTypeConversion.o: ../../source/core/xml/BaseTypeConversion.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ClassExport.o: ../../source/core/xml/ClassExport.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/PropertyBase.o: ../../source/core/xml/PropertyBase.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/Tokenizer.o: ../../source/core/xml/Tokenizer.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/elevationdata.o: ../../source/apps/adddata/elevationdata.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/imagedata.o: ../../source/apps/adddata/imagedata.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/resample.o: ../../source/apps/resample/resample.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/resample_elevation.o: ../../source/apps/resample/resample_elevation.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/deploy.o: ../../source/apps/deploy/deploy.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/triangulate.o: ../../source/apps/triangulate/triangulate.cpp
	g++ -c -o $@ $(CFLAGS) $<

clean:
	rm -f ../../bin/*.o
	rm -f ../../bin/libOpenWebGlobeProcessing.a
	rm -f ../../bin/adddata
	rm -f ../../bin/calcextent
	rm -f ../../bin/createlayer
	rm -f ../../bin/deploy
	rm -f ../../bin/locktest
	rm -f ../../bin/resample
	rm -f ../../bin/resample_mpi
	rm -f ../../bin/triangulate
