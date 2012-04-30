/*
 * Copyright (c) 2012 - Maxwell Dayvson <dayvson@gmail.com>
 * Copyright (c) 2012 - Tiago de Pádua <tiagopadua@gmail.com>
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

#include <stdio.h>
#include "thumbnail.h"
#include "storyboard.h"
#include "querystring.h"

void usage(const char* binName) {
  printf("Usage:\r\n");
  printf("  %s filename position(in seconds)\r\n\r\n", binName);
}

void test_query_string() {
  void* ctx;
  parse_query_string(&ctx, "width=640&height=360&video=bigbunny.ogg");
  const char* width = get_parameter(ctx, "width");
  const char* height = get_parameter(ctx, "height");
  const char* video = get_parameter(ctx, "video");
  if(!strcmp(width,"640")) {
      printf("Querystring width esperado:%s - recebido:%s\n", "640", width);
  }
  if(!strcmp(height,"360")) {
      printf("Querystring height esperado:%s - recebido:%s\n", "360", height);
  }
  if(!strcmp(video,"bigbunny.ogg")) {
      printf("Querystring video esperado:%s - recebido:%s\n", "bigbunny.ogg", video);
  }
  release_context(ctx);
}

void test_get_thumbnail_by_second(const char* videoFile, const char* jpegFile) {
  init_libraries();
  
  RequestInfo req;
  req.file = videoFile;
  req.second = 15;
  req.height = 300;
  req.width = 0;
  ImageBuffer jpeg = get_thumbnail(req);

  if (jpeg.buffer) {
    printf("Writing image by second with name: %s\n", jpegFile);
    FILE* f = fopen(jpegFile, "w");
    if (f) {
      fwrite(jpeg.buffer, jpeg.size, 1, f);
      fclose(f);
      printf("Image written!\n");
    }
  } else {
    printf("Error to save jpeg a video frame!\n");
  }
}

void test_get_storyboard(const char* videoFile, const char* jpegFile){
  RequestInfo req;
  req.file = videoFile;
  req.split = 9;
  req.columns = 3;
  req.pageSize = 9;
  req.currentPage = 1;
  req.width = 200;
  
  ImageBuffer jpeg = get_storyboard(req);
  if (jpeg.buffer) {
    printf("Writing image by storyboard with name: %s\n", jpegFile);
    FILE* f = fopen(jpegFile, "w");
    if (f) {
      fwrite(jpeg.buffer, jpeg.size, 1, f);
      fclose(f);
      printf("Storyboard written!\n");
    }
  } else {
    printf("Error to save jpeg a storyboard!\n");
  }
  
}

int main(int argc, char**argv)
{
  test_query_string();
  test_get_thumbnail_by_second("video_sample/big.ogg", "video.jpg");
  test_get_storyboard("video_sample/big.ogg", "storyboard.jpg");
  return 0;
}
