#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern "C" {
#include "../sexypsf/driver.h"
}

#define VERSION "1.0"

void version() {
  printf(
    "psf2wav version "VERSION" Copyright 2014 @__mtm\n"
    " based on sexypsf 0.4.8\n"
    );
}

void help() {
  printf(
    "Usage: psf2wav [options] <file>\n"
    "Convert psf and minipsf file to 16bit stereo raw pcm and write it to stdout.\n"
    " if you need to convert to other format, use ffmpeg like a following.\n"
    "  psf2wav xxx.mdx | ffmpeg -f s16le -ar 44.1k -ac 2 -i - xxx.wav\n"
    " if you want to listen now, use aplay like a following.\n"
    "  psf2wav xxx.mdx | aplay -f cd\n"
    "Options:\n"
    //"  -d <sec>  : limit song duration. 0 means nolimit. (default:300)\n"
    //"  -f        : enable fadeout.\n"
    //"  -l <loop> : set loop limit. (default:2)\n"
    //"  -m        : measure play time as sec.\n"
    //"  -r <rate> : set sampling rate. (default:44100)\n"
    "  -t        : get song title.\n"
    "  -v        : print version.\n"
    //"  -V        : verbose, write debug log to stderr.\n"
    );
}

extern "C"
void sexyd_update(unsigned char* pSound,long lBytes) {
  if (!pSound)
    return;
  fwrite(pSound, 1, lBytes, stdout);
}

static bool verbose = false;

int main(int argc, char **argv) {
  //int SAMPLE_RATE = 44100;
  //bool measure_play_time = false;
  bool get_title = false;
  //float max_song_duration = 300.0f;
  //int loop = 2;
  //int fadeout = 0;

  int opt;
  while ((opt = getopt(argc, argv, "tv")) != -1) {
    switch (opt) {
      //case 'd':
      //  max_song_duration = atof(optarg);
      //  break;
      case 't':
        get_title = true;
        break;
      case 'v':
        version();
        return 0;
      case 'V':
        verbose = true;
        break;
      default:
        help();
        return 0;
    }
  }

  const char *file_name = argv[optind];

  PSFINFO *pi;
  if(!(pi = sexy_load((char*)file_name))) {
    fprintf(stderr, "Cannot load %s\n", file_name);
    return -1;
  }

  if (get_title) {
    printf("%s\n", pi->title);
    return 0;
  }

  sexy_execute();

  return 0;
}

