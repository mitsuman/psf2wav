
cmake_minimum_required(VERSION 2.8)

project(psf2wav)

set(CMAKE_C_FLAGS_RELEASE "-Wall -O3")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3")
set(CMAKE_C_FLAGS_DEBUG "-Wall -g")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g")

add_definitions(-DEMU_COMPILE)
add_definitions(-DEMU_LITTLE_ENDIAN)

add_executable(psf2wav
        src/psf2wav.cpp

        # sexy psf
        sexypsf/PsxBios.c
        sexypsf/PsxCounters.c
        sexypsf/PsxDma.c
        sexypsf/Spu.c
        sexypsf/PsxHw.c
        sexypsf/PsxMem.c
        sexypsf/Misc.c
        sexypsf/R3000A.c
        sexypsf/PsxInterpreter.c
        sexypsf/PsxHLE.c
        sexypsf/spu/spu.c

        # highly experimental
        highly_experimental/psf.cpp
        highly_experimental/psflib/psflib.c
        highly_experimental/psflib/psf2fs.c
        highly_experimental/Highly_Experimental/Core/bios.c
        highly_experimental/Highly_Experimental/Core/iop.c
        highly_experimental/Highly_Experimental/Core/ioptimer.c
        highly_experimental/Highly_Experimental/Core/psx.c
        highly_experimental/Highly_Experimental/Core/r3000.c
        highly_experimental/Highly_Experimental/Core/spu.c
        highly_experimental/Highly_Experimental/Core/spucore.c
        highly_experimental/Highly_Experimental/Core/vfs.c
        highly_experimental/Highly_Experimental/Core/mkhebios.c
        hebios.bin
)

target_link_libraries(psf2wav z)

add_custom_command(
  OUTPUT hebios.bin
  DEPENDS foo_psf.fb2k-component
  COMMAND unzip foo_psf.fb2k-component hebios.bin && touch hebios.bin
)

add_custom_command(
  OUTPUT foo_psf.fb2k-component
  COMMAND wget http://kode54.foobar2000.org/foo_psf.fb2k-component
)


