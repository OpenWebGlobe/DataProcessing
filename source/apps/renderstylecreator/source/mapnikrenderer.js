/*******************************************************************************
 #      ____               __          __  _      _____ _       _               #
 #     / __ \              \ \        / / | |    / ____| |     | |              #
 #    | |  | |_ __   ___ _ __ \  /\  / /__| |__ | |  __| | ___ | |__   ___      #
 #    | |  | | '_ \ / _ \ '_ \ \/  \/ / _ \ '_ \| | |_ | |/ _ \| '_ \ / _ \     #
 #    | |__| | |_) |  __/ | | \  /\  /  __/ |_) | |__| | | (_) | |_) |  __/     #
 #     \____/| .__/ \___|_| |_|\/  \/ \___|_.__/ \_____|_|\___/|_.__/ \___|     #
 #           | |                                                                #
 #           |_|                 _____ _____  _  __                             #
 #                              / ____|  __ \| |/ /                             #
 #                             | (___ | |  | | ' /                              #
 #                              \___ \| |  | |  <                               #
 #                              ____) | |__| | . \                              #
 #                             |_____/|_____/|_|\_\                             #
 #                                                                              #
 #                              (c) 2011-2012 by                                #
 #           University of Applied Sciences Northwestern Switzerland            #
 #                     Institute of Geomatics Engineering                       #
 #                          Author:robert.wst@gmail.com                         #
 ********************************************************************************
 *     Licensed under MIT License. Read the file LICENSE for more information   *
 *******************************************************************************/
goog.provide('owg.process.MapnikRenderer');
//-----------------------------------------------------------------------------
/**
 * @class MapnikRenderer
 * @constructor
 *
 * @description main class of mapnik
 *
 * @author Robert Wüest robert.wst@gmail.ch
 * @param {string} path
 */
function MapnikRenderer(path)
{

}
//-----------------------------------------------------------------------------
/**
 * @description init game and preload data
 * @param {string} params
 */
MapnikRenderer.prototype.RenderTile = function(params,callback)
{
    var url = "http://localhost:8000/";
    if(GetURLParameter("debug")!=1)
    {
        url = "mapnik.py";
    }

    var img = new Image();
    img.onload = function(){
        callback(img);
    };
    img.src = url+"?"+params;
};
//-----------------------------------------------------------------------------
/**
 * @description Get URL params
 * @param {string} name
 */
function GetURLParameter(name) {
    return decodeURI(
        (RegExp(name + '=' + '(.+?)(&|$)').exec(document.location.search)||[,null])[1]
    );
}
goog.exportSymbol('MapnikRenderer', MapnikRenderer);
goog.exportProperty(MapnikRenderer.prototype, 'RenderTile', MapnikRenderer.prototype.RenderTile);
goog.exportSymbol('GetURLParameter', GetURLParameter);