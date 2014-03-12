# psf2wav

psf2wav is a converter to make raw pcm data from PSF(PSf1) file on Linux.

## Usage

psf2wav dumps 16bit stereo 44.1kHz pcm data to stdin, so you need to use it with other commands.

To Play now,
```shell
psf2wav xxx.psf | aplay -f cd
```

To convert to other format,
```shell
psf2wav xxx.psf | ffmpeg -f s16le -ar 44.1k -ac 2 -i - xxx.wav
psf2wav xxx.psf | ffmpeg -f s16le -ar 44.1k -ac 2 -i - -ab 192 -f ogg file.ogg
```

### Options

```shell
  -e        : set decoder type, he or sexypsf (default:he).
  -t        : get song title (charset is SHIFT-JIS).
  -v        : print version.

```

## Build

```shell
git clone https://github.com/mitsuman/psf2wav.git
cd psf2wav
mkdir Release
cd Release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

