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
#include "util.h"

#define OUTPUT_BUF_SIZE 4096
#define VIDEO_SPLIT_COUNT 36
typedef my_mem_destination_mgr * my_mem_dest_ptr;

void tve_init_libraries(void)
{
  av_register_all();
}

ImageSize get_new_frame_size(int input_width, int input_height, int output_width, int output_height) 
{
  float scale = 0.0;

  ImageSize imageSize;
  imageSize.width = output_width;
  imageSize.height = output_height;

  if ((imageSize.width>0)&&(imageSize.height>0)) {
    // This "IF" is needed even if it is empty, or else the program would fall inside another case
  }
  else if ((imageSize.width==0) && (imageSize.height==0)) {
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

ImageBuffer jpeg_compress(ImageConf imageConf, uint8_t * buffer, int out_width, int out_height)
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
      printf(">>>>>>>>>>>>>>>>>> buffer null\n");
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

    // TODO: PRECISA LIBERAR ESSA MEMORIA LÁ FORA DA FUNCTION!!!!!!!!!!!!!!!!!!!!!
    // free(new_buffer);

    return f;
}

#define PIXEL_LENGTH 3
ImageBuffer joinImages(AVFrame** frames, int count, int width, int height) {
  ImageBuffer result;
  result.buffer = NULL;
  result.size = 0;
  result.width = 0;
  result.height = 0;
  if ((!frames) || (count<=0) || (width<=0) || (height<=0)) return result;

  int cols = (int)abs(sqrt((float)count));
  int rows = cols + ((count%cols)>0 ? 1 : 0);
  printf("cols: %d\n", cols);
  printf("rows: %d\n", rows);

  result.size = width * height * cols * rows * PIXEL_LENGTH;
  result.buffer = malloc(result.size);
  printf(">>> buffer size: %d\n", result.size);
  result.width = cols * width;
  result.height = rows * height;

  int byteCount = 0;
  int iW = 0;
  for (; iW<width; ++iW) {
    int iH = 0;
    for (; iH<height; ++iH) {
      int iCol = 0;
      for (; iCol<cols; ++iCol) {
        int iRow = 0;
        for (; iRow<rows; ++iRow) {
          if (count <= (iCol + iRow*cols)) break; // evita escrever a mais

          int offsetSingle = PIXEL_LENGTH*(iW + iH*width);
          int offsetJoined = PIXEL_LENGTH*(iRow*cols*width*height + iCol*width + iW + iH*width*cols);

          int iPixel = 0;
          for (; iPixel<PIXEL_LENGTH; ++iPixel) {
            ++byteCount;
            int currentFrame = iCol + iRow*cols;
            result.buffer[iPixel + offsetJoined] = frames[currentFrame]->data[0][iPixel + offsetSingle];
          }
        }
      }
    }
  }
  return result;
}

AVFrame *resizeFrame(AVCodecContext *codec_ctx, AVFrame *frame_av, ImageSize* imageSize) {
  uint8_t *Buffer; 
  int     BufSiz; 
  int     BufSizActual; 
  int     ImgFmt = PIX_FMT_YUVJ420P;

  //Alloc frame
  AVFrame *frameRGB_av = avcodec_alloc_frame();
  BufSiz = avpicture_get_size (PIX_FMT_RGB24, imageSize->width, imageSize->height );
  Buffer = (uint8_t *)malloc(BufSiz);
  if (Buffer == NULL) {
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

AVFrame* getFrameInSecond(AVCodecContext* codec_ctx, AVFormatContext *format_ctx, int video_stream, int64_t second) {
  AVFrame* frame = avcodec_alloc_frame();
  AVPacket packet;
  int frame_end = 0;
  int rc = 0;
  if ((rc = av_seek_frame(format_ctx, -1, second, 0)) < 0) {
    LOG_ERROR("Seek on invalid time");
    return frame;
  }
  while (!frame_end && (av_read_frame(format_ctx, &packet) >= 0)) {
    if (packet.stream_index == video_stream) {
      avcodec_decode_video2(codec_ctx, frame, &frame_end, &packet);
    }
    av_free_packet(&packet);
    printf(".");
    if (frame_end) printf("!\n");
  }
  return frame;
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
    LOG_ERROR("avformat_open_input() has failed: %s", fname);
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

  int64_t *framePosition = (int64_t*)malloc(sizeof(int64_t)*VIDEO_SPLIT_COUNT);
  splitInteger(format_ctx->duration, VIDEO_SPLIT_COUNT, framePosition);

  ImageSize finalSize = get_new_frame_size(codec_ctx->width, codec_ctx->height, width, height);

  AVFrame *frameList[VIDEO_SPLIT_COUNT];
  int counter = 0;

  // AVFrame* temp = getFrameInSecond(codec_ctx, format_ctx, video_stream, second / AV_TIME_BASE);
  // frameList[0] = resizeFrame(codec_ctx, temp, width, height, &finalSize);
  for (; counter<VIDEO_SPLIT_COUNT; ++counter) {
    AVFrame* currentFrame = getFrameInSecond(codec_ctx, format_ctx, video_stream, framePosition[counter]);
    frameList[counter] = resizeFrame(codec_ctx, currentFrame, &finalSize);
  }

  ImageBuffer rawImage = joinImages(frameList, VIDEO_SPLIT_COUNT, finalSize.width, finalSize.height);
  if (!rawImage.buffer) {
   printf("Invalid RAW joined image. Buffer size: %d\n", rawImage.size);
   return memJpeg;
  }
  printf("RAW size: %d\n", rawImage.size);

  ImageConf cf;
  cf.quality = 100;
  cf.dpi = 72;
  cf.smooth = 1;
  cf.baseline = 1;

  // memJpeg = jpeg_compress(cf, frameList[0]->data[0], finalSize.width, finalSize.height);
  memJpeg = jpeg_compress(cf, rawImage.buffer, rawImage.width, rawImage.height);

  for (counter=0; counter<VIDEO_SPLIT_COUNT; ++counter) {
    if (frameList[counter]) av_free(frameList[counter]);
  }
  free(framePosition);

  return memJpeg;
}
