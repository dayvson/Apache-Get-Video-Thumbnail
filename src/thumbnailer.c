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


int tve_open_video (const char *fname)
{
  AVFormatContext *format_ctx;
  AVCodecContext *codec_ctx;
  AVCodec *codec;
  size_t i;
  int videostream;

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

  videostream = -1;
  for (i = 0; i < format_ctx->nb_streams; i++)
    {
      if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
          videostream = i;
          break;
        }
    }

  if (videostream == -1)
    error("videostream not found");

  codec_ctx = format_ctx->streams[videostream]->codec;
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

  return 0;
}
