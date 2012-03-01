#include <stdio.h>
#include "thumbnailer.h"

void usage(const char* binName) {
  printf("Usage:\r\n");
  printf("  %s filename position(in seconds)\r\n\r\n", binName);
}

int main(int argc, char*argv[])
{
  if (argc < 3) {
    usage(argv[0]);
    return -1;
  }
	tve_init_libraries();
  tve_open_video(argv[1], atoi(argv[2]));
  return 0;
}
