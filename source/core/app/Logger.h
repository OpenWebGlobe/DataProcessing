
#include "og.h"
#include <string>
#include <fstream>

#ifndef _OG_LOGGER_H
#define _OG_LOGGER_H

class OPENGLOBE_API Logger
{
public:
   Logger(const std::string& sLogPath, const std::string& appname, bool bCloneOutput = false);
   virtual ~Logger();

   void Warn(const std::string& warning);
   void Info(const std::string& info);
   void Error(const std::string& error);
   
protected:
   std::ofstream out;
   bool _bCloneOutput;
};

#endif

