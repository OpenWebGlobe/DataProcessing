OpenWebGlobe Data Processing
============================

For performance reasons geodata must be preprocessed before being used on the virtual globe.

This repository contains preprocessing applications for image data, elevation data, 3D-objects, point clouds.


Building
========

Data Processing requires external dependencies
For Windows / Visual Studio 2010 you can download them here:

http://www.openwebglobe.org/downloads/dataprocessing_external_win32.tar.gz

Dependencies:

* boost
* gdal
* proj.4
* zlib
* jpeg-8c
* icu
* libcurl
* libpng
* libtiff
* libxml2
* stxxl
* mapnik
* libpq
* freetype2
* libtool
* libxml2
* mpi

Building on linux:
==================

Step by step

==Install Git==
sudo apt-get install git-core

==Clone Repository==
git clone https://{username}@github.com/OpenWebGlobe/DataProcessing.git

==Install GDAL==
sudo apt-get install gdal-bin
sudo apt-get install libgdal1-dev

==Install Mapnik Dependencies==
sudo apt-get install -y libltdl3-dev libpng12-dev libtiff4-dev libicu-dev
sudo apt-get install -y python-dev python-cairo-dev python-nose
sudo apt-get install libboost-python1.46-dev
sudo apt-get install libboost-filesystem1.46-dev
sudo apt-get install libboost-system1.46-dev
sudo apt-get install libboost-regex1.46-dev
sudo apt-get install libboost-thread1.46-dev
sudo apt-get install libboost-program-options1.46-dev
sudo apt-get install libboost-iostreams1.46-dev
sudo apt-get install -y libfreetype6-dev libcairo2-dev libcairomm-1.0-dev
sudo apt-get install -y libgeotiff-dev libtiff4 libtiff4-dev libtiffxx0c2
sudo apt-get install -y libsigc++-dev libsigc++0c2 libsigx-2.0-2 libsigx-2.0-dev
sudo apt-get install -y python-gdal python-imaging
sudo apt-get install  ttf-unifont ttf-dejavu ttf-dejavu-core ttf-dejavu-extra
sudo apt-get install -y imagemagick ttf-dejavu
sudo apt-get install libgdal1-dev python-gdal postgresql-8.4 postgresql-server-dev-8.4 postgresql-contrib-8.4 postgresql-8.4-postgis libsqlite3-dev
sudo apt-get install -y g++ cpp libicu-dev libxml2 libxml2-dev libjpeg-dev libltdl7 libltdl-dev libpng-dev build-essential python-nose

==Install Mapnik In {your_path}/DataProcessing==

mkdir external
cd external
git clone http://github.com/mapnik/mapnik
cd mapnik
./configure && make && sudo make install

