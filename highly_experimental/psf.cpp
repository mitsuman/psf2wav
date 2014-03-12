// based on foo_psf
// https://github.com/kode54/foo_psf
#include <stdio.h>
#include <string.h>

#include "Highly_Experimental/Core/psx.h"
#include "Highly_Experimental/Core/iop.h"
#include "Highly_Experimental/Core/r3000.h"
#include "Highly_Experimental/Core/spu.h"
#include "Highly_Experimental/Core/bios.h"
#include "Highly_Experimental/Core/mkhebios.h"

#include "psflib/psflib.h"
#include "psflib/psf2fs.h"

extern bool VERBOSE;

typedef struct {
  uint32_t pc0;
  uint32_t gp0;
  uint32_t t_addr;
  uint32_t t_size;
  uint32_t d_addr;
  uint32_t d_size;
  uint32_t b_addr;
  uint32_t b_size;
  uint32_t s_ptr;
  uint32_t s_size;
  uint32_t sp,fp,gp,ret,base;
} exec_header_t;

typedef struct {
  char key[8];
  uint32_t text;
  uint32_t data;
  exec_header_t exec;
  char title[60];
} psxexe_hdr_t;

volatile long psf_count = 0, psf2_count = 0;


bool cfg_enable_overlapping = true;

void *bios_data;
bool psf_init_and_load_bios(const char *name) {
  FILE *fp = fopen(name, "rb");
  if (!fp) {
    fprintf(stderr, "Cannot load %s.\n", name);
    return false;
  }

  int bios_size = 0x400000;
  bios_data = new char[bios_size];

  bios_size = fread(bios_data, 1, bios_size, fp);
  fclose(fp);

  if (VERBOSE) {
    fprintf(stderr, "bios size:%08x\n", bios_size);
  }

  if (bios_size >= 0x400000) {
    void *bios = mkhebios_create(bios_data, &bios_size);
    bios_data = bios;
  }

  bios_set_image( (unsigned char *) bios_data, bios_size );
  int ret = psx_init();
  if (ret != 0) {
    fprintf(stderr, "Failed psx_init (%d).\n", ret);
    return false;
  }

  return true;
}

static void * psf_file_fopen( const char * uri )
{
  return fopen(uri, "rb");
}

static size_t psf_file_fread( void * buffer, size_t size, size_t count, void * handle )
{
  return fread(buffer, size, count, (FILE*) handle);
}

static int psf_file_fseek( void * handle, int64_t offset, int whence )
{
  return fseek((FILE*) handle, offset, whence);
}

static int psf_file_fclose( void * handle )
{
  return fclose((FILE*) handle);
}

static long psf_file_ftell( void * handle )
{
  return ftell((FILE*) handle);
}

const psf_file_callbacks psf_file_system = {
  "\\/|:",
  psf_file_fopen,
  psf_file_fread,
  psf_file_fseek,
  psf_file_fclose,
  psf_file_ftell
};

static int EMU_CALL virtual_readfile(void *context, const char *path, int offset, char *buffer, int length)
{
  return psf2fs_virtual_readfile(context, path, offset, buffer, length);
}

struct psf1_load_state
{
  void * emu;
  bool first;
  unsigned refresh;
};

static int psf1_info(void * context, const char * name, const char * value)
{
  return 0;
}

int psf1_load(void * context, const uint8_t * exe, size_t exe_size,
                                  const uint8_t * reserved, size_t reserved_size)
{
  psf1_load_state * state = ( psf1_load_state * ) context;

  psxexe_hdr_t *psx = (psxexe_hdr_t *) exe;

  if ( exe_size < 0x800 ) return -1;

  uint32_t addr = psx->exec.t_addr;
  uint32_t size = exe_size - 0x800;

  addr &= 0x1fffff;
  if ( ( addr < 0x10000 ) || ( size > 0x1f0000 ) || ( addr + size > 0x200000 ) ) return -1;

  void * pIOP = psx_get_iop_state( state->emu );
  iop_upload_to_ram( pIOP, addr, exe + 0x800, size );

  if ( !state->refresh )
    {
      if (!strncasecmp((const char *) exe + 113, "Japan", 5)) state->refresh = 60;
      else if (!strncasecmp((const char *) exe + 113, "Europe", 6)) state->refresh = 50;
      else if (!strncasecmp((const char *) exe + 113, "North America", 13)) state->refresh = 60;
    }

  if ( state->first )
    {
      void * pR3000 = iop_get_r3000_state( pIOP );
      r3000_setreg(pR3000, R3000_REG_PC, psx->exec.pc0);
      r3000_setreg(pR3000, R3000_REG_GEN+29, psx->exec.s_ptr);
      state->first = false;
    }

  return 0;
}

bool psf_play(const char *psf_name) {
  int cfg_compat = IOP_COMPAT_FRIENDLY;
  int cfg_reverb = 1;

  void *psx_state;

  int psf_version = psf_load( psf_name, &psf_file_system, 0, 0, 0, 0, 0, 0);
  if (VERBOSE) {
    fprintf(stderr, "psf_load: version:%d\n", psf_version);
  }

  if (psf_version < 0) {
    fprintf(stderr, "psf_load: failed to open %s\n", psf_name);
    return false;
  }

  if (psf_version > 2) {
    fprintf(stderr, "psf_load: Not a PSF file(%d). %s\n", psf_version, psf_name);
    return false;
  }

  psx_state = new char[psx_get_state_size( psf_version )];

  psx_clear_state( psx_state, psf_version );

  psf1_load_state state;
  if ( psf_version == 1 ) {
    state.emu = psx_state;
    state.first = true;
    state.refresh = 0;

    if ( psf_load( psf_name, &psf_file_system, 1, psf1_load, &state, psf1_info, &state, 0) < 0 ) {
      fprintf(stderr, "Invalid PSF1 file.\n");
      return false;
    }


    if ( state.refresh ) {
      psx_set_refresh( psx_state, state.refresh );
    }
  } else if ( psf_version == 2 ) {
    void *psf2fs = psf2fs_create();

    state.refresh = 0;

    if ( psf_load( psf_name, &psf_file_system, 2, psf2fs_load_callback, psf2fs, psf1_info, &state, 0 ) < 0 ) {
      fprintf(stderr, "Invalid PSF2 file.\n");
      return false;
    }

    if ( state.refresh ) {
      psx_set_refresh( psx_state, state.refresh );
    }

    psx_set_readfile( psx_state, virtual_readfile, psf2fs );
  }

  {
    void * pIOP = psx_get_iop_state(psx_state);
    iop_set_compat(pIOP, cfg_compat);
    spu_enable_reverb(iop_get_spu_state(pIOP), cfg_reverb);
  }

  unsigned int audio_bufsize = 1024;
  short *audio_buf = new short[audio_bufsize * 4];

  while (true) {
    unsigned int audio_size = audio_bufsize;
    int err = psx_execute( psx_state, 0x7FFFFFFF, audio_buf, &audio_size, 0 );

    if (err < 0) {
      fprintf(stderr, "psx_execute failed. (ret:%d)\n", err);
      return false;
    }
    fwrite(audio_buf, audio_size, 4, stdout);
  }

  return true;
}
