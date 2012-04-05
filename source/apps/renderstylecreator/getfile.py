################################################################################
#!c:\python27\python.exe
# Author: Robert Wueest, robert.wueest@fhnw.ch
#
# (c) 2012 by FHNW University of Applied Sciences and Arts Northwestern Switzerland
################################################################################
import os.path
import os
import sys
import cgi
import urllib
import json
################################################################################
def application(environ, start_response):
    form = cgi.FieldStorage(fp=environ['wsgi.input'],environ=environ)
    data = form.getvalue("data")
    out = json.loads(data)
    output = json.dumps(out, sort_keys=False, indent=3)
    headers = [
        ("Content-Disposition", "attachment; filename=manik_style.json"),
        ("Content-Type", "text/force-download"),
        ("Content-Length", str(len(output))),
        ("Connection", "close")
    ]
    start_response("200 OK", headers)
    return output

################################################################################
# FOR STAND ALONE EXECUTION / DEBUGGING:
################################################################################
if __name__ == '__main__':
    # this runs when script is started directly from commandline
    try:
        # create a simple WSGI server and run the application
        from wsgiref import simple_server
        print "Running test application - point your browser at http://localhost:8000/ ..."
        httpd = simple_server.WSGIServer(('', 8000), simple_server.WSGIRequestHandler)
        httpd.set_app(application)
        httpd.serve_forever()
    except ImportError:
        # wsgiref not installed, just output html to stdout
        for content in application({}, lambda status, headers: None):
            print content
            #-------------------------------------------------------------------------------