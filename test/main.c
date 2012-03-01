#include "thumbnailer.h"

int main(int argc, char*argv[])
{
	tve_init_libraries();
  tve_open_video(argv[1], atoi(argv[2]));
  return 0;
}
