BOOST=../../external/boost_1_46_1
CFLAGS=-fopenmp -I$(BOOST) -I../../source/core -I/usr/include/gdal
# FIXME there are far too many hardcode paths in the line below
LIBS=\
     -Wl,--whole-archive ../../bin/libOpenWebGlobeProcessing.a -Wl,--no-whole-archive \
     $(BOOST)/bin.v2/libs/date_time/build/gcc-4.4.3/release/link-static/threading-multi/libboost_date_time.a \
     $(BOOST)/bin.v2/libs/filesystem/build/gcc-4.4.3/release/link-static/threading-multi/libboost_filesystem.a \
     $(BOOST)/bin.v2/libs/program_options/build/gcc-4.4.3/release/link-static/threading-multi/libboost_program_options.a \
     $(BOOST)/bin.v2/libs/system/build/gcc-4.4.3/release/link-static/threading-multi/libboost_system.a \
     -lgdal1.7.0 \
     -lxerces-c

all: \
	../../bin/adddata \
	../../bin/calcextent \
	../../bin/createlayer

../../bin/adddata: \
	../../source/apps/adddata/main.cpp \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< $(LIBS)

../../bin/calcextent: \
	../../source/apps/calcextent/main.cpp \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< $(LIBS)

../../bin/createlayer: \
	../../source/apps/createlayer/main.cpp \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< $(LIBS)

../../bin/libOpenWebGlobeProcessing.a: \
	../../bin/Logger.o \
	../../bin/ProcessingSettings.o \
	../../bin/CoordinateTransformation.o \
	../../bin/ImageLayerSettings.o \
	../../bin/MercatorQuadtree.o \
	../../bin/ImageHandler.o \
	../../bin/ImageWriter.o \
	../../bin/lodepng.o \
	../../bin/CommonPath.o \
	../../bin/FileSystem.o \
	../../bin/ogprocess.o \
	../../bin/vec3.o \
	../../bin/ConvertUTF.o \
	../../bin/ConvertUTF.o \
	../../bin/FilenameUtils.o \
	../../bin/StringUtils.o \
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

../../bin/ImageLayerSettings.o: ../../source/core/geo/ImageLayerSettings.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/MercatorQuadtree.o: ../../source/core/geo/MercatorQuadtree.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ImageHandler.o: ../../source/core/image/ImageHandler.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ImageWriter.o: ../../source/core/image/ImageWriter.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/lodepng.o: ../../source/core/image/lodepng/lodepng.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/CommonPath.o: ../../source/core/io/CommonPath.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/FileSystem.o: ../../source/core/io/FileSystem.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ogprocess.o: ../../source/core/ogprocess.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/vec3.o: ../../source/core/math/vec3.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ConvertUTF.o: ../../source/core/string/ConvertUTF.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/FilenameUtils.o: ../../source/core/string/FilenameUtils.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/StringUtils.o: ../../source/core/string/StringUtils.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/BaseTypeConversion.o: ../../source/core/xml/BaseTypeConversion.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/ClassExport.o: ../../source/core/xml/ClassExport.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/PropertyBase.o: ../../source/core/xml/PropertyBase.cpp
	g++ -c -o $@ $(CFLAGS) $<

../../bin/Tokenizer.o: ../../source/core/xml/Tokenizer.cpp
	g++ -c -o $@ $(CFLAGS) $<

clean:
	rm -f ../../bin/*.o
	rm -f ../../bin/libOpenWebGlobeProcessing.a
	rm -f ../../bin/adddata
	rm -f ../../bin/calcextent
	rm -f ../../bin/createlayer
