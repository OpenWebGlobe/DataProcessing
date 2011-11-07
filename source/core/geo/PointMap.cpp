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
#                           martin.christen@fhnw.ch                            #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/

#include "PointMap.h"


//------------------------------------------------------------------------
PointMap::PointMap(int levelofdetail)
{
   _numpts = 0;
   _numkeys = 0;
   _lod = levelofdetail;
   _pow = int64(1) << _lod; 
   _dpow = _pow * _pow;
}
//------------------------------------------------------------------------
virtual PointMap::~PointMap()
{
}
//------------------------------------------------------------------------
void PointMap::Clear()
{
   _map.clear();
   _numpts = 0;
   _numkeys = 0;
}
//------------------------------------------------------------------------
void PointMap::AddPoint(int64 i, int64 j, int64 k, const CloudPoint& pt)
{
   int64 key = _dpow*k + _pow*j + i;

   _numpts++;
   
   std::map<int64, std::list<CloudPoint> >::iterator it;
   it = _map.find(key);
   if (it != _map.end())
   {
      // this key already exists, append point
      it->second.push_back(pt);
   }
   else
   {
      // this key doesn't exist yet. create new entry
      std::list<CloudPoint> newList;
      newList.push_back(pt);
      _map.insert(std::pair<int64, std::list<CloudPoint> >(key, newList));
      _numkeys++;
   }
}
//------------------------------------------------------------------------
size_t PointMap::GetNumPoints()
{
   return _numpts;
}
//------------------------------------------------------------------------
size_t PointMap::GetNumKeys()
{
    return _index.size();
}
//------------------------------------------------------------------------

void PointMap::ExportData(const std::string& path)
{
   std::map<int64, std::list<CloudPoint> >::iterator it = _map.begin();

   while (it != _map.end())
   {
      int64 key = it->first;
      _index.insert(key);
      int64 i,j,k;

      k = key / _dpow;
      j = (key - _dpow*k) / _pow;
      i = key - _dpow*k - j*_pow;

      //std::string sOctocode = Octocode::IndexToOctocode(i,j,k,_lod);
      //create or open existing file: path/lod/x/y-z.json

      std::ostringstream oss;
      oss << path << _lod << "/" << i << "/" << j << "-" << k << ".dat";
      std::string sFilename = oss.str();


      std::ofstream of;


      if (FileSystem::FileExists(sFilename))
      {
         of.open(sFilename.c_str(), std::ios::app|std::ios::binary);
      }
      else
      {
         FileSystem::makeallsubdirs(sFilename);
         of.open(sFilename.c_str(), std::ios::binary);
      }

      of.precision(17);

      std::list<CloudPoint>& lstCloudPoints = it->second;

      std::list<CloudPoint>::iterator jt = lstCloudPoints.begin();
      while (jt != lstCloudPoints.end())
      {
         of.write((char*)&jt->x, sizeof(double));
         of.write((char*)&jt->y, sizeof(double));
         of.write((char*)&jt->elevation, sizeof(double));
         of.write((char*)&jt->r, sizeof(double));
         of.write((char*)&jt->g, sizeof(double));
         of.write((char*)&jt->b, sizeof(double));
         of.write((char*)&jt->intensity, sizeof(int));

         jt++;
      }

      of.close();

      ++it;
   }
}

//------------------------------------------------------------------------

void PointMap::ExportIndex(const std::string& sFilename)
{
   std::set<int64>::iterator it = _index.begin();

   std::ofstream of(sFilename.c_str(), std::ios::binary);
  
   if (of.good())
   {
      while (it != _index.end())
      {
         int64 elm = *it;
         of.write((char*)&elm, sizeof(int64));
      
         it++;
      }
   }

   of.close();
}

//------------------------------------------------------------------------

std::set<int64>& PointMap::GetIndex()
{
   return _index;
}

//------------------------------------------------------------------------

