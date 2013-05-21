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
#include "thumbnail.h"

ImageBuffer get_thumbnail (RequestInfo request, apr_pool_t* pool)
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
  int openResult = avformat_open_input(&format_ctx, request.file, NULL, NULL);
  if (openResult != 0)
  {
    av_free(format_ctx);
    LOG_ERROR("avformat_open_input() has failed: %s", request.file);
    char errBuffer[1000];
    av_strerror(openResult, errBuffer, 1000);
    LOG_ERROR("ERROR: %s", errBuffer);
    return memJpeg;
  }
  if (avformat_find_stream_info(format_ctx, NULL) < 0)
  {
    av_free(format_ctx);
    LOG_ERROR("av_find_stream_info() has failed");
    av_free(format_ctx);
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
    av_free(format_ctx);
    LOG_ERROR("videostream not found");
  }
  codec_ctx = format_ctx->streams[video_stream]->codec;
  if ((codec = avcodec_find_decoder(codec_ctx->codec_id)) == NULL)
  {
    av_free(format_ctx);
    LOG_ERROR("codec not found");
    return memJpeg;
  }
  if (avcodec_open2(codec_ctx, codec, NULL) < 0)
  {
    av_free(format_ctx);
    LOG_ERROR("unable to open codec");
    return memJpeg;
  }
  ImageSize finalSize = get_new_frame_size(codec_ctx->width, codec_ctx->height, request.width, request.height);
  AVFrame* frame;
  AVFrame* currentFrame = get_frame_by_second(codec_ctx, format_ctx, video_stream, request.second * AV_TIME_BASE);
  if(currentFrame)
  {
    frame = resize_frame(codec_ctx, currentFrame, &finalSize, pool);
    av_free(currentFrame);    
  }
  ImageConf cf;
  cf.quality = request.jpegQuality;
  cf.dpi = 72;
  cf.smooth = 1;
  cf.baseline = 1;
  memJpeg = compress_jpeg(cf, frame->data[0], finalSize.width, finalSize.height);
  av_free(frame);
  avformat_close_input(format_ctx);
  return memJpeg;
}

