################################################################################
#!c:\python27\python.exe
# Author: Robert Wüest, robert.wueest@fhnw.ch
#
# (c) 2012 by FHNW University of Applied Sciences and Arts Northwestern Switzerland
################################################################################

import os.path
import os
import sys
import cgi
from ctypes import *

################################################################################
# Mapnik Templates
################################################################################
mapnik_header = """\
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE Map [
%(entities)s]>
"""
mapnik_map = """\
<Map bgcolor=\"%(bgcolor)s\" srs=\"%(srs)s\">
%(definitions)s
</Map>
"""

mapnik_entity = """\
    <!ENTITY %(name)s \"%(value)s\">
"""

mapnik_minscaledenominator_entity = """\
    <!ENTITY minscale_zoom%(zoom)i \"<MinScaleDenominator>%(denominator)s</MinScaleDenominator>\">
"""

mapnik_maxscaledenominator_entity = """\
    <!ENTITY maxscale_zoom%(zoom)i \"<MaxScaleDenominator>%(denominator)s</MaxScaleDenominator>\">
"""

mapnik_style = """\
    <style name = \"%(name)s\">
%(rules)s
    </style>
"""

mapnik_style_rule = """\
        <Rule>
%(zoom)s
%(filter)s
%(symbolizers)s
        </Rule>
"""

mapnik_style_polygonsymbolizer = """\
            <PolygonSymbolizer>
%(cssparams)s
            </PolygonSymbolizer>
"""

mapnik_style_linesymbolizer = """\
            <LineSymbolizer>
%(cssparams)s
            </LineSymbolizer>
"""

mapnik_style_polygonpatternsymbolizer = """\
            <PolygonPatternSymbolizer  file=\"%(file)s\" type=\"%(type)s\" width=\"%(width)s\" height=\"%(height)s\"/>
"""

mapnik_style_textsymbolizer = """\
            <TextSymbolizer name=\"%(name)s\" face_name=\"%(font_face)s\" size=\"%(size)s\" dx=\"%(dx)s\"dy=\"%(dy)s\" fill=\"%(fill)s\" halo_radius=\"%(halo_radius)s\" wrap_width=\"%(wrap_width)s\"/>
"""

mapnik_style_cssparameter = """\
                <CssParameter name=\"%(name)s\"/>%(value)s</CssParameter>
"""

mapnik_layer = """\
    <Layer name=\"%(name)s\" status=\"%(status)s\" srs=\"%(srs)\">
        <StyleName>%(name)s</StyleName>
        <Datasource>
%(params)s
        </Datasource>
    </Layer>
"""

mapnik_layer_param = """\
            <Parameter name=\"%(name)s\">%(value)s</Parameter>
"""
################################################################################
# application
################################################################################
def application(environ, start_response):

    status = "200 OK"
    headers = [('Content-Type', 'application/json'), ('Access-Control-Allow-Origin', '*') ]
    #headers = [('content-type', 'text/plain')]
    start_response(status, headers)

    form = cgi.FieldStorage(fp=environ['wsgi.input'],environ=environ)

    return [content]

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
			

