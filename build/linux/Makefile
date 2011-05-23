BOOST=../../external/boost_1_46_1
CFLAGS=-fopenmp -I$(BOOST) -I../../source/core -I/usr/include/gdal
# FIXME there are far too many hardcode paths in the line below
LIBS=\
     ../../bin/libOpenWebGlobeProcessing.a \
     $(BOOST)/bin.v2/libs/date_time/build/gcc-4.4.3/release/link-static/threading-multi/libboost_date_time.a \
     $(BOOST)/bin.v2/libs/filesystem/build/gcc-4.4.3/release/link-static/threading-multi/libboost_filesystem.a \
     $(BOOST)/bin.v2/libs/program_options/build/gcc-4.4.3/release/link-static/threading-multi/libboost_program_options.a \
     $(BOOST)/bin.v2/libs/system/build/gcc-4.4.3/release/link-static/threading-multi/libboost_system.a \
     -lgdal1.7.0 \
     -lxerces-c

all: \
	../../bin/calcextent \
	../../bin/createlayer

../../bin/calcextent: \
	../../source/apps/calcextent/main.cpp \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< $(LIBS)

../../bin/createlayer: \
	../../source/apps/createlayer/main.cpp \
	../../bin/libOpenWebGlobeProcessing.a
	g++ -o $@ $(CFLAGS) $< $(LIBS)

../../bin/libOpenWebGlobeProcessing.a: \
	../../source/core/app/Logger.o \
	../../source/core/app/ProcessingSettings.o \
	../../source/core/geo/CoordinateTransformation.o \
	../../source/core/geo/ImageLayerSettings.o \
	../../source/core/geo/MercatorQuadtree.o \
	../../source/core/image/ImageHandler.o \
	../../source/core/image/ImageWriter.o \
	../../source/core/image/lodepng/lodepng.o \
	../../source/core/io/CommonPath.o \
	../../source/core/io/FileSystem.o \
	../../source/core/ogprocess.o \
	../../source/core/math/vec3.o \
	../../source/core/string/ConvertUTF.o \
	../../source/core/string/ConvertUTF.o \
	../../source/core/string/FilenameUtils.o \
	../../source/core/string/StringUtils.o \
	../../source/core/xml/BaseTypeConversion.o \
	../../source/core/xml/ClassExport.o \
	../../source/core/xml/PropertyBase.o \
	../../source/core/xml/Tokenizer.o
	ar rcs $@ $?

%.o: %.cpp
	g++ -c -o $@ $(CFLAGS) $<
