#include <stdio.h>

#include "thumbnailer.h"

int main(int argc, char*argv)
{
  av_register_all();

  if (tve_open_video("big.ogg", 10) == 0)
    printf("Achou\r\n");
  else
    printf("NAO achou\r\n");
  return 0;
}
