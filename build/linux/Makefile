AR=ar
CXX=g++
MPICXX=mpic++

BOOST=/usr/lib/
BOOST_LIBS=date_time filesystem program_options thread system
BOOST_TOOLSET=gcc-4.4.3
BOOST_VARIANT=release
BOOST_LINK=static
BOOST_THREADING=multi

GDAL_VERSION=

CFLAGS= -DMAPNIK_2 -fopenmp -I$(BOOST) -I/usr/include/freetype2 -I../../source/core -I/usr/include/gdal -O2 -Werror

TARGETS=\
	../../bin/ogAddData \
	../../bin/ogCalcExtent \
	../../bin/ogCreateLayer \
	../../bin/ogDeploy \
	../../bin/ogFileLockTest \
	../../bin/ogResample \
	../../bin/ogTileRenderer \
	../../bin/ogHillshading \
	../../bin/ogTriangulate \
	../../bin/libOpenWebGlobeProcessing.so \
	../../bin/libOpenWebGlobeProcessing.a

LIBS=\
     $(foreach lib,$(BOOST_LIBS),$(BOOST)/libboost_$(lib).so) \
     -lgdal$(GDAL_VERSION) \
     -lxerces-c \
     -lmapnik2

LIBSSTATIC=\
	-Wl,--whole-archive ../../bin/libOpenWebGlobeProcessing.a -Wl,--no-whole-archive \
	$(foreach lib,$(BOOST_LIBS),$(BOOST)/libboost_$(lib).a) \
	-lgdal$(GDAL_VERSION) \
	-lxerces-c \
	-lmapnik2

OGADDDATA_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/adddata -name *.cpp))
OGCALCEXTENT_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/calcextent -name *.cpp))
OGCREATELAYER_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/createlayer -name *.cpp))
OGDEPLOY_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/deploy -name *.cpp -not -name main_mpi.cpp))
OGFILELOCKTEST_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/locktest -name *.cpp))
OGTILERENDER_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/tilerenderer -name *.cpp -not -name main_mpi.cpp -and -not -name main_mpi_mdb.cpp -and -not -name main.cpp -and -not -name render_image.cpp -and -not -name rundemo.cpp))
OGHILLSHADING_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/hillshading -name *.cpp -not -name main_mpi.cpp -and -not -name main.cpp))
OGRESAMPLE_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/resample -name *.cpp -not -name main_mpi.cpp))
RESAMPLE_MPI_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/resample -name *.cpp -not -name main.cpp))
OGTRIANGULATE_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/triangulate -name *.cpp))
LIBOPENWEBGLOBEPROCESSING_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/core -name lodepng -prune -o -name \*.cpp -print))

all: $(TARGETS)

../../bin/ogAddData: $(OGADDDATA_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGADDDATA_OBJS) $(LIBSSTATIC)

../../bin/ogCalcExtent: $(OGCALCEXTENT_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGCALCEXTENT_OBJS) $(LIBSSTATIC)

../../bin/ogCreateLayer: $(OGCREATELAYER_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGCREATELAYER_OBJS) $(LIBSSTATIC)

../../bin/ogDeploy: $(OGDEPLOY_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGDEPLOY_OBJS) $(LIBSSTATIC)

../../bin/ogFileLockTest: $(OGFILELOCKTEST_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGFILELOCKTEST_OBJS) $(LIBSSTATIC)

../../bin/ogTileRenderer: $(OGTILERENDER_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGTILERENDER_OBJS) $(LIBSSTATIC)

../../bin/ogHillshading: $(OGHILLSHADING_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGHILLSHADING_OBJS) $(LIBSSTATIC)

../../bin/ogResample: $(OGRESAMPLE_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGRESAMPLE_OBJS) $(LIBSSTATIC)

../../bin/resample_mpi: $(RESAMPLE_MPI_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(MPICXX) -o $@ $(CFLAGS) $(RESAMPLE_MPI_OBJS) $(LIBSSTATIC)

../../source/apps/resample/main_mpi.o: ../../source/apps/resample/main_mpi.cpp
	$(MPICXX) -c -o $@ $(CFLAGS) $<

../../bin/ogTriangulate: $(OGTRIANGULATE_OBJS) ../../bin/libOpenWebGlobeProcessing.so
	$(CXX) -o $@ $(CFLAGS) $(OGTRIANGULATE_OBJS) $(LIBSSTATIC)

../../bin/libOpenWebGlobeProcessing.o: $(LIBOPENWEBGLOBEPROCESSING_OBJS)
	$(CXX) -c -fPIC $(CFLAGS) $@ $?

../../bin/libOpenWebGlobeProcessing.so: $(LIBOPENWEBGLOBEPROCESSING_OBJS)
	$(CXX) -Wall -Wextra -shared -rdynamic -o ../../bin/libOpenWebGlobeProcessing.so $? $(LIBS)

../../bin/libOpenWebGlobeProcessing.a: $(LIBOPENWEBGLOBEPROCESSING_OBJS)
	$(AR) rcs $@ $?

clean:
	rm -f $(OGADDDATA_OBJS)
	rm -f $(OGCALCEXTENT_OBJS)
	rm -f $(OGCREATELAYER_OBJS)
	rm -f $(OGDEPLOY_OBJS)
	rm -f $(OGFILELOCKTEST_OBJS)
	rm -f $(OGRESAMPLE_OBJS)
	rm -f $(RESAMPLE_MPI_OBJS)
	rm -f $(OGTRIANGULATE_OBJS)
	rm -f $(OGTILERENDERER_OBJS)
	rm -f $(OGHILLSHADING_OBJS)
	rm -f $(LIBOPENWEBGLOBEPROCESSING_OBJS)
	rm -f $(TARGETS)

%.o: %.cpp
	$(CXX) -c -o  $@ -fPIC $(CFLAGS) $<
