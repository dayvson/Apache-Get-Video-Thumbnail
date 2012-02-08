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

static void
error (const char *msg)
{
  fprintf (stderr, msg);
  fprintf (stderr, "\n");
}


int tve_open_video (const char *fname, int64_t second)
{
  AVFormatContext *format_ctx;
  AVCodecContext *codec_ctx;
  AVCodec *codec;
  AVPacket packet_av;
  AVFrame *frame_av = NULL;
  AVFrame *frameRGB_av = NULL;
  uint8_t *frame_buffer = NULL;
  size_t uncompressed_size;
  size_t i;
  float scale = 0.0;
  float scale_x = 0.0;
  float scale_y = 0.0;
  float scale_new = 0.0;
  float scale_sws = 0.0;

  int sws_width = 0;
  int sws_height = 0;
  int rc = 0;  
  int video_stream = 0;
  int frame_end = 0;


  if (avformat_open_input(&format_ctx, fname, NULL, NULL) != 0)
  {
    error("avformat_open_input() has failed");
    return -1;
  }
  if (avformat_find_stream_info(format_ctx, NULL) < 0)
  {
    error("av_find_stream_info() has failed");
    return -1;
  }
  if ((format_ctx->duration > 0) && (second > (format_ctx->duration / AV_TIME_BASE))) 
  {
    error("duration zero or second request over duration");    
    return -3;
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
    error("videostream not found");    
  }

  codec_ctx = format_ctx->streams[video_stream]->codec;
  if ((codec = avcodec_find_decoder(codec_ctx->codec_id)) == NULL)
	{
    error("codec not found");
		return -1;
	}
	if (avcodec_open2 (codec_ctx, codec, NULL) < 0)
	{
		error("unable to open codec");
		return -2;		
	}
  
  int64_t width = codec_ctx->width;
  int64_t height = codec_ctx->height;
  
  scale     = (float) codec_ctx->width / codec_ctx->height;
  scale_new = (float) width / height;

  sws_width = width;
  sws_height = height;

  if (scale != scale_new) 
  {
    scale_x = (float) width / codec_ctx->width;
    scale_y = (float) height / codec_ctx->height;

    if(scale_x > scale_y){
      scale_sws = scale_x;
    }else{
      scale_sws = scale_y;
    }
    sws_width = codec_ctx->width * scale_sws + 0.5;
    sws_height = codec_ctx->height * scale_sws + 0.5;  
  }

  frame_av = avcodec_alloc_frame();
  frameRGB_av = avcodec_alloc_frame();
    
  if ((frame_av == NULL) || (frameRGB_av == NULL)) 
  {
    error("Can't allocate memory to frame");
    return -2;  
  }
  uncompressed_size = avpicture_get_size(PIX_FMT_RGB24, sws_width, sws_height) * sizeof(uint8_t);
  frame_buffer = (uint8_t *) av_malloc(uncompressed_size);
  avpicture_fill((AVPicture *) frameRGB_av, frame_buffer, PIX_FMT_RGB24, sws_width, sws_height);  
  if ((rc = av_seek_frame(format_ctx, -1, second * AV_TIME_BASE, 0)) < 0) 
  {
    error("Seek on invalid time");
    return -4;
  }
  while (!frame_end && av_read_frame(format_ctx, &packet_av) >= 0) 
  {  
    if (packet_av.stream_index == video_stream) 
    {
      avcodec_decode_video2(codec_ctx, frame_av, &frame_end, &packet_av); 
    }
    if (frame_end) 
    {
      struct SwsContext *image_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, 
              codec_ctx->pix_fmt, sws_width, sws_height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
      sws_scale(image_ctx, (const uint8_t * const *) frame_av->data, frame_av->linesize, 0, 
                          codec_ctx->height, frameRGB_av->data, frameRGB_av->linesize);
      sws_freeContext(image_ctx);
    }  
  }
  return 0;
}






