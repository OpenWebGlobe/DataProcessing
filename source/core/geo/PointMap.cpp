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

#pragma warning (disable : 4290 )
#pragma warning (disable : 4250 )
#pragma warning (disable : 4244 )

#define STXXL_BOOST_RANDOM
#define STXXL_BOOST_CONFIG
#define STXXL_BOOSTss_RANDOM
#define STXXL_BOOST_TIMESTAMP
#define STXXL_BOOST_THREADS
#include "stxxl/all"


//------------------------------------------------------------------------

typedef int64 key_type;          // key is octocode
typedef unsigned int data_type;  // data: number of points in octocode

struct cmp : public std::less<key_type>
{
    static key_type min_value()
    {
        return (std::numeric_limits<key_type>::min)();
    }
    static key_type max_value()
    {
        return (std::numeric_limits<key_type>::max)();
    }
};

#define BLOCK_SIZE (32 * 1024)
#define CACHE_SIZE (2 * 1024 * 1024 / BLOCK_SIZE)
#define CACHE_ELEMENTS (BLOCK_SIZE * CACHE_SIZE / (sizeof(key_type) + sizeof(data_type)))

typedef stxxl::map<key_type, data_type, cmp, BLOCK_SIZE, BLOCK_SIZE> map_type;
typedef map_type::iterator map_iterator;

class PointMap_private
{
public:
   // ctor
   PointMap_private()
   {  
      _index = new map_type(CACHE_SIZE * BLOCK_SIZE / 2, CACHE_SIZE * BLOCK_SIZE / 2);
      _resetiterator = true;
   }
   // dtor
   virtual ~PointMap_private()
   {
      if (_index)
      {
         delete _index;
      }
   }
   // map
   map_type* _index;
   bool      _resetiterator;
   map_iterator _it;
   
};

//------------------------------------------------------------------------
PointMap::PointMap(int levelofdetail)
{
   _numpts = 0;
   _numkeys = 0;
   _lod = levelofdetail;
   _pow = int64(1) << _lod; 
   _dpow = _pow * _pow;
   _pPriv = new PointMap_private();

   
}
//------------------------------------------------------------------------
PointMap::~PointMap()
{
   if (_pPriv)
   {
      delete _pPriv;
      _pPriv = 0;
   }
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

      _pPriv->_index->insert(std::pair<int64,unsigned int>(key, 0));
   }
}
//------------------------------------------------------------------------
size_t PointMap::GetNumPoints()
{
   return _numpts;
}
//------------------------------------------------------------------------

void PointMap::ExportData(const std::string& path)
{
   std::map<int64, std::list<CloudPoint> >::iterator it = _map.begin();

   while (it != _map.end())
   {
      int64 key = it->first;
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
         of.write((char*)&jt->r, sizeof(unsigned char));
         of.write((char*)&jt->g, sizeof(unsigned char));
         of.write((char*)&jt->b, sizeof(unsigned char));
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
   map_iterator it = _pPriv->_index->begin();

   std::ofstream of(sFilename.c_str(), std::ios::binary);
  
   if (of.good())
   {
      while (it != _pPriv->_index->end())
      {
         int64 elm = it->first;
         of.write((char*)&elm, sizeof(int64));
         it++;
      }
   }

   of.close();
}

//-------------------------------------------------------------------------


void PointMap::ImportIndex(const std::vector<std::string>& sIndexFiles)
{
   Clear();
   _pPriv->_index->clear();
   
   // create map of all tiles:
   for (size_t i=0;i<sIndexFiles.size();i++)
   {
      //std::cout << "found: " << sIndexFiles[i] << "\n";
      std::ifstream ifs(sIndexFiles[i].c_str(), std::ios::binary);
      int64 value;
   
      if (ifs.good())
      {
         while (!ifs.eof())
         {
            ifs.read((char*)&value, sizeof(int64));
            _pPriv->_index->insert(std::pair<int64,unsigned int>(value, 0));
         }
      }

     
   
      ifs.close();
   }

}

//------------------------------------------------------------------------------

int64 PointMap::GetIndexSize()
{
   return _pPriv->_index->size();
}

//------------------------------------------------------------------------------

bool PointMap::GetNextIndex(int64& idx)
{
   if (_pPriv->_resetiterator)
   {
      _pPriv->_it = _pPriv->_index->begin();
      _pPriv->_resetiterator = false;
   }

   if (_pPriv->_it != _pPriv->_index->end())
   {
      idx = _pPriv->_it->first;
      _pPriv->_it++;
      return true;
   }
   else
   {
      return false;
   }
}

//------------------------------------------------------------------------------