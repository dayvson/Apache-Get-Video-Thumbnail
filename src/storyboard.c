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
#include "storyboard.h"
#include "jpegencoder.h"
#include <math.h>

ImageBuffer join_images(AVFrame** frames, int count, int columns, int width, int height, apr_pool_t* pool) 
{
  ImageBuffer result;
  result.buffer = NULL;
  result.size = 0;
  result.width = 0;
  result.height = 0;
  if ((!frames) || (count<=0) || (width<=0) || (height<=0)) return result;

  int cols = columns;
  int rows = (count / cols) + ((count%cols) > 0 ? 1 : 0);

  result.size = width * height * cols * rows * PIXEL_LENGTH;
  result.buffer = apr_palloc(pool, result.size);
  result.width = cols * width;
  result.height = rows * height;

  int currentColumn = 0;
  int currentRow = 0;
  int iCount;
  for (iCount=0; iCount<count; ++iCount)
  {
    int iHeight;
    for (iHeight=0; iHeight<height; ++iHeight) 
    {
      int offsetSrc = PIXEL_LENGTH * iHeight * width;
      int offsetDest = PIXEL_LENGTH * (width*(currentColumn + (currentRow*height+iHeight)*cols));
      if (offsetDest >= result.size) 
      {
        continue;
      }
      void* src = &(frames[iCount]->data[0][offsetSrc]);
      void* dst = &(result.buffer[offsetDest]);
      memcpy(dst, src, width*PIXEL_LENGTH);
    }

    ++currentColumn;
    if (currentColumn >= cols)
    {
      currentColumn = 0;
      ++currentRow;
    }
  }
  return result;
}

ImageBuffer get_storyboard(RequestInfo info, apr_pool_t* pool)
{
  AVFormatContext *format_ctx;
  AVCodecContext *codec_ctx;
  AVCodec *codec;
  AVPacket packet_av;
  ImageBuffer memJpeg;
  memJpeg.buffer = NULL;
  memJpeg.size = 0;
  size_t i;
  int rc = 0;  
  int video_stream = 0;
  int frame_end = 0;

  format_ctx = avformat_alloc_context();
  int openResult = avformat_open_input(&format_ctx, info.file, NULL, NULL);
  if (openResult != 0)
  {
    avformat_close_input(&format_ctx);
    LOG_ERROR("avformat_open_input() has failed: %s", info.file);
    char errBuffer[1000];
    av_strerror(openResult, errBuffer, 1000);
    LOG_ERROR("Error was: %s", errBuffer);
    return memJpeg;
  }
  if (avformat_find_stream_info(format_ctx, NULL) < 0)
  {
    avformat_close_input(&format_ctx);
    LOG_ERROR("av_find_stream_info() has failed");
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
    avformat_close_input(&format_ctx);
    LOG_ERROR("videostream not found");
    return memJpeg;
  }
  codec_ctx = format_ctx->streams[video_stream]->codec;
  if ((codec = avcodec_find_decoder(codec_ctx->codec_id)) == NULL)
  {
    avformat_close_input(&format_ctx);
    LOG_ERROR("codec not found");
    return memJpeg;
  }
  if (avcodec_open2(codec_ctx, codec, NULL) < 0)
  {
    avformat_close_input(&format_ctx);
    LOG_ERROR("unable to open codec");
    return memJpeg;
  }
  int counter = 0;
  int pageSize = info.pageSize;
  int totalP = ceil((double)info.split/(double)pageSize);
  int start = (info.currentPage-1) * pageSize;
  int end = info.currentPage * pageSize;
  if(info.currentPage == totalP)
  {
    end = info.split;
    pageSize = end-start;  
  }

  int64_t *framePosition = (int64_t*)apr_palloc(pool, sizeof(int64_t)*info.split);  
  split_integer(format_ctx->duration, info.split, framePosition);

  ImageSize finalSize = get_new_frame_size(codec_ctx->width, codec_ctx->height, info.width, info.height);
  AVFrame *frameList[pageSize];
  for (start; start<end; ++start)
  {
    AVFrame* currentFrame = get_frame_by_second(codec_ctx, format_ctx, video_stream, framePosition[start]);
    frameList[counter] = resize_frame(codec_ctx, currentFrame, &finalSize, pool);
    av_free(currentFrame);
    ++counter;
  }
  avcodec_close(codec_ctx);
  ImageBuffer rawImage = join_images(frameList, pageSize, info.columns, finalSize.width, finalSize.height, pool);
  if (!rawImage.buffer) 
  {
   avformat_close_input(&format_ctx);
   for (counter=0; counter<pageSize; ++counter) 
   {
     if (frameList[counter]) av_free(frameList[counter]);
   }
   LOG_ERROR("Invalid RAW joined image. Buffer size: %d\n", rawImage.size);
   return memJpeg;
  }

  ImageConf cf;
  cf.quality = info.jpegQuality;
  cf.dpi = 72;
  cf.smooth = 1;
  cf.baseline = 1;

  memJpeg = compress_jpeg(cf, rawImage.buffer, rawImage.width, rawImage.height);

  for (counter=0; counter<pageSize; ++counter) 
  {
    if (frameList[counter]) av_free(frameList[counter]);
  }
  avformat_close_input(&format_ctx);
  return memJpeg;
}

