//------------------------------------------------------------------------------

#include "gdal.h"
#include "ogr_api.h"
#include "cpl_conv.h"

//------------------------------------------------------------------------------

void init_gdal()
{
   GDALAllRegister();
   CPLSetConfigOption("GDAL_CACHEMAX", "0");  
   CPLSetConfigOption("GDAL_DATA", "gdal-data");
   OGRRegisterAll();
}

//------------------------------------------------------------------------------

void exit_gdal()
{
   GDALDestroyDriverManager();
   OGRCleanupAll();
}


//------------------------------------------------------------------------------




int main(void)
{
   return 0;
}