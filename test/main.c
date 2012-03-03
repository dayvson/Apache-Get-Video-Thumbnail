#include <stdio.h>
#include "thumbnailer.h"
#include "querystring.h"

void usage(const char* binName) {
  printf("Usage:\r\n");
  printf("  %s filename position(in seconds)\r\n\r\n", binName);
}
void test_querystring(){
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
int main(int argc, char**argv)
{
  test_querystring();
  return 0;
}
