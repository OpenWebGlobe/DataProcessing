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

#include "Get.h"

#include "string/FilenameUtils.h"
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

using boost::asio::ip::tcp;

//------------------------------------------------------------------------------
void operator<<(std::vector<unsigned char>& vec, std::streambuf* sb)
{
   while (sb->in_avail())
   {
      unsigned char c = sb->sbumpc();
      vec.push_back(c);
   }
}
//------------------------------------------------------------------------------

unsigned int HttpGet::Request(const std::string& url, std::vector<unsigned char>& vData, Header* pHeader)
{
   std::string host;
   std::string filename;
   unsigned int status_code = 0;

   vData.clear();
   if (pHeader)
   {
      pHeader->GetHeader().clear();
   }

   FilenameUtils::ParseUrl(url, host, filename);

   try
	{
      boost::asio::io_service io_service;
      tcp::resolver resolver(io_service);
      tcp::resolver::query query( host, "http" );
      tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
      tcp::resolver::iterator end;
      tcp::socket socket(io_service);
      boost::system::error_code error = boost::asio::error::host_not_found;

      while (error && endpoint_iterator != end)
		{
			socket.close();
			socket.connect(*endpoint_iterator++, error);
		}

		if (error)
		{
      	throw boost::system::system_error(error);
      }

      boost::asio::streambuf request;
		std::ostream request_stream(&request);
		request_stream << "GET " << filename << "" << " HTTP/1.1\r\n";
		request_stream << "Host: " << host << "\r\n";   // The domain name of the server (for virtual hosting), mandatory since HTTP/1.1
		request_stream << "Accept: */*\r\n";                  // Content-Types that are acceptable
		request_stream << "Connection: Close\r\n\r\n";        // What type of connection the user-agent would prefer
      
      int n = boost::asio::write( socket, request );

      boost::asio::streambuf header;
		std::istream header_stream(&header);
		boost::asio::read_until( socket, header, "\r\n\r\n" );
      
      status_code = 0;

      std::string http_version;  
		std::string status_message;  
		std::string headBuff;
		header_stream >> http_version;  
		header_stream >> status_code;

      std::string first, second;

		while ( std::getline(header_stream, headBuff) && headBuff != "\r" )
		{
         first.clear();
         second.clear();
         int stat = 0;
			for (size_t i=0;i<headBuff.length();i++)
         {
            if (stat == 0)
            {
               if (headBuff[i] == ':')
               {
                 stat = 1;
               }
               else
               {
                  if (headBuff[i] >= 32)
                     first += headBuff[i];
               }
            }
            else
            {
              if (stat == 1)
              {
                  // ignore spaces
                  if (headBuff[i] != ' ')
                  {
                     stat = 2;
                     if (headBuff[i] >= 32)
                        second += headBuff[i];
                  }
              }
              else
              {   
                  if (headBuff[i] >= 32)
                     second += headBuff[i];
              }
            }
         }
        
         if (first.length()>0 && second.length()>0)
         {
            if (pHeader)
            {
               pHeader->GetHeader().push_back(std::pair<std::string, std::string>(first, second));
            }
         }
		}


		if( status_code/100 != 2 || http_version.substr( 0, 5 ) != "HTTP/" )
		{
			throw boost::system::system_error( error, (boost::lexical_cast<std::string>(status_code)).c_str() );
		}


      std::ostringstream oss;
		while( true )
		{
			boost::asio::read( socket, header, boost::asio::transfer_at_least(1), error );
			if(header.size() == 0)
			{
				break;
			}
         
         vData << &header;

		}

		if (error != boost::asio::error::eof)
		{
			throw boost::system::system_error(error);
		}
		socket.close();


   }
   catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << __LINE__ << "\n";
      if (status_code == 0)
      {
         status_code = 404;
      }
		return status_code;
	}

   return status_code;
}

