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
from ctypes import *
import json

################################################################################
# Mapnik Templates
################################################################################
mapnik_header = """<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE Map [
%(entities)s]>
"""
mapnik_map = """<Map bgcolor=\"%(bgcolor)s\" srs=\"%(srs)s\">
%(layers)s%(styles)s</Map>
"""

mapnik_entity = """    <!ENTITY %(name)s \"%(value)s\">
"""

mapnik_minscaledenominator_entity = """    <!ENTITY minscale_zoom%(zoom)i \"<MinScaleDenominator>%(denominator)s</MinScaleDenominator>\">
"""

mapnik_maxscaledenominator_entity = """    <!ENTITY maxscale_zoom%(zoom)i \"<MaxScaleDenominator>%(denominator)s</MaxScaleDenominator>\">
"""

mapnik_style = """    <Style name = \"%(name)s\">
%(rules)s    </Style>
"""

mapnik_style_rule = """        <Rule>
%(zoom)s%(filter)s%(symbolizers)s        </Rule>
"""

mapnik_style_polygonsymbolizer = """            <PolygonSymbolizer>
%(cssparams)s            </PolygonSymbolizer>
"""

mapnik_style_linesymbolizer = """            <LineSymbolizer>
%(cssparams)s            </LineSymbolizer>
"""

mapnik_style_polygonpatternsymbolizer = """            <PolygonPatternSymbolizer  %(params)s />
"""

mapnik_style_textsymbolizer = """            <TextSymbolizer %(params)s />
"""

mapnik_style_cssparameter = """                <CssParameter name=\"%(name)s\"/>%(value)s</CssParameter>
"""

mapnik_style_parameter = """ %(name)s=\"%(value)s\""""

mapnik_layer = """    <Layer name=\"%(name)s\" status=\"%(status)s\" srs=\"%(srs)s\">
        <StyleName>%(stylename)s</StyleName>
        <Datasource>
%(params)s        </Datasource>
    </Layer>
"""

mapnik_layer_param = """            <Parameter name=\"%(name)s\">%(value)s</Parameter>
"""
################################################################################
# render app
################################################################################
def application(environ, start_response):

    status = "200 OK"
    headers = [('Content-Type', 'application/json'), ('Access-Control-Allow-Origin', '*') ]
    #headers = [('content-type', 'text/plain')]
    start_response(status, headers)

    form = cgi.FieldStorage(fp=environ['wsgi.input'],environ=environ)
    s = form.getvalue("style")
    z = form.getvalue("zooms")
    l = form.getvalue("layers")
    p = form.getvalue("srs")
    c = form.getvalue("bgcolor")
    styles = json.loads(form.getvalue("style"))
    zooms = json.loads(form.getvalue("zooms"))
    layers = json.loads(form.getvalue("layers"))
    srs = form.getvalue("srs")
    bgcolor = form.getvalue("bgcolor")
    # -- template
    tMapnik = ""
    tHeaders = ""
    for i in range(0, len(zooms)-1):
        tHeaders += mapnik_maxscaledenominator_entity % {'zoom': i,'denominator': zooms[i]}
    for i in range(2, len(zooms)):
        ii = i-1
        tHeaders += mapnik_minscaledenominator_entity % {'zoom': ii,'denominator': zooms[i]}
    tLayers = ""

    for h in range(len(layers)):
        tParams = ""
        cLayer = layers[h]
        if cLayer["type"] == "db":
            tParams += mapnik_layer_param % { 'name': "type", 'value': cLayer["params"]["dbtype"] }
            tParams += mapnik_layer_param % { 'name': "host", 'value': cLayer["params"]["dbhost"] }
            tParams += mapnik_layer_param % { 'name': "port", 'value': cLayer["params"]["dbport"] }
            tParams += mapnik_layer_param % { 'name': "dbname", 'value': cLayer["params"]["dbname"] }
            tParams += mapnik_layer_param % { 'name': "user", 'value': cLayer["params"]["dbuser"] }
            tParams += mapnik_layer_param % { 'name': "password", 'value': cLayer["params"]["dbpass"] }
            tParams += mapnik_layer_param % { 'name': "estimate_extent", 'value': cLayer["params"]["dbestimate"] }
            tParams += mapnik_layer_param % { 'name': "table", 'value': cLayer["params"]["dbtable"] }
        elif cLayer["type"] == "file":
            tParams += mapnik_layer_param % { 'name': "type", 'value': cLayer["params"]["type"] }
            tParams += mapnik_layer_param % { 'name': "file", 'value': cLayer["params"]["file"] }
        tLayers += mapnik_layer % { 'name': cLayer["name"], 'stylename': cLayer["styleId"],'srs': cLayer["srs"], 'status': cLayer["status"], 'params': tParams }
    tStyles = ""
    for i in range(len(styles)):
        tRules = ''
        cStyle = styles[i]
        for j in range(len(cStyle["rules"])):
            tSymbolizers = ""
            cRule = cStyle["rules"][j]
            ruleFilter = ""
            ruleZooms = ""
            if cRule["filter"]:
                ruleFilter += "            <Filter>"+cRule["filter"]+"</Filter>\n"
            if cRule["maxzoom"]:
                ruleZooms += "            &maxscale_zoom"+str(cRule["maxzoom"])+";\n"
            if cRule["minzoom"]:
                ruleZooms += "            &minscale_zoom"+str(cRule["minzoom"])+";\n"
            for k in range(len(cRule["symbolizers"])):
                tCssParams = ""
                tParams = ""
                cSym = cRule["symbolizers"][k]
                for l in range(len(cSym["params"])):
                    tCssParams += mapnik_style_cssparameter % { 'name': cSym["params"][l][0], 'value' : cSym["params"][l][1]}
                    tParams += mapnik_style_parameter % { 'name': cSym["params"][l][0], 'value' : cSym["params"][k][l]}
                if cSym["type"] == "polygon":
                    tSymbolizers += mapnik_style_polygonsymbolizer % {'cssparams' : tCssParams }
                elif cSym["type"] == "polygonpattern":
                    tSymbolizers += mapnik_style_polygonpatternsymbolizer % {'params' : tParams }
                elif cSym["type"] == "line":
                    tSymbolizers += mapnik_style_linesymbolizer % {'cssparams' : tCssParams }
                elif cSym["type"] == "text":
                    tSymbolizers += mapnik_style_textsymbolizer % {'params' : tParams }
            tRules += mapnik_style_rule % { "zoom" : ruleZooms, "filter" : ruleFilter, "symbolizers" : tSymbolizers }
        tStyles += mapnik_style % { 'name' : styles[i]['name'], "rules" : tRules}

    tMapnik += mapnik_header % {'entities' : tHeaders}
    tMapnik += mapnik_map % { 'bgcolor' : bgcolor, 'srs': srs, 'layers': tLayers, 'styles': tStyles }
    print tMapnik
    return ""

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
			

