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

#include "util.h"


int parse_integer(const char* intStr, int defaultValue) 
{
  if (intStr) 
  {
    return atoi(intStr);
  }
  return defaultValue;
}

void split_integer(int64_t duration, int count, int64_t *result) 
{
  if (duration < count) return;
  if (!result) return;

  int i;
  int64_t current = 0;
  int64_t increment = duration / count;
  for (i = 0; i<count; ++i) 
  {
    result[i] = current;
    current += increment;
  }
}

ImageSize get_new_frame_size(int input_width, int input_height, int output_width, int output_height) 
{
  float scale = 0.0;

  ImageSize imageSize;
  imageSize.width = output_width;
  imageSize.height = output_height;

  if ((imageSize.width>0) && (imageSize.height>0)) 
  {
    return imageSize;
  }
  else if ((imageSize.width==0) && (imageSize.height==0)) 
  {
    imageSize.width = input_width;
    imageSize.height = input_height;
  }
  else if (imageSize.width > 0) {
    scale = (float) output_width / input_width;
    imageSize.height = input_height * scale;
  }
  else if (imageSize.height > 0) {
    scale = (float) output_height / input_height;
    imageSize.width = input_width * scale;
  }
  return imageSize;
}

AVFrame* get_frame_by_second(AVCodecContext* codec_ctx, AVFormatContext *format_ctx,
                                  int video_stream, int64_t second) 
{
  AVFrame* frame = avcodec_alloc_frame();
  AVPacket packet;
  int frame_end = 0;
  int rc = 0;

  if ((rc = av_seek_frame(format_ctx, -1, second , 0)) < 0) 
  {
    LOG_ERROR("Seek on invalid time");
    return frame;
  }
  while (!frame_end && (av_read_frame(format_ctx, &packet) >= 0)) 
  {
    if (packet.stream_index == video_stream) 
    {
      avcodec_decode_video2(codec_ctx, frame, &frame_end, &packet);
    }
    av_free_packet(&packet);
  }
  return frame;
}

AVFrame *resize_frame(AVCodecContext *codec_ctx, AVFrame *frame_av, ImageSize* imageSize) 
{
  uint8_t *Buffer; 
  int     BufSiz; 
  int     BufSizActual; 
  int     ImgFmt = PIX_FMT_YUVJ420P;

  //Alloc frame
  AVFrame *frameRGB_av = avcodec_alloc_frame();
  BufSiz = avpicture_get_size (PIX_FMT_RGB24, imageSize->width, imageSize->height );
  Buffer = (uint8_t *)malloc(BufSiz);
  if (Buffer == NULL) 
  {
      return NULL;
  }
  memset (Buffer, 0, BufSiz);
  avpicture_fill((AVPicture *) frameRGB_av, Buffer, PIX_FMT_RGB24, imageSize->width, imageSize->height);
  //Resize frame
  struct SwsContext *image_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, 
          ImgFmt, imageSize->width, imageSize->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

  sws_scale(image_ctx, (const uint8_t * const *) frame_av->data, frame_av->linesize, 0, 
                      codec_ctx->height, frameRGB_av->data, frameRGB_av->linesize);
  sws_freeContext(image_ctx);

  return frameRGB_av;
}

void init_libraries(void)
{
  av_register_all();
}

