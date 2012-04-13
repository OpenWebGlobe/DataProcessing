AR=ar
CXX=g++
MPICXX=mpic++

BOOST=/usr/lib/
BOOST_LIBS=date_time filesystem program_options thread system
BOOST_TOOLSET=gcc-4.4.3
BOOST_VARIANT=release
BOOST_LINK=static
BOOST_THREADING=multi

GDAL_VERSION=1.8.0

CFLAGS=-fopenmp -I$(BOOST) -I/usr/include/freetype2 -I../../source/core -I/usr/include/gdal -O2 -Werror

TARGETS=\
	../../bin/ogAddData \
	../../bin/ogCalcExtent \
	../../bin/ogCreateLayer \
	../../bin/ogDeploy \
	../../bin/ogFileLockTest \
	../../bin/ogResample \
	../../bin/ogTriangulate \
	../../bin/libOpenWebGlobeProcessing.a

LIBS=\
     -Wl,--whole-archive ../../bin/libOpenWebGlobeProcessing.a -Wl,--no-whole-archive \
     $(foreach lib,$(BOOST_LIBS),$(BOOST)/libboost_$(lib).a) \
     -lgdal$(GDAL_VERSION) \
     -lxerces-c \
     -lmapnik

OGADDDATA_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/adddata -name *.cpp))
OGCALCEXTENT_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/calcextent -name *.cpp))
OGCREATELAYER_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/createlayer -name *.cpp))
OGDEPLOY_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/deploy -name *.cpp -not -name main_mpi.cpp))
OGFILELOCKTEST_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/locktest -name *.cpp))
OGRESAMPLE_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/resample -name *.cpp -not -name main_mpi.cpp))
RESAMPLE_MPI_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/resample -name *.cpp -not -name main.cpp))
OGTRIANGULATE_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/apps/triangulate -name *.cpp))
LIBOPENWEBGLOBEPROCESSING_OBJS=$(patsubst %.cpp,%.o,$(shell find ../../source/core -name lodepng -prune -o -name \*.cpp -print))

all: $(TARGETS)

../../bin/ogAddData: $(OGADDDATA_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGADDDATA_OBJS) $(LIBS)

../../bin/ogCalcExtent: $(OGCALCEXTENT_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGCALCEXTENT_OBJS) $(LIBS)

../../bin/ogCreateLayer: $(OGCREATELAYER_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGCREATELAYER_OBJS) $(LIBS)

../../bin/ogDeploy: $(OGDEPLOY_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGDEPLOY_OBJS) $(LIBS)

../../bin/ogFileLockTest: $(OGFILELOCKTEST_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGFILELOCKTEST_OBJS) $(LIBS)

../../bin/ogResample: $(OGRESAMPLE_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGRESAMPLE_OBJS) $(LIBS)

../../bin/resample_mpi: $(RESAMPLE_MPI_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(MPICXX) -o $@ $(CFLAGS) $(RESAMPLE_MPI_OBJS) $(LIBS)

../../source/apps/resample/main_mpi.o: ../../source/apps/resample/main_mpi.cpp
	$(MPICXX) -c -o $@ $(CFLAGS) $<

../../bin/ogTriangulate: $(OGTRIANGULATE_OBJS) ../../bin/libOpenWebGlobeProcessing.a
	$(CXX) -o $@ $(CFLAGS) $(OGTRIANGULATE_OBJS) $(LIBS)

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
	rm -f $(LIBOPENWEBGLOBEPROCESSING_OBJS)
	rm -f $(TARGETS)

%.o: %.cpp
	$(CXX) -c -o $@ $(CFLAGS) $<
