#include <stdio.h>
#include "thumbnailer.h"
#include "querystring.h"

void usage(const char* binName) {
  printf("Usage:\r\n");
  printf("  %s filename position(in seconds)\r\n\r\n", binName);
}
void testQueryString() {
  void* ctx;
  parseQueryString(&ctx, "width=640&height=360&video=bigbunny.ogg");
  const char* width = getParameter(ctx, "width");
  const char* height = getParameter(ctx, "height");
  const char* video = getParameter(ctx, "video");
  if(!strcmp(width,"640")) {
      printf("Querystring width esperado:%s - recebido:%s\n", "640", width);
  }
  if(!strcmp(height,"360")) {
      printf("Querystring height esperado:%s - recebido:%s\n", "360", height);
  }
  if(!strcmp(video,"bigbunny.ogg")) {
      printf("Querystring video esperado:%s - recebido:%s\n", "bigbunny.ogg", video);
  }
  releaseContext(ctx);
}
void testVideoParse(const char* videoFile, const char* jpegFile) {
  av_register_all();
  ImageBuffer jpeg = tve_open_video(videoFile, 400, 0, 50);

  if (jpeg.buffer) {
    printf("Writing file: %s\n", jpegFile);
    FILE* f = fopen(jpegFile, "w");
    if (f) {
      fwrite(jpeg.buffer, jpeg.size, 1, f);
      fclose(f);
      printf("File written!\n");
    }
  } else {
    printf("JPEG INVALIDO!!!!!!!!!!!!!!!!\n");
  }
}
int main(int argc, char**argv)
{
  testQueryString();
  testVideoParse("/Users/tiagopadua/dev/Apache-Get-Video-Thumbnail/video_sample/cartoons.avi", "/Users/tiagopadua/Desktop/video.jpg");
  return 0;
}
