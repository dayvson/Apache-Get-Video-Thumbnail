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
  //Resize frame
  ImageSize imageSize = get_new_frame_size(codec_ctx->width, codec_ctx->height, width, height);
  struct SwsContext *image_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, 
          ImgFmt, imageSize.width, imageSize.height, ImgFmt, SWS_BICUBIC, NULL, NULL, NULL);
  
  sws_scale(image_ctx, (const uint8_t * const *) frame_av->data, frame_av->linesize, 0, 
                      codec_ctx->height, frame_av->data, frame_av->linesize);
  sws_freeContext(image_ctx);
  
  //Alloc frame
  BufSiz = avpicture_get_size (ImgFmt, imageSize.width, imageSize.height );
  Buffer = (uint8_t *)malloc ( BufSiz ); 
  if ( Buffer == NULL ) return ( f ); 
  memset ( Buffer, 0, BufSiz ); 


  p_codec_ctx = avcodec_alloc_context3( NULL ); 
  if ( !p_codec_ctx ) { 
    free ( Buffer ); 
    return ( f ); 
  } 
  LOG_ERROR("uhul");
  
  p_codec_ctx->bit_rate      = codec_ctx->bit_rate; 
  p_codec_ctx->width         = imageSize.width;
  p_codec_ctx->height        = imageSize.height;

  p_codec_ctx->pix_fmt       = ImgFmt; 
  p_codec_ctx->codec_id      = CODEC_ID_MJPEG; 
  p_codec_ctx->codec_type    = AVMEDIA_TYPE_VIDEO; 
  p_codec_ctx->time_base.num = codec_ctx->time_base.num; 
  p_codec_ctx->time_base.den = codec_ctx->time_base.den; 

  pOCodec = avcodec_find_encoder ( p_codec_ctx->codec_id ); 
  if ( !pOCodec ) { 
    free ( Buffer ); 
    return ( f ); 
  }
  if (avcodec_open2 ( p_codec_ctx, pOCodec, NULL ) < 0 ) { 
    free ( Buffer ); 
    return ( f ); 
  } 
  p_codec_ctx->mb_lmin        = p_codec_ctx->lmin =  2 * FF_QP2LAMBDA; 
  p_codec_ctx->mb_lmax        = p_codec_ctx->lmax =  2 * FF_QP2LAMBDA; 
  p_codec_ctx->flags          = CODEC_FLAG_QSCALE; 
  p_codec_ctx->global_quality = p_codec_ctx->qmin * FF_QP2LAMBDA; 
  frame_av->pts     = 1; 
  frame_av->quality = 100; 
  BufSizActual = avcodec_encode_video( p_codec_ctx,Buffer,BufSiz,frame_av ); 

  avcodec_close ( p_codec_ctx ); 
  f.buffer = Buffer;
  f.size = BufSizActual;
  return f; 
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
