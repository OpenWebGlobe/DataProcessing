################################################################################
#!c:\python27\python.exe
# Author: Robert Wueest, robert.wueest@fhnw.ch
#
# (c) 2012 by FHNW University of Applied Sciences and Arts Northwestern Switzerland
################################################################################

import sys
import cgi
import urllib
import string
from ctypes import *
from os import *
import json
from PIL import Image
import cStringIO

################################################################################
# Mapnik Templates
################################################################################
mapnik_header = """<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE Map [
%(entities)s]>
"""
mapnik_map = """<Map bgcolor=\"%(bgcolor)s\" srs=\"%(srs)s\">
%(layers)s%(styles)s</Map>"""

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

mapnik_style_cssparameter = """                <CssParameter name=\"%(name)s\">%(value)s</CssParameter>
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
            if 'filter' in cRule:
                ruleFilter += "            <Filter>"+cRule["filter"]+"</Filter>\n"
            if 'maxzoom' in cRule:
                ruleZooms += "            &maxscale_zoom"+str(cRule["maxzoom"])+";\n"
            if 'minzoom' in cRule:
                ruleZooms += "            &minscale_zoom"+str(cRule["minzoom"])+";\n"
            for k in range(len(cRule["symbolizers"])):
                tCssParams = ""
                tParams = ""
                cSym = cRule["symbolizers"][k]

                for l in range(len(list(cSym["params"].viewkeys()))):
                    cParam = cSym["params"][list(cSym["params"].viewkeys())[l]]
                    tCssParams += mapnik_style_cssparameter % { 'name': list(cSym["params"].viewkeys())[l], 'value' : cParam}
                    tParams += mapnik_style_parameter % { 'name': list(cSym["params"].viewkeys())[l], 'value' : cParam}
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
    #print tMapnik
    chdir("E:/WebViewer/DataProcessing/bin/")
    libtest = cdll.LoadLibrary('ogMapnikBindings.dll')
    mapnik_dir = c_char_p("mapnik/")
    mapdef = c_char_p(str(tMapnik))
    width = c_int(int(form.getvalue("height")))
    height = c_int(int(form.getvalue("width")))
    lon0 = c_double(float(form.getvalue("lon0")))
    lat0 = c_double(float(form.getvalue("lat0")))
    lon1 = c_double(float(form.getvalue("lon1")))
    lat1 = c_double(float(form.getvalue("lat1")))
    libtest.PyRenderTile.restype = c_char_p
    asize = width.value*height.value*4
    output = create_string_buffer(asize)
    cpo = libtest.PyRenderTile(mapnik_dir,mapdef,width,height,lon0,lat0,lon1,lat1,output)
    lnr = 0
    #for j in range(asize):
     #   print ord(output[j])
    status = "200 OK"
    headers = [('Content-Type', 'image/png'),('Content-Size', str(asize)), ('Access-Control-Allow-Origin', '*') ]
    start_response(status, headers)
    img = Image.frombuffer('RGBA', (width.value, height.value), output, 'raw', 'RGBA', 0, 1)
    f = cStringIO.StringIO()
    img.save(f, "PNG")
    f.seek(0)
    return f.read()

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
			

