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

#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

#include <string>
#include <vector>
#include <boost/thread/thread.hpp>


typedef std::vector<std::pair<std::string,std::string> > HeaderFieldDefinition;

class downloadTask
{
public:
	enum alertType
	{
		process,
		finish_all,
		abort
	};

	typedef void(*mpf)(downloadTask*,unsigned long, void*);

   downloadTask( std::string remoteUrl, std::string locationUrl, mpf callback = NULL, void* userData = NULL);
	virtual ~downloadTask();

   // request file size from server
	unsigned long getFileLength();

   // start download
	bool download();

   // Get size of downloaded file
	uintmax_t getDownloaded();

   // Get current download rate
	unsigned long getDownRate();

   // Get URL
	std::string getUrl() const { return m_remoteUrl; }

   // Get name of requested file
	std::string getFileName() const { return m_fileName; }

   // Get target file
	std::string getLocationUrl() const { return m_locationUrl; }

   // parse URL
	static void parseUrl( std::string url, std::string &host, std::string &fileName );

   // Returns true if finished.
	bool isFinish() const { return m_isFinish; }

   // retrieve status code, such as:
   // 200 = Ok, 403 = Forbidden, 404 = Not Found
   // for a list of status codes see here: http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
   unsigned int GetStatusCode(){return _status_code;}

   // Returns complete header field definitions (only after download is complete)
   const HeaderFieldDefinition& GetHeaderFieldDefinitions() { return _vHeader;}

protected:
	std::string			m_remoteUrl;
	std::string			m_locationUrl;

	std::string			m_hostName;
	std::string			m_fileName;
	unsigned long		m_blockCount;

	unsigned long     m_fileLength;
	uintmax_t         m_downloaded;
	unsigned long     m_downRate;

	mpf					m_event;
	void*				   m_userData;

	bool				   m_isFinish;

   unsigned int      _status_code;  

   

   HeaderFieldDefinition _vHeader;
	
protected:
	void alert( alertType type );
};

#endif



