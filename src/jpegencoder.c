/*
 * Copyright (c) Maxwell Dayvson <dayvson@gmail.com>
 * Copyright (c) Tiago de Pádua <tiagopadua@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "jpegencoder.h"

typedef my_mem_destination_mgr * my_mem_dest_ptr;
void init_mem_destination (j_compress_ptr cinfo){ }
boolean empty_mem_output_buffer (j_compress_ptr cinfo)
{
  size_t nextsize;
  JOCTET * nextbuffer;
  my_mem_dest_ptr dest = (my_mem_dest_ptr) cinfo->dest;
  nextsize = dest->bufsize * 2;
  nextbuffer = (JOCTET *)malloc(nextsize);
  if (nextbuffer == NULL)
  {
    return;
  } 
  memcpy(nextbuffer, dest->buffer, dest->bufsize);
  if (dest->newbuffer != NULL) 
  {
    free(dest->newbuffer);
  }
  dest->newbuffer = nextbuffer;
  dest->pub.next_output_byte = nextbuffer + dest->bufsize;
  dest->pub.free_in_buffer = dest->bufsize;
  dest->buffer = nextbuffer;
  dest->bufsize = nextsize;
  return TRUE;
}

void term_mem_destination (j_compress_ptr cinfo)
{
  my_mem_dest_ptr dest = (my_mem_dest_ptr) cinfo->dest;
  *dest->outbuffer = dest->buffer;
  *dest->outsize = dest->bufsize - dest->pub.free_in_buffer;
}

void jpeg_mem_dest (j_compress_ptr cinfo, unsigned char ** outbuffer, unsigned long * outsize)
{
  my_mem_dest_ptr dest;
  if (outbuffer == NULL || outsize == NULL)
    return;
  if (cinfo->dest == NULL) {	
      cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
      sizeof(my_mem_destination_mgr));
  }
  dest = (my_mem_dest_ptr) cinfo->dest;
  dest->pub.init_destination = init_mem_destination;
  dest->pub.empty_output_buffer = empty_mem_output_buffer;
  dest->pub.term_destination = term_mem_destination;
  dest->outbuffer = outbuffer;
  dest->outsize = outsize;
  dest->newbuffer = NULL;
  if (*outbuffer == NULL || *outsize == 0) 
  {
    dest->newbuffer = *outbuffer = (unsigned char*)malloc(OUTPUT_BUF_SIZE);
    if (dest->newbuffer == NULL)
    {
      return;
    }
    *outsize = OUTPUT_BUF_SIZE;
  }
  dest->pub.next_output_byte = dest->buffer = *outbuffer;
  dest->pub.free_in_buffer = dest->bufsize = *outsize;
}

ImageBuffer compress_jpeg(ImageConf imageConf, uint8_t * buffer, int out_width, int out_height)
{
    ImageBuffer             f;
    f.buffer              = NULL;
    f.size                = 0;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;
    unsigned long outlen = OUTPUT_BUF_SIZE;
    unsigned char *outbuffer = malloc(outlen);
    if ( !buffer )
    {
      printf("Error buffer null\n");
      return f;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest (&cinfo, &outbuffer, &outlen);

    cinfo.image_width = out_width;
    cinfo.image_height = out_height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    cinfo.write_JFIF_header = TRUE;
    cinfo.JFIF_major_version = 1;
    cinfo.JFIF_minor_version = 2;
    cinfo.density_unit = 1;
    cinfo.write_Adobe_marker = TRUE;

    jpeg_set_quality(&cinfo, imageConf.quality, imageConf.baseline);
    cinfo.optimize_coding = imageConf.optimize;
    cinfo.smoothing_factor = imageConf.smooth;

    jpeg_simple_progression(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);

    row_stride = out_width * 3;
    while (cinfo.next_scanline < cinfo.image_height) 
    {
        row_pointer[0] = &buffer[cinfo.next_scanline * row_stride];
        (void)jpeg_write_scanlines(&cinfo, row_pointer,1);
    }

    my_mem_dest_ptr dest = (my_mem_dest_ptr) cinfo.dest;
    jpeg_finish_compress(&cinfo);
    f.buffer = outbuffer;
    f.size = outlen;

    jpeg_destroy_compress(&cinfo);
    return f;
}

