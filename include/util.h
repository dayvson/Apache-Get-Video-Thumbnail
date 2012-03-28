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

#ifndef __VIDEOTHUMB_UTIL_H__
#define __VIDEOTHUMB_UTIL_H__

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <jpeglib.h>

#define PIXEL_LENGTH 3
#define OUTPUT_BUF_SIZE 4096

typedef struct _imageConf {
  int quality;
  int dpi;
  int optimize;
  int smooth;
  int baseline;
} ImageConf;

typedef struct _reqInfo {
  const char* file;
  int split;
  int columns;
  int jpegQuality;
  int width;
  int height;
  int second;
  int pageSize;
  int currentPage;
} RequestInfo;

typedef struct _imgSize {
  char* file;
  int width;
  int height;
} ImageSize;

typedef struct _imgBuf {
  uint8_t *buffer;
  int size;
  int width;
  int height;
} ImageBuffer;

typedef struct {
  struct jpeg_destination_mgr pub;
  unsigned char ** outbuffer;
  unsigned long * outsize;
  unsigned char * newbuffer;
  JOCTET * buffer;
  size_t bufsize;
} my_mem_destination_mgr;

int parse_integer(const char* intStr, int defaultValue);
void split_integer(int64_t duration, int count, int64_t *result);
AVFrame* get_frame_by_second(AVCodecContext* codec_ctx, AVFormatContext *format_ctx, int video_stream, int64_t second);
ImageSize get_new_frame_size(int input_width, int input_height, int output_width, int output_height);
AVFrame* resize_frame(AVCodecContext *codec_ctx, AVFrame *frame_av, ImageSize* imageSize);
void init_libraries(void);
static void LOG_ERROR_DATE(){
  time_t log_time;
  time(&log_time);
  struct tm* timeinfo = localtime(&log_time);
  char st_time[80];
  strftime(st_time, 80, "%c", timeinfo);
  fprintf(stderr, "[%s] ", st_time);
}

#define LOG_ERROR(...) LOG_ERROR_DATE();fprintf(stderr,"[mod_videothumb] %s:%d - ", __FILE__, __LINE__);fprintf(stderr,__VA_ARGS__);fprintf(stderr,"\r\n");

#endif // __VIDEOTHUMB_UTIL_H__
