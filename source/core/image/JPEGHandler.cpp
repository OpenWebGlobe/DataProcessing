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

#include "JPEGHandler.h"
#include <iostream>
#include <cstring>

extern "C" 
{ 
#include "jpeglib.h"
}

using namespace std;

/*******************************************************************************
CUSTOM DESTINATION MANAGER FOR JPEGLIB
FOR Memory to Memory compression / decompression
*******************************************************************************/
extern "C"
{
   typedef struct 
   {
      struct jpeg_destination_mgr pub; /* base class */
      JOCTET*  buffer;                 /* buffer start address */
      int      bufsize;                /* size of buffer */
      size_t   datasize;               /* final size of compressed data */
      int*     outsize;                /* user pointer to datasize */
      int      errcount;               /* counts up write errors due to buffer overruns */	
   } memory_destination_mgr;

   typedef memory_destination_mgr* mem_dest_ptr;

   //-----------------------------------------------------------------------------

   /* This function is called by the library before any data gets written */
   METHODDEF(void) init_destination (j_compress_ptr cinfo)
   {
      mem_dest_ptr dest = (mem_dest_ptr)cinfo->dest;

      dest->pub.next_output_byte  = dest->buffer;     /* set destination buffer */
      dest->pub.free_in_buffer   = dest->bufsize;     /* input buffer size */
      dest->datasize = 0;                             /* reset output size */
      dest->errcount = 0;                             /* reset error count */
   }

   //-----------------------------------------------------------------------------

   /* This function is called by the library if the buffer fills up 
   I just reset destination pointer and buffer size here.
   Note that this behavior, while preventing seg faults
   will lead to invalid output streams as data is over-
   written.
   */

   METHODDEF(boolean) empty_output_buffer (j_compress_ptr cinfo)
   {
      mem_dest_ptr dest = (mem_dest_ptr)cinfo->dest;
      dest->pub.next_output_byte = dest->buffer;
      dest->pub.free_in_buffer = dest->bufsize;
      ++dest->errcount;	/* need to increase error count */

      return TRUE;
   }

   //-----------------------------------------------------------------------------
   /* Usually the library wants to flush output here.
   I will calculate output buffer size here.
   Note that results become incorrect, once
   empty_output_buffer was called.
   This situation is notified by errcount.
   */

   METHODDEF(void) term_destination (j_compress_ptr cinfo)
   {
      mem_dest_ptr dest = (mem_dest_ptr)cinfo->dest;
      dest->datasize = dest->bufsize - dest->pub.free_in_buffer;
      if (dest->outsize) *dest->outsize += (int)dest->datasize;
   }

   //-----------------------------------------------------------------------------
   /* Override the default destination manager initialization
   provided by jpeglib. Since we want to use memory-to-memory
   compression, we need to use our own destination manager.
   */

   GLOBAL(void) jpeg_memory_dest(j_compress_ptr cinfo, JOCTET* buffer, int bufsize, int* outsize)
   {
      mem_dest_ptr dest;

      /* first call for this instance - need to setup */
      if (cinfo->dest == 0) {
         cinfo->dest = (struct jpeg_destination_mgr *)
            (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
            sizeof (memory_destination_mgr));
      }

      dest = (mem_dest_ptr) cinfo->dest;
      dest->bufsize = bufsize;
      dest->buffer  = buffer;
      dest->outsize = outsize;
      /* set method callbacks */
      dest->pub.init_destination = init_destination;
      dest->pub.empty_output_buffer = empty_output_buffer;
      dest->pub.term_destination = term_destination;
   }

   //-----------------------------------------------------------------------------

   /* Called before data is read */
   METHODDEF(void) init_source (j_decompress_ptr dinfo)
   {
   }

   /* Called if the decoder wants some bytes that we cannot provide... */
   METHODDEF(boolean) fill_input_buffer (j_decompress_ptr dinfo)
   {
      return FALSE;
   }

   //-----------------------------------------------------------------------------

   /* From IJG docs: "it's not clear that being smart is worth much trouble"
   So I save myself some trouble by ignoring this bit.
   */
   METHODDEF(void) skip_input_data (j_decompress_ptr dinfo, long num_bytes)
   {
      /*	There might be more data to skip than available in buffer.
      This clearly is an error, so screw this mess. */
      if ((size_t)num_bytes > dinfo->src->bytes_in_buffer) 
      {
         dinfo->src->next_input_byte = 0; /* no buffer byte */
         dinfo->src->bytes_in_buffer = 0; /* no input left */
      } 
      else 
      {
         dinfo->src->next_input_byte += num_bytes;
         dinfo->src->bytes_in_buffer -= num_bytes;
      }
   }

   //-----------------------------------------------------------------------------

   /* Finished with decompression */
   METHODDEF(void) term_source (j_decompress_ptr dinfo)
   {
   }

   //-----------------------------------------------------------------------------

   GLOBAL(void) jpeg_memory_src(j_decompress_ptr dinfo, unsigned char* buffer, size_t size)
   {
      struct jpeg_source_mgr* src;

      /* first call for this instance - need to setup */
      if (dinfo->src == 0) 
      {
         dinfo->src = (struct jpeg_source_mgr *)(*dinfo->mem->alloc_small) ((j_common_ptr) dinfo, JPOOL_PERMANENT, sizeof (struct jpeg_source_mgr));
      }

      src = dinfo->src;
      src->next_input_byte = buffer;
      src->bytes_in_buffer = size;
      src->init_source = init_source;
      src->fill_input_buffer = fill_input_buffer;
      src->skip_input_data = skip_input_data;
      src->term_source = term_source;
      /* IJG recommend to use their function - as I don't know ****
      about how to do better, I follow this recommendation */
      src->resync_to_restart = jpeg_resync_to_restart;
   }
}; // extern "C"
//-----------------------------------------------------------------------------


/******************************************************************************
END CUSTOM DESTINATION MANAGER FOR JPEGLIB
******************************************************************************/

bool JPEGHandler::JpegToRGB(unsigned char* inJpeg, 
                            int inJpegSize, 
                            boost::shared_array<unsigned char>& outRGB, 
                            int& outWidth, 
                            int& outHeight)
{
   if (inJpeg == 0)
   {
      //ERROR: No JPEG data specified
      return false;
   }


   struct jpeg_decompress_struct cinfo;
   struct jpeg_error_mgr jerr;
   JSAMPARRAY buffer;		/* Output row buffer */
   int row_stride;		/* physical row width in output buffer */

  
   cinfo.err = jpeg_std_error(&jerr);   
   jpeg_create_decompress(&cinfo);

   jpeg_memory_src(&cinfo, inJpeg, inJpegSize);

   (void) jpeg_read_header(&cinfo, TRUE);
   (void) jpeg_start_decompress(&cinfo);

   if (cinfo.output_width == 0 || cinfo.output_height == 0)
      return false;

   outRGB = boost::shared_array<unsigned char>(new unsigned char[3*cinfo.output_width*cinfo.output_height]);
 
   if (outRGB.get() == 0)
      return false;

   outWidth = cinfo.output_width; 
   outHeight = cinfo.output_height;

   row_stride = cinfo.output_width * cinfo.output_components;
   buffer = (*cinfo.mem->alloc_sarray)
      ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

   int line=0;	
   while (cinfo.output_scanline < cinfo.output_height) 
   {
      (void) jpeg_read_scanlines(&cinfo, buffer, 1);
      //put_scanline_someplace(buffer[0], row_stride);
      memcpy(outRGB.get()+row_stride*line, buffer[0], row_stride);
      line++;
   }	

   (void) jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);

   return true;
}

bool JPEGHandler::RGBToJpeg(unsigned char* inRgb, int inWidth, int inHeight, int inQuality, boost::shared_array<unsigned char>& outJpeg, int& outJpegSize)
{
   struct jpeg_compress_struct cinfo;
   struct jpeg_error_mgr jerr;
   JSAMPROW row_pointer[1];
   int row_stride;

   if (inRgb == 0)
   {
      return false;
   }

   outJpeg = boost::shared_array<unsigned char>(new unsigned char[3*inWidth*inHeight]);
   outJpegSize = 0;       // It is important to set to 0, otherwise result will be wrong!

   cinfo.err = jpeg_std_error(&jerr);
   jpeg_create_compress(&cinfo);

   JOCTET *jpgbuff = (JOCTET*)outJpeg.get(); //JOCTET pointer to buffer
   jpeg_memory_dest(&cinfo,jpgbuff,3*inHeight*inWidth,&outJpegSize);

   cinfo.image_width = inWidth; 	// image width and height, in pixels 
   cinfo.image_height = inHeight;
   cinfo.input_components = 3;		   // # of color components per pixel 
   cinfo.in_color_space = JCS_RGB;     // colorspace of input image 

   jpeg_set_defaults(&cinfo);
   jpeg_set_quality(&cinfo, inQuality, TRUE);
   jpeg_start_compress(&cinfo, TRUE);

   row_stride = inWidth * 3;

   while (cinfo.next_scanline < cinfo.image_height) 
   {
      row_pointer[0] = & inRgb[cinfo.next_scanline * row_stride];
      (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
   }

   jpeg_finish_compress(&cinfo);
   jpeg_destroy_compress(&cinfo);

   return true;
}

