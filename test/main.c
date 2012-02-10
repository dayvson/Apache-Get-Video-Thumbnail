#include "thumbnailer.h"

int main(int argc, char*argv[])
{
	av_register_all();
  tve_open_video(argv[1], atoi(argv[2]));
  return 0;
}
