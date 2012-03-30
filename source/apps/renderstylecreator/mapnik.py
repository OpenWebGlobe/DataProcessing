################################################################################
#!c:\python27\python.exe
#
# Server side python execution using WSGI
# Author: Martin Christen, martin.christen@fhnw.ch
#
# Run this script then open your webbrowser and enter:
#   localhost:8000/?a=5
#   localhost:8000/?a=1&b=20
#   localhost:8000/?c=123
#
# or install apache with mod_wsgi and run it directly on your website!
#
# (c) 2012 by FHNW University of Applied Sciences and Arts Northwestern Switzerland
################################################################################

import os.path
import os
import sys
import cgi

json_template = """\
[
  Version: "1.0",
  Tool: "JSON-Test",
  Message: \"%(GET)s\",
  PythonVersion: \"%(version)s\"
]
"""

################################################################################
message_text = "empty!"
################################################################################

def application(environ, start_response):
    # emit status / headers
    status = "200 OK"
    headers = [('Content-Type', 'application/json'), ('Access-Control-Allow-Origin', '*') ]
    #headers = [('content-type', 'text/plain')]
    start_response(status, headers)

    form = cgi.FieldStorage(fp=environ['wsgi.input'],environ=environ)

    #---------------------------------------------------------------------------
    #check if input is correct!
    if "a" not in form or "b" not in form:
       message_text = "please specify a or b!!"

    if "a" in form and "b" not in form:
       message_text = "only a is defined: a=" + str(form.getvalue("a"))

    if "b" in form and "a" not in form:
       message_text = "only b is defined: b=" + str(form.getvalue("b"))

    if "a" in form and "b" in form:
       message_text = "a=" + str(form.getvalue("a")) + " b=" + str(form.getvalue("b"))
    #---------------------------------------------------------------------------

    # assemble and return content
    content = json_template % {
	    'GET': message_text,
       'version': sys.version
    }
	
    return [content]

#-------------------------------------------------------------------------------
# FOR STAND ALONE EXECUTION / DEBUGGING:
#-------------------------------------------------------------------------------
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
			

