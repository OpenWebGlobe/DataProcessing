
#include "og.h"
#include <string>
#include <fstream>

class OPENGLOBE_API Logger
{
public:
   Logger(const std::string& sLogPath);
   virtual ~Logger();

   void Warn(const std::string& warning);
   void Info(const std::string& info);
   void Error(const std::string& error);
   
   // direct access to file stream
   std::ofstream out;
};

