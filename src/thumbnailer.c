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
#include "thumbnailer.h"
#include "stdlib.h"
#include "stdio.h"
#include "log.h"

void tve_init_libraries(void)
{
  av_register_all();
}

ImageSize get_new_frame_size(int input_width, int input_height, int output_width, int output_height) {
  float scale           = 0.0;
  float scale_x         = 0.0;
  float scale_y         = 0.0;
  float scale_out       = 0.0;
  float scale_sws       = 0.0;
  int sws_width         = 0;
  int sws_height        = 0;
  if (output_width == 0) {
      output_width = input_width;
      output_height = input_height;
  } else if (output_width == 0) {
      output_width = output_height * input_width / input_height;
  }
  
  scale     = (float) input_width / input_height;
  scale_out = (float) output_width / output_height;
  sws_width = output_width;
  sws_height = output_height;
  if (scale != scale_out) 
  {
    scale_x = (float) output_width / input_width;
    scale_y = (float) output_height / input_height;
    if (scale_x > scale_y){
      scale_sws = scale_x;
    } else {
      scale_sws = scale_y;
    } 
    sws_width = input_width * scale_sws + 0.5;
    sws_height = input_height * scale_sws + 0.5;
  }
  ImageSize imageSize;
  imageSize.width = sws_width;
  imageSize.height = sws_height;
  return imageSize;

}


#define OUTPUT_BUF_SIZE 4096	/* choose an efficiently fwrite’able size */
/* Expanded data destination object for memory output */
typedef struct {
  struct jpeg_destination_mgr pub; /* public fields */
  unsigned char ** outbuffer;	/* target buffer */
  unsigned long * outsize;
  unsigned char * newbuffer;	/* newly allocated buffer */
  JOCTET * buffer;	 /* start of buffer */
  size_t bufsize;
} my_mem_destination_mgr;

typedef my_mem_destination_mgr * my_mem_dest_ptr;

void
init_mem_destination (j_compress_ptr cinfo)
{
/* no work necessary here */
}
boolean
empty_mem_output_buffer (j_compress_ptr cinfo)
{
  size_t nextsize;
  JOCTET * nextbuffer;
  my_mem_dest_ptr dest = (my_mem_dest_ptr) cinfo->dest;
  /* Try to allocate new buffer with double size */
  nextsize = dest->bufsize * 2;
  nextbuffer = (JOCTET *)malloc(nextsize);
  if (nextbuffer == NULL)
    return;
  memcpy(nextbuffer, dest->buffer, dest->bufsize);
  if (dest->newbuffer != NULL)
    free(dest->newbuffer);
  dest->newbuffer = nextbuffer;
  dest->pub.next_output_byte = nextbuffer + dest->bufsize;
  dest->pub.free_in_buffer = dest->bufsize;
  dest->buffer = nextbuffer;
  dest->bufsize = nextsize;
  return TRUE;
}
void
term_mem_destination (j_compress_ptr cinfo)
{
  my_mem_dest_ptr dest = (my_mem_dest_ptr) cinfo->dest;
  *dest->outbuffer = dest->buffer;
  *dest->outsize = dest->bufsize - dest->pub.free_in_buffer;
}

void
jpeg_mem_dest (j_compress_ptr cinfo, unsigned char ** outbuffer, unsigned long * outsize)
{
  my_mem_dest_ptr dest;
  if (outbuffer == NULL || outsize == NULL)	/* sanity check */
    return;
/* The destination object is made permanent so that multiple JPEG images
* can be written to the same buffer without re-executing jpeg_mem_dest.
*/
  if (cinfo->dest == NULL) {	/* first time for this JPEG object? */
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
  if (*outbuffer == NULL || *outsize == 0) {
    /* Allocate initial buffer */
    dest->newbuffer = *outbuffer = (unsigned char*)malloc(OUTPUT_BUF_SIZE);
    if (dest->newbuffer == NULL)
      return;
    *outsize = OUTPUT_BUF_SIZE;
  }
  dest->pub.next_output_byte = dest->buffer = *outbuffer;
  dest->pub.free_in_buffer = dest->bufsize = *outsize;
}



ImageBuffer
jpeg_compress( ImageConf imageConf, uint8_t * buffer, int out_width, int out_height)
{
  ImageBuffer             f;
  f.buffer              = NULL;
  f.size                = 0;  

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;
    //int image_d_width = in_width;
    //int image_d_height = in_height;
    unsigned long outlen;
    unsigned char *outbuffer;
    

    if ( !buffer ) return f;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest (&cinfo,&outbuffer,&outlen );

    cinfo.image_width = out_width;
    cinfo.image_height = out_height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    /* Important: Header info must be set AFTER jpeg_set_defaults() */
    cinfo.write_JFIF_header = TRUE;
    cinfo.JFIF_major_version = 1;
    cinfo.JFIF_minor_version = 2;
    cinfo.density_unit = 1; /* 0=unknown, 1=dpi, 2=dpcm */
    /* Image DPI is determined by Y_density, so we leave that at
       jpeg_dpi if possible and crunch X_density instead (PAR > 1) */

    

    //cinfo.X_density = imageConf.dpi * out_width / image_d_width;
    //cinfo.Y_density = imageConf.dpi * out_height / image_d_height;
    cinfo.write_Adobe_marker = TRUE;

    jpeg_set_quality(&cinfo, imageConf.quality, imageConf.baseline);
    cinfo.optimize_coding = imageConf.optimize;
    cinfo.smoothing_factor = imageConf.smooth;

    jpeg_simple_progression(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);

    row_stride = out_width * 3;
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &buffer[cinfo.next_scanline * row_stride];
        (void)jpeg_write_scanlines(&cinfo, row_pointer,1);
    }
    
    
    
    struct jpeg_destination_mgr *dest = cinfo.dest;
    f.buffer = outbuffer;
    f.size = outlen;
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return f;
}


ImageBuffer write_jpeg (AVCodecContext *codec_ctx, AVFrame *frame_av, int width, int height)
{ 
  AVCodecContext         *p_codec_ctx; 
  AVCodec                *pOCodec; 
  uint8_t                *Buffer; 
  int                     BufSiz; 
  int                     BufSizActual; 
  int                     ImgFmt = PIX_FMT_YUVJ420P;
  ImageBuffer             f;
  f.buffer              = NULL;
  f.size                = 0;  
   ImageSize imageSize = get_new_frame_size(codec_ctx->width, codec_ctx->height, width, height);
  AVFrame *frameRGB_av = avcodec_alloc_frame();

  //Alloc frame
  BufSiz = avpicture_get_size (PIX_FMT_RGB24, imageSize.width, imageSize.height );
  Buffer = (uint8_t *)malloc ( BufSiz ); 
  if ( Buffer == NULL ) return ( f ); 
  memset ( Buffer, 0, BufSiz ); 

  avpicture_fill((AVPicture *) frameRGB_av, Buffer, PIX_FMT_RGB24, imageSize.width, imageSize.height);
  //Resize frame
 
  struct SwsContext *image_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, 
          ImgFmt, imageSize.width, imageSize.height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
  
  sws_scale(image_ctx, (const uint8_t * const *) frame_av->data, frame_av->linesize, 0, 
                      codec_ctx->height, frameRGB_av->data, frameRGB_av->linesize);
  sws_freeContext(image_ctx);
  ImageConf cf;
  cf.quality = 100;
  cf.dpi = 100;
  cf.smooth = 1;
  cf.baseline = 1;
  
  ImageBuffer jpeg = jpeg_compress(cf, frameRGB_av->data[0], imageSize.width, imageSize.height);
  return jpeg; 
}

ImageBuffer tve_open_video (const char *fname, int second, int width, int height)
{
  AVFormatContext *format_ctx;
  AVCodecContext *codec_ctx;
  AVCodec *codec;
  AVPacket packet_av;
  AVFrame *frame_av = NULL;
  ImageBuffer memJpeg;
  memJpeg.buffer = NULL;
  memJpeg.size = 0;
  size_t i;
  int rc = 0;  
  int video_stream = 0;
  int frame_end = 0;

  format_ctx = avformat_alloc_context();
  if (avformat_open_input(&format_ctx, fname, NULL, NULL) != 0)
  {
    LOG_ERROR("avformat_open_input() has failed");
    return memJpeg;
  }
  if (avformat_find_stream_info(format_ctx, NULL) < 0)
  {
    LOG_ERROR("av_find_stream_info() has failed");
    return memJpeg;
  }
  if ((format_ctx->duration > 0) && (second > (format_ctx->duration / AV_TIME_BASE)))
  {
    LOG_ERROR("duration zero or second request over duration %llu segundos", format_ctx->duration / AV_TIME_BASE);
    return memJpeg;
  }
  video_stream = -1;
  for (i = 0; i < format_ctx->nb_streams; i++)
  {
    if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
      video_stream = i;
      break;
    }
  }
  if (video_stream == -1)
  {
    LOG_ERROR("videostream not found");
  }
  codec_ctx = format_ctx->streams[video_stream]->codec;
  if ((codec = avcodec_find_decoder(codec_ctx->codec_id)) == NULL)
  {
    LOG_ERROR("codec not found");
    return memJpeg;
  }
  if (avcodec_open2 (codec_ctx, codec, NULL) < 0)
  {
    LOG_ERROR("unable to open codec");
    return memJpeg;
  }
  frame_av = avcodec_alloc_frame();
  if ((frame_av == NULL))
  {
    LOG_ERROR("Can't allocate memory to frame");
    return memJpeg;
  }
  if ((rc = av_seek_frame(format_ctx, -1, second * AV_TIME_BASE, 0)) < 0) 
  {
    LOG_ERROR("Seek on invalid time");
    return memJpeg;
  }
  int count = 0;
  while (!frame_end && av_read_frame(format_ctx, &packet_av) >= 0) 
  {  
    count++;
    if (packet_av.stream_index == video_stream) 
    {
      avcodec_decode_video2(codec_ctx, frame_av, &frame_end, &packet_av);
    }
    if (frame_end)
    {
      memJpeg = write_jpeg(codec_ctx, frame_av, width, height);
    }  
    av_free_packet(&packet_av);
  }
  av_free_packet(&packet_av);

  return memJpeg;
}
